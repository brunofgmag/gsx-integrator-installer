#include "DirCopy.h"

#include <algorithm>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

bool CopyDirRecursively(const QString& sourceDir, const QString& destDir)
{
    const QDir source(sourceDir);
    if (!source.exists() || !QDir().mkpath(destDir))
    {
        return false;
    }

    const QFileInfoList entries = source.entryInfoList(
        QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);

    return std::ranges::all_of(entries, [&destDir](const QFileInfo& entry)
    {
        const QString destPath = destDir + u'/' + entry.fileName();
        return entry.isDir()
                   ? CopyDirRecursively(entry.absoluteFilePath(), destPath)
                   : QFile::copy(entry.absoluteFilePath(), destPath);
    });
}

bool RemoveDirContentsExcept(const QString& dir, const QString& keepName)
{
    const QDir target(dir);
    if (!target.exists())
    {
        return true;
    }

    const QFileInfoList entries = target.entryInfoList(
        QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);

    bool ok = true;
    for (const QFileInfo& entry : entries)
    {
        if (entry.fileName().compare(keepName, Qt::CaseInsensitive) == 0)
        {
            continue;
        }

        const bool removed = entry.isDir()
                                 ? QDir(entry.absoluteFilePath()).removeRecursively()
                                 : QFile::remove(entry.absoluteFilePath());
        ok = ok && removed;
    }

    return ok;
}
