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
    void SetObserver(InstallServiceObserver* o) override { observer = o; }

    void Install(const InstallRequest& request) override
    {
        lastRequest = request;
        ++installCount;
    }

    void LaunchClient() override { ++launchClient; }
    void Uninstall() override { ++uninstallCount; }
};

#endif // GSX_INTEGRATOR_INSTALLER_TESTS_FAKEINSTALLSERVICE_H
