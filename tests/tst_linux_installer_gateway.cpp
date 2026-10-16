#include <QtTest/QTest>

#include <sys/stat.h>

#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>

#include "../src/application/install/InstallOrchestrator.h"
#include "../src/infrastructure/install/LinuxInstallerGateway.h"
#include "../src/infrastructure/system/InstallLocations.h"
#include "../src/infrastructure/system/LinuxInstallLocations.h"
#include "../src/infrastructure/system/LinuxSystemInspector.h"
#include "support/FakeSteamTree.h"

namespace
{
    ino_t InodeOf(const QString& path)
    {
        struct stat info{};
        return ::stat(path.toLocal8Bit().constData(), &info) == 0 ? info.st_ino : 0;
    }

    constexpr const char* kManagedVariables[] = {"HOME", "XDG_DATA_HOME", "XDG_CONFIG_HOME", "XDG_DESKTOP_DIR",
                                                 "APPIMAGE", "PATH"};

    class EnvironmentGuard
    {
    public:
        EnvironmentGuard()
        {
            for (const char* name : kManagedVariables)
            {
                saved_.append({name, qEnvironmentVariableIsSet(name), qgetenv(name)});
            }
        }

        ~EnvironmentGuard()
        {
            for (const Saved& entry : saved_)
            {
                if (entry.wasSet)
                {
                    qputenv(entry.name, entry.value);
                }
                else
                {
                    qunsetenv(entry.name);
                }
            }
        }

        EnvironmentGuard(const EnvironmentGuard&) = delete;
        EnvironmentGuard& operator=(const EnvironmentGuard&) = delete;

    private:
        struct Saved
        {
            const char* name;
            bool wasSet;
            QByteArray value;
        };

        QList<Saved> saved_;
    };

    QString FakeSystemctlBin(const QString& dir)
    {
        const QString bin = dir + QStringLiteral("/bin");
        const QString script = QStringLiteral(
            "#!/bin/sh\n"
            "echo \"$@\" >> \"%1/systemctl.log\"\n"
            "if [ -e \"%1/fail-$2\" ]; then exit 1; fi\n"
            "exit 0\n").arg(dir);
        if (!WriteTestFile(bin + QStringLiteral("/systemctl"), script.toUtf8())
            || !QFile::setPermissions(bin + QStringLiteral("/systemctl"),
                                      QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner))
        {
            return {};
        }

        return bin;
    }

    QStringList SystemctlLog(const QString& dir)
    {
        return QString::fromUtf8(ReadTestFile(dir + QStringLiteral("/systemctl.log"))).split(u'\n', Qt::SkipEmptyParts);
    }

    struct Sandbox
    {
        EnvironmentGuard guard;
        QTemporaryDir dir;
        QString home = dir.filePath(QStringLiteral("home"));
        QString dataHome = dir.filePath(QStringLiteral("share dir"));
        QString configHome = dir.filePath(QStringLiteral("config"));

        Sandbox()
        {
            (void)QDir().mkpath(home);
            const QString bin = FakeSystemctlBin(dir.path());
            qputenv("HOME", home.toUtf8());
            qputenv("XDG_DATA_HOME", dataHome.toUtf8());
            qputenv("XDG_CONFIG_HOME", configHome.toUtf8());
            qunsetenv("XDG_DESKTOP_DIR");
            qunsetenv("APPIMAGE");
            qputenv("PATH", (bin + u':' + QString::fromLocal8Bit(qgetenv("PATH"))).toLocal8Bit());
        }

        [[nodiscard]] QString Source(const QString& name, const QByteArray& content = "payload") const
        {
            const QString path = dir.filePath(QStringLiteral("downloads/") + name);
            (void)WriteTestFile(path, content);
            return path;
        }
    };

}

