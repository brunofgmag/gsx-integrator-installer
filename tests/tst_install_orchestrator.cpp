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
    static void abortsWhenPackageMissingExe();
    static void installsClientInOrder();
    static void abortsWhenSimRunningForCommbus();
    static void appliesShortcutsWithWantedFlags();
    static void updatesExeXmlTargets();
    static void uninstallRunsFullSequence();
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

void InstallOrchestratorTest::abortsWhenPackageMissingExe()
{
    FakeInstallerGateway gateway;
    gateway.clientExe = false;
    InstallRequest request;
    request.installClient = true;

    const InstallOutcome result = InstallOrchestrator(gateway).RunInstall(request, {});

    QVERIFY(!result.ok());
    QCOMPARE(result.status, InstallStatus::ClientExeMissing);
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
    QVERIFY(gateway.calls.contains(QStringLiteral("registry:1.2.3")));
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

void InstallOrchestratorTest::appliesShortcutsWithWantedFlags()
{
    FakeInstallerGateway gateway;
    InstallRequest request;
    request.desktopShortcut = true;
    request.clientStartMenuShortcut = false;
    request.setupShortcut = true;

    const InstallOutcome result = InstallOrchestrator(gateway).RunInstall(request, {});

    QVERIFY(result.ok());
    QVERIFY(gateway.calls.contains(
        QStringLiteral("shortcut:%1:1").arg(static_cast<int>(ShortcutKind::Desktop))));
    QVERIFY(gateway.calls.contains(
        QStringLiteral("shortcut:%1:0").arg(static_cast<int>(ShortcutKind::ClientStartMenu))));
    QVERIFY(gateway.calls.contains(
        QStringLiteral("shortcut:%1:1").arg(static_cast<int>(ShortcutKind::Setup))));
}

void InstallOrchestratorTest::updatesExeXmlTargets()
{
    FakeInstallerGateway gateway;
    InstallRequest request;
    request.exeXmlRemoveTargets << QStringLiteral("C:/a/EXE.xml");
    request.exeXmlAddTargets << QStringLiteral("C:/b/EXE.xml");

    const InstallOutcome result = InstallOrchestrator(gateway).RunInstall(request, {});

    QVERIFY(result.ok());
    QVERIFY(gateway.calls.contains(QStringLiteral("exeXmlRemove:C:/a/EXE.xml")));
    QVERIFY(gateway.calls.contains(QStringLiteral("exeXmlAdd:C:/b/EXE.xml")));
}

void InstallOrchestratorTest::uninstallRunsFullSequence()
{
    FakeInstallerGateway gateway;
    gateway.commbusPackageDirs << QStringLiteral("C:/Community/gsx-integrator-commbus");
    gateway.exeXmlPaths << QStringLiteral("C:/a/EXE.xml") << QStringLiteral("C:/b/EXE.xml");

    InstallOrchestrator(gateway).RunUninstall();

    QVERIFY(gateway.calls.contains(QStringLiteral("removeDir:C:/Community/gsx-integrator-commbus")));
    QVERIFY(gateway.calls.contains(QStringLiteral("exeXmlRemove:C:/a/EXE.xml")));
    QVERIFY(gateway.calls.contains(QStringLiteral("exeXmlRemove:C:/b/EXE.xml")));
    QVERIFY(gateway.calls.contains(QStringLiteral("removeRegistry")));
    QVERIFY(gateway.calls.contains(QStringLiteral("scheduleRemoval")));
    QVERIFY(gateway.calls.contains(QStringLiteral("removeShortcut:%1").arg(static_cast<int>(ShortcutKind::Desktop))));
}

QTEST_GUILESS_MAIN(InstallOrchestratorTest)

#include "tst_install_orchestrator.moc"
