#ifndef GSX_INTEGRATOR_INSTALLER_DOMAIN_MODEL_SIMINSTALL_H
#define GSX_INTEGRATOR_INSTALLER_DOMAIN_MODEL_SIMINSTALL_H

#include <QtCore/QString>

struct SimInstall
{
    QString id;
    QString label;
    QString communityPath;
    QString processName;
    QString exeXmlPath;
    int generation = 0;
};

#endif
