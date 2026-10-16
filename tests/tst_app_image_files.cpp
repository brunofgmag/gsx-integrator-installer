#include <QtTest/QTest>

#include <unistd.h>

#include <QtCore/QFileInfo>
#include <QtCore/QTemporaryDir>

#include "../src/infrastructure/install/AppImageFiles.h"
#include "support/FileTree.h"

class AppImageFilesTest final : public QObject
{
    Q_OBJECT

private slots:
    static void copyExecutableSetsTheExecuteBits();
    static void copyExecutableReplacesAnExistingFileAndLeavesNoPart();
    static void copyExecutableCreatesMissingDirectories();
    static void copyExecutableFailsWhenTheSourceIsMissing();
    static void replaceFileOverwritesTheTargetInPlace();
    static void replaceFileFailsWhenTheStagedFileIsMissing();
    static void noEnvironmentPathIsNotAnAppImage();
    static void missingFileIsNotAnAppImage();
    static void fileInAWritableDirectoryIsATarget();
    static void fileInAReadOnlyDirectoryNamesTheDirectory();
};

void AppImageFilesTest::copyExecutableSetsTheExecuteBits()
{
    const QTemporaryDir dir;
    QVERIFY(WriteTestFile(dir.filePath(QStringLiteral("src.AppImage")), "payload"));

    QVERIFY(CopyExecutable(dir.filePath(QStringLiteral("src.AppImage")), dir.filePath(QStringLiteral("out/app.AppImage"))));

    const QFileInfo out(dir.filePath(QStringLiteral("out/app.AppImage")));
    QVERIFY(out.isExecutable());
    QVERIFY(out.permissions().testFlag(QFileDevice::ExeGroup));
    QVERIFY(out.permissions().testFlag(QFileDevice::ExeOther));
    QCOMPARE(ReadTestFile(out.filePath()), QByteArray("payload"));
}

void AppImageFilesTest::copyExecutableReplacesAnExistingFileAndLeavesNoPart()
{
    const QTemporaryDir dir;
    QVERIFY(WriteTestFile(dir.filePath(QStringLiteral("src")), "new"));
    QVERIFY(WriteTestFile(dir.filePath(QStringLiteral("app")), "old"));
    QVERIFY(WriteTestFile(dir.filePath(QStringLiteral("app.part")), "stale"));

    QVERIFY(CopyExecutable(dir.filePath(QStringLiteral("src")), dir.filePath(QStringLiteral("app"))));

    QCOMPARE(ReadTestFile(dir.filePath(QStringLiteral("app"))), QByteArray("new"));
    QVERIFY(!QFile::exists(dir.filePath(QStringLiteral("app.part"))));
}

void AppImageFilesTest::copyExecutableCreatesMissingDirectories()
{
    const QTemporaryDir dir;
    QVERIFY(WriteTestFile(dir.filePath(QStringLiteral("src")), "x"));

    QVERIFY(CopyExecutable(dir.filePath(QStringLiteral("src")), dir.filePath(QStringLiteral("a/b/c/app"))));
    QVERIFY(QFile::exists(dir.filePath(QStringLiteral("a/b/c/app"))));
}

void AppImageFilesTest::copyExecutableFailsWhenTheSourceIsMissing()
{
    const QTemporaryDir dir;

    QVERIFY(!CopyExecutable(dir.filePath(QStringLiteral("nope")), dir.filePath(QStringLiteral("app"))));
    QVERIFY(!QFile::exists(dir.filePath(QStringLiteral("app"))));
}

void AppImageFilesTest::replaceFileOverwritesTheTargetInPlace()
{
    const QTemporaryDir dir;
    QVERIFY(WriteTestFile(dir.filePath(QStringLiteral("staged")), "v2"));
    QVERIFY(WriteTestFile(dir.filePath(QStringLiteral("app")), "v1"));

    QVERIFY(ReplaceFile(dir.filePath(QStringLiteral("staged")), dir.filePath(QStringLiteral("app"))));

    QCOMPARE(ReadTestFile(dir.filePath(QStringLiteral("app"))), QByteArray("v2"));
    QVERIFY(QFileInfo(dir.filePath(QStringLiteral("app"))).isExecutable());
    QVERIFY(!QFile::exists(dir.filePath(QStringLiteral("staged"))));
}

void AppImageFilesTest::replaceFileFailsWhenTheStagedFileIsMissing()
{
    const QTemporaryDir dir;
    QVERIFY(WriteTestFile(dir.filePath(QStringLiteral("app")), "v1"));

    QVERIFY(!ReplaceFile(dir.filePath(QStringLiteral("staged")), dir.filePath(QStringLiteral("app"))));
    QCOMPARE(ReadTestFile(dir.filePath(QStringLiteral("app"))), QByteArray("v1"));
}

void AppImageFilesTest::noEnvironmentPathIsNotAnAppImage()
{
    QCOMPARE(ResolveAppImageTarget(QString()).status, AppImageTargetStatus::NotAnAppImage);
}

void AppImageFilesTest::missingFileIsNotAnAppImage()
{
    QCOMPARE(ResolveAppImageTarget(QStringLiteral("/nonexistent/i.AppImage")).status,
             AppImageTargetStatus::NotAnAppImage);
}

void AppImageFilesTest::fileInAWritableDirectoryIsATarget()
{
    const QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("i.AppImage"));
    QVERIFY(WriteTestFile(path, "x"));

    const AppImageTarget target = ResolveAppImageTarget(path);

    QCOMPARE(target.status, AppImageTargetStatus::Writable);
    QCOMPARE(target.path, path);
    QCOMPARE(target.dir, dir.path());
}

void AppImageFilesTest::fileInAReadOnlyDirectoryNamesTheDirectory()
{
    if (geteuid() == 0)
    {
        QSKIP("root can write anywhere, so a read-only directory proves nothing");
    }

    const QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("i.AppImage"));
    QVERIFY(WriteTestFile(path, "x"));
    QVERIFY(QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));

    const AppImageTarget target = ResolveAppImageTarget(path);
    QFile::setPermissions(dir.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    QCOMPARE(target.status, AppImageTargetStatus::DirNotWritable);
    QCOMPARE(target.dir, dir.path());
}

QTEST_APPLESS_MAIN(AppImageFilesTest)

#include "tst_app_image_files.moc"
