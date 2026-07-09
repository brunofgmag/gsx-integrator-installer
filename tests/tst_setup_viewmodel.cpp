#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include "doubles/FakeInstallService.h"
#include "doubles/FakeReleaseProvider.h"
#include "doubles/FakeSelfUpdateService.h"
#include "doubles/FakeSettingsRepository.h"
#include "doubles/FakeSystemInspector.h"
#include "../src/domain/model/ReleaseInfo.h"
#include "../src/domain/model/SimInstall.h"
#include "../src/viewmodel/SetupViewModel.h"

namespace
{
    ReleaseInfo ValidRelease(const QString& version)
    {
        ReleaseInfo info;
        info.valid = true;
        info.version = version;
        info.zipName = QStringLiteral("pkg-%1.zip").arg(version);
        return info;
    }

    ReleaseInfo FailedRelease(const ReleaseError kind)
    {
        ReleaseInfo info;
        info.errorKind = kind;
        return info;
    }

    SimInstall MakeSim(const QString& id, const QString& community,
                       const QString& process, const QString& exeXml, int generation)
    {
        return {id, id, community, process, exeXml, generation};
    }
}

class SetupViewModelTest final : public QObject
{
    Q_OBJECT

private slots:
    static void fetchFailureEntersErrorState();
    static void readyBuildsInstallRequestForOutdatedClientAndSim();
    static void autoStart2020OnlyTargetsOnlyMatchingGeneration();
    static void autoStartAllSimsTargetsEveryGeneration();
    static void autoStartDisableRemovesAllCandidateExeXml();
    static void forceReinstallMakesUpToDateClientNeedInstall();
    static void onInstallFinishedReReadsInstalledVersions();
    static void setSimSelectedPersistsAndRestoresAcrossSessions();
    static void setSimSelectedReselectingClearsPersistedSkip();
    static void shortcutTogglesPersistAndRestoreAcrossSessions();
    static void uninstallRunsAndEntersUninstalledState();
    static void uninstallBlockedWhileClientIsRunning();
    static void setLanguagePersistsAndSignals();
    static void installerUpdateAvailableWhenNewerReleaseExists();
    static void invalidInstallerReleaseStaysReadyWithoutError();
    static void selfUpdateStartsOnceWhileUpdating();
    static void selfUpdateRecoversAfterFailure();
    static void installBlockedWhileSelfUpdating();
};

void SetupViewModelTest::fetchFailureEntersErrorState()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;
    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.refresh();

    QCOMPARE(releases.fetchCount, 1);

    releases.Deliver(FailedRelease(ReleaseError::Network), ValidRelease(QStringLiteral("0.2.0")), ReleaseInfo{});

    QCOMPARE(vm.GetState(), QStringLiteral("error"));
    QVERIFY(!vm.GetErrorText().isEmpty());
}

void SetupViewModelTest::readyBuildsInstallRequestForOutdatedClientAndSim()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;

    inspector.clientVersion = QStringLiteral("0.1.0");
    inspector.sims = {
        MakeSim(QStringLiteral("s24"), QStringLiteral("C:/Community24"),
                QStringLiteral("FlightSimulator2024.exe"),
                QStringLiteral("C:/exe24.xml"), 2024)
    };
    inspector.commbusVersions[QStringLiteral("C:/Community24")] = QString();

    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")), ReleaseInfo{});

    QCOMPARE(vm.GetState(), QStringLiteral("ready"));
    QVERIFY(vm.GetClientNeedsInstall());
    QVERIFY(vm.HasAnythingToDo());

    vm.install();

    QCOMPARE(install.installCount, 1);
    QVERIFY(install.lastRequest.installClient);
    QCOMPARE(install.lastRequest.commbusTargets.size(), 1);
    QCOMPARE(install.lastRequest.commbusTargets.first().communityPath, QStringLiteral("C:/Community24"));
}

