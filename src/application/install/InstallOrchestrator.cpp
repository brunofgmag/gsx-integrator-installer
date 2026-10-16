#include "InstallOrchestrator.h"

InstallOrchestrator::InstallOrchestrator(InstallerGateway& gateway) : gateway_(gateway)
{
}

InstallOutcome InstallOrchestrator::RunInstall(const InstallRequest& request, const ProgressFn& progress) const
{
    if (const InstallOutcome r = InstallClient(request, progress); !r.ok())
    {
        return r;
    }

    if (const InstallOutcome r = InstallCommbus(request, progress); !r.ok())
    {
        return r;
    }

    ApplyShortcuts(request);

    if (const InstallOutcome r = ConfigureAutoStart(request, progress); !r.ok())
    {
        return r;
    }

    return {};
}

InstallOutcome InstallOrchestrator::InstallClient(const InstallRequest& request, const ProgressFn& progress) const
{
    if (!request.installClient)
    {
        return {};
    }

    const auto report = [&](const InstallPhase phase, const QString& detail, const double fraction)
    {
        if (progress)
        {
            progress({phase, detail, fraction});
        }
    };

    if (gateway_.IsClientRunning())
    {
        return {InstallStatus::ClientRunning, {}};
    }

    if (gateway_.IsInstallerInsideInstallDir())
    {
        return {InstallStatus::InstallerInsideInstallDir, {}};
    }

    const QString& version = request.clientRelease.version;
    report(InstallPhase::DownloadingClient, version, 0.0);

    const DownloadResult download = gateway_.DownloadRelease(
        request.clientRelease,
        [&](const double fraction) { report(InstallPhase::DownloadingClient, version, fraction); });
    if (download.status != InstallStatus::Success)
    {
        return {download.status, download.detail};
    }

    report(InstallPhase::InstallingClient, version, -1.0);
    if (!gateway_.PrepareCleanInstallDir())
    {
        return {InstallStatus::CleanInstallDirFailed, gateway_.ClientInstallDir()};
    }

    if (const InstallOutcome extract = gateway_.InstallClientPackage(download.packagePath); !extract.ok())
    {
        return extract;
    }

    if (!gateway_.ClientExecutableExists())
    {
        return {InstallStatus::ClientExecutableMissing, {}};
    }

    gateway_.InstallUninstallerCopy();
    gateway_.WriteUninstallEntry(version);

    return {};
}

InstallOutcome InstallOrchestrator::InstallCommbus(const InstallRequest& request, const ProgressFn& progress) const
{
    if (request.commbusTargets.isEmpty())
    {
        return {};
    }

    const auto report = [&](const InstallPhase phase, const QString& detail, const double fraction)
    {
        if (progress)
        {
            progress({phase, detail, fraction});
        }
    };

    const auto simRunning = [](const CommbusTarget& target) -> InstallOutcome
    {
        return {InstallStatus::SimRunning, target.label};
    };

    for (const CommbusTarget& target : request.commbusTargets)
    {
        if (gateway_.IsProcessRunning(target.processName))
        {
            return simRunning(target);
        }
    }

    const QString& version = request.commbusRelease.version;
    report(InstallPhase::DownloadingCommbus, version, 0.0);

    const DownloadResult download = gateway_.DownloadRelease(request.commbusRelease,
                                                             [&](const double fraction)
                                                             {
                                                                 report(InstallPhase::DownloadingCommbus,
                                                                        version,
                                                                        fraction);
                                                             });
    if (download.status != InstallStatus::Success)
    {
        return {download.status, download.detail};
    }

    for (const CommbusTarget& target : request.commbusTargets)
    {
        if (gateway_.IsProcessRunning(target.processName))
        {
            return simRunning(target);
        }

        report(InstallPhase::InstallingCommbus, target.communityPath, -1.0);
        if (const InstallOutcome extract =
                gateway_.ExtractCommbusPackage(download.packagePath, target.communityPath, version);
            !extract.ok())
        {
            return extract;
        }
    }

    return {};
}

void InstallOrchestrator::ApplyShortcuts(const InstallRequest& request) const
{
    gateway_.ApplyShortcut(ShortcutKind::ClientMenu, request.clientMenuShortcut);
    gateway_.ApplyShortcut(ShortcutKind::Desktop, request.desktopShortcut);
    gateway_.ApplyShortcut(ShortcutKind::Setup, request.setupShortcut);
}

InstallOutcome InstallOrchestrator::ConfigureAutoStart(const InstallRequest& request, const ProgressFn& progress) const
{
    if (request.autoStartRemoveTargets.isEmpty() && request.autoStartAddTargets.isEmpty())
    {
        return {};
    }

    if (progress)
    {
        progress({InstallPhase::ConfiguringAutoStart, {}, -1.0});
    }

    for (const QString& autoStartConfigPath : request.autoStartRemoveTargets)
    {
        if (!gateway_.RemoveAutoStartEntry(autoStartConfigPath))
        {
            return {InstallStatus::AutoStartUpdateFailed, autoStartConfigPath};
        }
    }

    if (!request.autoStartAddTargets.isEmpty())
    {
        if (!gateway_.ClientExecutableExists())
        {
            return {InstallStatus::AutoStartClientMissing, {}};
        }
        for (const QString& autoStartConfigPath : request.autoStartAddTargets)
        {
            if (!gateway_.AddAutoStartEntry(autoStartConfigPath))
            {
                return {InstallStatus::AutoStartUpdateFailed, autoStartConfigPath};
            }
        }
    }

    return {};
}

UninstallOutcome InstallOrchestrator::RunUninstall() const
{
    const QList<CommbusPackage> packages = gateway_.DetectedCommbusPackages();

    for (const CommbusPackage& package : packages)
    {
        if (gateway_.IsProcessRunning(package.processName))
        {
            return {UninstallStatus::SimRunning, package.label, {}};
        }
    }

    QStringList leftovers;

    for (const CommbusPackage& package : packages)
    {
        if (!gateway_.RemoveDirRecursively(package.packageDir))
        {
            leftovers.append(package.packageDir);
        }
    }

    for (const QString& autoStartConfigPath : gateway_.AllAutoStartConfigPaths())
    {
        if (!gateway_.RemoveAutoStartEntry(autoStartConfigPath))
        {
            leftovers.append(autoStartConfigPath);
        }
    }

    if (!leftovers.isEmpty())
    {
        return {UninstallStatus::LeftoversRemain, {}, leftovers};
    }

    if (!gateway_.ScheduleInstallDirRemoval())
    {
        return {UninstallStatus::LeftoversRemain, {}, {gateway_.ClientInstallDir()}};
    }

    gateway_.RemoveShortcut(ShortcutKind::ClientMenu);
    gateway_.RemoveShortcut(ShortcutKind::Desktop);
    gateway_.RemoveShortcut(ShortcutKind::Setup);
    gateway_.RemoveUninstallEntry();

    return {};
}
