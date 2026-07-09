#include "DirCopy.h"

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

    for (const QFileInfo& entry : entries)
    {
        const QString destPath = destDir + u'/' + entry.fileName();
        const bool ok = entry.isDir()
                            ? CopyDirRecursively(entry.absoluteFilePath(), destPath)
                            : QFile::copy(entry.absoluteFilePath(), destPath);
        if (!ok)
        {
            return false;
        }
    }

    return true;
}
