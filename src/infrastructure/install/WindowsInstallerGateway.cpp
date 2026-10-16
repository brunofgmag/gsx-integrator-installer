#include "WindowsInstallerGateway.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QScopeGuard>
#include <QtCore/QSettings>

#include <windows.h>
#include <ole2.h>
#include <shlguid.h>
#include <shobjidl_core.h>

#include "../system/InstallLocations.h"
#include "../system/SimDetection.h"
#include "DirCopy.h"
#include "ExeXml.h"
#include "PackageFiles.h"
#include "RemovalScript.h"

namespace
{
    bool CreateShortcutFile(const QString& lnkPath, const QString& targetPath,
                            const QString& workingDir, const QString& description)
    {
        IShellLinkW* link = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                                      IID_IShellLinkW, reinterpret_cast<void**>(&link));
        if (FAILED(hr))
        {
            return false;
        }

        (void)link->SetPath(targetPath.toStdWString().c_str());
        (void)link->SetWorkingDirectory(workingDir.toStdWString().c_str());
        (void)link->SetDescription(description.toStdWString().c_str());

        IPersistFile* file = nullptr;
        hr = link->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&file));

        bool ok = false;

        if (SUCCEEDED(hr))
        {
            ok = SUCCEEDED(file->Save(lnkPath.toStdWString().c_str(), TRUE));
            file->Release();
        }

        link->Release();

        return ok;
    }

    QString MaintenanceDir()
    {
        return QDir::toNativeSeparators(ClientInstallDir() + QStringLiteral("/maintenance"));
    }

    QString UninstallerPath()
    {
        return QDir::toNativeSeparators(MaintenanceDir() + u'/' + QLatin1String(kInstallerExeName));
    }

    struct ShortcutDesc
    {
        QString lnkPath;
        QString target;
        QString description;
    };

    ShortcutDesc DescribeShortcut(const ShortcutKind kind)
    {
        switch (kind)
        {
        case ShortcutKind::ClientMenu:
            return {StartMenuShortcutPath(), ClientExePath(), QStringLiteral("GSX Integrator")};
        case ShortcutKind::Desktop:
            return {DesktopShortcutPath(), ClientExePath(), QStringLiteral("GSX Integrator")};
        case ShortcutKind::Setup:
            return {
                SetupStartMenuShortcutPath(), UninstallerPath(),
                QStringLiteral("Update or uninstall GSX Integrator")
            };
        }

        return {};
    }
}

bool WindowsInstallerGateway::IsClientRunning() const
{
    return ::IsProcessRunning(QLatin1String(kClientExeName));
}

bool WindowsInstallerGateway::IsProcessRunning(const QString& processName) const
{
    return ::IsProcessRunning(processName);
}

bool WindowsInstallerGateway::IsInstallerInsideInstallDir() const
{
    const QString selfDir = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());

    if (selfDir.startsWith(MaintenanceDir(), Qt::CaseInsensitive))
    {
        return false;
    }

    return selfDir.startsWith(ClientInstallDir(), Qt::CaseInsensitive);
}

bool WindowsInstallerGateway::ClientExecutableExists() const
{
    return QFile::exists(ClientExePath());
}

QString WindowsInstallerGateway::ClientInstallDir() const
{
    return ::ClientInstallDir();
}

DownloadResult WindowsInstallerGateway::DownloadRelease(
    const ReleaseInfo& release, const std::function<void(double)>& onFraction)
{
    return DownloadVerifiedRelease(release, onFraction);
}

bool WindowsInstallerGateway::PrepareCleanInstallDir()
{
    const QString installDir = ClientInstallDir();
    (void)QDir().mkpath(QFileInfo(installDir).absolutePath());

    return RemoveDirContentsExcept(installDir, QFileInfo(MaintenanceDir()).fileName());
}

InstallOutcome WindowsInstallerGateway::InstallClientPackage(const QString& packagePath)
{
    return ExtractZipTo(packagePath, QFileInfo(ClientInstallDir()).absolutePath());
}