void SetupViewModelTest::autoStart2020OnlyTargetsOnlyMatchingGeneration()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;

    inspector.clientVersion = QStringLiteral("0.2.0");
    inspector.sims = {
        MakeSim(QStringLiteral("s20"), QStringLiteral("C:/C20"), QStringLiteral("FlightSimulator.exe"),
                QStringLiteral("C:/exe20.xml"), 2020),
        MakeSim(QStringLiteral("s24"), QStringLiteral("C:/C24"), QStringLiteral("FlightSimulator2024.exe"),
                QStringLiteral("C:/exe24.xml"), 2024)
    };
    inspector.commbusVersions[QStringLiteral("C:/C20")] = QStringLiteral("0.2.0");
    inspector.commbusVersions[QStringLiteral("C:/C24")] = QStringLiteral("0.2.0");
    inspector.candidateExeXml[2024] = {QStringLiteral("C:/exe24.xml")};

    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")), ReleaseInfo{});

    vm.SetAutoStartMode(SetupViewModel::kAutoStart2020Only);
    vm.install();

    QCOMPARE(install.lastRequest.exeXmlAddTargets, QStringList{QStringLiteral("C:/exe20.xml")});
    QCOMPARE(install.lastRequest.exeXmlRemoveTargets, QStringList{QStringLiteral("C:/exe24.xml")});
}

void SetupViewModelTest::autoStartAllSimsTargetsEveryGeneration()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;

    inspector.clientVersion = QStringLiteral("0.2.0");
    inspector.sims = {
        MakeSim(QStringLiteral("s20"), QStringLiteral("C:/C20"), QStringLiteral("FlightSimulator.exe"),
                QStringLiteral("C:/exe20.xml"), 2020),
        MakeSim(QStringLiteral("s24"), QStringLiteral("C:/C24"), QStringLiteral("FlightSimulator2024.exe"),
                QStringLiteral("C:/exe24.xml"), 2024)
    };
    inspector.commbusVersions[QStringLiteral("C:/C20")] = QStringLiteral("0.2.0");
    inspector.commbusVersions[QStringLiteral("C:/C24")] = QStringLiteral("0.2.0");

    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")), ReleaseInfo{});

    vm.SetAutoStartMode(SetupViewModel::kAutoStartAllSims);
    vm.install();

    const QStringList expectedAdds{QStringLiteral("C:/exe20.xml"), QStringLiteral("C:/exe24.xml")};

    QCOMPARE(install.lastRequest.exeXmlAddTargets, expectedAdds);
    QVERIFY(install.lastRequest.exeXmlRemoveTargets.isEmpty());
}

void SetupViewModelTest::autoStartDisableRemovesAllCandidateExeXml()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;

    inspector.clientVersion = QStringLiteral("0.2.0");
    inspector.sims = {
        MakeSim(QStringLiteral("s20"), QStringLiteral("C:/C20"), QStringLiteral("FlightSimulator.exe"),
                QStringLiteral("C:/exe20.xml"), 2020),
        MakeSim(QStringLiteral("s24"), QStringLiteral("C:/C24"), QStringLiteral("FlightSimulator2024.exe"),
                QStringLiteral("C:/exe24.xml"), 2024)
    };
    inspector.commbusVersions[QStringLiteral("C:/C20")] = QStringLiteral("0.2.0");
    inspector.commbusVersions[QStringLiteral("C:/C24")] = QStringLiteral("0.2.0");
    inspector.candidateExeXml[0] = {QStringLiteral("C:/exe20.xml"), QStringLiteral("C:/exe24.xml")};

    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")), ReleaseInfo{});

    vm.SetAutoStartMode(SetupViewModel::kAutoStartDisable);
    vm.install();

    QVERIFY(install.lastRequest.exeXmlAddTargets.isEmpty());

    const QStringList expectedRemovals{QStringLiteral("C:/exe20.xml"), QStringLiteral("C:/exe24.xml")};

    QCOMPARE(install.lastRequest.exeXmlRemoveTargets, expectedRemovals);
}

