#include <QtTest/QTest>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>

#include "../src/infrastructure/install/DirCopy.h"

namespace
{
    void WriteAll(const QString& path, const QString& content)
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
        file.write(content.toUtf8());
    }
}

class DirCopyTest final : public QObject
{
    Q_OBJECT

private slots:
    static void copiesFilesAndNestedSubdirs();
    static void failsWhenSourceMissing();
    static void removesContentsButKeepsNamedSubdir();
};

void DirCopyTest::copiesFilesAndNestedSubdirs()
{
    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    const QString source = sandbox.filePath(QStringLiteral("deploy"));
    QVERIFY(QDir().mkpath(source + QStringLiteral("/platforms")));

    WriteAll(source + QStringLiteral("/gsx-integrator-installer.exe"), QStringLiteral("exe"));
    WriteAll(source + QStringLiteral("/Qt6Quick.dll"), QStringLiteral("dll"));
    WriteAll(source + QStringLiteral("/platforms/qwindows.dll"), QStringLiteral("plugin"));

    const QString dest = sandbox.filePath(QStringLiteral("install/maintenance"));

    QVERIFY(CopyDirRecursively(source, dest));

    QVERIFY(QFile::exists(dest + QStringLiteral("/gsx-integrator-installer.exe")));
    QVERIFY(QFile::exists(dest + QStringLiteral("/Qt6Quick.dll")));
    QVERIFY(QFile::exists(dest + QStringLiteral("/platforms/qwindows.dll")));
}

void DirCopyTest::failsWhenSourceMissing()
{
    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    QVERIFY(!CopyDirRecursively(sandbox.filePath(QStringLiteral("nope")),
                                sandbox.filePath(QStringLiteral("dest"))));
}

void DirCopyTest::removesContentsButKeepsNamedSubdir()
{
    const QTemporaryDir sandbox;

    QVERIFY(sandbox.isValid());

    const QString installDir = sandbox.filePath(QStringLiteral("install"));

    QVERIFY(QDir().mkpath(installDir + QStringLiteral("/maintenance")));
    QVERIFY(QDir().mkpath(installDir + QStringLiteral("/resources")));

    WriteAll(installDir + QStringLiteral("/gsx-integrator-client.exe"), QStringLiteral("old"));
    WriteAll(installDir + QStringLiteral("/resources/data.bin"), QStringLiteral("x"));
    WriteAll(installDir + QStringLiteral("/maintenance/gsx-integrator-installer.exe"),
             QStringLiteral("keep"));

    QVERIFY(RemoveDirContentsExcept(installDir, QStringLiteral("maintenance")));

    QVERIFY(QDir(installDir).exists());
    QVERIFY(!QFile::exists(installDir + QStringLiteral("/gsx-integrator-client.exe")));
    QVERIFY(!QDir(installDir + QStringLiteral("/resources")).exists());
    QVERIFY(QFile::exists(installDir + QStringLiteral("/maintenance/gsx-integrator-installer.exe")));
}

QTEST_GUILESS_MAIN(DirCopyTest)

#include "tst_dir_copy.moc"
