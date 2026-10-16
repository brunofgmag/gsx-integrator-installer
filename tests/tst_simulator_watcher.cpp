#include <QtTest/QTest>

#include <functional>

#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QTemporaryDir>

#include "../src/infrastructure/install/SimulatorWatcher.h"
#include "support/FileTree.h"

namespace
{
    QString FakeSystemctl(const QString& dir)
    {
        const QString path = dir + QStringLiteral("/bin/systemctl");
        const QString script = QStringLiteral(
            "#!/bin/sh\n"
            "echo \"$@\" >> \"%1/systemctl.log\"\n"
            "if [ -e \"%1/fail-$2\" ]; then exit 1; fi\n"
            "exit 0\n").arg(dir);
        if (!WriteTestFile(path, script.toUtf8()) || !QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner))
        {
            return {};
        }

        return path;
    }

    QStringList SystemctlLog(const QString& dir)
    {
        const QString text = QString::fromUtf8(ReadTestFile(dir + QStringLiteral("/systemctl.log")));
        return text.split(u'\n', Qt::SkipEmptyParts);
    }

    QByteArray Cmdline(const QList<QByteArray>& arguments)
    {
        QByteArray cmdline;
        for (const QByteArray& argument : arguments)
        {
            cmdline += argument;
            cmdline.append('\0');
        }

        return cmdline;
    }

    bool WaitFor(const std::function<bool()>& condition, const int timeoutMs = 8000)
    {
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < timeoutMs)
        {
            if (condition())
            {
                return true;
            }
            QTest::qWait(25);
        }

        return condition();
    }

    int CountLines(const QString& path, const QString& needle)
    {
        int count = 0;
        for (const QString& line : QString::fromUtf8(ReadTestFile(path)).split(u'\n', Qt::SkipEmptyParts))
        {
            if (line.startsWith(needle))
            {
                ++count;
            }
        }

        return count;
    }

    struct Sandbox
    {
        QTemporaryDir dir;
        QString script = dir.filePath(QStringLiteral("data/gsx-integrator-client/gsx-integrator-watcher.sh"));
        QString unitDir = dir.filePath(QStringLiteral("config/systemd/user"));
        QString unit = unitDir + QStringLiteral("/gsx-integrator-client.service");
        QString systemctl = FakeSystemctl(dir.path());

        [[nodiscard]] SimulatorWatcher Watcher() const { return SimulatorWatcher(script, unitDir, systemctl); }
    };
}

class SimulatorWatcherTest final : public QObject
{
    Q_OBJECT

private slots:
    static void unitTextIsExactlyWhatTheClientMustAlsoWrite();
    static void unitEscapesPercentAndQuotesInTheScriptPath();
    static void scriptTextNamesTheClientAndTheTrayFlag();
    static void unitKeepsSimulatorNamesOutOfTheScriptArguments();
    static void parsesTheWatchedProcessesBackFromTheUnit();
    static void enableWritesExecutableScriptAndUnitThenEnablesNow();
    static void enableTwiceWithTheSameSetRestartsNothing();
    static void addingASecondSimulatorRestartsTheService();
    static void removingOneSimulatorKeepsTheOther();
    static void removingTheLastSimulatorDisablesAndDeletesBothFiles();
    static void removingAnUnwatchedSimulatorTouchesNothing();
    static void addWritesNothingWhenSystemdIsUnavailable();
    static void disableFailureKeepsTheFilesForARetry();
    static void disableRemovesTheFilesEvenWhenSystemctlIsGone();
    static void addRejectsNamesThatCouldBreakTheUnit();
    static void scriptStartsTheClientOncePerSimulatorRunAndStopsItWithTheSimulator();
    static void scriptDoesNotStartASecondClient();
    static void scriptRecognisesTheClientBehindAppRunByItsExecutableLink();
    static void scriptStartsTheClientWhenAnotherProcessOnlyMentionsItsName();
    static void scriptDoesNotCountItsOwnProcessAsTheSimulator();
    static void scriptRefusesToRunWithoutSimulators();
};

