#include "LinuxInstallerGateway.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include "../system/InstallLocations.h"
#include "../system/LinuxInstallLocations.h"
#include "../system/LinuxSimDetection.h"
#include "../system/ProcessScan.h"
#include "../system/SimDetection.h"
#include "AppImageFiles.h"
#include "DesktopEntry.h"
#include "PackageFiles.h"
#include "SimulatorWatcher.h"

namespace
{
    struct ShortcutDesc
    {
        QString desktopPath;
        QString target;
        QString name;
        QString comment;
        bool onDesktopFolder = false;
    };

    ShortcutDesc DescribeShortcut(const ShortcutKind kind)
    {
        const QString name = QLatin1String(kAppDisplayName);
        switch (kind)
        {
        case ShortcutKind::ClientMenu:
            return {StartMenuShortcutPath(), ClientExePath(), name, name, false};
        case ShortcutKind::Desktop:
            return {DesktopShortcutPath(), ClientExePath(), name, name, true};
        case ShortcutKind::Setup:
            return {
                SetupStartMenuShortcutPath(), MaintenanceInstallerPath(),
                QStringLiteral("GSX Integrator Installer"),
                QStringLiteral("Update or uninstall GSX Integrator"), false
            };
        }

        return {};
    }

    bool EnsureDesktopIcon()
    {
        const QString iconPath = DesktopIconPath();
        if (QFile::exists(iconPath))
        {
            return true;
        }

        QFile resource(QStringLiteral(":/icons/app-icon.png"));
        if (!resource.open(QIODevice::ReadOnly))
        {
            return false;
        }

        QFile icon(iconPath);
        if (!QDir().mkpath(QFileInfo(iconPath).absolutePath()) || !icon.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            return false;
        }

        return icon.write(resource.readAll()) >= 0;
    }

    bool WriteDesktopFile(const ShortcutDesc& desc)
    {
        if (!QDir().mkpath(QFileInfo(desc.desktopPath).absolutePath()))
        {
            return false;
        }

        QFile file(desc.desktopPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            return false;
        }

        const QString iconPath = EnsureDesktopIcon() ? DesktopIconPath() : QString();
        const DesktopEntry entry{desc.name, desc.comment, desc.target, iconPath};
        if (file.write(BuildDesktopEntry(entry).toUtf8()) < 0)
        {
            return false;
        }

        file.close();

        return desc.onDesktopFolder ? MakeExecutable(desc.desktopPath) : true;
    }

    SimulatorWatcher Watcher()
    {
        return SimulatorWatcher(WatcherScriptPath(), WatcherUnitDir());
    }
}

bool LinuxInstallerGateway::IsClientRunning() const
{
    return IsAnyProcessRunning(QLatin1String(kProcRoot), ClientProcessNames());
}

bool LinuxInstallerGateway::IsProcessRunning(const QString& processName) const
{
    return ::IsProcessRunning(processName);
}

bool LinuxInstallerGateway::IsInstallerInsideInstallDir() const
{
    const QString self = RunningAppImagePath().isEmpty() ? QCoreApplication::applicationFilePath()
                                                         : RunningAppImagePath();
    const QString selfDir = QDir::cleanPath(QFileInfo(self).absolutePath());
    const QString installDir = ClientInstallDir();
    const QString maintenanceDir = MaintenanceDir();

    if (selfDir == maintenanceDir || selfDir.startsWith(maintenanceDir + u'/'))
    {
        return false;
    }

    return selfDir == installDir || selfDir.startsWith(installDir + u'/');
}

bool LinuxInstallerGateway::ClientExecutableExists() const
{
    return QFile::exists(ClientExePath());
}

QString LinuxInstallerGateway::ClientInstallDir() const
{
    return ::ClientInstallDir();
}

DownloadResult LinuxInstallerGateway::DownloadRelease(
    const ReleaseInfo& release, const std::function<void(double)>& onFraction)
{
    return DownloadVerifiedRelease(release, onFraction);
}

