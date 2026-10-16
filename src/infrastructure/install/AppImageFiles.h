#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_APPIMAGEFILES_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_APPIMAGEFILES_H

#include <QtCore/QString>

enum class AppImageTargetStatus
{
    Writable,
    NotAnAppImage,
    DirNotWritable,
};

struct AppImageTarget
{
    AppImageTargetStatus status = AppImageTargetStatus::NotAnAppImage;
    QString path;
    QString dir;
};

[[nodiscard]] bool MakeExecutable(const QString& path);

[[nodiscard]] bool ReplaceFile(const QString& stagedPath, const QString& targetPath);

[[nodiscard]] bool CopyExecutable(const QString& sourcePath, const QString& targetPath);

[[nodiscard]] AppImageTarget ResolveAppImageTarget(const QString& appImagePath);

#endif
