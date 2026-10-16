#include <QtTest/QTest>

#include "../src/application/install/InstallOrchestrator.h"
#include "doubles/FakeInstallerGateway.h"

class InstallOrchestratorTest final : public QObject
{
    Q_OBJECT

private slots:
    static void abortsWhenClientRunning();
    static void abortsWhenInstallerInsideInstallDir();
    static void abortsOnDownloadError();
    static void abortsWhenPackageMissingExecutable();
    static void installsClientInOrder();
    static void abortsWhenSimRunningForCommbus();
    static void stampsCommbusVersionOnExtract();
    static void appliesShortcutsWithWantedFlags();
    static void updatesAutoStartTargets();
    static void uninstallRunsFullSequence();
    static void uninstallRefusedWhenSimRunningRemovesNothing();
    static void uninstallIgnoresRunningSimWithoutTheModule();
    static void uninstallReportsPackageThatCouldNotBeRemoved();
    static void uninstallReportsAutoStartEntryThatCouldNotBeRemoved();
    static void uninstallReportsInstallDirThatCouldNotBeRemoved();
    static void uninstallRemovesShortcutsAndEntryOnlyAfterTheInstallDirIsGone();
};

void InstallOrchestratorTest::abortsWhenClientRunning()
{
    FakeInstallerGateway gateway;
    gateway.clientRunning = true;
    InstallRequest request;
    request.installClient = true;

    const InstallOutcome result = InstallOrchestrator(gateway).RunInstall(request, {});

    QVERIFY(!result.ok());
    QCOMPARE(result.status, InstallStatus::ClientRunning);
    QVERIFY(!gateway.calls.contains(QStringLiteral("download")));
}

void InstallOrchestratorTest::abortsWhenInstallerInsideInstallDir()
{
    FakeInstallerGateway gateway;
    gateway.installerInsideInstallDir = true;
    InstallRequest request;
    request.installClient = true;

    const InstallOutcome result = InstallOrchestrator(gateway).RunInstall(request, {});

    QVERIFY(!result.ok());
    QCOMPARE(result.status, InstallStatus::InstallerInsideInstallDir);
    QVERIFY(!gateway.calls.contains(QStringLiteral("download")));
}

void InstallOrchestratorTest::abortsOnDownloadError()
{
    FakeInstallerGateway gateway;
    gateway.downloadStatus = InstallStatus::DownloadFailed;
    gateway.downloadDetail = QStringLiteral("boom");
    InstallRequest request;
    request.installClient = true;

    const InstallOutcome result = InstallOrchestrator(gateway).RunInstall(request, {});

    QVERIFY(!result.ok());
    QCOMPARE(result.status, InstallStatus::DownloadFailed);
    QCOMPARE(result.detail, QStringLiteral("boom"));
    QVERIFY(!gateway.calls.contains(QStringLiteral("extractClient")));
}

void InstallOrchestratorTest::abortsWhenPackageMissingExecutable()
{
    FakeInstallerGateway gateway;
    gateway.clientExe = false;
    InstallRequest request;
    request.installClient = true;

    const InstallOutcome result = InstallOrchestrator(gateway).RunInstall(request, {});

    QVERIFY(!result.ok());
    QCOMPARE(result.status, InstallStatus::ClientExecutableMissing);
    QVERIFY(gateway.calls.contains(QStringLiteral("extractClient")));
    QVERIFY(!gateway.calls.contains(QStringLiteral("uninstallerCopy")));
}

void InstallOrchestratorTest::installsClientInOrder()
{
    FakeInstallerGateway gateway;
    InstallRequest request;
    request.installClient = true;
    request.clientRelease.version = QStringLiteral("1.2.3");

    const InstallOutcome result = InstallOrchestrator(gateway).RunInstall(request, {});

    QVERIFY(result.ok());

    const qsizetype download = gateway.calls.indexOf(QStringLiteral("download"));
    const qsizetype prepare = gateway.calls.indexOf(QStringLiteral("prepare"));
    const qsizetype extract = gateway.calls.indexOf(QStringLiteral("extractClient"));

    QVERIFY(download >= 0);
    QVERIFY(download < prepare);
    QVERIFY(prepare < extract);
    QVERIFY(gateway.calls.contains(QStringLiteral("uninstallerCopy")));
    QVERIFY(gateway.calls.contains(QStringLiteral("uninstallEntry:1.2.3")));
}

void InstallOrchestratorTest::abortsWhenSimRunningForCommbus()
{
    FakeInstallerGateway gateway;
    gateway.runningProcesses.insert(QStringLiteral("FlightSimulator.exe"));
    InstallRequest request;
    CommbusTarget target;
    target.processName = QStringLiteral("FlightSimulator.exe");
    target.label = QStringLiteral("MSFS 2020");
    target.communityPath = QStringLiteral("C:/Community");
    request.commbusTargets.append(target);

    const InstallOutcome result = InstallOrchestrator(gateway).RunInstall(request, {});

    QVERIFY(!result.ok());
    QCOMPARE(result.status, InstallStatus::SimRunning);
    QVERIFY(!gateway.calls.contains(QStringLiteral("download")));
}

