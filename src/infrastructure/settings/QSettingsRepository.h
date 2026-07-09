#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SETTINGS_QSETTINGSREPOSITORY_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SETTINGS_QSETTINGSREPOSITORY_H

#include "../../application/ports/SettingsRepository.h"

class QSettingsRepository final : public SettingsRepository
{
public:
    [[nodiscard]] InstallerSettings Load() const override;
    void Save(const InstallerSettings& settings) override;
};

#endif
