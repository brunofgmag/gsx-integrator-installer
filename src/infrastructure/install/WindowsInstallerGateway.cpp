#include "WindowsInstallerGateway.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QRegularExpression>
#include <QtCore/QScopeGuard>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include <windows.h>
#include <ole2.h>
#include <shlguid.h>
#include <shobjidl_core.h>

#include "../github/GithubDownloader.h"
#include "../system/InstallLocations.h"
#include "../system/SimDetection.h"
#include "DirCopy.h"
#include "ExeXml.h"
#include "RemovalScript.h"

namespace
{
    QString TarExecutable()
    {
        QString systemTar = qEnvironmentVariable("SystemRoot", QStringLiteral("C:/Windows"))
            + QStringLiteral("/System32/tar.exe");
        if (QFile::exists(systemTar))
        {
            return systemTar;
        }

        return QStandardPaths::findExecutable(QStringLiteral("tar"));
    }

    InstallOutcome ExtractZipTo(const QString& zipPath, const QString& destParent)
    {
        QProcess tar;
        tar.start(TarExecutable(),
                  {
                      QStringLiteral("-xf"), QDir::toNativeSeparators(zipPath),
                      QStringLiteral("-C"), QDir::toNativeSeparators(destParent)
                  });

        if (!tar.waitForStarted(5000))
        {
            return {InstallStatus::TarMissing, {}};
        }

        tar.waitForFinished(-1);

        if (tar.exitStatus() != QProcess::NormalExit || tar.exitCode() != 0)
        {
            return {
                InstallStatus::ExtractFailed,
                QString::fromLocal8Bit(tar.readAllStandardError()).trimmed()
            };
        }

        return {};
    }

    QString ReadExpectedSha(const QString& shaFilePath)
    {
        QFile file(shaFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }

        const QString content = QString::fromUtf8(file.readAll()).trimmed();

        static const QRegularExpression whitespace(QStringLiteral("\\s"));

        return content.section(whitespace, 0, 0).toLower();
    }

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
        case ShortcutKind::ClientStartMenu:
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

bool WindowsInstallerGateway::ClientExeExists() const
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
    const QString tempDir = QDir::tempPath() + QStringLiteral("/gsx-integrator-installer");
    (void)QDir().mkpath(tempDir);
    const QString zipPath = tempDir + u'/' + release.zipName;

    QString error = DownloadFile(release.zipUrl, zipPath,
                                 [&](const qint64 received, const qint64 total)
                                 {
                                     if (onFraction && total > 0)
                                     {
                                         onFraction(static_cast<double>(received) / static_cast<double>(total));
                                     }
                                 });
    if (!error.isEmpty())
    {
        return {InstallStatus::DownloadFailed, error, {}};
    }

    const QString shaPath = zipPath + QStringLiteral(".sha256");
    error = DownloadFile(release.shaUrl, shaPath);
    if (!error.isEmpty())
    {
        return {InstallStatus::DownloadFailed, error, {}};
    }

    const QString expected = ReadExpectedSha(shaPath);
    if (expected.isEmpty() || Sha256File(zipPath) != expected)
    {
        return {InstallStatus::ChecksumMismatch, release.zipName, {}};
    }

    return {InstallStatus::Success, {}, zipPath};
}

bool WindowsInstallerGateway::PrepareCleanInstallDir()
{
    const QString installDir = ClientInstallDir();
    (void)QDir().mkpath(QFileInfo(installDir).absolutePath());

    return RemoveDirContentsExcept(installDir, QFileInfo(MaintenanceDir()).fileName());
}

InstallOutcome WindowsInstallerGateway::ExtractClientPackage(const QString& zipPath)
{
    return ExtractZipTo(zipPath, QFileInfo(ClientInstallDir()).absolutePath());
}

InstallOutcome WindowsInstallerGateway::ExtractCommbusPackage(const QString& zipPath,
                                                              const QString& communityPath)
{
    const QString packageDir = communityPath + u'/' + QLatin1String(kCommbusPackageName);
    if (QDir(packageDir).exists() && !QDir(packageDir).removeRecursively())
    {
        return {InstallStatus::CommbusReplaceFailed, communityPath};
    }

    return ExtractZipTo(zipPath, communityPath);
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

bool WindowsInstallerGateway::ExeXmlAdd(const QString& exeXmlPath)
{
    return ExeXmlAddUpdate(exeXmlPath, ClientExePath(), QLatin1String(kAppDisplayName));
}

bool WindowsInstallerGateway::ExeXmlRemove(const QString& exeXmlPath)
{
    return ::ExeXmlRemove(exeXmlPath, QLatin1String(kClientExeName));
}

QStringList WindowsInstallerGateway::DetectedCommbusPackageDirs() const
{
    QStringList dirs;
    for (const SimInstall& sim : DetectSims())
    {
        const QString packageDir = sim.communityPath + u'/' + QLatin1String(kCommbusPackageName);
        if (QDir(packageDir).exists())
        {
            dirs.append(packageDir);
        }
    }

    return dirs;
}

QStringList WindowsInstallerGateway::AllExeXmlPaths() const
{
    return CandidateExeXmlPaths();
}

bool WindowsInstallerGateway::RemoveDirRecursively(const QString& dir)
{
    return QDir(dir).removeRecursively();
}

void WindowsInstallerGateway::RemoveShortcut(const ShortcutKind kind)
{
    QFile::remove(DescribeShortcut(kind).lnkPath);
}

void WindowsInstallerGateway::RemoveUninstallRegistryEntry()
{
    RegDeleteTreeW(HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\gsx-integrator-client");
}

void WindowsInstallerGateway::ScheduleInstallDirRemoval()
{
    const QString installDir = ClientInstallDir();
    if (!QDir(installDir).exists())
    {
        return;
    }

    const QString scriptPath =
        QDir::toNativeSeparators(QDir::tempPath() + QStringLiteral("/gsxi-uninstall.cmd"));
    if (WriteRemovalScript(scriptPath, installDir))
    {
        QProcess::startDetached(QStringLiteral("cmd.exe"),
                                {QStringLiteral("/d"), QStringLiteral("/c"), scriptPath},
                                QDir::tempPath());
    }
}
