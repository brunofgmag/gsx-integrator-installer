#ifndef GSX_INTEGRATOR_INSTALLER_APPLICATION_MODEL_INSTALLERSETTINGS_H
#define GSX_INTEGRATOR_INSTALLER_APPLICATION_MODEL_INSTALLERSETTINGS_H

#include <QtCore/QString>
#include <QtCore/QStringList>

struct InstallerSettings
{
    QString language = QStringLiteral("system");
    bool desktopShortcut = true;
    bool clientStartMenuShortcut = true;
    bool setupShortcut = true;
    QStringList disabledCommbusSims;
};

#endif