class LinuxInstallerGatewayTest final : public QObject
{
    Q_OBJECT

private slots:
    static void installDirFollowsXdgDataHome();
    static void installDirFallsBackToTheHomeShareFolder();
    static void relativeXdgDataHomeIsIgnored();
    static void clientAppImageIsPlacedExecutable();
    static void cleaningKeepsMaintenanceAndWatcherAndDropsTheOldAppImage();
    static void clientVersionComesFromInstallIni();
    static void clientVersionIsEmptyWithoutTheAppImage();
    static void installIniIsReadableByAnotherQSettingsWithTheSameKey();
    static void maintenanceCopyIsTheRunningAppImage();
    static void maintenanceCopyIsSkippedWhenRunningFromMaintenance();
    static void maintenanceCopyIsSkippedOutsideAnAppImage();
    static void installerInsideInstallDirIsRefusedExceptForMaintenance();
    static void commbusReplacesALinkedPackageWithoutEmptyingItsTarget();
    static void commbusExtractWritesTheVersionMarker();
    static void commbusUnsafeEntryIsRefused();
    static void removingALinkedPackageRemovesOnlyTheLink();
    static void removingADanglingLinkWorks();
    static void clientShortcutPointsAtTheAppImageWithQuotedExec();
    static void setupShortcutPointsAtTheMaintenanceCopy();
    static void desktopShortcutIsExecutableAndInTheDesktopFolder();
    static void shortcutIsNotCreatedForAMissingTarget();
    static void unwantedShortcutIsRemoved();
    static void autoStartWritesTheWatcherForTheSimulatorOfThatExeXml();
    static void autoStartRemoveDropsTheWatcher();
    static void autoStartForAnUnknownExeXmlFails();
    static void autoStartRemoveForAnUnknownExeXmlIsHarmless();
    static void inspectorReportsAutoStartAvailabilityFromSystemctl();
    static void inspectorSearchLocationsListEveryLibrary();
    static void uninstallRemovesEverythingTheInstallLeftAndSparesLinkTargets();
};

void LinuxInstallerGatewayTest::installDirFollowsXdgDataHome()
{
    const Sandbox box;

    QCOMPARE(LinuxInstallerGateway().ClientInstallDir(), box.dataHome + QStringLiteral("/gsx-integrator-client"));
    QCOMPARE(ClientExePath(), box.dataHome + QStringLiteral("/gsx-integrator-client/gsx-integrator-client.AppImage"));
}

void LinuxInstallerGatewayTest::installDirFallsBackToTheHomeShareFolder()
{
    const Sandbox box;
    qunsetenv("XDG_DATA_HOME");

    QCOMPARE(LinuxInstallerGateway().ClientInstallDir(), box.home + QStringLiteral("/.local/share/gsx-integrator-client"));
}

void LinuxInstallerGatewayTest::relativeXdgDataHomeIsIgnored()
{
    const Sandbox box;
    qputenv("XDG_DATA_HOME", "relative/share");

    QCOMPARE(LinuxInstallerGateway().ClientInstallDir(), box.home + QStringLiteral("/.local/share/gsx-integrator-client"));
}

void LinuxInstallerGatewayTest::clientAppImageIsPlacedExecutable()
{
    const Sandbox box;
    LinuxInstallerGateway gateway;

    QVERIFY(gateway.PrepareCleanInstallDir());
    const InstallOutcome outcome = gateway.InstallClientPackage(box.Source(QStringLiteral("gsx-integrator-client-1.2.3-x86_64.AppImage"), "app-bytes"));

    QVERIFY(outcome.ok());
    QVERIFY(gateway.ClientExecutableExists());
    QVERIFY(QFileInfo(ClientExePath()).isExecutable());
    QCOMPARE(ReadTestFile(ClientExePath()), QByteArray("app-bytes"));
    QVERIFY(!QFile::exists(ClientExePath() + QStringLiteral(".part")));
}

