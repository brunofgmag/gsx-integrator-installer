#include "ReleasePackage.h"

#include <optional>

#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QRegularExpression>
#include <QtCore/QScopeGuard>
#include <QtCore/QStringList>

#include "miniz.h"

namespace
{
    constexpr auto kUtf8Bom = "\xEF\xBB\xBF";

    struct WriteContext
    {
        QFile* file = nullptr;
        bool failed = false;
    };

    size_t ReadFromFile(void* opaque, const mz_uint64 offset, void* buffer, const size_t size)
    {
        auto* file = static_cast<QFile*>(opaque);
        if (!file->seek(static_cast<qint64>(offset)))
        {
            return 0;
        }

        const qint64 read = file->read(static_cast<char*>(buffer), static_cast<qint64>(size));

        return read < 0 ? 0 : static_cast<size_t>(read);
    }

    size_t WriteToFile(void* opaque, const mz_uint64, const void* buffer, const size_t size)
    {
        auto* context = static_cast<WriteContext*>(opaque);
        const qint64 written = context->file->write(static_cast<const char*>(buffer), static_cast<qint64>(size));
        if (written < 0 || static_cast<size_t>(written) != size)
        {
            context->failed = true;
            return 0;
        }

        return size;
    }

    QString MinizError(mz_zip_archive& zip)
    {
        return QString::fromLatin1(mz_zip_get_error_string(mz_zip_get_last_error(&zip)));
    }

    std::optional<QString> SafeRelativePath(const QString& entryName)
    {
        const QString name = QString(entryName).replace(u'\\', u'/');
        if (name.startsWith(u'/') || name.contains(u':'))
        {
            return std::nullopt;
        }

        QStringList kept;
        for (const QString& part : name.split(u'/', Qt::SkipEmptyParts))
        {
            if (part == QLatin1String(".."))
            {
                return std::nullopt;
            }
            if (part != QLatin1String("."))
            {
                kept.append(part);
            }
        }

        return kept.join(u'/');
    }

    struct PlannedEntry
    {
        mz_uint index = 0;
        QString name;
        QString relative;
        bool isDirectory = false;
    };

    ExtractResult PlanEntries(mz_zip_archive& zip, QList<PlannedEntry>& planned)
    {
        const mz_uint count = mz_zip_reader_get_num_files(&zip);
        for (mz_uint index = 0; index < count; ++index)
        {
            mz_zip_archive_file_stat stat;
            if (!mz_zip_reader_file_stat(&zip, index, &stat))
            {
                return {ExtractStatus::InvalidArchive, MinizError(zip)};
            }

            PlannedEntry entry;
            entry.index = index;
            entry.name = QString::fromUtf8(stat.m_filename);
            entry.isDirectory = mz_zip_reader_is_file_a_directory(&zip, index) != MZ_FALSE;

            const std::optional<QString> relative = SafeRelativePath(entry.name);
            if (!relative || (relative->isEmpty() && !entry.isDirectory))
            {
                return {ExtractStatus::UnsafeEntry, entry.name};
            }

            entry.relative = *relative;
            planned.append(entry);
        }

        return {};
    }

    ExtractResult ExtractEntry(mz_zip_archive& zip, const PlannedEntry& entry, const QString& destDir)
    {
        const QString target = QDir::cleanPath(destDir + u'/' + entry.relative);

        if (entry.isDirectory)
        {
            return QDir().mkpath(target)
                       ? ExtractResult{}
                       : ExtractResult{ExtractStatus::WriteFailed, target};
        }

        if (mz_zip_reader_is_file_supported(&zip, entry.index) == MZ_FALSE)
        {
            return {ExtractStatus::InvalidArchive, entry.name + QStringLiteral(": unsupported entry")};
        }

        if (!QDir().mkpath(QFileInfo(target).absolutePath()))
        {
            return {ExtractStatus::WriteFailed, target};
        }

        QFile out(target);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            return {ExtractStatus::WriteFailed, target};
        }

        WriteContext context;
        context.file = &out;

        const bool extracted = mz_zip_reader_extract_to_callback(&zip, entry.index, WriteToFile, &context, 0)
            != MZ_FALSE;
        out.close();

        if (!extracted)
        {
            (void)QFile::remove(target);
            if (context.failed)
            {
                return {ExtractStatus::WriteFailed, target};
            }

            return {ExtractStatus::InvalidArchive, entry.name + QStringLiteral(": ") + MinizError(zip)};
        }

        return {};
    }
}

ExtractResult ExtractZip(const QString& zipPath, const QString& destDir)
{
    QFile file(zipPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return {ExtractStatus::OpenFailed, zipPath};
    }

    mz_zip_archive zip{};
    zip.m_pRead = ReadFromFile;
    zip.m_pIO_opaque = &file;
    if (!mz_zip_reader_init(&zip, static_cast<mz_uint64>(file.size()), 0))
    {
        return {ExtractStatus::InvalidArchive, MinizError(zip)};
    }

    const auto closeZip = qScopeGuard([&zip] { (void)mz_zip_reader_end(&zip); });

    QList<PlannedEntry> planned;
    if (const ExtractResult plan = PlanEntries(zip, planned); !plan.ok())
    {
        return plan;
    }

    if (!QDir().mkpath(destDir))
    {
        return {ExtractStatus::WriteFailed, destDir};
    }

    for (const PlannedEntry& entry : planned)
    {
        if (const ExtractResult extracted = ExtractEntry(zip, entry, destDir); !extracted.ok())
        {
            return extracted;
        }
    }

    return {};
}

QString ParseSha256(const QByteArray& content)
{
    QByteArray data = content;
    if (data.startsWith(kUtf8Bom))
    {
        data.remove(0, 3);
    }

    const QString token = QString::fromLatin1(data.simplified().split(' ').value(0));

    static const QRegularExpression sha256Hex(QStringLiteral("^[0-9a-fA-F]{64}$"));
    if (!sha256Hex.match(token).hasMatch())
    {
        return {};
    }

    return token.toLower();
}

QString ReadSha256File(const QString& shaFilePath)
{
    QFile file(shaFilePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }

    return ParseSha256(file.readAll());
}

QString Sha256File(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file))
    {
        return {};
    }

    return QString::fromLatin1(hash.result().toHex());
}

bool Sha256Matches(const QString& filePath, const QString& shaFilePath)
{
    const QString expected = ReadSha256File(shaFilePath);

    return !expected.isEmpty() && Sha256File(filePath) == expected;
}
