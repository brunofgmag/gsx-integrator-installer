#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_SELFUPDATESCRIPT_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_SELFUPDATESCRIPT_H

#include <QtCore/QString>

bool WriteSelfUpdateScript(const QString& scriptPath, const QString& payloadDir,
                           const QString& appDir, const QString& exePath,
                           const QString& cleanupDir);

#endif
