#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_APPIMAGEDESKTOPENTRY_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_APPIMAGEDESKTOPENTRY_H

#include <QtCore/QByteArray>
#include <QtCore/QString>

bool EnsureWindowIconEntry(const QString& appImagePath, const QString& dataHome, const QByteArray& iconBytes);

#endif
