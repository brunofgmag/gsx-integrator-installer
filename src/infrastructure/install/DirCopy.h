#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_DIRCOPY_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_DIRCOPY_H

#include <QtCore/QString>

bool CopyDirRecursively(const QString& sourceDir, const QString& destDir);

bool RemoveDirContentsExcept(const QString& dir, const QString& keepName);

#endif