void SimulatorWatcherTest::unitTextIsExactlyWhatTheClientMustAlsoWrite()
{
    const WatcherFiles files = BuildWatcherFiles(
        QStringLiteral("/home/u/.local/share/gsx-integrator-client/gsx-integrator-watcher.sh"),
        {QStringLiteral("FlightSimulator.exe"), QStringLiteral("FlightSimulator2024.exe")});

    QCOMPARE(files.unit,
             QStringLiteral("[Unit]\n"
                            "Description=Start GSX Integrator with the simulator\n"
                            "\n"
                            "[Service]\n"
                            "Type=simple\n"
                            "Environment=\"GSXI_SIMULATORS=FlightSimulator.exe FlightSimulator2024.exe\"\n"
                            "ExecStart=\"/home/u/.local/share/gsx-integrator-client/gsx-integrator-watcher.sh\"\n"
                            "Restart=on-failure\n"
                            "RestartSec=5\n"
                            "\n"
                            "[Install]\n"
                            "WantedBy=default.target\n"));
}

void SimulatorWatcherTest::unitEscapesPercentAndQuotesInTheScriptPath()
{
    const WatcherFiles files = BuildWatcherFiles(QStringLiteral("/home/100%/we\"ird/$x/w.sh"),
                                                 {QStringLiteral("FlightSimulator.exe")});

    QVERIFY(files.unit.contains(QStringLiteral("ExecStart=\"/home/100%%/we\\\"ird/$$x/w.sh\"\n")));
}

void SimulatorWatcherTest::scriptTextNamesTheClientAndTheTrayFlag()
{
    const WatcherFiles files = BuildWatcherFiles(QStringLiteral("/x/w.sh"), {QStringLiteral("FlightSimulator.exe")});

    QVERIFY(files.script.startsWith(QStringLiteral("#!/usr/bin/env bash\n")));
    QVERIFY(files.script.contains(QStringLiteral("client=\"${GSXI_CLIENT:-$here/gsx-integrator-client.AppImage}\"")));
    QVERIFY(files.script.contains(QStringLiteral("client_names=(gsx-integrator-client gsx-integrator-client.AppImage)")));
    QVERIFY(files.script.contains(QStringLiteral("setsid \"$client\" --tray")));
    QVERIFY(files.script.contains(QStringLiteral("read -r -a simulators <<<\"${GSXI_SIMULATORS:-}\"")));
}

void SimulatorWatcherTest::unitKeepsSimulatorNamesOutOfTheScriptArguments()
{
    const WatcherFiles files = BuildWatcherFiles(QStringLiteral("/x/w.sh"), {QStringLiteral("FlightSimulator2024.exe")});

    QStringList execLines;
    for (const QString& line : files.unit.split(u'\n'))
    {
        if (line.startsWith(QStringLiteral("ExecStart=")))
        {
            execLines.append(line);
        }
    }

    QCOMPARE(execLines.size(), 1);
    QVERIFY(!execLines[0].contains(QStringLiteral("FlightSimulator")));
}

void SimulatorWatcherTest::parsesTheWatchedProcessesBackFromTheUnit()
{
    const WatcherFiles files = BuildWatcherFiles(QStringLiteral("/x/w.sh"),
                                                 {QStringLiteral("FlightSimulator.exe"), QStringLiteral("FlightSimulator2024.exe")});

    QCOMPARE(ParseWatchedProcesses(files.unit),
             (QStringList{QStringLiteral("FlightSimulator.exe"), QStringLiteral("FlightSimulator2024.exe")}));
    QVERIFY(ParseWatchedProcesses(QStringLiteral("[Unit]\n")).isEmpty());
}

void SimulatorWatcherTest::enableWritesExecutableScriptAndUnitThenEnablesNow()
{
    const Sandbox box;
    QVERIFY(!box.systemctl.isEmpty());

    QVERIFY(box.Watcher().Add(QStringLiteral("FlightSimulator2024.exe")));

    QVERIFY(QFileInfo(box.script).isExecutable());
    QVERIFY(QFile::exists(box.unit));
    QVERIFY(!QFileInfo(box.unit).isExecutable());
    QCOMPARE(box.Watcher().WatchedProcesses(), QStringList{QStringLiteral("FlightSimulator2024.exe")});

    const WatcherFiles expected = BuildWatcherFiles(box.script, {QStringLiteral("FlightSimulator2024.exe")});
    QCOMPARE(QString::fromUtf8(ReadTestFile(box.unit)), expected.unit);
    QCOMPARE(QString::fromUtf8(ReadTestFile(box.script)), expected.script);

    const QStringList log = SystemctlLog(box.dir.path());
    const qsizetype reload = log.indexOf(QStringLiteral("--user daemon-reload"));
    const qsizetype enable = log.indexOf(QStringLiteral("--user enable --now gsx-integrator-client.service"));
    QVERIFY(reload >= 0);
    QVERIFY(enable > reload);
    QVERIFY(!log.contains(QStringLiteral("--user restart gsx-integrator-client.service")));
}