void LinuxInstallerGatewayTest::cleaningKeepsMaintenanceAndWatcherAndDropsTheOldAppImage()
{
    const Sandbox box;
    QVERIFY(WriteTestFile(ClientExePath(), "old"));
    QVERIFY(WriteTestFile(ClientExePath() + QStringLiteral(".part"), "half"));
    QVERIFY(WriteTestFile(MaintenanceInstallerPath(), "installer"));
    QVERIFY(WriteTestFile(WatcherScriptPath(), "script"));
    QVERIFY(WriteTestFile(InstallMetadataPath(), "[General]\nDisplayVersion=1.0.0\n"));

    QVERIFY(LinuxInstallerGateway().PrepareCleanInstallDir());

    QVERIFY(!QFile::exists(ClientExePath()));
    QVERIFY(!QFile::exists(ClientExePath() + QStringLiteral(".part")));
    QVERIFY(QFile::exists(MaintenanceInstallerPath()));
    QVERIFY(QFile::exists(WatcherScriptPath()));
}

void LinuxInstallerGatewayTest::clientVersionComesFromInstallIni()
{
    const Sandbox box;
    LinuxInstallerGateway gateway;
    QVERIFY(gateway.PrepareCleanInstallDir());
    QVERIFY(gateway.InstallClientPackage(box.Source(QStringLiteral("c.AppImage"))).ok());

    gateway.WriteUninstallEntry(QStringLiteral("1.43.1"));

    QCOMPARE(InstalledClientVersion(), QStringLiteral("1.43.1"));
    QCOMPARE(LinuxSystemInspector().InstalledClientVersion(), QStringLiteral("1.43.1"));
    QVERIFY(QFile::exists(InstallMetadataPath()));
    QVERIFY(QString::fromUtf8(ReadTestFile(InstallMetadataPath())).contains(QStringLiteral("DisplayVersion=1.43.1")));
}

void LinuxInstallerGatewayTest::clientVersionIsEmptyWithoutTheAppImage()
{
    const Sandbox box;
    LinuxInstallerGateway gateway;
    gateway.WriteUninstallEntry(QStringLiteral("1.43.1"));

    QVERIFY(QFile::exists(InstallMetadataPath()));
    QVERIFY(InstalledClientVersion().isEmpty());
}

void LinuxInstallerGatewayTest::installIniIsReadableByAnotherQSettingsWithTheSameKey()
{
    const Sandbox box;
    LinuxInstallerGateway().WriteUninstallEntry(QStringLiteral("2.0.1"));

    const QSettings reader(InstallMetadataPath(), QSettings::IniFormat);

    QCOMPARE(reader.value(QStringLiteral("DisplayVersion")).toString(), QStringLiteral("2.0.1"));
    QCOMPARE(reader.status(), QSettings::NoError);
}

void LinuxInstallerGatewayTest::maintenanceCopyIsTheRunningAppImage()
{
    const Sandbox box;
    const QString running = box.Source(QStringLiteral("gsx-integrator-installer-0.5.0-x86_64.AppImage"), "installer-bytes");
    qputenv("APPIMAGE", running.toUtf8());

    LinuxInstallerGateway().InstallUninstallerCopy();

    QCOMPARE(ReadTestFile(MaintenanceInstallerPath()), QByteArray("installer-bytes"));
    QVERIFY(QFileInfo(MaintenanceInstallerPath()).isExecutable());
    QCOMPARE(MaintenanceInstallerPath(), ClientInstallDir() + QStringLiteral("/maintenance/gsx-integrator-installer.AppImage"));
}

void LinuxInstallerGatewayTest::maintenanceCopyIsSkippedWhenRunningFromMaintenance()
{
    const Sandbox box;
    QVERIFY(WriteTestFile(MaintenanceInstallerPath(), "self"));
    qputenv("APPIMAGE", MaintenanceInstallerPath().toUtf8());
    const ino_t before = InodeOf(MaintenanceInstallerPath());
    QVERIFY(before != 0);

    LinuxInstallerGateway().InstallUninstallerCopy();

    QCOMPARE(InodeOf(MaintenanceInstallerPath()), before);
    QCOMPARE(ReadTestFile(MaintenanceInstallerPath()), QByteArray("self"));
    QVERIFY(!QFile::exists(MaintenanceInstallerPath() + QStringLiteral(".part")));
}

