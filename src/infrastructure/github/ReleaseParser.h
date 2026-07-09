#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_GITHUB_RELEASEPARSER_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_GITHUB_RELEASEPARSER_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include "domain/model/ReleaseInfo.h"

ReleaseInfo ParseLatestRelease(const QByteArray& json, const QString& assetPrefix,
                               const QString& extension = QStringLiteral(".zip"));

#endif
