#include <QtTest/QTest>

#include <QtCore/QDir>
#include <QtCore/QTemporaryDir>

#include "../src/infrastructure/system/ProtonPrefix.h"
#include "support/FakeSteamTree.h"

class ProtonPrefixTest final : public QObject
{
    Q_OBJECT

private slots:
    static void defaultRootsAreTheTwoSteamLocations();
    static void parsesLibraryPathsAndIgnoresOtherKeys();
    static void parsesEscapedBackslashesAndQuotes();
    static void libraryDirsIncludeListedLibrariesOnce();
    static void findsThePrefixInTheLibraryThatHoldsIt();
    static void noPrefixWhenTheAppIsNotInstalled();
    static void resolvesEveryComponentIgnoringCase();
    static void exactNameWinsOverCaseFolding();
    static void missingComponentResolvesToNothing();
    static void translatesDriveLettersThroughDosdevices();
    static void translatesTheCDriveWithoutADosdevicesEntry();
    static void unknownDriveResolvesToNothing();
    static void knownFoldersOfTheProtonUser();
};

void ProtonPrefixTest::defaultRootsAreTheTwoSteamLocations()
{
    const QStringList roots = DefaultSteamappsDirs(QStringLiteral("/home/u"));

    QCOMPARE(roots, (QStringList{QStringLiteral("/home/u/.local/share/Steam/steamapps"),
                                 QStringLiteral("/home/u/.steam/steam/steamapps")}));
}

void ProtonPrefixTest::parsesLibraryPathsAndIgnoresOtherKeys()
{
    const QByteArray vdf =
        "\"libraryfolders\"\n{\n\t\"0\"\n\t{\n\t\t\"path\"\t\t\"/home/u/.local/share/Steam\"\n"
        "\t\t\"label\"\t\t\"\"\n\t\t\"apps\"\n\t\t{\n\t\t\t\"1250410\"\t\t\"123\"\n\t\t}\n\t}\n"
        "\t\"1\"\n\t{\n\t\t\"path\"\t\t\"/mnt/games/SteamLibrary\"\n\t}\n}\n";

    QCOMPARE(ParseLibraryFolders(vdf), (QStringList{QStringLiteral("/home/u/.local/share/Steam"),
                                                    QStringLiteral("/mnt/games/SteamLibrary")}));
}

void ProtonPrefixTest::parsesEscapedBackslashesAndQuotes()
{
    const QByteArray vdf = "\"0\" { \"path\" \"D:\\\\Games\\\\Steam\" }\n\"1\" { \"PATH\" \"/mnt/a \\\"b\\\"\" }\n";

    QCOMPARE(ParseLibraryFolders(vdf), (QStringList{QStringLiteral("D:\\Games\\Steam"),
                                                    QStringLiteral("/mnt/a \"b\"")}));
}

void ProtonPrefixTest::libraryDirsIncludeListedLibrariesOnce()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    const QStringList dirs = SteamLibrarySteamappsDirs(tree.SteamappsRoots());

    QCOMPARE(dirs.size(), 2);
    QVERIFY(dirs.contains(tree.DefaultSteamapps()));
    QVERIFY(dirs.contains(tree.GamesLibrary() + QStringLiteral("/steamapps")));
}

void ProtonPrefixTest::findsThePrefixInTheLibraryThatHoldsIt()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    QCOMPARE(FindProtonPrefix(tree.SteamappsRoots(), kMsfs2024AppId), tree.Prefix2024());
    QCOMPARE(FindProtonPrefix(tree.SteamappsRoots(), kMsfs2020AppId), tree.Prefix2020());
}

void ProtonPrefixTest::noPrefixWhenTheAppIsNotInstalled()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    QVERIFY(FindProtonPrefix(tree.SteamappsRoots(), 999).isEmpty());
    QVERIFY(FindProtonPrefix({tree.Home() + QStringLiteral("/nowhere/steamapps")}, kMsfs2024AppId).isEmpty());
}

