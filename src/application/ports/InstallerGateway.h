#ifndef GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_INSTALLERGATEWAY_H
#define GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_INSTALLERGATEWAY_H

#include <functional>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include "../../domain/model/InstallOutcome.h"
#include "../../domain/model/ReleaseInfo.h"

enum class ShortcutKind
{
    ClientStartMenu,
    Desktop,
    Setup,
};

struct DownloadResult
{
    InstallStatus status = InstallStatus::Success;
    QString detail;
    QString zipPath;
};

class InstallerGateway
{
public:
    virtual ~InstallerGateway() = default;

    [[nodiscard]] virtual bool IsClientRunning() const = 0;
    [[nodiscard]] virtual bool IsProcessRunning(const QString& processName) const = 0;
    [[nodiscard]] virtual bool IsInstallerInsideInstallDir() const = 0;
    [[nodiscard]] virtual bool ClientExeExists() const = 0;
    [[nodiscard]] virtual QString ClientInstallDir() const = 0;

    [[nodiscard]] virtual DownloadResult DownloadRelease(
        const ReleaseInfo& release, const std::function<void(double)>& onFraction) = 0;
    [[nodiscard]] virtual bool PrepareCleanInstallDir() = 0;
    [[nodiscard]] virtual InstallOutcome ExtractClientPackage(const QString& zipPath) = 0;
    [[nodiscard]] virtual InstallOutcome ExtractCommbusPackage(const QString& zipPath,
                                                               const QString& communityPath) = 0;
    virtual void InstallUninstallerCopy() = 0;
    virtual void WriteUninstallEntry(const QString& version) = 0;
    virtual void ApplyShortcut(ShortcutKind kind, bool wanted) = 0;
    [[nodiscard]] virtual bool ExeXmlAdd(const QString& exeXmlPath) = 0;
    [[nodiscard]] virtual bool ExeXmlRemove(const QString& exeXmlPath) = 0;

    [[nodiscard]] virtual QStringList DetectedCommbusPackageDirs() const = 0;
    [[nodiscard]] virtual QStringList AllExeXmlPaths() const = 0;
    virtual bool RemoveDirRecursively(const QString& dir) = 0;
    virtual void RemoveShortcut(ShortcutKind kind) = 0;
    virtual void RemoveUninstallRegistryEntry() = 0;
    virtual void ScheduleInstallDirRemoval() = 0;
};

#endif
