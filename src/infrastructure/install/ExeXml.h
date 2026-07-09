#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_EXEXML_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_EXEXML_H

#include <QtCore/QString>

bool ExeXmlAddUpdate(const QString& exeXmlPath, const QString& exePath, const QString& appName);
bool ExeXmlRemove(const QString& exeXmlPath, const QString& exeName);

#endif
