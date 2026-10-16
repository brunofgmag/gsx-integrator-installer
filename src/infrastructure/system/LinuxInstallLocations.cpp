#include "InstallLocations.h"
#include "LinuxInstallLocations.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include "ChildEnvironment.h"
#include "ProcessScan.h"

namespace
{
    QString AbsoluteEnvironmentDir(const char* variable, const QString& fallbackUnderHome)
    {
        const QString value = qEnvironmentVariable(variable);
        if (!value.isEmpty() && QDir::isAbsolutePath(value))
        {
            return QDir::cleanPath(value);
        }

        return QDir::cleanPath(QDir::homePath() + u'/' + fallbackUnderHome);
    }
}

QString DataHome()
{
    return AbsoluteEnvironmentDir("XDG_DATA_HOME", QStringLiteral(".local/share"));
}

QString ConfigHome()
{
    return AbsoluteEnvironmentDir("XDG_CONFIG_HOME", QStringLiteral(".config"));
}

QString WindowIconPath(const QString& dataHome, const QString& iconName)
{
    return dataHome + QStringLiteral("/icons/hicolor/256x256/apps/") + iconName;
}

QString ClientInstallDir()
{
    return DataHome() + u'/' + QLatin1String(kClientInstallDirName);
}

QString ClientExePath()
{
    return ClientInstallDir() + u'/' + QLatin1String(kClientAppImageName);
}

QString MaintenanceDir()
{
    return ClientInstallDir() + u'/' + QLatin1String(kMaintenanceDirName);
}

QString MaintenanceInstallerPath()
{
    return MaintenanceDir() + u'/' + QLatin1String(kInstallerAppImageName);
}

QString InstallMetadataPath()
{
    return ClientInstallDir() + u'/' + QLatin1String(kInstallMetadataName);
}

QString DesktopIconPath()
{
    return ClientInstallDir() + u'/' + QLatin1String(kDesktopIconName);
}

QString WatcherScriptPath()
{
    return ClientInstallDir() + u'/' + QLatin1String(kWatcherScriptName);
}

QString WatcherUnitDir()
{
    return ConfigHome() + QStringLiteral("/systemd/user");
}

QString RunningAppImagePath()
{
    const QString appImage = qEnvironmentVariable("APPIMAGE");
    return appImage.isEmpty() ? QString() : QDir::cleanPath(appImage);
}

QStringList ClientProcessNames()
{
    return {QStringLiteral("gsx-integrator-client"), QLatin1String(kClientAppImageName)};
}

QString InstalledClientVersion()
{
    if (!QFile::exists(ClientExePath()))
    {
        return {};
    }

    const QSettings metadata(InstallMetadataPath(), QSettings::IniFormat);
    return metadata.value(QLatin1String(kInstalledVersionKey)).toString();
}

bool IsProcessRunning(const QString& exeName)
{
    return IsAnyProcessRunning(QLatin1String(kProcRoot), {exeName});
}

QString StartMenuShortcutPath()
{
    return DataHome() + QStringLiteral("/applications/") + QLatin1String(kClientDesktopFileName);
}

QString SetupStartMenuShortcutPath()
{
    return DataHome() + QStringLiteral("/applications/") + QLatin1String(kSetupDesktopFileName);
}

QString DesktopShortcutPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
           + QStringLiteral("/GSX Integrator.desktop");
}

bool StartClientProcess()
{
    QProcess process;
    process.setProgram(ClientExePath());
    process.setWorkingDirectory(ClientInstallDir());
    process.setProcessEnvironment(ChildProcessEnvironment(QProcessEnvironment::systemEnvironment()));
    return process.startDetached();
}
