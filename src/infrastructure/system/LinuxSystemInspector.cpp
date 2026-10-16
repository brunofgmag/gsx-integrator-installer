#include "LinuxSystemInspector.h"

#include <QtCore/QFile>

#include "../install/PackageFiles.h"
#include "../install/SimulatorWatcher.h"
#include "InstallLocations.h"
#include "LinuxInstallLocations.h"
#include "LinuxSimDetection.h"
#include "ProcessScan.h"
#include "ProtonPrefix.h"
#include "SimDetection.h"

QString LinuxSystemInspector::InstalledClientVersion() const
{
    return ::InstalledClientVersion();
}

bool LinuxSystemInspector::IsClientRunning() const
{
    return IsAnyProcessRunning(QLatin1String(kProcRoot), ClientProcessNames());
}

QList<SimInstall> LinuxSystemInspector::DetectSims() const
{
    return ::DetectSims();
}

QString LinuxSystemInspector::InstalledCommbusVersion(const QString& communityPath) const
{
    return ReadInstalledCommbusVersion(communityPath);
}

bool LinuxSystemInspector::IsSimRunning(const QString& processName) const
{
    return IsProcessRunning(processName);
}

bool LinuxSystemInspector::DesktopShortcutExists() const
{
    return QFile::exists(DesktopShortcutPath());
}

bool LinuxSystemInspector::ClientMenuShortcutExists() const
{
    return QFile::exists(StartMenuShortcutPath());
}

bool LinuxSystemInspector::SetupMenuShortcutExists() const
{
    return QFile::exists(SetupStartMenuShortcutPath());
}

QStringList LinuxSystemInspector::CandidateAutoStartConfigPaths(const int generation) const
{
    return ::CandidateExeXmlPaths(generation);
}

bool LinuxSystemInspector::AutoStartAvailable() const
{
    return SimulatorWatcher(WatcherScriptPath(), WatcherUnitDir()).SystemdAvailable();
}

QStringList LinuxSystemInspector::SimSearchLocations() const
{
    QStringList locations = SteamappsDirsOfCurrentUser();
    for (const QString& library : SteamLibrarySteamappsDirs(locations))
    {
        if (!locations.contains(library))
        {
            locations.append(library);
        }
    }

    return locations;
}
