#ifndef GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_SETTINGSREPOSITORY_H
#define GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_SETTINGSREPOSITORY_H

#include "../model/InstallerSettings.h"

class SettingsRepository
{
public:
    virtual ~SettingsRepository() = default;

    [[nodiscard]] virtual InstallerSettings Load() const = 0;
    virtual void Save(const InstallerSettings& settings) = 0;
};

#endif
