#include <QtTest/QTest>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QTemporaryDir>

#include "../src/infrastructure/install/ReleasePackage.h"
#include "miniz.h"

Q_DECLARE_METATYPE(ExtractStatus)

namespace
{
    struct ZipEntry
    {
        QByteArray name;
        QByteArray data;
        mz_uint level = MZ_DEFAULT_COMPRESSION;
    };

    QByteArray BuildZip(const QList<ZipEntry>& entries)
    {
        mz_zip_archive zip{};
        if (!mz_zip_writer_init_heap(&zip, 0, 0))
        {
            return {};
        }

        for (const ZipEntry& entry : entries)
        {
            if (!mz_zip_writer_add_mem(&zip, entry.name.constData(), entry.data.constData(),
                                       static_cast<size_t>(entry.data.size()), entry.level))
            {
                (void)mz_zip_writer_end(&zip);
                return {};
            }
        }

        void* buffer = nullptr;
        size_t size = 0;
        if (!mz_zip_writer_finalize_heap_archive(&zip, &buffer, &size))
        {
            (void)mz_zip_writer_end(&zip);
            return {};
        }

        const QByteArray bytes(static_cast<const char*>(buffer), static_cast<qsizetype>(size));
        (void)mz_zip_writer_end(&zip);

        return bytes;
    }

