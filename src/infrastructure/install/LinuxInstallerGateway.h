#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_LINUXINSTALLERGATEWAY_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_LINUXINSTALLERGATEWAY_H

#include "../../application/ports/InstallerGateway.h"

class LinuxInstallerGateway final : public InstallerGateway
{
public:
    [[nodiscard]] bool IsClientRunning() const override;
    [[nodiscard]] bool IsProcessRunning(const QString& processName) const override;
    [[nodiscard]] bool IsInstallerInsideInstallDir() const override;
    [[nodiscard]] bool ClientExeExists() const override;
    [[nodiscard]] QString ClientInstallDir() const override;

    [[nodiscard]] DownloadResult DownloadRelease(
        const ReleaseInfo& release, const std::function<void(double)>& onFraction) override;
    [[nodiscard]] bool PrepareCleanInstallDir() override;
    [[nodiscard]] InstallOutcome ExtractClientPackage(const QString& zipPath) override;
    [[nodiscard]] InstallOutcome ExtractCommbusPackage(const QString& zipPath,
                                                       const QString& communityPath,
                                                       const QString& version) override;
    void InstallUninstallerCopy() override;
    void WriteUninstallEntry(const QString& version) override;
    void ApplyShortcut(ShortcutKind kind, bool wanted) override;
    [[nodiscard]] bool ExeXmlAdd(const QString& exeXmlPath) override;
    [[nodiscard]] bool ExeXmlRemove(const QString& exeXmlPath) override;

    [[nodiscard]] QList<CommbusPackage> DetectedCommbusPackages() const override;
    [[nodiscard]] QStringList AllExeXmlPaths() const override;
    [[nodiscard]] bool RemoveDirRecursively(const QString& dir) override;
    void RemoveShortcut(ShortcutKind kind) override;
    void RemoveUninstallRegistryEntry() override;
    void ScheduleInstallDirRemoval() override;
};

#endif
