#include "AppImageFiles.h"

#include <filesystem>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

namespace
{
    constexpr QFileDevice::Permissions kExecutablePermissions =
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
        | QFileDevice::ReadGroup | QFileDevice::ExeGroup | QFileDevice::ReadOther | QFileDevice::ExeOther;
}

bool MakeExecutable(const QString& path)
{
    return QFile::setPermissions(path, kExecutablePermissions);
}

bool ReplaceFile(const QString& stagedPath, const QString& targetPath)
{
    if (!MakeExecutable(stagedPath))
    {
        return false;
    }

    std::error_code error;
    std::filesystem::rename(stagedPath.toStdString(), targetPath.toStdString(), error);

    return !error;
}

bool CopyExecutable(const QString& sourcePath, const QString& targetPath)
{
    if (!QDir().mkpath(QFileInfo(targetPath).absolutePath()))
    {
        return false;
    }

    const QString partPath = targetPath + QStringLiteral(".part");
    QFile::remove(partPath);
    if (!QFile::copy(sourcePath, partPath))
    {
        return false;
    }

    if (!ReplaceFile(partPath, targetPath))
    {
        QFile::remove(partPath);
        return false;
    }

    return true;
}

AppImageTarget ResolveAppImageTarget(const QString& appImagePath)
{
    if (appImagePath.isEmpty() || !QFileInfo(appImagePath).isFile())
    {
        return {};
    }

    const QFileInfo info(appImagePath);
    const QString dir = info.absolutePath();
    if (!QFileInfo(dir).isWritable())
    {
        return {AppImageTargetStatus::DirNotWritable, info.absoluteFilePath(), dir};
    }

    return {AppImageTargetStatus::Writable, info.absoluteFilePath(), dir};
}
