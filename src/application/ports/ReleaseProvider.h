#ifndef GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_RELEASEPROVIDER_H
#define GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_RELEASEPROVIDER_H

#include "../../domain/model/ReleaseInfo.h"

class ReleaseProviderObserver
{
public:
    virtual ~ReleaseProviderObserver() = default;

    virtual void OnReleasesFetched(const ReleaseInfo& client, const ReleaseInfo& commbus,
                                   const ReleaseInfo& installer) = 0;
};

class ReleaseProvider
{
public:
    virtual ~ReleaseProvider() = default;

    virtual void SetObserver(ReleaseProviderObserver* observer) = 0;
    virtual void FetchLatest() = 0;
};

#endif