void LinuxInstallerGatewayTest::maintenanceCopyIsSkippedOutsideAnAppImage()
{
    const Sandbox box;

    LinuxInstallerGateway().InstallUninstallerCopy();

    QVERIFY(!QFile::exists(MaintenanceInstallerPath()));
}

void LinuxInstallerGatewayTest::installerInsideInstallDirIsRefusedExceptForMaintenance()
{
    const Sandbox box;
    const LinuxInstallerGateway gateway;

    qputenv("APPIMAGE", (ClientInstallDir() + QStringLiteral("/i.AppImage")).toUtf8());
    QVERIFY(gateway.IsInstallerInsideInstallDir());

    qputenv("APPIMAGE", (ClientInstallDir() + QStringLiteral("/sub/i.AppImage")).toUtf8());
    QVERIFY(gateway.IsInstallerInsideInstallDir());

    qputenv("APPIMAGE", MaintenanceInstallerPath().toUtf8());
    QVERIFY(!gateway.IsInstallerInsideInstallDir());

    qputenv("APPIMAGE", (box.home + QStringLiteral("/Downloads/i.AppImage")).toUtf8());
    QVERIFY(!gateway.IsInstallerInsideInstallDir());

    qputenv("APPIMAGE", (ClientInstallDir() + QStringLiteral("-other/i.AppImage")).toUtf8());
    QVERIFY(!gateway.IsInstallerInsideInstallDir());
}

namespace
{
    QByteArray CommbusZip()
    {
        return BuildTestZip({{"gsx-integrator-commbus/manifest.json", "{\"package_version\":\"0.1.0\"}"},
                             {"gsx-integrator-commbus/modules/a.wasm", "wasm"}});
    }
}

void LinuxInstallerGatewayTest::commbusReplacesALinkedPackageWithoutEmptyingItsTarget()
{
    const Sandbox box;
    const QString community = box.dir.filePath(QStringLiteral("sim/Community"));
    const QString store = box.dir.filePath(QStringLiteral("store/gsx-integrator-commbus"));
    QVERIFY(WriteTestFile(store + QStringLiteral("/keep.txt"), "keep"));
    QVERIFY(MakeTestSymlink(store, community + QStringLiteral("/gsx-integrator-commbus")));

    const InstallOutcome outcome = LinuxInstallerGateway().ExtractCommbusPackage(
        box.Source(QStringLiteral("commbus.zip"), CommbusZip()), community, QStringLiteral("0.9.0"));

    QVERIFY2(outcome.ok(), qPrintable(outcome.detail));
    QVERIFY(!QFileInfo(community + QStringLiteral("/gsx-integrator-commbus")).isSymLink());
    QVERIFY(QFile::exists(community + QStringLiteral("/gsx-integrator-commbus/modules/a.wasm")));
    QVERIFY(QFile::exists(store + QStringLiteral("/keep.txt")));
    QVERIFY(!QFile::exists(store + QStringLiteral("/manifest.json")));
}

void LinuxInstallerGatewayTest::commbusExtractWritesTheVersionMarker()
{
    const Sandbox box;
    const QString community = box.dir.filePath(QStringLiteral("sim/Community"));
    QVERIFY(QDir().mkpath(community));

    QVERIFY(LinuxInstallerGateway().ExtractCommbusPackage(box.Source(QStringLiteral("c.zip"), CommbusZip()),
                                                          community, QStringLiteral("0.9.0")).ok());

    QCOMPARE(LinuxSystemInspector().InstalledCommbusVersion(community), QStringLiteral("0.9.0"));
}

