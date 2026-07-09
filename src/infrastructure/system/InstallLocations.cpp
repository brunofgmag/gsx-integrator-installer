#include "InstallLocations.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include <windows.h>
#include <tlhelp32.h>

QString ClientInstallDir()
{
    return QDir::toNativeSeparators(
        qEnvironmentVariable("LOCALAPPDATA") + QStringLiteral("/Programs/gsx-integrator-client"));
}

QString ClientExePath()
{
    return QDir::toNativeSeparators(ClientInstallDir() + QStringLiteral("/gsx-integrator-client.exe"));
}

QString InstalledClientVersion()
{
    const QSettings key(QLatin1String(kUninstallKey), QSettings::NativeFormat);
    QString version = key.value(QStringLiteral("DisplayVersion")).toString();
    if (version.isEmpty() || !QFile::exists(ClientExePath()))
    {
        return {};
    }
    return version;
}

bool IsProcessRunning(const QString& exeName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    
    bool found = false;
    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            if (exeName.compare(QString::fromWCharArray(entry.szExeFile), Qt::CaseInsensitive) == 0)
            {
                found = true;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return found;
}

QString StartMenuShortcutPath()
{
    return QDir::toNativeSeparators(
        QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + u'/'
        + QLatin1String(kShortcutName));
}

QString SetupStartMenuShortcutPath()
{
    return QDir::toNativeSeparators(
        QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + u'/'
        + QLatin1String(kSetupShortcutName));
}

QString DesktopShortcutPath()
{
    return QDir::toNativeSeparators(
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + u'/'
        + QLatin1String(kShortcutName));
}
