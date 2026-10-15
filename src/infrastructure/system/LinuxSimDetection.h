#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_LINUXSIMDETECTION_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_LINUXSIMDETECTION_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include "../../domain/model/SimInstall.h"

QList<SimInstall> DetectSims(const QStringList& steamappsDirs);

QStringList CandidateExeXmlPaths(int generation, const QStringList& steamappsDirs);

QString SimProcessNameForExeXml(const QString& exeXmlPath, const QStringList& steamappsDirs);

QStringList SteamappsDirsOfCurrentUser();

#endif
