#ifndef GSX_INTEGRATOR_INSTALLER_APPLICATION_INSTALL_INSTALLORCHESTRATOR_H
#define GSX_INTEGRATOR_INSTALLER_APPLICATION_INSTALL_INSTALLORCHESTRATOR_H

#include <functional>
#include "../../domain/model/InstallOutcome.h"
#include "../../domain/model/InstallRequest.h"
#include "../ports/InstallerGateway.h"

class InstallOrchestrator
{
public:
    using ProgressFn = std::function<void(const InstallProgress& update)>;

    explicit InstallOrchestrator(InstallerGateway& gateway);

    [[nodiscard]] InstallOutcome RunInstall(const InstallRequest& request, const ProgressFn& progress) const;
    void RunUninstall() const;

private:
    [[nodiscard]] InstallOutcome InstallClient(const InstallRequest& request, const ProgressFn& progress) const;
    [[nodiscard]] InstallOutcome InstallCommbus(const InstallRequest& request, const ProgressFn& progress) const;
    void ApplyShortcuts(const InstallRequest& request) const;
    [[nodiscard]] InstallOutcome ConfigureAutoStart(const InstallRequest& request, const ProgressFn& progress) const;

    InstallerGateway& gateway_;
};

#endif
