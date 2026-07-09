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

    if (const InstallOutcome extract = gateway_.ExtractClientPackage(download.zipPath); !extract.ok())
    {
        return extract;
    }

    if (!gateway_.ClientExeExists())
    {
        return {InstallStatus::ClientExeMissing, {}};
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
                gateway_.ExtractCommbusPackage(download.zipPath, target.communityPath);
            !extract.ok())
        {
            return extract;
        }
    }

    return {};
}

void InstallOrchestrator::ApplyShortcuts(const InstallRequest& request) const
{
    gateway_.ApplyShortcut(ShortcutKind::ClientStartMenu, request.clientStartMenuShortcut);
    gateway_.ApplyShortcut(ShortcutKind::Desktop, request.desktopShortcut);
    gateway_.ApplyShortcut(ShortcutKind::Setup, request.setupShortcut);
}

InstallOutcome InstallOrchestrator::ConfigureAutoStart(const InstallRequest& request, const ProgressFn& progress) const
{
    if (request.exeXmlRemoveTargets.isEmpty() && request.exeXmlAddTargets.isEmpty())
    {
        return {};
    }

    if (progress)
    {
        progress({InstallPhase::ConfiguringAutoStart, {}, -1.0});
    }

    for (const QString& exeXmlPath : request.exeXmlRemoveTargets)
    {
        if (!gateway_.ExeXmlRemove(exeXmlPath))
        {
            return {InstallStatus::ExeXmlUpdateFailed, exeXmlPath};
        }
    }

    if (!request.exeXmlAddTargets.isEmpty())
    {
        if (!gateway_.ClientExeExists())
        {
            return {InstallStatus::AutoStartClientMissing, {}};
        }
        for (const QString& exeXmlPath : request.exeXmlAddTargets)
        {
            if (!gateway_.ExeXmlAdd(exeXmlPath))
            {
                return {InstallStatus::ExeXmlUpdateFailed, exeXmlPath};
            }
        }
    }

    return {};
}

void InstallOrchestrator::RunUninstall() const
{
    for (const QString& dir : gateway_.DetectedCommbusPackageDirs())
    {
        gateway_.RemoveDirRecursively(dir);
    }

    for (const QString& exeXmlPath : gateway_.AllExeXmlPaths())
    {
        static_cast<void>(gateway_.ExeXmlRemove(exeXmlPath));
    }

    gateway_.RemoveShortcut(ShortcutKind::ClientStartMenu);
    gateway_.RemoveShortcut(ShortcutKind::Desktop);
    gateway_.RemoveShortcut(ShortcutKind::Setup);
    gateway_.RemoveUninstallRegistryEntry();
    gateway_.ScheduleInstallDirRemoval();
}