bool LinuxInstallerGateway::PrepareCleanInstallDir()
{
    if (!QDir().mkpath(ClientInstallDir()))
    {
        return false;
    }

    QFile::remove(ClientExePath() + QStringLiteral(".part"));
    if (QFile::exists(ClientExePath()))
    {
        return QFile::remove(ClientExePath());
    }

    return true;
}

InstallOutcome LinuxInstallerGateway::InstallClientPackage(const QString& packagePath)
{
    if (!CopyExecutable(packagePath, ClientExePath()))
    {
        return {InstallStatus::ExtractWriteFailed, ClientExePath()};
    }

    return {};
}

InstallOutcome LinuxInstallerGateway::ExtractCommbusPackage(const QString& packagePath,
                                                            const QString& communityPath,
                                                            const QString& version)
{
    const QString packageDir = communityPath + u'/' + QLatin1String(kCommbusPackageName);
    if (!RemoveExistingPackageDir(packageDir))
    {
        return {InstallStatus::CommbusReplaceFailed, communityPath};
    }

    if (const InstallOutcome extract = ExtractZipTo(packagePath, communityPath); !extract.ok())
    {
        return extract;
    }

    WriteCommbusVersionMarker(packageDir, version);

    return {};
}

void LinuxInstallerGateway::InstallUninstallerCopy()
{
    const QString self = RunningAppImagePath();
    if (self.isEmpty())
    {
        return;
    }

    const QString selfDir = QDir::cleanPath(QFileInfo(self).absolutePath());
    if (selfDir == MaintenanceDir())
    {
        return;
    }

    (void)CopyExecutable(self, MaintenanceInstallerPath());
}

void LinuxInstallerGateway::WriteUninstallEntry(const QString& version)
{
    QSettings metadata(InstallMetadataPath(), QSettings::IniFormat);
    metadata.setValue(QLatin1String(kInstalledVersionKey), version);
    metadata.sync();
}

void LinuxInstallerGateway::ApplyShortcut(const ShortcutKind kind, const bool wanted)
{
    const ShortcutDesc desc = DescribeShortcut(kind);
    if (wanted && QFile::exists(desc.target))
    {
        (void)WriteDesktopFile(desc);
    }
    else if (!wanted)
    {
        QFile::remove(desc.desktopPath);
    }
}

bool LinuxInstallerGateway::AddAutoStartEntry(const QString& autoStartConfigPath)
{
    const QString processName = SimProcessNameForExeXml(autoStartConfigPath, SteamappsDirsOfCurrentUser());
    return !processName.isEmpty() && Watcher().Add(processName);
}

bool LinuxInstallerGateway::RemoveAutoStartEntry(const QString& autoStartConfigPath)
{
    const QString processName = SimProcessNameForExeXml(autoStartConfigPath, SteamappsDirsOfCurrentUser());
    return processName.isEmpty() || Watcher().Remove(processName);
}

QList<CommbusPackage> LinuxInstallerGateway::DetectedCommbusPackages() const
{
    return FindInstalledCommbusPackages();
}

QStringList LinuxInstallerGateway::AllAutoStartConfigPaths() const
{
    return CandidateExeXmlPaths();
}

bool LinuxInstallerGateway::RemoveDirRecursively(const QString& dir)
{
    return RemoveExistingPackageDir(dir);
}

void LinuxInstallerGateway::RemoveShortcut(const ShortcutKind kind)
{
    QFile::remove(DescribeShortcut(kind).desktopPath);
}

void LinuxInstallerGateway::RemoveUninstallEntry()
{
    QFile::remove(InstallMetadataPath());
}

bool LinuxInstallerGateway::ScheduleInstallDirRemoval()
{
    if (!Watcher().Disable())
    {
        return false;
    }

    return QDir(ClientInstallDir()).removeRecursively();
}