void InstallOrchestratorTest::stampsCommbusVersionOnExtract()
{
    FakeInstallerGateway gateway;
    InstallRequest request;
    CommbusTarget target;

    request.commbusRelease.version = QStringLiteral("0.2.0");
    target.communityPath = QStringLiteral("C:/Community");
    request.commbusTargets.append(target);

    const InstallOutcome result = InstallOrchestrator(gateway).RunInstall(request, {});

    QVERIFY(result.ok());
    QVERIFY(gateway.calls.contains(QStringLiteral("extractCommbus:C:/Community:0.2.0")));
}

void InstallOrchestratorTest::appliesShortcutsWithWantedFlags()
{
    FakeInstallerGateway gateway;
    InstallRequest request;
    request.desktopShortcut = true;
    request.clientMenuShortcut = false;
    request.setupShortcut = true;

    const InstallOutcome result = InstallOrchestrator(gateway).RunInstall(request, {});

    QVERIFY(result.ok());
    QVERIFY(gateway.calls.contains(
        QStringLiteral("shortcut:%1:1").arg(static_cast<int>(ShortcutKind::Desktop))));
    QVERIFY(gateway.calls.contains(
        QStringLiteral("shortcut:%1:0").arg(static_cast<int>(ShortcutKind::ClientMenu))));
    QVERIFY(gateway.calls.contains(
        QStringLiteral("shortcut:%1:1").arg(static_cast<int>(ShortcutKind::Setup))));
}

void InstallOrchestratorTest::updatesAutoStartTargets()
{
    FakeInstallerGateway gateway;
    InstallRequest request;
    request.autoStartRemoveTargets << QStringLiteral("C:/a/EXE.xml");
    request.autoStartAddTargets << QStringLiteral("C:/b/EXE.xml");

    const InstallOutcome result = InstallOrchestrator(gateway).RunInstall(request, {});

    QVERIFY(result.ok());
    QVERIFY(gateway.calls.contains(QStringLiteral("autoStartRemove:C:/a/EXE.xml")));
    QVERIFY(gateway.calls.contains(QStringLiteral("autoStartAdd:C:/b/EXE.xml")));
}

void InstallOrchestratorTest::uninstallRunsFullSequence()
{
    FakeInstallerGateway gateway;
    gateway.commbusPackages << CommbusPackage{
        QStringLiteral("C:/Community/gsx-integrator-commbus"), QStringLiteral("FlightSimulator.exe"),
        QStringLiteral("MSFS 2020")
    };
    gateway.autoStartConfigPaths << QStringLiteral("C:/a/EXE.xml") << QStringLiteral("C:/b/EXE.xml");

    const UninstallOutcome result = InstallOrchestrator(gateway).RunUninstall();

    QVERIFY(result.ok());
    QVERIFY(result.leftovers.isEmpty());
    QVERIFY(gateway.calls.contains(QStringLiteral("removeDir:C:/Community/gsx-integrator-commbus")));
    QVERIFY(gateway.calls.contains(QStringLiteral("autoStartRemove:C:/a/EXE.xml")));
    QVERIFY(gateway.calls.contains(QStringLiteral("autoStartRemove:C:/b/EXE.xml")));
    QVERIFY(gateway.calls.contains(QStringLiteral("removeUninstallEntry")));
    QVERIFY(gateway.calls.contains(QStringLiteral("scheduleRemoval")));
    QVERIFY(gateway.calls.contains(QStringLiteral("removeShortcut:%1").arg(static_cast<int>(ShortcutKind::Desktop))));
}

void InstallOrchestratorTest::uninstallRefusedWhenSimRunningRemovesNothing()
{
    FakeInstallerGateway gateway;
    gateway.commbusPackages
        << CommbusPackage{QStringLiteral("C:/2020/gsx-integrator-commbus"), QStringLiteral("FlightSimulator.exe"),
                          QStringLiteral("MSFS 2020")}
        << CommbusPackage{QStringLiteral("C:/2024/gsx-integrator-commbus"),
                          QStringLiteral("FlightSimulator2024.exe"), QStringLiteral("MSFS 2024")};
    gateway.autoStartConfigPaths << QStringLiteral("C:/a/EXE.xml");
    gateway.runningProcesses.insert(QStringLiteral("FlightSimulator2024.exe"));

    const UninstallOutcome result = InstallOrchestrator(gateway).RunUninstall();

    QVERIFY(!result.ok());
    QCOMPARE(result.status, UninstallStatus::SimRunning);
    QCOMPARE(result.detail, QStringLiteral("MSFS 2024"));
    QVERIFY(result.leftovers.isEmpty());
    QVERIFY(gateway.calls.isEmpty());
}