void SimulatorWatcherTest::enableTwiceWithTheSameSetRestartsNothing()
{
    const Sandbox box;

    QVERIFY(box.Watcher().Add(QStringLiteral("FlightSimulator2024.exe")));
    QVERIFY(box.Watcher().Add(QStringLiteral("FlightSimulator2024.exe")));

    QVERIFY(!SystemctlLog(box.dir.path()).contains(QStringLiteral("--user restart gsx-integrator-client.service")));
    QCOMPARE(box.Watcher().WatchedProcesses(), QStringList{QStringLiteral("FlightSimulator2024.exe")});
}

void SimulatorWatcherTest::addingASecondSimulatorRestartsTheService()
{
    const Sandbox box;

    QVERIFY(box.Watcher().Add(QStringLiteral("FlightSimulator2024.exe")));
    QVERIFY(box.Watcher().Add(QStringLiteral("FlightSimulator.exe")));

    QCOMPARE(box.Watcher().WatchedProcesses(),
             (QStringList{QStringLiteral("FlightSimulator.exe"), QStringLiteral("FlightSimulator2024.exe")}));
    QVERIFY(SystemctlLog(box.dir.path()).contains(QStringLiteral("--user restart gsx-integrator-client.service")));
}

void SimulatorWatcherTest::removingOneSimulatorKeepsTheOther()
{
    const Sandbox box;
    QVERIFY(box.Watcher().Add(QStringLiteral("FlightSimulator2024.exe")));
    QVERIFY(box.Watcher().Add(QStringLiteral("FlightSimulator.exe")));

    QVERIFY(box.Watcher().Remove(QStringLiteral("FlightSimulator.exe")));

    QCOMPARE(box.Watcher().WatchedProcesses(), QStringList{QStringLiteral("FlightSimulator2024.exe")});
    QVERIFY(QFile::exists(box.script));
    QVERIFY(!SystemctlLog(box.dir.path()).contains(QStringLiteral("--user disable --now gsx-integrator-client.service")));
}

void SimulatorWatcherTest::removingTheLastSimulatorDisablesAndDeletesBothFiles()
{
    const Sandbox box;
    QVERIFY(box.Watcher().Add(QStringLiteral("FlightSimulator2024.exe")));

    QVERIFY(box.Watcher().Remove(QStringLiteral("FlightSimulator2024.exe")));

    QVERIFY(!QFile::exists(box.unit));
    QVERIFY(!QFile::exists(box.script));
    QVERIFY(SystemctlLog(box.dir.path()).contains(QStringLiteral("--user disable --now gsx-integrator-client.service")));
}

void SimulatorWatcherTest::removingAnUnwatchedSimulatorTouchesNothing()
{
    const Sandbox box;

    QVERIFY(box.Watcher().Remove(QStringLiteral("FlightSimulator2024.exe")));

    QVERIFY(SystemctlLog(box.dir.path()).isEmpty());
    QVERIFY(!QFile::exists(box.unit));
}

void SimulatorWatcherTest::addWritesNothingWhenSystemdIsUnavailable()
{
    const Sandbox box;
    QVERIFY(WriteTestFile(box.dir.filePath(QStringLiteral("fail-show-environment")), ""));

    QVERIFY(!box.Watcher().Add(QStringLiteral("FlightSimulator2024.exe")));

    QVERIFY(!QFile::exists(box.unit));
    QVERIFY(!QFile::exists(box.script));
    QVERIFY(!box.Watcher().SystemdAvailable());
}

