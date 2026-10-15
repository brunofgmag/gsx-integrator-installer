#include <QtTest/QTest>

#include <QtCore/QTemporaryDir>

#include "../src/infrastructure/system/ProcessScan.h"
#include "support/FileTree.h"

namespace
{
    bool AddProcess(const QTemporaryDir& proc, const QString& pid, const QList<QByteArray>& arguments)
    {
        QByteArray cmdline;
        for (const QByteArray& argument : arguments)
        {
            cmdline += argument;
            cmdline.append('\0');
        }

        return WriteTestFile(proc.filePath(pid + QStringLiteral("/cmdline")), cmdline);
    }
}

class ProcessScanTest final : public QObject
{
    Q_OBJECT

private slots:
    static void baseNameSplitsOnBothSeparatorsAndStripsQuotes();
    static void findsWineProcessByWindowsPathInArgv();
    static void findsProtonLauncherByUnixPathInArgv();
    static void matchesAnyArgumentNotJustTheFirst();
    static void ignoresCaseOfTheExecutableName();
    static void doesNotMatchOtherExecutablesOrSubstrings();
    static void ignoresNonPidEntriesAndEmptyCommandLines();
    static void findsTheAppImageClientByEitherName();
    static void findsTheClientBehindAppRunByItsExecutableLink();
    static void executableLinkOfADeletedFileStillMatches();
    static void unrelatedExecutableLinkDoesNotMatch();
    static void emptyNameListMatchesNothing();
    static void missingProcRootMatchesNothing();
};

void ProcessScanTest::baseNameSplitsOnBothSeparatorsAndStripsQuotes()
{
    QCOMPARE(ArgumentBaseName(QStringLiteral("S:\\Games\\MSFS\\FlightSimulator2024.exe")),
             QStringLiteral("FlightSimulator2024.exe"));
    QCOMPARE(ArgumentBaseName(QStringLiteral("/home/u/x/FlightSimulator.exe")), QStringLiteral("FlightSimulator.exe"));
    QCOMPARE(ArgumentBaseName(QStringLiteral("\"C:\\a b\\sim.exe\"")), QStringLiteral("sim.exe"));
    QCOMPARE(ArgumentBaseName(QStringLiteral("plain")), QStringLiteral("plain"));
}

void ProcessScanTest::findsWineProcessByWindowsPathInArgv()
{
    const QTemporaryDir proc;
    QVERIFY(AddProcess(proc, QStringLiteral("4242"),
                       {"S:\\SteamLibrary\\steamapps\\common\\MSFS2024\\FlightSimulator2024.exe"}));

    QVERIFY(IsAnyProcessRunning(proc.path(), {QStringLiteral("FlightSimulator2024.exe")}));
    QVERIFY(!IsAnyProcessRunning(proc.path(), {QStringLiteral("FlightSimulator.exe")}));
}

void ProcessScanTest::findsProtonLauncherByUnixPathInArgv()
{
    const QTemporaryDir proc;
    QVERIFY(AddProcess(proc, QStringLiteral("77"),
                       {"/home/u/.steam/proton/proton", "waitforexitandrun",
                        "/mnt/games/MSFS2020/FlightSimulator.exe"}));

    QVERIFY(IsAnyProcessRunning(proc.path(), {QStringLiteral("FlightSimulator.exe")}));
}

void ProcessScanTest::matchesAnyArgumentNotJustTheFirst()
{
    const QTemporaryDir proc;
    QVERIFY(AddProcess(proc, QStringLiteral("9"), {"wine64-preloader", "--flag", "Z:\\x\\y\\FlightSimulator2024.exe"}));

    QVERIFY(IsAnyProcessRunning(proc.path(), {QStringLiteral("FlightSimulator2024.exe")}));
}

void ProcessScanTest::ignoresCaseOfTheExecutableName()
{
    const QTemporaryDir proc;
    QVERIFY(AddProcess(proc, QStringLiteral("5"), {"S:\\g\\flightsimulator2024.EXE"}));

    QVERIFY(IsAnyProcessRunning(proc.path(), {QStringLiteral("FlightSimulator2024.exe")}));
}

