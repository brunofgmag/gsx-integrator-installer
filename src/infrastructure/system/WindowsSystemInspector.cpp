#include "WindowsSystemInspector.h"

#include <QtCore/QFile>
#include "../../domain/Versions.h"
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
    const QString manifest = communityPath + u'/'
        + QLatin1String(kCommbusPackageName) + QStringLiteral("/manifest.json");

    QFile file(manifest);

    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }

    return ParsePackageVersion(file.readAll());
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
