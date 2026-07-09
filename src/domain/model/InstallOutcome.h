#ifndef GSX_INTEGRATOR_INSTALLER_DOMAIN_MODEL_INSTALLOUTCOME_H
#define GSX_INTEGRATOR_INSTALLER_DOMAIN_MODEL_INSTALLOUTCOME_H

#include <QtCore/QString>

enum class InstallStatus
{
    Success = 0,
    ClientRunning,
    InstallerInsideInstallDir,
    DownloadFailed,
    ChecksumMismatch,
    CleanInstallDirFailed,
    TarMissing,
    ExtractFailed,
    ClientExeMissing,
    SimRunning,
    CommbusReplaceFailed,
    AutoStartClientMissing,
    ExeXmlUpdateFailed,
};

struct InstallOutcome
{
    InstallStatus status = InstallStatus::Success;
    QString detail;

    [[nodiscard]] bool ok() const { return status == InstallStatus::Success; }
};

enum class InstallPhase
{
    DownloadingClient,
    InstallingClient,
    DownloadingCommbus,
    InstallingCommbus,
    ConfiguringAutoStart,
};

struct InstallProgress
{
    InstallPhase phase = InstallPhase::DownloadingClient;
    QString detail;
    double fraction = -1.0;
};

#endif
