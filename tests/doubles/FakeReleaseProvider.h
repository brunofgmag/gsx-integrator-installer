#ifndef GSX_INTEGRATOR_INSTALLER_TESTS_FAKERELEASEPROVIDER_H
#define GSX_INTEGRATOR_INSTALLER_TESTS_FAKERELEASEPROVIDER_H

#include "../../src/application/ports/ReleaseProvider.h"

class FakeReleaseProvider final : public ReleaseProvider
{
public:
    ReleaseProviderObserver* observer = nullptr;
    int fetchCount = 0;
    void SetObserver(ReleaseProviderObserver* o) override { observer = o; }
    void FetchLatest() override { ++fetchCount; }

    void Deliver(const ReleaseInfo& client, const ReleaseInfo& commbus,
                 const ReleaseInfo& installer) const
    {
        observer->OnReleasesFetched(client, commbus, installer);
    }
};

#endif // GSX_INTEGRATOR_INSTALLER_TESTS_FAKERELEASEPROVIDER_H