void LinuxInstallerGatewayTest::commbusUnsafeEntryIsRefused()
{
    const Sandbox box;
    const QString community = box.dir.filePath(QStringLiteral("sim/Community"));
    QVERIFY(QDir().mkpath(community));
    const QByteArray zip = BuildTestZip({{"../escape.txt", "x"}});

    const InstallOutcome outcome = LinuxInstallerGateway().ExtractCommbusPackage(
        box.Source(QStringLiteral("bad.zip"), zip), community, QStringLiteral("0.9.0"));

    QCOMPARE(outcome.status, InstallStatus::ExtractUnsafeEntry);
    QVERIFY(!QFile::exists(box.dir.filePath(QStringLiteral("sim/escape.txt"))));
}

void LinuxInstallerGatewayTest::removingALinkedPackageRemovesOnlyTheLink()
{
    const Sandbox box;
    const QString store = box.dir.filePath(QStringLiteral("store/pkg"));
    const QString link = box.dir.filePath(QStringLiteral("sim/Community/gsx-integrator-commbus"));
    QVERIFY(WriteTestFile(store + QStringLiteral("/keep.txt"), "keep"));
    QVERIFY(MakeTestSymlink(store, link));

    QVERIFY(LinuxInstallerGateway().RemoveDirRecursively(link));

    QVERIFY(!QFileInfo(link).isSymLink());
    QVERIFY(!QFileInfo::exists(link));
    QCOMPARE(ReadTestFile(store + QStringLiteral("/keep.txt")), QByteArray("keep"));
}

void LinuxInstallerGatewayTest::removingADanglingLinkWorks()
{
    const Sandbox box;
    const QString link = box.dir.filePath(QStringLiteral("sim/Community/gsx-integrator-commbus"));
    QVERIFY(MakeTestSymlink(box.dir.filePath(QStringLiteral("gone")), link));

    QVERIFY(LinuxInstallerGateway().RemoveDirRecursively(link));

    QVERIFY(!QFileInfo(link).isSymLink());
}

void LinuxInstallerGatewayTest::clientShortcutPointsAtTheAppImageWithQuotedExec()
{
    const Sandbox box;
    QVERIFY(WriteTestFile(ClientExePath(), "app"));

    LinuxInstallerGateway().ApplyShortcut(ShortcutKind::ClientMenu, true);

    const QString desktop = QString::fromUtf8(ReadTestFile(StartMenuShortcutPath()));
    QCOMPARE(StartMenuShortcutPath(), box.dataHome + QStringLiteral("/applications/gsx-integrator-client.desktop"));
    QVERIFY(desktop.contains(QStringLiteral("\nName=GSX Integrator\n")));
    QVERIFY(desktop.contains(QStringLiteral("\nExec=\"") + ClientExePath() + QStringLiteral("\"\n")));
    QVERIFY(ClientExePath().contains(u' '));
    QVERIFY(LinuxSystemInspector().ClientMenuShortcutExists());
}

void LinuxInstallerGatewayTest::setupShortcutPointsAtTheMaintenanceCopy()
{
    const Sandbox box;
    QVERIFY(WriteTestFile(MaintenanceInstallerPath(), "installer"));

    LinuxInstallerGateway().ApplyShortcut(ShortcutKind::Setup, true);

    const QString desktop = QString::fromUtf8(ReadTestFile(SetupStartMenuShortcutPath()));
    QCOMPARE(SetupStartMenuShortcutPath(), box.dataHome + QStringLiteral("/applications/gsx-integrator-installer.desktop"));
    QVERIFY(desktop.contains(QStringLiteral("\nName=GSX Integrator Installer\n")));
    QVERIFY(desktop.contains(QStringLiteral("\nExec=\"") + MaintenanceInstallerPath() + QStringLiteral("\"\n")));
    QVERIFY(LinuxSystemInspector().SetupMenuShortcutExists());
}

