#ifndef GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_SELFUPDATESERVICE_H
#define GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_SELFUPDATESERVICE_H

#include "../../domain/model/ReleaseInfo.h"

enum class SelfUpdateError
{
    DownloadFailed,
    ChecksumMismatch,
    SwapFailed,
    RelaunchFailed,
};

class SelfUpdateObserver
{
public:
    virtual ~SelfUpdateObserver() = default;

    virtual void OnSelfUpdateProgress(double fraction) = 0;
    virtual void OnSelfUpdateFailed(SelfUpdateError kind, const QString& detail) = 0;
};

class SelfUpdateService
{
public:
    virtual ~SelfUpdateService() = default;

    virtual void SetObserver(SelfUpdateObserver* observer) = 0;
    virtual void StartUpdate(const ReleaseInfo& release) = 0;
};

#endif
