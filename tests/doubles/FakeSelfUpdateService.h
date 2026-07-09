#ifndef GSX_INTEGRATOR_INSTALLER_TESTS_FAKESELFUPDATESERVICE_H
#define GSX_INTEGRATOR_INSTALLER_TESTS_FAKESELFUPDATESERVICE_H

#include "../../src/application/ports/SelfUpdateService.h"

class FakeSelfUpdateService final : public SelfUpdateService
{
public:
    SelfUpdateObserver* observer = nullptr;
    int startCount = 0;
    ReleaseInfo lastRelease;
    void SetObserver(SelfUpdateObserver* o) override { observer = o; }

    void StartUpdate(const ReleaseInfo& release) override
    {
        lastRelease = release;
        ++startCount;
    }
};

#endif // GSX_INTEGRATOR_INSTALLER_TESTS_FAKESELFUPDATESERVICE_H