InstallOutcome WindowsInstallerGateway::ExtractCommbusPackage(const QString& packagePath,
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

void WindowsInstallerGateway::InstallUninstallerCopy()
{
    const QString maintenanceDir = MaintenanceDir();
    const QString selfDir = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());

    if (selfDir.startsWith(maintenanceDir, Qt::CaseInsensitive))
    {
        return;
    }

    QDir(maintenanceDir).removeRecursively();
    CopyDirRecursively(QCoreApplication::applicationDirPath(), maintenanceDir);
}

void WindowsInstallerGateway::WriteUninstallEntry(const QString& version)
{
    QSettings key(QLatin1String(kUninstallKey), QSettings::NativeFormat);
    key.setValue(QStringLiteral("DisplayName"), QStringLiteral("GSX Integrator"));
    key.setValue(QStringLiteral("DisplayVersion"), version);
    key.setValue(QStringLiteral("DisplayIcon"), QDir::toNativeSeparators(ClientExePath()));
    key.setValue(QStringLiteral("InstallLocation"), QDir::toNativeSeparators(ClientInstallDir()));
    key.setValue(QStringLiteral("UninstallString"), QStringLiteral("\"%1\" --uninstall").arg(UninstallerPath()));
    key.setValue(QStringLiteral("Publisher"), QStringLiteral("brunofgmag"));
    key.setValue(QStringLiteral("NoModify"), 1);
    key.setValue(QStringLiteral("NoRepair"), 1);
}

void WindowsInstallerGateway::ApplyShortcut(const ShortcutKind kind, const bool wanted)
{
    const ShortcutDesc desc = DescribeShortcut(kind);
    if (wanted && QFile::exists(desc.target))
    {
        const bool comInitialized = SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
        const auto comCleanup = qScopeGuard([comInitialized] { if (comInitialized) CoUninitialize(); });
        CreateShortcutFile(desc.lnkPath, desc.target, ClientInstallDir(), desc.description);
    }
    else if (!wanted)
    {
        QFile::remove(desc.lnkPath);
    }
}

bool WindowsInstallerGateway::AddAutoStartEntry(const QString& autoStartConfigPath)
{
    return ExeXmlAddUpdate(autoStartConfigPath, ClientExePath(), QLatin1String(kAppDisplayName));
}

bool WindowsInstallerGateway::RemoveAutoStartEntry(const QString& autoStartConfigPath)
{
    return ::ExeXmlRemove(autoStartConfigPath, QLatin1String(kClientExeName));
}

QList<CommbusPackage> WindowsInstallerGateway::DetectedCommbusPackages() const
{
    return FindInstalledCommbusPackages();
}

QStringList WindowsInstallerGateway::AllAutoStartConfigPaths() const
{
    return CandidateExeXmlPaths();
}

bool WindowsInstallerGateway::RemoveDirRecursively(const QString& dir)
{
    return RemoveExistingPackageDir(dir);
}

void WindowsInstallerGateway::RemoveShortcut(const ShortcutKind kind)
{
    QFile::remove(DescribeShortcut(kind).lnkPath);
}

void WindowsInstallerGateway::RemoveUninstallEntry()
{
    RegDeleteTreeW(HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\gsx-integrator-client");
}

bool WindowsInstallerGateway::ScheduleInstallDirRemoval()
{
    const QString installDir = ClientInstallDir();
    if (!QDir(installDir).exists())
    {
        return true;
    }

    const QString scriptPath =
        QDir::toNativeSeparators(QDir::tempPath() + QStringLiteral("/gsxi-uninstall.cmd"));
    if (!WriteRemovalScript(scriptPath, installDir))
    {
        return false;
    }

    return QProcess::startDetached(QStringLiteral("cmd.exe"),
                                   {QStringLiteral("/d"), QStringLiteral("/c"), scriptPath},
                                   QDir::tempPath());
}