void LinuxInstallerGatewayTest::desktopShortcutIsExecutableAndInTheDesktopFolder()
{
    const Sandbox box;
    QVERIFY(WriteTestFile(ClientExePath(), "app"));

    LinuxInstallerGateway().ApplyShortcut(ShortcutKind::Desktop, true);

    QCOMPARE(DesktopShortcutPath(), box.home + QStringLiteral("/Desktop/GSX Integrator.desktop"));
    QVERIFY(QFileInfo(DesktopShortcutPath()).isExecutable());
    QVERIFY(LinuxSystemInspector().DesktopShortcutExists());
}

void LinuxInstallerGatewayTest::shortcutIsNotCreatedForAMissingTarget()
{
    const Sandbox box;

    LinuxInstallerGateway().ApplyShortcut(ShortcutKind::ClientMenu, true);

    QVERIFY(!QFile::exists(StartMenuShortcutPath()));
}

void LinuxInstallerGatewayTest::unwantedShortcutIsRemoved()
{
    const Sandbox box;
    QVERIFY(WriteTestFile(ClientExePath(), "app"));
    LinuxInstallerGateway gateway;
    gateway.ApplyShortcut(ShortcutKind::ClientMenu, true);
    QVERIFY(QFile::exists(StartMenuShortcutPath()));

    gateway.ApplyShortcut(ShortcutKind::ClientMenu, false);

    QVERIFY(!QFile::exists(StartMenuShortcutPath()));
}

void LinuxInstallerGatewayTest::autoStartWritesTheWatcherForTheSimulatorOfThatExeXml()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());
    const Sandbox box;
    qputenv("HOME", tree.Home().toUtf8());

    QVERIFY(LinuxInstallerGateway().AddAutoStartEntry(tree.ExeXml2024()));

    const QString unit = QString::fromUtf8(ReadTestFile(box.configHome + QStringLiteral("/systemd/user/gsx-integrator-client.service")));
    QVERIFY(unit.contains(QStringLiteral("GSXI_SIMULATORS=FlightSimulator2024.exe\"")));
    QVERIFY(unit.contains(QStringLiteral("ExecStart=\"") + WatcherScriptPath() + QStringLiteral("\"")));
    QVERIFY(QFileInfo(WatcherScriptPath()).isExecutable());
    QCOMPARE(WatcherScriptPath(), ClientInstallDir() + QStringLiteral("/gsx-integrator-watcher.sh"));
    QVERIFY(SystemctlLog(box.dir.path()).contains(QStringLiteral("--user enable --now gsx-integrator-client.service")));
}

void LinuxInstallerGatewayTest::autoStartRemoveDropsTheWatcher()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());
    const Sandbox box;
    qputenv("HOME", tree.Home().toUtf8());
    LinuxInstallerGateway gateway;
    QVERIFY(gateway.AddAutoStartEntry(tree.ExeXml2024()));

    QVERIFY(gateway.RemoveAutoStartEntry(tree.ExeXml2024()));

    QVERIFY(!QFile::exists(box.configHome + QStringLiteral("/systemd/user/gsx-integrator-client.service")));
    QVERIFY(!QFile::exists(WatcherScriptPath()));
    QVERIFY(SystemctlLog(box.dir.path()).contains(QStringLiteral("--user disable --now gsx-integrator-client.service")));
}

void LinuxInstallerGatewayTest::autoStartForAnUnknownExeXmlFails()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());
    const Sandbox box;
    qputenv("HOME", tree.Home().toUtf8());

    QVERIFY(!LinuxInstallerGateway().AddAutoStartEntry(QStringLiteral("/elsewhere/EXE.xml")));
    QVERIFY(!QFile::exists(WatcherScriptPath()));
}

