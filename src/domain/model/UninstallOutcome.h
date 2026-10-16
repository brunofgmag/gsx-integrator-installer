#ifndef GSX_INTEGRATOR_INSTALLER_DOMAIN_MODEL_UNINSTALLOUTCOME_H
#define GSX_INTEGRATOR_INSTALLER_DOMAIN_MODEL_UNINSTALLOUTCOME_H

#include <QtCore/QString>
#include <QtCore/QStringList>

enum class UninstallStatus
{
    Success = 0,
    SimRunning,
    LeftoversRemain,
};

struct UninstallOutcome
{
    UninstallStatus status = UninstallStatus::Success;
    QString detail;
    QStringList leftovers;

    [[nodiscard]] bool ok() const { return status == UninstallStatus::Success; }
};

#endif