void SetupViewModelTest::forceReinstallMakesUpToDateClientNeedInstall()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;

    inspector.clientVersion = QStringLiteral("0.2.0");

    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);
    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")), ReleaseInfo{});

    QVERIFY(!vm.GetClientNeedsInstall());

    vm.SetForceReinstall(true);

    QVERIFY(vm.GetClientNeedsInstall());
    QVERIFY(vm.HasAnythingToDo());
}

void SetupViewModelTest::onInstallFinishedReReadsInstalledVersions()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;

    inspector.clientVersion = QStringLiteral("0.1.0");
    inspector.sims = {
        MakeSim(QStringLiteral("s24"), QStringLiteral("C:/C24"),
                QStringLiteral("FlightSimulator2024.exe"),
                QStringLiteral("C:/exe24.xml"), 2024)
    };
    inspector.commbusVersions[QStringLiteral("C:/C24")] = QStringLiteral("0.1.0");

    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);
    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")), ReleaseInfo{});

    QVERIFY(vm.GetSims().first().toMap().value(QStringLiteral("needsInstall")).toBool());

    inspector.clientVersion = QStringLiteral("0.2.0");
    inspector.commbusVersions[QStringLiteral("C:/C24")] = QStringLiteral("0.2.0");
    vm.OnInstallFinished(InstallOutcome{});

    QCOMPARE(vm.GetState(), QStringLiteral("done"));
    QCOMPARE(vm.GetClientInstalled(), QStringLiteral("0.2.0"));

    const QVariantMap sim = vm.GetSims().first().toMap();

    QCOMPARE(sim.value(QStringLiteral("installedVersion")).toString(), QStringLiteral("0.2.0"));
    QVERIFY(!sim.value(QStringLiteral("needsInstall")).toBool());
}

void SetupViewModelTest::setSimSelectedPersistsAndRestoresAcrossSessions()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;

    inspector.sims = {
        MakeSim(QStringLiteral("fs2020"), QStringLiteral("C:/C20"),
                QStringLiteral("FlightSimulator.exe"),
                QStringLiteral("C:/exe20.xml"), 2020)
    };

    {
        SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);
        vm.refresh();
        releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")), ReleaseInfo{});

        QVERIFY(vm.GetSims().first().toMap().value(QStringLiteral("selected")).toBool());

        vm.setSimSelected(0, false);
    }

    QCOMPARE(settings.settings.disabledCommbusSims, QStringList{QStringLiteral("fs2020")});

    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")), ReleaseInfo{});

    QVERIFY(!vm.GetSims().first().toMap().value(QStringLiteral("selected")).toBool());

    vm.install();

    QCOMPARE(install.installCount, 1);
    QVERIFY(install.lastRequest.commbusTargets.isEmpty());
}

void SetupViewModelTest::setSimSelectedReselectingClearsPersistedSkip()
{
    FakeSettingsRepository settings;
    settings.settings.disabledCommbusSims = {QStringLiteral("fs2020")};
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;

    inspector.sims = {
        MakeSim(QStringLiteral("fs2020"), QStringLiteral("C:/C20"),
                QStringLiteral("FlightSimulator.exe"),
                QStringLiteral("C:/exe20.xml"), 2020)
    };

    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")), ReleaseInfo{});

    QVERIFY(!vm.GetSims().first().toMap().value(QStringLiteral("selected")).toBool());

    vm.setSimSelected(0, true);

    QVERIFY(settings.settings.disabledCommbusSims.isEmpty());
    QVERIFY(vm.GetSims().first().toMap().value(QStringLiteral("selected")).toBool());
}

void SetupViewModelTest::shortcutTogglesPersistAndRestoreAcrossSessions()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;

    {
        SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);
        vm.refresh();
        vm.SetDesktopShortcut(false);
        vm.SetSetupShortcut(false);
    }

    QVERIFY(!settings.settings.desktopShortcut);
    QVERIFY(settings.settings.clientStartMenuShortcut);
    QVERIFY(!settings.settings.setupShortcut);

    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.refresh();

    QVERIFY(!vm.GetDesktopShortcut());
    QVERIFY(vm.GetClientStartMenuShortcut());
    QVERIFY(!vm.GetSetupShortcut());
}

