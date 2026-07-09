#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_SIMDETECTION_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_SIMDETECTION_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include "../../domain/model/SimInstall.h"

QString ParseInstalledPackagesPath(const QString& userCfgContent);

QList<SimInstall> DetectSims();

QStringList CandidateExeXmlPaths(int generation = 0);

#endif