    bool WriteFile(const QString& path, const QByteArray& content)
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            return false;
        }

        return file.write(content) == content.size();
    }

    QByteArray ReadFile(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
        {
            return {};
        }

        return file.readAll();
    }

    const QString kAbcSha = QStringLiteral("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    const QString kEmptySha = QStringLiteral("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

class ReleasePackageTest final : public QObject
{
    Q_OBJECT

private slots:
    static void parsesOneColumnHash();
    static void parsesTwoColumnHash();
    static void parsesBinaryModeMarker();
    static void lowercasesUppercaseHash();
    static void toleratesCrlfAndByteOrderMark();
    static void takesTheFirstLineOnly();
    static void rejectsGarbage_data();
    static void rejectsGarbage();
    static void readsShaFileFromDisk();
    static void hashesKnownContent();
    static void matchesOnlyWhenShaFileAgrees();
    static void extractsNestedFilesAndEmptyDirectories();
    static void overwritesExistingFiles();
    static void rejectsEntriesThatEscapeTheDestination_data();
    static void rejectsEntriesThatEscapeTheDestination();
    static void rejectedArchiveWritesNothing();
    static void reportsMissingArchive();
    static void reportsNonZipFile();
    static void reportsCorruptedEntry();
    static void reportsUnwritableDestination();
};

void ReleasePackageTest::parsesOneColumnHash()
{
    QCOMPARE(ParseSha256(kAbcSha.toLatin1() + "\n"), kAbcSha);
    QCOMPARE(ParseSha256(kAbcSha.toLatin1()), kAbcSha);
}

void ReleasePackageTest::parsesTwoColumnHash()
{
    QCOMPARE(ParseSha256(kAbcSha.toLatin1() + "  gsx-integrator-client-1.0.0.zip\n"), kAbcSha);
    QCOMPARE(ParseSha256(kAbcSha.toLatin1() + "\tpackage.zip\n"), kAbcSha);
}

void ReleasePackageTest::parsesBinaryModeMarker()
{
    QCOMPARE(ParseSha256(kAbcSha.toLatin1() + " *package.zip\n"), kAbcSha);
}

void ReleasePackageTest::lowercasesUppercaseHash()
{
    QCOMPARE(ParseSha256(kAbcSha.toUpper().toLatin1() + "  package.zip\n"), kAbcSha);
}

void ReleasePackageTest::toleratesCrlfAndByteOrderMark()
{
    QCOMPARE(ParseSha256(kAbcSha.toLatin1() + "  package.zip\r\n"), kAbcSha);
    QCOMPARE(ParseSha256("\xEF\xBB\xBF" + kAbcSha.toLatin1() + "\r\n"), kAbcSha);
    QCOMPARE(ParseSha256("\r\n  " + kAbcSha.toLatin1() + "  \r\n"), kAbcSha);
}

void ReleasePackageTest::takesTheFirstLineOnly()
{
    QCOMPARE(ParseSha256(kAbcSha.toLatin1() + "  a.zip\n" + kEmptySha.toLatin1() + "  b.zip\n"), kAbcSha);
}

void ReleasePackageTest::rejectsGarbage_data()
{
    QTest::addColumn<QByteArray>("content");

    QTest::newRow("empty") << QByteArray();
    QTest::newRow("blank") << QByteArray("  \r\n\t\n");
    QTest::newRow("prose") << QByteArray("Not Found");
    QTest::newRow("html") << QByteArray("<!DOCTYPE html><html></html>");
    QTest::newRow("tooShort") << kAbcSha.left(63).toLatin1();
    QTest::newRow("tooLong") << (kAbcSha + QLatin1Char('0')).toLatin1();
    QTest::newRow("nonHex") << (QString(63, QLatin1Char('a')) + QLatin1Char('g')).toLatin1();
    QTest::newRow("split") << (kAbcSha.left(32) + QLatin1Char(' ') + kAbcSha.mid(32)).toLatin1();
}

void ReleasePackageTest::rejectsGarbage()
{
    QFETCH(QByteArray, content);

    QVERIFY(ParseSha256(content).isEmpty());
}

void ReleasePackageTest::readsShaFileFromDisk()
{
    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    const QString shaPath = sandbox.filePath(QStringLiteral("package.zip.sha256"));
    QVERIFY(WriteFile(shaPath, kAbcSha.toUpper().toLatin1() + "  package.zip\r\n"));

    QCOMPARE(ReadSha256File(shaPath), kAbcSha);
    QVERIFY(ReadSha256File(sandbox.filePath(QStringLiteral("missing.sha256"))).isEmpty());
}

void ReleasePackageTest::hashesKnownContent()
{
    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    const QString abc = sandbox.filePath(QStringLiteral("abc.bin"));
    const QString empty = sandbox.filePath(QStringLiteral("empty.bin"));
    QVERIFY(WriteFile(abc, "abc"));
    QVERIFY(WriteFile(empty, {}));

    QCOMPARE(Sha256File(abc), kAbcSha);
    QCOMPARE(Sha256File(empty), kEmptySha);
    QVERIFY(Sha256File(sandbox.filePath(QStringLiteral("missing.bin"))).isEmpty());
}

void ReleasePackageTest::matchesOnlyWhenShaFileAgrees()
{
    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    const QString file = sandbox.filePath(QStringLiteral("abc.bin"));
    const QString good = sandbox.filePath(QStringLiteral("good.sha256"));
    const QString wrong = sandbox.filePath(QStringLiteral("wrong.sha256"));
    const QString garbage = sandbox.filePath(QStringLiteral("garbage.sha256"));
    QVERIFY(WriteFile(file, "abc"));
    QVERIFY(WriteFile(good, kAbcSha.toLatin1() + "  abc.bin\n"));
    QVERIFY(WriteFile(wrong, kEmptySha.toLatin1() + "  abc.bin\n"));
    QVERIFY(WriteFile(garbage, "Not Found"));

    QVERIFY(Sha256Matches(file, good));
    QVERIFY(!Sha256Matches(file, wrong));
    QVERIFY(!Sha256Matches(file, garbage));
    QVERIFY(!Sha256Matches(file, sandbox.filePath(QStringLiteral("missing.sha256"))));
    QVERIFY(!Sha256Matches(sandbox.filePath(QStringLiteral("missing.bin")), good));
}

void ReleasePackageTest::extractsNestedFilesAndEmptyDirectories()
{
    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    const QByteArray zip = BuildZip({
        {"pkg/", {}},
        {"pkg/app.exe", "exe-bytes"},
        {"pkg/data/", {}},
        {"pkg/data/deep/config.json", QByteArray(100000, 'x')},
        {"pkg/empty/", {}},
        {"pkg/stored.txt", "stored", MZ_NO_COMPRESSION},
        {"pkg/zero.bin", {}},
    });
    QVERIFY(!zip.isEmpty());

    const QString zipPath = sandbox.filePath(QStringLiteral("package.zip"));
    QVERIFY(WriteFile(zipPath, zip));

    const QString dest = sandbox.filePath(QStringLiteral("out/parent"));
    const ExtractResult result = ExtractZip(zipPath, dest);

    QVERIFY2(result.ok(), qPrintable(result.detail));
    QCOMPARE(ReadFile(dest + QStringLiteral("/pkg/app.exe")), QByteArray("exe-bytes"));
    QCOMPARE(ReadFile(dest + QStringLiteral("/pkg/data/deep/config.json")), QByteArray(100000, 'x'));
    QCOMPARE(ReadFile(dest + QStringLiteral("/pkg/stored.txt")), QByteArray("stored"));
    QVERIFY(QFile::exists(dest + QStringLiteral("/pkg/zero.bin")));
    QCOMPARE(QFileInfo(dest + QStringLiteral("/pkg/zero.bin")).size(), 0);
    QVERIFY(QDir(dest + QStringLiteral("/pkg/empty")).exists());
}

void ReleasePackageTest::overwritesExistingFiles()
{
    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    const QString zipPath = sandbox.filePath(QStringLiteral("package.zip"));
    QVERIFY(WriteFile(zipPath, BuildZip({{"pkg/app.exe", "new"}})));

    const QString dest = sandbox.filePath(QStringLiteral("out"));
    QVERIFY(QDir().mkpath(dest + QStringLiteral("/pkg")));
    QVERIFY(WriteFile(dest + QStringLiteral("/pkg/app.exe"), "old and much longer"));

    QVERIFY(ExtractZip(zipPath, dest).ok());
    QCOMPARE(ReadFile(dest + QStringLiteral("/pkg/app.exe")), QByteArray("new"));
}

void ReleasePackageTest::rejectsEntriesThatEscapeTheDestination_data()
{
    QTest::addColumn<QByteArray>("entryName");
    QTest::addColumn<ExtractStatus>("status");

    constexpr ExtractStatus unsafe = ExtractStatus::UnsafeEntry;
    QTest::newRow("parent") << QByteArray("../evil.txt") << unsafe;
    QTest::newRow("nestedParent") << QByteArray("pkg/../../evil.txt") << unsafe;
    QTest::newRow("backslashParent") << QByteArray("pkg\\..\\..\\evil.txt") << unsafe;
    QTest::newRow("absolute") << QByteArray("/evil.txt") << ExtractStatus::InvalidArchive;
    QTest::newRow("backslashAbsolute") << QByteArray("\\evil.txt") << unsafe;
    QTest::newRow("driveLetter") << QByteArray("C:/evil.txt") << unsafe;
    QTest::newRow("driveRelative") << QByteArray("C:evil.txt") << unsafe;
    QTest::newRow("alternateStream") << QByteArray("pkg/app.exe:evil") << unsafe;
    QTest::newRow("onlyParent") << QByteArray("..") << unsafe;
}

void ReleasePackageTest::rejectsEntriesThatEscapeTheDestination()
{
    QFETCH(QByteArray, entryName);
    QFETCH(ExtractStatus, status);

    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    const QString zipPath = sandbox.filePath(QStringLiteral("package.zip"));
    QVERIFY(WriteFile(zipPath, BuildZip({{entryName, "payload"}})));

    const QString dest = sandbox.filePath(QStringLiteral("out/inner"));
    const ExtractResult result = ExtractZip(zipPath, dest);

    QCOMPARE(result.status, status);
    QVERIFY(!result.ok());
    QVERIFY(!QFile::exists(sandbox.filePath(QStringLiteral("out/evil.txt"))));
    QVERIFY(!QFile::exists(sandbox.filePath(QStringLiteral("evil.txt"))));
}

void ReleasePackageTest::rejectedArchiveWritesNothing()
{
    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    const QString zipPath = sandbox.filePath(QStringLiteral("package.zip"));
    QVERIFY(WriteFile(zipPath, BuildZip({{"pkg/good.txt", "good"}, {"pkg/../../evil.txt", "evil"}})));

    const QString dest = sandbox.filePath(QStringLiteral("out"));
    const ExtractResult result = ExtractZip(zipPath, dest);

    QCOMPARE(result.status, ExtractStatus::UnsafeEntry);
    QVERIFY(!QFile::exists(dest + QStringLiteral("/pkg/good.txt")));
}

void ReleasePackageTest::reportsMissingArchive()
{
    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    const ExtractResult result = ExtractZip(sandbox.filePath(QStringLiteral("nope.zip")),
                                            sandbox.filePath(QStringLiteral("out")));

    QCOMPARE(result.status, ExtractStatus::OpenFailed);
    QVERIFY(!result.detail.isEmpty());
}

void ReleasePackageTest::reportsNonZipFile()
{
    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    const QString zipPath = sandbox.filePath(QStringLiteral("package.zip"));
    QVERIFY(WriteFile(zipPath, "<html>404 Not Found</html>"));

    const ExtractResult result = ExtractZip(zipPath, sandbox.filePath(QStringLiteral("out")));

    QCOMPARE(result.status, ExtractStatus::InvalidArchive);
    QVERIFY(!result.detail.isEmpty());
}

void ReleasePackageTest::reportsCorruptedEntry()
{
    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    QByteArray zip = BuildZip({{"pkg/app.exe", "uncompressed-payload", MZ_NO_COMPRESSION}});
    const qsizetype at = zip.indexOf("uncompressed-payload");
    QVERIFY(at > 0);
    zip[at] = 'X';

    const QString zipPath = sandbox.filePath(QStringLiteral("package.zip"));
    QVERIFY(WriteFile(zipPath, zip));

    const QString dest = sandbox.filePath(QStringLiteral("out"));
    const ExtractResult result = ExtractZip(zipPath, dest);

    QCOMPARE(result.status, ExtractStatus::InvalidArchive);
    QVERIFY(result.detail.startsWith(QStringLiteral("pkg/app.exe")));
    QVERIFY(!QFile::exists(dest + QStringLiteral("/pkg/app.exe")));
}

void ReleasePackageTest::reportsUnwritableDestination()
{
    const QTemporaryDir sandbox;
    QVERIFY(sandbox.isValid());

    const QString zipPath = sandbox.filePath(QStringLiteral("package.zip"));
    QVERIFY(WriteFile(zipPath, BuildZip({{"pkg/app.exe", "exe"}})));

    const QString dest = sandbox.filePath(QStringLiteral("out"));
    QVERIFY(QDir().mkpath(dest));
    QVERIFY(WriteFile(dest + QStringLiteral("/pkg"), "a file where the directory should be"));

    const ExtractResult result = ExtractZip(zipPath, dest);

    QCOMPARE(result.status, ExtractStatus::WriteFailed);
    QVERIFY(result.detail.contains(QStringLiteral("pkg")));
}

QTEST_GUILESS_MAIN(ReleasePackageTest)

#include "tst_release_package.moc"