void SimulatorWatcherTest::disableFailureKeepsTheFilesForARetry()
{
    const Sandbox box;
    QVERIFY(box.Watcher().Add(QStringLiteral("FlightSimulator2024.exe")));
    QVERIFY(WriteTestFile(box.dir.filePath(QStringLiteral("fail-disable")), ""));

    QVERIFY(!box.Watcher().Disable());

    QVERIFY(QFile::exists(box.unit));
    QVERIFY(QFile::exists(box.script));
}

void SimulatorWatcherTest::disableRemovesTheFilesEvenWhenSystemctlIsGone()
{
    const Sandbox box;
    QVERIFY(box.Watcher().Add(QStringLiteral("FlightSimulator2024.exe")));

    SimulatorWatcher gone(box.script, box.unitDir, box.dir.filePath(QStringLiteral("bin/no-such-systemctl")));

    QVERIFY(gone.Disable());
    QVERIFY(!QFile::exists(box.unit));
    QVERIFY(!QFile::exists(box.script));
}

void SimulatorWatcherTest::addRejectsNamesThatCouldBreakTheUnit()
{
    const Sandbox box;

    QVERIFY(!box.Watcher().Add(QStringLiteral("evil\"\nExecStart=/bin/sh")));
    QVERIFY(!box.Watcher().Add(QStringLiteral("a b")));
    QVERIFY(!box.Watcher().Add(QString()));
    QVERIFY(!QFile::exists(box.unit));
}

namespace
{
    struct ScriptRun
    {
        QTemporaryDir dir;
        QString proc = dir.filePath(QStringLiteral("proc"));
        QString log = dir.filePath(QStringLiteral("client.log"));
        QProcess process;

        bool Start(const QString& simulators)
        {
            const QString scriptPath = dir.filePath(QStringLiteral("watcher.sh"));
            const QString clientPath = dir.filePath(QStringLiteral("fake-client.sh"));
            const WatcherFiles files = BuildWatcherFiles(scriptPath, {QStringLiteral("FlightSimulator2024.exe")});
            const QString client = QStringLiteral(
                "#!/bin/sh\n"
                "echo \"started $*\" >> \"%1\"\n"
                "trap 'echo stopped >> \"%1\"; exit 0' TERM\n"
                "while true; do sleep 0.05; done\n").arg(log);

            if (!QDir().mkpath(proc) || !WriteTestFile(scriptPath, files.script.toUtf8())
                || !WriteTestFile(clientPath, client.toUtf8())
                || !QFile::setPermissions(scriptPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner)
                || !QFile::setPermissions(clientPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner))
            {
                return false;
            }

            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert(QStringLiteral("GSXI_PROC_ROOT"), proc);
            env.insert(QStringLiteral("GSXI_POLL_SECONDS"), QStringLiteral("0.05"));
            env.insert(QStringLiteral("GSXI_SIMULATORS"), simulators);
            env.insert(QStringLiteral("GSXI_CLIENT"), clientPath);
            process.setProcessEnvironment(env);
            process.start(scriptPath, {});

            return process.waitForStarted();
        }

        bool AddProcess(const QString& pid, const QByteArray& cmdline) const
        {
            return WriteTestFile(proc + u'/' + pid + QStringLiteral("/cmdline"), cmdline);
        }

        bool RemoveProcess(const QString& pid) const
        {
            return QDir(proc + u'/' + pid).removeRecursively();
        }

        ~ScriptRun()
        {
            process.kill();
            process.waitForFinished(3000);
        }
    };
}

void SimulatorWatcherTest::scriptStartsTheClientOncePerSimulatorRunAndStopsItWithTheSimulator()
{
    ScriptRun run;
    QVERIFY(run.Start(QStringLiteral("FlightSimulator2024.exe")));

    QTest::qWait(300);
    QCOMPARE(CountLines(run.log, QStringLiteral("started")), 0);

    QVERIFY(run.AddProcess(QStringLiteral("100"), Cmdline({"S:\\Steam\\MSFS2024\\FlightSimulator2024.exe"})));
    QVERIFY(WaitFor([&] { return CountLines(run.log, QStringLiteral("started")) == 1; }));
    QVERIFY(ReadTestFile(run.log).contains("started --tray"));

    QVERIFY(run.AddProcess(QStringLiteral("101"), Cmdline({"proton", "waitforexitandrun", "/x/FlightSimulator2024.exe"})));
    QTest::qWait(500);
    QCOMPARE(CountLines(run.log, QStringLiteral("started")), 1);
    QCOMPARE(CountLines(run.log, QStringLiteral("stopped")), 0);

    QVERIFY(run.RemoveProcess(QStringLiteral("100")));
    QTest::qWait(300);
    QCOMPARE(CountLines(run.log, QStringLiteral("stopped")), 0);

    QVERIFY(run.RemoveProcess(QStringLiteral("101")));
    QVERIFY(WaitFor([&] { return CountLines(run.log, QStringLiteral("stopped")) == 1; }));

    QVERIFY(run.AddProcess(QStringLiteral("200"), Cmdline({"S:\\x\\FlightSimulator2024.exe"})));
    QVERIFY(WaitFor([&] { return CountLines(run.log, QStringLiteral("started")) == 2; }));
}

