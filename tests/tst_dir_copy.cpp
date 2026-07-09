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

QTEST_GUILESS_MAIN(DirCopyTest)

#include "tst_dir_copy.moc"
