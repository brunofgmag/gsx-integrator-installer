#ifndef GSX_INTEGRATOR_INSTALLER_TESTS_FAKESYSTEMINSPECTOR_H
#define GSX_INTEGRATOR_INSTALLER_TESTS_FAKESYSTEMINSPECTOR_H

#include <QtCore/QHash>

#include "../../src/application/ports/SystemInspector.h"

class FakeSystemInspector final : public SystemInspector
{
public:
    QString clientVersion;
    bool clientRunning = false;
    QList<SimInstall> sims;
    QHash<QString, QString> commbusVersions;
    QHash<QString, bool> running;
    bool desktop = false, clientMenu = false, setupMenu = false;
    QHash<int, QStringList> candidateAutoStartConfigPaths;
    bool autoStartAvailable = true;
    QStringList searchLocations;

    [[nodiscard]] QString InstalledClientVersion() const override { return clientVersion; }
    [[nodiscard]] bool IsClientRunning() const override { return clientRunning; }
    [[nodiscard]] QList<SimInstall> DetectSims() const override { return sims; }

    [[nodiscard]] QString InstalledCommbusVersion(const QString& community) const override
    {
        return commbusVersions.value(community);
    }

    [[nodiscard]] bool IsSimRunning(const QString& process) const override { return running.value(process, false); }
    [[nodiscard]] bool DesktopShortcutExists() const override { return desktop; }
    [[nodiscard]] bool ClientMenuShortcutExists() const override { return clientMenu; }
    [[nodiscard]] bool SetupMenuShortcutExists() const override { return setupMenu; }

    [[nodiscard]] QStringList CandidateAutoStartConfigPaths(const int generation) const override
    {
        return candidateAutoStartConfigPaths.value(generation);
    }

    [[nodiscard]] bool AutoStartAvailable() const override { return autoStartAvailable; }
    [[nodiscard]] QStringList SimSearchLocations() const override { return searchLocations; }
};

#endif // GSX_INTEGRATOR_INSTALLER_TESTS_FAKESYSTEMINSPECTOR_H
