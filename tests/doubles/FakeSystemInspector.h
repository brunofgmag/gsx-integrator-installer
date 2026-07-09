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
    bool desktop = false, clientStartMenu = false, setupStartMenu = false;
    QHash<int, QStringList> candidateExeXml;

    [[nodiscard]] QString InstalledClientVersion() const override { return clientVersion; }
    [[nodiscard]] bool IsClientRunning() const override { return clientRunning; }
    [[nodiscard]] QList<SimInstall> DetectSims() const override { return sims; }

    [[nodiscard]] QString InstalledCommbusVersion(const QString& community) const override
    {
        return commbusVersions.value(community);
    }

    [[nodiscard]] bool IsSimRunning(const QString& process) const override { return running.value(process, false); }
    [[nodiscard]] bool DesktopShortcutExists() const override { return desktop; }
    [[nodiscard]] bool ClientStartMenuShortcutExists() const override { return clientStartMenu; }
    [[nodiscard]] bool SetupStartMenuShortcutExists() const override { return setupStartMenu; }

    [[nodiscard]] QStringList CandidateExeXmlPaths(const int generation) const override
    {
        return candidateExeXml.value(generation);
    }
};

#endif // GSX_INTEGRATOR_INSTALLER_TESTS_FAKESYSTEMINSPECTOR_H
