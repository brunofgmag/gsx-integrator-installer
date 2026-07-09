#ifndef GSX_INTEGRATOR_INSTALLER_TESTS_FAKESETTINGSREPOSITORY_H
#define GSX_INTEGRATOR_INSTALLER_TESTS_FAKESETTINGSREPOSITORY_H

#include "../../src/application/ports/SettingsRepository.h"

class FakeSettingsRepository final : public SettingsRepository
{
public:
    InstallerSettings settings;
    [[nodiscard]] InstallerSettings Load() const override { return settings; }
    void Save(const InstallerSettings& value) override { settings = value; }
};

#endif // GSX_INTEGRATOR_INSTALLER_TESTS_FAKESETTINGSREPOSITORY_H
