#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_LINUXSYSTEMINSPECTOR_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_LINUXSYSTEMINSPECTOR_H

#include "../../application/ports/SystemInspector.h"

class LinuxSystemInspector final : public SystemInspector
{
public:
    [[nodiscard]] QString InstalledClientVersion() const override;
    [[nodiscard]] bool IsClientRunning() const override;

    [[nodiscard]] QList<SimInstall> DetectSims() const override;
    [[nodiscard]] QString InstalledCommbusVersion(const QString& communityPath) const override;
    [[nodiscard]] bool IsSimRunning(const QString& processName) const override;

    [[nodiscard]] bool DesktopShortcutExists() const override;
    [[nodiscard]] bool ClientMenuShortcutExists() const override;
    [[nodiscard]] bool SetupMenuShortcutExists() const override;

    [[nodiscard]] QStringList CandidateAutoStartConfigPaths(int generation) const override;
    [[nodiscard]] bool AutoStartAvailable() const override;
    [[nodiscard]] QStringList SimSearchLocations() const override;
};

#endif
