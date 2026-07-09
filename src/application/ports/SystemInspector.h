#ifndef GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_SYSTEMINSPECTOR_H
#define GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_SYSTEMINSPECTOR_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include "../../domain/model/SimInstall.h"

class SystemInspector
{
public:
    virtual ~SystemInspector() = default;

    [[nodiscard]] virtual QString InstalledClientVersion() const = 0;
    [[nodiscard]] virtual bool IsClientRunning() const = 0;

    [[nodiscard]] virtual QList<SimInstall> DetectSims() const = 0;
    [[nodiscard]] virtual QString InstalledCommbusVersion(const QString& communityPath) const = 0;
    [[nodiscard]] virtual bool IsSimRunning(const QString& processName) const = 0;

    [[nodiscard]] virtual bool DesktopShortcutExists() const = 0;
    [[nodiscard]] virtual bool ClientStartMenuShortcutExists() const = 0;
    [[nodiscard]] virtual bool SetupStartMenuShortcutExists() const = 0;

    [[nodiscard]] virtual QStringList CandidateExeXmlPaths(int generation) const = 0;
};

#endif
