#ifndef GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_INSTALLSERVICE_H
#define GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_INSTALLSERVICE_H

#include "../../domain/model/InstallOutcome.h"
#include "../../domain/model/InstallRequest.h"

class InstallServiceObserver
{
public:
    virtual ~InstallServiceObserver() = default;

    virtual void OnInstallProgress(const InstallProgress& update) = 0;
    virtual void OnInstallFinished(const InstallOutcome& outcome) = 0;
};

class InstallService
{
public:
    virtual ~InstallService() = default;

    virtual void SetObserver(InstallServiceObserver* observer) = 0;
    virtual void Install(const InstallRequest& request) = 0;
    virtual void LaunchClient() = 0;
    virtual void Uninstall() = 0;
};

#endif
