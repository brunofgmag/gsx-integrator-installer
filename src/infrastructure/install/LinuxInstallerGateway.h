#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_LINUXINSTALLERGATEWAY_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_LINUXINSTALLERGATEWAY_H

#include <QtCore/QString>

#include "../../application/ports/InstallerGateway.h"
#include "../system/ProcessScan.h"

class LinuxInstallerGateway final : public InstallerGateway
{
public:
    explicit LinuxInstallerGateway(QString procRoot = QString::fromLatin1(kProcRoot));

    [[nodiscard]] bool IsClientRunning() const override;
    [[nodiscard]] bool IsProcessRunning(const QString& processName) const override;
    [[nodiscard]] bool IsInstallerInsideInstallDir() const override;
    [[nodiscard]] bool ClientExecutableExists() const override;
    [[nodiscard]] QString ClientInstallDir() const override;

    [[nodiscard]] DownloadResult DownloadRelease(
        const ReleaseInfo& release, const std::function<void(double)>& onFraction) override;
    [[nodiscard]] bool PrepareCleanInstallDir() override;
    [[nodiscard]] InstallOutcome InstallClientPackage(const QString& packagePath) override;
    [[nodiscard]] InstallOutcome ExtractCommbusPackage(const QString& packagePath,
                                                       const QString& communityPath,
                                                       const QString& version) override;
    void InstallUninstallerCopy() override;
    void WriteUninstallEntry(const QString& version) override;
    void ApplyShortcut(ShortcutKind kind, bool wanted) override;
    [[nodiscard]] bool AddAutoStartEntry(const QString& autoStartConfigPath) override;
    [[nodiscard]] bool RemoveAutoStartEntry(const QString& autoStartConfigPath) override;

    [[nodiscard]] QList<CommbusPackage> DetectedCommbusPackages() const override;
    [[nodiscard]] QStringList AllAutoStartConfigPaths() const override;
    [[nodiscard]] bool RemoveDirRecursively(const QString& dir) override;
    void RemoveShortcut(ShortcutKind kind) override;
    void RemoveUninstallEntry() override;
    [[nodiscard]] bool ScheduleInstallDirRemoval() override;

private:
    QString procRoot_;
};

#endif
