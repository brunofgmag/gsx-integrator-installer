#include <QtTest/QTest>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QTemporaryDir>

#include "../src/infrastructure/install/SelfUpdateScript.h"

namespace
{
    void WriteAll(const QString& path, const QString& content)
    {
        QFile file(path);

        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));

        file.write(content.toUtf8());
    }

    QString ReadAll(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }

        return QString::fromUtf8(file.readAll());
    }

    void RunScript(const QString& scriptPath)
    {
        QProcess cmd;
        cmd.start(QStringLiteral("cmd.exe"),
                  {
                      QStringLiteral("/d"), QStringLiteral("/c"),
                      QDir::toNativeSeparators(scriptPath)
                  });

        QVERIFY(cmd.waitForStarted(5000));
        QVERIFY(cmd.waitForFinished(60000));
    }
}

class SelfUpdateScriptTest final : public QObject
{
    Q_OBJECT

private slots:
    static void replacesBundleAndCleansUp();
    static void handlesAccentedPaths();
};

void SelfUpdateScriptTest::replacesBundleAndCleansUp()
{
    const QTemporaryDir sandbox;

    QVERIFY(sandbox.isValid());

    const QString staging = sandbox.filePath(QStringLiteral("staging"));
    const QString payload = staging + QStringLiteral("/gsx-integrator-installer");
    const QString app = sandbox.filePath(QStringLiteral("app"));

    QVERIFY(QDir().mkpath(payload + QStringLiteral("/plugins")));
    QVERIFY(QDir().mkpath(app));

    WriteAll(payload + QStringLiteral("/gsx-integrator-installer.exe"), QStringLiteral("new-exe"));
    WriteAll(payload + QStringLiteral("/plugins/extra.dll"), QStringLiteral("new-dll"));
    WriteAll(app + QStringLiteral("/gsx-integrator-installer.exe"), QStringLiteral("outdated-exe"));
    WriteAll(app + QStringLiteral("/leftover.txt"), QStringLiteral("keep"));

    const QString scriptPath = sandbox.filePath(QStringLiteral("update.cmd"));
    const auto relaunch = QStringLiteral("C:/Windows/System32/where.exe");

    QVERIFY(WriteSelfUpdateScript(scriptPath, payload, app, relaunch, staging));

    RunScript(scriptPath);

    QCOMPARE(ReadAll(app + QStringLiteral("/gsx-integrator-installer.exe")), QStringLiteral("new-exe"));
    QCOMPARE(ReadAll(app + QStringLiteral("/plugins/extra.dll")), QStringLiteral("new-dll"));
    QCOMPARE(ReadAll(app + QStringLiteral("/leftover.txt")), QStringLiteral("keep"));
    QVERIFY(!QDir(staging).exists());
    QVERIFY(!QFile::exists(scriptPath));
}

void SelfUpdateScriptTest::handlesAccentedPaths()
{
    const QTemporaryDir sandbox;

    QVERIFY(sandbox.isValid());

    const QString accented = QStringLiteral("instala") + QChar(0x00E7) + QChar(0x00E3) + QStringLiteral("o");
    const QString staging = sandbox.filePath(accented + QStringLiteral("-staging"));
    const QString payload = staging + QStringLiteral("/gsx-integrator-installer");
    const QString app = sandbox.filePath(accented);

    QVERIFY(QDir().mkpath(payload));
    QVERIFY(QDir().mkpath(app));

    WriteAll(payload + QStringLiteral("/gsx-integrator-installer.exe"), QStringLiteral("new-exe"));

    const QString scriptPath = sandbox.filePath(QStringLiteral("update.cmd"));
    const auto relaunch = QStringLiteral("C:/Windows/System32/where.exe");

    QVERIFY(WriteSelfUpdateScript(scriptPath, payload, app, relaunch, staging));

    RunScript(scriptPath);

    QCOMPARE(ReadAll(app + QStringLiteral("/gsx-integrator-installer.exe")), QStringLiteral("new-exe"));
    QVERIFY(!QDir(staging).exists());
    QVERIFY(!QFile::exists(scriptPath));
}

QTEST_GUILESS_MAIN(SelfUpdateScriptTest)

#include "tst_self_update_script.moc"
