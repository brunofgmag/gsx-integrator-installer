#ifndef GSX_INTEGRATOR_INSTALLER_TESTS_FILETREE_H
#define GSX_INTEGRATOR_INSTALLER_TESTS_FILETREE_H

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QPair>

#include "miniz.h"

inline bool WriteTestFile(const QString& path, const QByteArray& content)
{
    if (!QDir().mkpath(QFileInfo(path).absolutePath()))
    {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    return file.write(content) == content.size();
}

inline QByteArray ReadTestFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }

    return file.readAll();
}

inline bool MakeTestSymlink(const QString& target, const QString& linkPath)
{
    if (!QDir().mkpath(QFileInfo(linkPath).absolutePath()))
    {
        return false;
    }

    return QFile::link(target, linkPath);
}

inline QByteArray BuildTestZip(const QList<QPair<QByteArray, QByteArray>>& entries)
{
    mz_zip_archive zip{};
    if (!mz_zip_writer_init_heap(&zip, 0, 0))
    {
        return {};
    }

    for (const auto& [name, data] : entries)
    {
        if (!mz_zip_writer_add_mem(&zip, name.constData(), data.constData(), static_cast<size_t>(data.size()),
                                   MZ_DEFAULT_COMPRESSION))
        {
            (void)mz_zip_writer_end(&zip);
            return {};
        }
    }

    void* buffer = nullptr;
    size_t size = 0;
    if (!mz_zip_writer_finalize_heap_archive(&zip, &buffer, &size))
    {
        (void)mz_zip_writer_end(&zip);
        return {};
    }

    const QByteArray bytes(static_cast<const char*>(buffer), static_cast<qsizetype>(size));
    (void)mz_zip_writer_end(&zip);

    return bytes;
}

#endif