void ProcessScanTest::doesNotMatchOtherExecutablesOrSubstrings()
{
    const QTemporaryDir proc;
    QVERIFY(AddProcess(proc, QStringLiteral("1"), {"S:\\g\\FlightSimulator2024Launcher.exe"}));
    QVERIFY(AddProcess(proc, QStringLiteral("2"), {"/usr/bin/tail", "-f", "/tmp/notFlightSimulator2024.exe.log"}));
    QVERIFY(AddProcess(proc, QStringLiteral("3"), {"bash", "gsx-integrator-watcher.sh"}));

    QVERIFY(!IsAnyProcessRunning(proc.path(), {QStringLiteral("FlightSimulator2024.exe")}));
}

void ProcessScanTest::ignoresNonPidEntriesAndEmptyCommandLines()
{
    const QTemporaryDir proc;
    QVERIFY(AddProcess(proc, QStringLiteral("self"), {"FlightSimulator2024.exe"}));
    QVERIFY(AddProcess(proc, QStringLiteral("12x"), {"FlightSimulator2024.exe"}));
    QVERIFY(WriteTestFile(proc.filePath(QStringLiteral("30/cmdline")), ""));

    QVERIFY(!IsAnyProcessRunning(proc.path(), {QStringLiteral("FlightSimulator2024.exe")}));
}

void ProcessScanTest::findsTheAppImageClientByEitherName()
{
    const QTemporaryDir proc;
    QVERIFY(AddProcess(proc, QStringLiteral("11"), {"/tmp/.mount_gsx-itAbCd/usr/bin/gsx-integrator-client", "--tray"}));
    QVERIFY(AddProcess(proc, QStringLiteral("12"), {"/home/u/.local/share/gsx-integrator-client/gsx-integrator-client.AppImage"}));

    QVERIFY(IsAnyProcessRunning(proc.path(), {QStringLiteral("gsx-integrator-client")}));
    QVERIFY(IsAnyProcessRunning(proc.path(), {QStringLiteral("gsx-integrator-client.AppImage")}));
    QVERIFY(!IsAnyProcessRunning(proc.path(), {QStringLiteral("gsx-integrator-installer")}));
}

void ProcessScanTest::findsTheClientBehindAppRunByItsExecutableLink()
{
    const QTemporaryDir proc;
    QVERIFY(AddProcess(proc, QStringLiteral("21"), {"/tmp/.mount_gsx-iQwErT/AppRun.wrapped"}));
    QVERIFY(MakeTestSymlink(QStringLiteral("/tmp/.mount_gsx-iQwErT/usr/bin/gsx-integrator-client"),
                            proc.filePath(QStringLiteral("21/exe"))));

    QVERIFY(IsAnyProcessRunning(proc.path(), {QStringLiteral("gsx-integrator-client")}));
}

void ProcessScanTest::executableLinkOfADeletedFileStillMatches()
{
    const QTemporaryDir proc;
    QVERIFY(AddProcess(proc, QStringLiteral("22"), {"x"}));
    QVERIFY(MakeTestSymlink(QStringLiteral("/home/u/.local/share/gsx-integrator-client/gsx-integrator-client.AppImage (deleted)"),
                            proc.filePath(QStringLiteral("22/exe"))));

    QVERIFY(IsAnyProcessRunning(proc.path(), {QStringLiteral("gsx-integrator-client.AppImage")}));
}

void ProcessScanTest::unrelatedExecutableLinkDoesNotMatch()
{
    const QTemporaryDir proc;
    QVERIFY(AddProcess(proc, QStringLiteral("23"), {"/usr/bin/firefox"}));
    QVERIFY(MakeTestSymlink(QStringLiteral("/usr/lib/firefox/firefox"), proc.filePath(QStringLiteral("23/exe"))));

    QVERIFY(!IsAnyProcessRunning(proc.path(), {QStringLiteral("gsx-integrator-client")}));
}

void ProcessScanTest::emptyNameListMatchesNothing()
{
    const QTemporaryDir proc;
    QVERIFY(AddProcess(proc, QStringLiteral("1"), {"anything"}));

    QVERIFY(!IsAnyProcessRunning(proc.path(), {}));
}

void ProcessScanTest::missingProcRootMatchesNothing()
{
    QVERIFY(!IsAnyProcessRunning(QStringLiteral("/nonexistent/proc"), {QStringLiteral("x")}));
}

QTEST_APPLESS_MAIN(ProcessScanTest)

#include "tst_process_scan.moc"
