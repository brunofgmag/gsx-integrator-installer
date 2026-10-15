#include "WindowsSystemInspector.h"

#include <QtCore/QFile>
#include "../install/PackageFiles.h"
#include "InstallLocations.h"
#include "SimDetection.h"

QString WindowsSystemInspector::InstalledClientVersion() const
{
    return ::InstalledClientVersion();
}

bool WindowsSystemInspector::IsClientRunning() const
{
    return IsProcessRunning(QLatin1String(kClientExeName));
}

QList<SimInstall> WindowsSystemInspector::DetectSims() const
{
    return ::DetectSims();
}

QString WindowsSystemInspector::InstalledCommbusVersion(const QString& communityPath) const
{
    return ReadInstalledCommbusVersion(communityPath);
}

bool WindowsSystemInspector::IsSimRunning(const QString& processName) const
{
    return IsProcessRunning(processName);
}

bool WindowsSystemInspector::DesktopShortcutExists() const
{
    return QFile::exists(DesktopShortcutPath());
}

bool WindowsSystemInspector::ClientStartMenuShortcutExists() const
{
    return QFile::exists(StartMenuShortcutPath());
}

bool WindowsSystemInspector::SetupStartMenuShortcutExists() const
{
    return QFile::exists(SetupStartMenuShortcutPath());
}

QStringList WindowsSystemInspector::CandidateExeXmlPaths(const int generation) const
{
    return ::CandidateExeXmlPaths(generation);
}

bool WindowsSystemInspector::AutoStartAvailable() const
{
    return true;
}

QStringList WindowsSystemInspector::SimSearchLocations() const
{
    return {};
}