void SimulatorWatcherTest::scriptDoesNotStartASecondClient()
{
    ScriptRun run;
    QVERIFY(run.Start(QStringLiteral("FlightSimulator2024.exe")));
    QVERIFY(run.AddProcess(QStringLiteral("300"), Cmdline({"/tmp/.mount_x/usr/bin/gsx-integrator-client", "--tray"})));

    QVERIFY(run.AddProcess(QStringLiteral("100"), Cmdline({"S:\\x\\FlightSimulator2024.exe"})));
    QTest::qWait(700);

    QCOMPARE(CountLines(run.log, QStringLiteral("started")), 0);

    QVERIFY(run.RemoveProcess(QStringLiteral("100")));
    QTest::qWait(300);
    QCOMPARE(CountLines(run.log, QStringLiteral("stopped")), 0);
}

void SimulatorWatcherTest::scriptRecognisesTheClientBehindAppRunByItsExecutableLink()
{
    ScriptRun run;
    QVERIFY(run.Start(QStringLiteral("FlightSimulator2024.exe")));
    QVERIFY(run.AddProcess(QStringLiteral("310"), Cmdline({"/tmp/.mount_gsx-iAbCdE/AppRun.wrapped"})));
    QVERIFY(MakeTestSymlink(QStringLiteral("/tmp/.mount_gsx-iAbCdE/usr/bin/gsx-integrator-client"),
                            run.proc + QStringLiteral("/310/exe")));

    QVERIFY(run.AddProcess(QStringLiteral("100"), Cmdline({"S:\\x\\FlightSimulator2024.exe"})));
    QTest::qWait(700);

    QCOMPARE(CountLines(run.log, QStringLiteral("started")), 0);
}

void SimulatorWatcherTest::scriptStartsTheClientWhenAnotherProcessOnlyMentionsItsName()
{
    ScriptRun run;
    QVERIFY(run.Start(QStringLiteral("FlightSimulator2024.exe")));
    QVERIFY(run.AddProcess(QStringLiteral("320"), Cmdline({"ninja", "gsx-integrator-client"})));
    QVERIFY(run.AddProcess(QStringLiteral("321"), Cmdline({"/usr/bin/tail", "-f", "/x/gsx-integrator-client.AppImage"})));

    QVERIFY(run.AddProcess(QStringLiteral("100"), Cmdline({"S:\\x\\FlightSimulator2024.exe"})));

    QVERIFY(WaitFor([&] { return CountLines(run.log, QStringLiteral("started")) == 1; }));
}

void SimulatorWatcherTest::scriptDoesNotCountItsOwnProcessAsTheSimulator()
{
    ScriptRun run;
    QVERIFY(run.Start(QStringLiteral("FlightSimulator2024.exe")));

    QVERIFY(run.AddProcess(QString::number(run.process.processId()),
                           Cmdline({"bash", "/x/watcher.sh", "FlightSimulator2024.exe"})));
    QTest::qWait(700);

    QCOMPARE(CountLines(run.log, QStringLiteral("started")), 0);
}

void SimulatorWatcherTest::scriptRefusesToRunWithoutSimulators()
{
    ScriptRun run;
    QVERIFY(run.Start(QString()));

    QVERIFY(run.process.waitForFinished(5000));
    QCOMPARE(run.process.exitCode(), 1);
}

QTEST_GUILESS_MAIN(SimulatorWatcherTest)

#include "tst_simulator_watcher.moc"
