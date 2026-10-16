#ifndef GSX_INTEGRATOR_INSTALLER_TESTS_FAKEINSTALLSERVICE_H
#define GSX_INTEGRATOR_INSTALLER_TESTS_FAKEINSTALLSERVICE_H

#include "../../src/application/ports/InstallService.h"

class FakeInstallService final : public InstallService
{
public:
    InstallServiceObserver* observer = nullptr;
    InstallRequest lastRequest;
    int installCount = 0;
    int launchClient = 0;
    int uninstallCount = 0;
    UninstallOutcome uninstallOutcome;
    void SetObserver(InstallServiceObserver* o) override { observer = o; }

    void Install(const InstallRequest& request) override
    {
        lastRequest = request;
        ++installCount;
    }

    void LaunchClient() override { ++launchClient; }
    UninstallOutcome Uninstall() override
    {
        ++uninstallCount;
        return uninstallOutcome;
    }
};

#endif // GSX_INTEGRATOR_INSTALLER_TESTS_FAKEINSTALLSERVICE_H
