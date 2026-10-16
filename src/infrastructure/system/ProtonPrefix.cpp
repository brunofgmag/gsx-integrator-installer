#include "ProtonPrefix.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QSet>

namespace
{
    QString UnescapeVdf(const QString& value)
    {
        QString result;
        result.reserve(value.size());
        for (qsizetype i = 0; i < value.size(); ++i)
        {
            if (value[i] == u'\\' && i + 1 < value.size() && (value[i + 1] == u'\\' || value[i + 1] == u'"'))
            {
                ++i;
            }
            result.append(value[i]);
        }

        return result;
    }

    bool EntryExists(const QString& path)
    {
        const QFileInfo info(path);
        return info.exists() || info.isSymLink();
    }

    QString CanonicalKey(const QString& path)
    {
        const QString canonical = QFileInfo(path).canonicalFilePath();
        return canonical.isEmpty() ? QDir::cleanPath(path) : canonical;
    }

    QString SubPathFor(const ProtonKnownFolder folder)
    {
        switch (folder)
        {
        case ProtonKnownFolder::Profile:
            return {};
        case ProtonKnownFolder::AppDataRoaming:
            return QStringLiteral("AppData/Roaming");
        case ProtonKnownFolder::AppDataLocal:
            return QStringLiteral("AppData/Local");
        }

        return {};
    }
}

QStringList DefaultSteamappsDirs(const QString& home)
{
    return {
        home + QStringLiteral("/.local/share/Steam/steamapps"),
        home + QStringLiteral("/.steam/steam/steamapps"),
    };
}

QStringList ParseLibraryFolders(const QByteArray& libraryFoldersVdf)
{
    static const QRegularExpression pattern(
        QStringLiteral(R"re("path"\s+"((?:[^"\\]|\\.)*)")re"), QRegularExpression::CaseInsensitiveOption);

    QStringList libraries;
    const QString text = QString::fromUtf8(libraryFoldersVdf);
    for (const QRegularExpressionMatch& match : pattern.globalMatch(text))
    {
        const QString path = UnescapeVdf(match.captured(1)).trimmed();
        if (!path.isEmpty())
        {
            libraries.append(path);
        }
    }

    return libraries;
}

QStringList SteamLibrarySteamappsDirs(const QStringList& steamappsDirs)
{
    QStringList found;
    QSet<QString> seen;

    const auto add = [&](const QString& dir)
    {
        if (!QFileInfo(dir).isDir())
        {
            return;
        }
        if (seen.contains(CanonicalKey(dir)))
        {
            return;
        }
        seen.insert(CanonicalKey(dir));
        found.append(dir);
    };

    for (const QString& root : steamappsDirs)
    {
        add(root);

        QFile vdf(root + QStringLiteral("/libraryfolders.vdf"));
        if (!vdf.open(QIODevice::ReadOnly))
        {
            continue;
        }

        for (const QString& library : ParseLibraryFolders(vdf.readAll()))
        {
            add(library + QStringLiteral("/steamapps"));
        }
    }

    return found;
}

QString FindProtonPrefix(const QStringList& steamappsDirs, const int appId)
{
    for (const QString& dir : SteamLibrarySteamappsDirs(steamappsDirs))
    {
        const QString prefix = dir + QStringLiteral("/compatdata/") + QString::number(appId)
                               + QStringLiteral("/pfx");
        if (QFileInfo(prefix).isDir())
        {
            return prefix;
        }
    }

    return {};
}

QString ResolveIgnoringCase(const QString& baseDir, const QString& relativePath)
{
    QString current = QDir::cleanPath(baseDir);
    if (!QFileInfo(current).isDir())
    {
        return {};
    }

    const QStringList parts = QString(relativePath).replace(u'\\', u'/').split(u'/', Qt::SkipEmptyParts);
    for (const QString& part : parts)
    {
        const QString exact = current + u'/' + part;
        if (EntryExists(exact))
        {
            current = exact;
            continue;
        }

        QStringList names = QDir(current).entryList(
            QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        names.sort();

        QString match;
        for (const QString& name : names)
        {
            if (name.compare(part, Qt::CaseInsensitive) == 0)
            {
                match = name;
                break;
            }
        }

        if (match.isEmpty())
        {
            return {};
        }

        current += u'/' + match;
    }

    return current;
}

QString WindowsPathToHost(const QString& prefixDir, const QString& windowsPath)
{
    const QString path = QString(windowsPath).replace(u'\\', u'/');
    if (path.size() < 2 || path[1] != u':')
    {
        return {};
    }

    const QString letter = path.left(1).toLower();
    const QString rest = path.mid(2);

    QString driveTarget;
    const QString drive = ResolveIgnoringCase(prefixDir + QStringLiteral("/dosdevices"), letter + u':');
    if (!drive.isEmpty())
    {
        driveTarget = QFileInfo(drive).canonicalFilePath();
    }
    else if (letter == QLatin1String("c"))
    {
        driveTarget = ResolveIgnoringCase(prefixDir, QStringLiteral("drive_c"));
    }

    if (driveTarget.isEmpty())
    {
        return {};
    }

    return ResolveIgnoringCase(driveTarget, rest);
}

QString ProtonKnownFolderPath(const QString& prefixDir, const ProtonKnownFolder folder)
{
    const QString profile = ResolveIgnoringCase(
        prefixDir, QStringLiteral("drive_c/users/") + QLatin1String(kProtonUserName));
    if (profile.isEmpty())
    {
        return {};
    }

    return ResolveIgnoringCase(profile, SubPathFor(folder));
}
