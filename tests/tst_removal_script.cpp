#include <QtTest/QTest>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QTemporaryDir>

#include "../src/infrastructure/install/RemovalScript.h"

namespace
{
    void WriteAll(const QString& path, const QString& content)
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
        file.write(content.toUtf8());
    }
}

class RemovalScriptTest final : public QObject
{
    Q_OBJECT

private slots:
    static void removesDirEvenWhenItIsTheWorkingDir();
    static void removesDirWithAccentedPath();
};

void RemovalScriptTest::removesDirEvenWhenItIsTheWorkingDir()
{
    const QTemporaryDir sandbox;

    QVERIFY(sandbox.isValid());

    const QString target = sandbox.filePath(QStringLiteral("install"));

    QVERIFY(QDir().mkpath(target + QStringLiteral("/sub")));

    WriteAll(target + QStringLiteral("/sub/data.txt"), QStringLiteral("x"));
    WriteAll(target + QStringLiteral("/gsx-integrator-client.exe"), QStringLiteral("stub"));

    const QString scriptPath = sandbox.filePath(QStringLiteral("remove.cmd"));

    QVERIFY(WriteRemovalScript(scriptPath, target));

    QProcess cmd;
    cmd.setWorkingDirectory(target);
    cmd.start(QStringLiteral("cmd.exe"),
              {
                  QStringLiteral("/d"), QStringLiteral("/c"),
                  QDir::toNativeSeparators(scriptPath)
              });

    QVERIFY(cmd.waitForStarted(5000));
    QVERIFY(cmd.waitForFinished(60000));

    QVERIFY(!QDir(target).exists());
    QVERIFY(!QFile::exists(scriptPath));
}

void RemovalScriptTest::removesDirWithAccentedPath()
{
    const QTemporaryDir sandbox;

    QVERIFY(sandbox.isValid());

    const QString accented = QStringLiteral("instala") + QChar(0x00E7) + QChar(0x00E3) + QStringLiteral("o");
    const QString target = sandbox.filePath(accented);

    QVERIFY(QDir().mkpath(target + QStringLiteral("/sub")));

    WriteAll(target + QStringLiteral("/sub/data.txt"), QStringLiteral("x"));

    const QString scriptPath = sandbox.filePath(QStringLiteral("remove.cmd"));

    QVERIFY(WriteRemovalScript(scriptPath, target));

    QProcess cmd;
    cmd.start(QStringLiteral("cmd.exe"),
              {
                  QStringLiteral("/d"), QStringLiteral("/c"),
                  QDir::toNativeSeparators(scriptPath)
              });

    QVERIFY(cmd.waitForStarted(5000));
    QVERIFY(cmd.waitForFinished(60000));

    QVERIFY(!QDir(target).exists());
    QVERIFY(!QFile::exists(scriptPath));
}

QTEST_GUILESS_MAIN(RemovalScriptTest)

#include "tst_removal_script.moc"
