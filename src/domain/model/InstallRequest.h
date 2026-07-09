#ifndef GSX_INTEGRATOR_INSTALLER_DOMAIN_MODEL_INSTALLREQUEST_H
#define GSX_INTEGRATOR_INSTALLER_DOMAIN_MODEL_INSTALLREQUEST_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include "ReleaseInfo.h"

struct CommbusTarget
{
    QString communityPath;
    QString processName;
    QString label;
};

struct InstallRequest
{
    bool installClient = false;
    ReleaseInfo clientRelease;
    ReleaseInfo commbusRelease;
    QList<CommbusTarget> commbusTargets;
    bool desktopShortcut = true;
    bool clientStartMenuShortcut = true;
    bool setupShortcut = true;
    QStringList exeXmlAddTargets;
    QStringList exeXmlRemoveTargets;
};

#endif