void LinuxInstallerGatewayTest::autoStartRemoveForAnUnknownExeXmlIsHarmless()
{
    const Sandbox box;

    QVERIFY(LinuxInstallerGateway().RemoveAutoStartEntry(QStringLiteral("/elsewhere/EXE.xml")));
    QVERIFY(SystemctlLog(box.dir.path()).isEmpty());
}

void LinuxInstallerGatewayTest::inspectorReportsAutoStartAvailabilityFromSystemctl()
{
    const Sandbox box;
    QVERIFY(LinuxSystemInspector().AutoStartAvailable());

    QVERIFY(WriteTestFile(box.dir.filePath(QStringLiteral("fail-show-environment")), ""));

    QVERIFY(!LinuxSystemInspector().AutoStartAvailable());
}

void LinuxInstallerGatewayTest::inspectorSearchLocationsListEveryLibrary()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());
    const Sandbox box;
    qputenv("HOME", tree.Home().toUtf8());

    const QStringList locations = LinuxSystemInspector().SimSearchLocations();

    QVERIFY(locations.contains(tree.DefaultSteamapps()));
    QVERIFY(locations.contains(tree.LegacySteamapps()));
    QVERIFY(locations.contains(tree.GamesLibrary() + QStringLiteral("/steamapps")));
    QCOMPARE(locations.size(), 3);
}

void LinuxInstallerGatewayTest::uninstallRemovesEverythingTheInstallLeftAndSparesLinkTargets()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());
    const Sandbox box;
    qputenv("HOME", tree.Home().toUtf8());

    LinuxInstallerGateway gateway;
    QVERIFY(gateway.PrepareCleanInstallDir());
    QVERIFY(gateway.InstallClientPackage(box.Source(QStringLiteral("c.AppImage"))).ok());
    gateway.WriteUninstallEntry(QStringLiteral("1.0.0"));
    QVERIFY(WriteTestFile(MaintenanceInstallerPath(), "installer"));
    gateway.ApplyShortcut(ShortcutKind::ClientMenu, true);
    gateway.ApplyShortcut(ShortcutKind::Setup, true);
    gateway.ApplyShortcut(ShortcutKind::Desktop, true);

    QVERIFY(gateway.ExtractCommbusPackage(box.Source(QStringLiteral("c.zip"), CommbusZip()), tree.Community2024(),
                                          QStringLiteral("0.9.0")).ok());
    const QString store = box.dir.filePath(QStringLiteral("aerosoft/gsx-integrator-commbus"));
    QVERIFY(WriteTestFile(store + QStringLiteral("/keep.txt"), "keep"));
    QVERIFY(MakeTestSymlink(store, tree.Community2020() + QStringLiteral("/gsx-integrator-commbus")));
    QVERIFY(gateway.AddAutoStartEntry(tree.ExeXml2024()));

    const UninstallOutcome outcome = InstallOrchestrator(gateway).RunUninstall();

    QVERIFY2(outcome.ok(), qPrintable(outcome.leftovers.join(u',')));
    QVERIFY(!QFileInfo::exists(ClientInstallDir()));
    QVERIFY(!QFile::exists(StartMenuShortcutPath()));
    QVERIFY(!QFile::exists(SetupStartMenuShortcutPath()));
    QVERIFY(!QFile::exists(DesktopShortcutPath()));
    QVERIFY(!QFileInfo::exists(tree.Community2024() + QStringLiteral("/gsx-integrator-commbus")));
    QVERIFY(!QFileInfo(tree.Community2020() + QStringLiteral("/gsx-integrator-commbus")).isSymLink());
    QCOMPARE(ReadTestFile(store + QStringLiteral("/keep.txt")), QByteArray("keep"));
    QVERIFY(!QFile::exists(box.configHome + QStringLiteral("/systemd/user/gsx-integrator-client.service")));
}

QTEST_GUILESS_MAIN(LinuxInstallerGatewayTest)

#include "tst_linux_installer_gateway.moc"