void ProtonPrefixTest::resolvesEveryComponentIgnoringCase()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    const QString resolved = ResolveIgnoringCase(
        tree.Prefix2024(), QStringLiteral("DRIVE_C/Users/STEAMUSER/appdata/roaming/microsoft flight simulator 2024/EXE.XML"));

    QCOMPARE(resolved, tree.ExeXml2024());
}

void ProtonPrefixTest::exactNameWinsOverCaseFolding()
{
    const QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(WriteTestFile(dir.filePath(QStringLiteral("EXE.xml")), "upper"));
    QVERIFY(WriteTestFile(dir.filePath(QStringLiteral("exe.xml")), "lower"));

    QCOMPARE(ResolveIgnoringCase(dir.path(), QStringLiteral("exe.xml")), dir.filePath(QStringLiteral("exe.xml")));
    QCOMPARE(ResolveIgnoringCase(dir.path(), QStringLiteral("EXE.xml")), dir.filePath(QStringLiteral("EXE.xml")));
}

void ProtonPrefixTest::missingComponentResolvesToNothing()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    QVERIFY(ResolveIgnoringCase(tree.Prefix2024(), QStringLiteral("drive_c/users/nobody")).isEmpty());
    QVERIFY(ResolveIgnoringCase(tree.Home() + QStringLiteral("/missing"), QStringLiteral("x")).isEmpty());
}

void ProtonPrefixTest::translatesDriveLettersThroughDosdevices()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    QCOMPARE(WindowsPathToHost(tree.Prefix2024(), QStringLiteral("S:\\MSFS2024\\Community")), tree.Community2024());
    QCOMPARE(WindowsPathToHost(tree.Prefix2024(), QStringLiteral("s:\\msfs2024\\COMMUNITY")), tree.Community2024());
    QCOMPARE(WindowsPathToHost(tree.Prefix2020(), QStringLiteral("S:\\common\\MSFS 2020\\Packages\\Community")),
             tree.Community2020());
    QCOMPARE(WindowsPathToHost(tree.Prefix2024(), QStringLiteral("S:\\")), QFileInfo(tree.GamesLibrary()).canonicalFilePath());
}

void ProtonPrefixTest::translatesTheCDriveWithoutADosdevicesEntry()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());
    QVERIFY(QFile::remove(tree.Prefix2024() + QStringLiteral("/dosdevices/c:")));

    QCOMPARE(WindowsPathToHost(tree.Prefix2024(), QStringLiteral("C:\\Users\\steamuser\\AppData\\Roaming")),
             tree.Prefix2024() + QStringLiteral("/drive_c/users/steamuser/AppData/Roaming"));
}

void ProtonPrefixTest::unknownDriveResolvesToNothing()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    QVERIFY(WindowsPathToHost(tree.Prefix2024(), QStringLiteral("Q:\\anything")).isEmpty());
    QVERIFY(WindowsPathToHost(tree.Prefix2024(), QStringLiteral("not-a-windows-path")).isEmpty());
}

void ProtonPrefixTest::knownFoldersOfTheProtonUser()
{
    const FakeSteamTree tree;
    QVERIFY(tree.IsValid());

    const QString profile = tree.Prefix2024() + QStringLiteral("/drive_c/users/steamuser");

    QCOMPARE(ProtonKnownFolderPath(tree.Prefix2024(), ProtonKnownFolder::Profile), profile);
    QCOMPARE(ProtonKnownFolderPath(tree.Prefix2024(), ProtonKnownFolder::AppDataRoaming),
             profile + QStringLiteral("/AppData/Roaming"));
    QVERIFY(ProtonKnownFolderPath(tree.Prefix2024(), ProtonKnownFolder::AppDataLocal).isEmpty());
}

QTEST_APPLESS_MAIN(ProtonPrefixTest)

#include "tst_proton_prefix.moc"
