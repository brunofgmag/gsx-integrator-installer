#ifndef GSX_INTEGRATOR_INSTALLER_DOMAIN_VERSIONS_H
#define GSX_INTEGRATOR_INSTALLER_DOMAIN_VERSIONS_H

#include <QtCore/QString>
#include <QtCore/QVersionNumber>

QVersionNumber ParseVersion(const QString& tagOrVersion);

bool IsNewer(const QString& candidate, const QString& installed);

QString ParsePackageVersion(const QByteArray& manifestJson);

#endif
