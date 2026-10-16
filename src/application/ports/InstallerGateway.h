#ifndef GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_INSTALLERGATEWAY_H
#define GSX_INTEGRATOR_INSTALLER_APPLICATION_PORTS_INSTALLERGATEWAY_H

#include <functional>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include "../../domain/model/InstallOutcome.h"
#include "../../domain/model/ReleaseInfo.h"

enum class ShortcutKind
{
    ClientMenu,
    Desktop,
    Setup,
};

struct DownloadResult
{
    InstallStatus status = InstallStatus::Success;
    QString detail;
    QString packagePath;
};

struct CommbusPackage
{
    QString packageDir;
    QString processName;
    QString label;
};

class InstallerGateway
{
public:
    virtual ~InstallerGateway() = default;

    [[nodiscard]] virtual bool IsClientRunning() const = 0;
    [[nodiscard]] virtual bool IsProcessRunning(const QString& processName) const = 0;
    [[nodiscard]] virtual bool IsInstallerInsideInstallDir() const = 0;
    [[nodiscard]] virtual bool ClientExecutableExists() const = 0;
    [[nodiscard]] virtual QString ClientInstallDir() const = 0;

    [[nodiscard]] virtual DownloadResult DownloadRelease(
        const ReleaseInfo& release, const std::function<void(double)>& onFraction) = 0;
    [[nodiscard]] virtual bool PrepareCleanInstallDir() = 0;
    [[nodiscard]] virtual InstallOutcome InstallClientPackage(const QString& packagePath) = 0;
    [[nodiscard]] virtual InstallOutcome ExtractCommbusPackage(const QString& packagePath,
                                                               const QString& communityPath,
                                                               const QString& version) = 0;
    virtual void InstallUninstallerCopy() = 0;
    virtual void WriteUninstallEntry(const QString& version) = 0;
    virtual void ApplyShortcut(ShortcutKind kind, bool wanted) = 0;
    [[nodiscard]] virtual bool AddAutoStartEntry(const QString& autoStartConfigPath) = 0;
    [[nodiscard]] virtual bool RemoveAutoStartEntry(const QString& autoStartConfigPath) = 0;

    [[nodiscard]] virtual QList<CommbusPackage> DetectedCommbusPackages() const = 0;
    [[nodiscard]] virtual QStringList AllAutoStartConfigPaths() const = 0;
    [[nodiscard]] virtual bool RemoveDirRecursively(const QString& dir) = 0;
    virtual void RemoveShortcut(ShortcutKind kind) = 0;
    virtual void RemoveUninstallEntry() = 0;
    [[nodiscard]] virtual bool ScheduleInstallDirRemoval() = 0;
};

#endif
