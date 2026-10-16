#ifndef GSX_INTEGRATOR_INSTALLER_TESTS_FAKEINSTALLERGATEWAY_H
#define GSX_INTEGRATOR_INSTALLER_TESTS_FAKEINSTALLERGATEWAY_H

#include <QtCore/QSet>
#include <QtCore/QStringList>

#include "../../src/application/ports/InstallerGateway.h"

class FakeInstallerGateway final : public InstallerGateway
{
public:
    bool clientRunning = false;
    QSet<QString> runningProcesses;
    bool installerInsideInstallDir = false;
    bool clientExe = true;
    InstallStatus downloadStatus = InstallStatus::Success;
    QString downloadDetail;
    InstallStatus extractClientStatus = InstallStatus::Success;
    InstallStatus extractCommbusStatus = InstallStatus::Success;
    bool prepareOk = true;
    bool autoStartAddOk = true;
    bool autoStartRemoveOk = true;
    QList<CommbusPackage> commbusPackages;
    QSet<QString> undeletableDirs;
    QStringList autoStartConfigPaths;

    mutable QStringList calls;

    bool IsClientRunning() const override { return clientRunning; }

    bool IsProcessRunning(const QString& process) const override
    {
        return runningProcesses.contains(process);
    }

    bool IsInstallerInsideInstallDir() const override { return installerInsideInstallDir; }
    bool ClientExecutableExists() const override { return clientExe; }
    QString ClientInstallDir() const override { return QStringLiteral("C:/install"); }

    DownloadResult DownloadRelease(const ReleaseInfo&,
                                   const std::function<void(double)>& onFraction) override
    {
        calls << QStringLiteral("download");
        if (onFraction)
        {
            onFraction(1.0);
        }
        if (downloadStatus != InstallStatus::Success)
        {
            return {downloadStatus, downloadDetail, {}};
        }
        return {InstallStatus::Success, {}, QStringLiteral("C:/tmp/pkg.zip")};
    }

    bool PrepareCleanInstallDir() override
    {
        calls << QStringLiteral("prepare");
        return prepareOk;
    }

    InstallOutcome InstallClientPackage(const QString&) override
    {
        calls << QStringLiteral("extractClient");
        return {extractClientStatus, {}};
    }

    InstallOutcome ExtractCommbusPackage(const QString&,
                                         const QString& community,
                                         const QString& version) override
    {
        calls << QStringLiteral("extractCommbus:%1:%2").arg(community, version);
        return {extractCommbusStatus, {}};
    }

    void InstallUninstallerCopy() override { calls << QStringLiteral("uninstallerCopy"); }

    void WriteUninstallEntry(const QString& version) override
    {
        calls << QStringLiteral("uninstallEntry:%1").arg(version);
    }

    void ApplyShortcut(ShortcutKind kind, const bool wanted) override
    {
        calls << QStringLiteral("shortcut:%1:%2").arg(static_cast<int>(kind)).arg(wanted ? 1 : 0);
    }

    bool AddAutoStartEntry(const QString& path) override
    {
        calls << QStringLiteral("autoStartAdd:%1").arg(path);
        return autoStartAddOk;
    }

    bool RemoveAutoStartEntry(const QString& path) override
    {
        calls << QStringLiteral("autoStartRemove:%1").arg(path);
        return autoStartRemoveOk;
    }

    QList<CommbusPackage> DetectedCommbusPackages() const override { return commbusPackages; }
    QStringList AllAutoStartConfigPaths() const override { return autoStartConfigPaths; }

    bool RemoveDirRecursively(const QString& dir) override
    {
        calls << QStringLiteral("removeDir:%1").arg(dir);
        return !undeletableDirs.contains(dir);
    }

    void RemoveShortcut(ShortcutKind kind) override
    {
        calls << QStringLiteral("removeShortcut:%1").arg(static_cast<int>(kind));
    }

    void RemoveUninstallEntry() override { calls << QStringLiteral("removeUninstallEntry"); }
    void ScheduleInstallDirRemoval() override { calls << QStringLiteral("scheduleRemoval"); }
};

#endif // GSX_INTEGRATOR_INSTALLER_TESTS_FAKEINSTALLERGATEWAY_H
