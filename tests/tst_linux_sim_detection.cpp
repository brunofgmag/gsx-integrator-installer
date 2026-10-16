#include <QtTest/QTest>

#include "../src/infrastructure/system/LinuxSimDetection.h"
#include "../src/infrastructure/system/SimDetection.h"
#include "support/FakeSteamTree.h"

class LinuxSimDetectionTest final : public QObject
{
    Q_OBJECT

private slots:
    static void detectsBothSimsThroughTheirPrefixes();
    static void communityFolderIsFoundWhateverItsCase();
    static void exeXmlKeepsTheLowercaseNameThePrefixUses();
    static void exeXmlNamedEvenWhenTheFileDoesNotExistYet();
    static void simWithoutUserCfgIsSkipped();
    static void simWhoseCommunityIsGoneIsSkipped();
    static void noSteamMeansNoSims();
    static void candidatesFilterByGeneration();
    static void exeXmlPathMapsBackToTheSimulatorProcess();
    static void unknownExeXmlPathMapsToNothing();
};

void LinuxSimDetectionTest::detectsBothSimsThroughTheirPrefixes()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    const QList<SimInstall> sims = DetectSims(tree.SteamappsRoots());

    QCOMPARE(sims.size(), 2);
    QCOMPARE(sims[0].id, QStringLiteral("msfs2020-steam"));
    QCOMPARE(sims[0].generation, 2020);
    QCOMPARE(sims[0].processName, QStringLiteral("FlightSimulator.exe"));
    QCOMPARE(sims[0].communityPath, tree.Community2020());
    QCOMPARE(sims[1].id, QStringLiteral("msfs2024-steam"));
    QCOMPARE(sims[1].generation, 2024);
    QCOMPARE(sims[1].processName, QStringLiteral("FlightSimulator2024.exe"));
}

void LinuxSimDetectionTest::communityFolderIsFoundWhateverItsCase()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    const QList<SimInstall> sims = DetectSims(tree.SteamappsRoots());

    QCOMPARE(sims[1].communityPath, tree.Community2024());
    QVERIFY(sims[1].communityPath.endsWith(QStringLiteral("/community")));
}

void LinuxSimDetectionTest::exeXmlKeepsTheLowercaseNameThePrefixUses()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    const QList<SimInstall> sims = DetectSims(tree.SteamappsRoots());

    QCOMPARE(sims[1].autoStartConfigPath, tree.ExeXml2024());
    QVERIFY(sims[1].autoStartConfigPath.endsWith(QStringLiteral("/exe.xml")));
}

void LinuxSimDetectionTest::exeXmlNamedEvenWhenTheFileDoesNotExistYet()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    const QList<SimInstall> sims = DetectSims(tree.SteamappsRoots());

    QCOMPARE(sims[0].autoStartConfigPath, tree.ExeXml2020());
    QVERIFY(!QFile::exists(sims[0].autoStartConfigPath));
}

void LinuxSimDetectionTest::simWithoutUserCfgIsSkipped()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());
    QVERIFY(QFile::remove(tree.UserCfgDir2020() + QStringLiteral("/UserCfg.opt")));

    const QList<SimInstall> sims = DetectSims(tree.SteamappsRoots());

    QCOMPARE(sims.size(), 1);
    QCOMPARE(sims[0].id, QStringLiteral("msfs2024-steam"));
}

void LinuxSimDetectionTest::simWhoseCommunityIsGoneIsSkipped()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());
    QVERIFY(QDir(tree.Community2024()).removeRecursively());

    const QList<SimInstall> sims = DetectSims(tree.SteamappsRoots());

    QCOMPARE(sims.size(), 1);
    QCOMPARE(sims[0].id, QStringLiteral("msfs2020-steam"));
}

void LinuxSimDetectionTest::noSteamMeansNoSims()
{
    const QTemporaryDir empty;
    QVERIFY(empty.isValid());

    QVERIFY(DetectSims({empty.filePath(QStringLiteral("steamapps"))}).isEmpty());
    QVERIFY(CandidateExeXmlPaths(0, {empty.filePath(QStringLiteral("steamapps"))}).isEmpty());
}

void LinuxSimDetectionTest::candidatesFilterByGeneration()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    QCOMPARE(CandidateExeXmlPaths(2024, tree.SteamappsRoots()), QStringList{tree.ExeXml2024()});
    QCOMPARE(CandidateExeXmlPaths(2020, tree.SteamappsRoots()), QStringList{tree.ExeXml2020()});
    QCOMPARE(CandidateExeXmlPaths(0, tree.SteamappsRoots()), (QStringList{tree.ExeXml2020(), tree.ExeXml2024()}));
}

void LinuxSimDetectionTest::exeXmlPathMapsBackToTheSimulatorProcess()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    QCOMPARE(SimProcessNameForExeXml(tree.ExeXml2024(), tree.SteamappsRoots()),
             QStringLiteral("FlightSimulator2024.exe"));
    QCOMPARE(SimProcessNameForExeXml(tree.ExeXml2020(), tree.SteamappsRoots()),
             QStringLiteral("FlightSimulator.exe"));
}

void LinuxSimDetectionTest::unknownExeXmlPathMapsToNothing()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    QVERIFY(SimProcessNameForExeXml(QStringLiteral("/elsewhere/EXE.xml"), tree.SteamappsRoots()).isEmpty());
}

QTEST_APPLESS_MAIN(LinuxSimDetectionTest)

#include "tst_linux_sim_detection.moc"