void InstallOrchestratorTest::uninstallIgnoresRunningSimWithoutTheModule()
{
    FakeInstallerGateway gateway;
    gateway.commbusPackages << CommbusPackage{
        QStringLiteral("C:/2020/gsx-integrator-commbus"), QStringLiteral("FlightSimulator.exe"),
        QStringLiteral("MSFS 2020")
    };
    gateway.runningProcesses.insert(QStringLiteral("FlightSimulator2024.exe"));

    const UninstallOutcome result = InstallOrchestrator(gateway).RunUninstall();

    QVERIFY(result.ok());
    QVERIFY(gateway.calls.contains(QStringLiteral("removeDir:C:/2020/gsx-integrator-commbus")));
}

void InstallOrchestratorTest::uninstallReportsPackageThatCouldNotBeRemoved()
{
    FakeInstallerGateway gateway;
    gateway.commbusPackages
        << CommbusPackage{QStringLiteral("C:/2020/gsx-integrator-commbus"), QStringLiteral("FlightSimulator.exe"),
                          QStringLiteral("MSFS 2020")}
        << CommbusPackage{QStringLiteral("C:/2024/gsx-integrator-commbus"),
                          QStringLiteral("FlightSimulator2024.exe"), QStringLiteral("MSFS 2024")};
    gateway.undeletableDirs.insert(QStringLiteral("C:/2020/gsx-integrator-commbus"));

    const UninstallOutcome result = InstallOrchestrator(gateway).RunUninstall();

    QVERIFY(!result.ok());
    QCOMPARE(result.status, UninstallStatus::LeftoversRemain);
    QCOMPARE(result.leftovers, QStringList{QStringLiteral("C:/2020/gsx-integrator-commbus")});
    QVERIFY(gateway.calls.contains(QStringLiteral("removeDir:C:/2024/gsx-integrator-commbus")));
    QVERIFY(!gateway.calls.contains(QStringLiteral("removeUninstallEntry")));
    QVERIFY(!gateway.calls.contains(QStringLiteral("scheduleRemoval")));
    QVERIFY(!gateway.calls.contains(
        QStringLiteral("removeShortcut:%1").arg(static_cast<int>(ShortcutKind::Desktop))));
}

void InstallOrchestratorTest::uninstallReportsAutoStartEntryThatCouldNotBeRemoved()
{
    FakeInstallerGateway gateway;
    gateway.autoStartConfigPaths << QStringLiteral("C:/a/EXE.xml");
    gateway.autoStartRemoveOk = false;

    const UninstallOutcome result = InstallOrchestrator(gateway).RunUninstall();

    QVERIFY(!result.ok());
    QCOMPARE(result.status, UninstallStatus::LeftoversRemain);
    QCOMPARE(result.leftovers, QStringList{QStringLiteral("C:/a/EXE.xml")});
    QVERIFY(!gateway.calls.contains(QStringLiteral("removeUninstallEntry")));
    QVERIFY(!gateway.calls.contains(QStringLiteral("scheduleRemoval")));
}

void InstallOrchestratorTest::uninstallReportsInstallDirThatCouldNotBeRemoved()
{
    FakeInstallerGateway gateway;
    gateway.scheduleRemovalOk = false;

    const UninstallOutcome result = InstallOrchestrator(gateway).RunUninstall();

    QVERIFY(!result.ok());
    QCOMPARE(result.status, UninstallStatus::LeftoversRemain);
    QCOMPARE(result.leftovers, QStringList{gateway.ClientInstallDir()});
    QVERIFY(gateway.calls.contains(QStringLiteral("scheduleRemoval")));
    QVERIFY(!gateway.calls.contains(QStringLiteral("removeUninstallEntry")));
    QVERIFY(!gateway.calls.contains(
        QStringLiteral("removeShortcut:%1").arg(static_cast<int>(ShortcutKind::Desktop))));
    QVERIFY(!gateway.calls.contains(
        QStringLiteral("removeShortcut:%1").arg(static_cast<int>(ShortcutKind::Setup))));
}

void InstallOrchestratorTest::uninstallRemovesShortcutsAndEntryOnlyAfterTheInstallDirIsGone()
{
    FakeInstallerGateway gateway;

    const UninstallOutcome result = InstallOrchestrator(gateway).RunUninstall();

    QVERIFY(result.ok());

    const qsizetype schedule = gateway.calls.indexOf(QStringLiteral("scheduleRemoval"));
    QVERIFY(schedule >= 0);
    QVERIFY(schedule < gateway.calls.indexOf(QStringLiteral("removeUninstallEntry")));
    QVERIFY(schedule < gateway.calls.indexOf(
        QStringLiteral("removeShortcut:%1").arg(static_cast<int>(ShortcutKind::Setup))));
}

QTEST_GUILESS_MAIN(InstallOrchestratorTest)

#include "tst_install_orchestrator.moc"