void SetupViewModelTest::uninstallRunsAndEntersUninstalledState()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;
    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.uninstall();

    QCOMPARE(install.uninstallCount, 1);
    QCOMPARE(vm.GetState(), QStringLiteral("uninstalled"));
}

void SetupViewModelTest::uninstallBlockedWhileClientIsRunning()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;

    inspector.clientRunning = true;
    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.uninstall();

    QCOMPARE(install.uninstallCount, 0);
    QVERIFY(!vm.GetErrorText().isEmpty());
    QVERIFY(vm.GetState() != QStringLiteral("uninstalled"));
}

void SetupViewModelTest::setLanguagePersistsAndSignals()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;
    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    const QSignalSpy spy(&vm, &SetupViewModel::LanguageChanged);
    vm.SetLanguage(QStringLiteral("pt_BR"));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(settings.settings.language, QStringLiteral("pt_BR"));
    QCOMPARE(vm.GetLanguage(), QStringLiteral("pt_BR"));
}

void SetupViewModelTest::installerUpdateAvailableWhenNewerReleaseExists()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;
    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")),
                     ValidRelease(QStringLiteral("1.0.0")));

    QVERIFY(vm.GetInstallerUpdateAvailable());
    QCOMPARE(vm.GetState(), QStringLiteral("ready"));
}

void SetupViewModelTest::invalidInstallerReleaseStaysReadyWithoutError()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;
    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")),
                     FailedRelease(ReleaseError::Network));

    QVERIFY(!vm.GetInstallerUpdateAvailable());
    QCOMPARE(vm.GetState(), QStringLiteral("ready"));
    QVERIFY(vm.GetErrorText().isEmpty());
}

void SetupViewModelTest::selfUpdateStartsOnceWhileUpdating()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;
    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")),
                     ValidRelease(QStringLiteral("1.0.0")));

    vm.selfUpdate();
    QCOMPARE(selfUpdate.startCount, 1);
    QCOMPARE(selfUpdate.lastRelease.version, QStringLiteral("1.0.0"));
    QCOMPARE(vm.GetInstallerUpdateState(), QStringLiteral("updating"));

    vm.selfUpdate();
    QCOMPARE(selfUpdate.startCount, 1);
}

void SetupViewModelTest::selfUpdateRecoversAfterFailure()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;
    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);

    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")),
                     ValidRelease(QStringLiteral("1.0.0")));

    vm.selfUpdate();
    QCOMPARE(selfUpdate.startCount, 1);

    vm.OnSelfUpdateFailed(SelfUpdateError::SwapFailed, QString());
    QCOMPARE(vm.GetInstallerUpdateState(), QStringLiteral("error"));
    QVERIFY(!vm.GetInstallerUpdateError().isEmpty());

    vm.selfUpdate();
    QCOMPARE(selfUpdate.startCount, 2);
}

void SetupViewModelTest::installBlockedWhileSelfUpdating()
{
    FakeSettingsRepository settings;
    FakeReleaseProvider releases;
    FakeSystemInspector inspector;
    FakeInstallService install;
    FakeSelfUpdateService selfUpdate;

    inspector.clientVersion = QStringLiteral("0.1.0");

    SetupViewModel vm(&settings, &releases, &inspector, &install, &selfUpdate);
    vm.refresh();
    releases.Deliver(ValidRelease(QStringLiteral("0.2.0")), ValidRelease(QStringLiteral("0.2.0")),
                     ValidRelease(QStringLiteral("1.0.0")));

    vm.selfUpdate();
    QCOMPARE(vm.GetInstallerUpdateState(), QStringLiteral("updating"));

    vm.install();
    QCOMPARE(install.installCount, 0);
}

QTEST_GUILESS_MAIN(SetupViewModelTest)

#include "tst_setup_viewmodel.moc"
