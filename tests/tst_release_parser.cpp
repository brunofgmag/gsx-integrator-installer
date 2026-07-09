#include <QtTest/QTest>

#include <QtCore/QUrl>

#include "../src/infrastructure/github/ReleaseParser.h"

class ReleaseParserTest final : public QObject
{
    Q_OBJECT

private slots:
    static void findsBothAssets();
    static void parsesInstallerExeRelease();
    static void missingShaIsInvalid();
    static void garbageIsInvalid();
};

void ReleaseParserTest::findsBothAssets()
{
    const QByteArray json =
        R"({"tag_name": "v0.2.0", "assets": [)"
        R"({"name": "gsx-integrator-client-0.2.0.zip", "browser_download_url": "https://example.com/client.zip"},)"
        R"({"name": "gsx-integrator-client-0.2.0.zip.sha256", "browser_download_url": "https://example.com/client.zip.sha256"}]})";
    const ReleaseInfo info =
        ParseLatestRelease(json, QStringLiteral("gsx-integrator-client"));

    QVERIFY(info.valid);
    QCOMPARE(info.version, QStringLiteral("0.2.0"));
    QCOMPARE(info.zipName, QStringLiteral("gsx-integrator-client-0.2.0.zip"));
    QCOMPARE(info.zipUrl, QUrl(QStringLiteral("https://example.com/client.zip")));
    QCOMPARE(info.shaUrl, QUrl(QStringLiteral("https://example.com/client.zip.sha256")));
}

void ReleaseParserTest::parsesInstallerExeRelease()
{
    const QByteArray json =
        R"({"tag_name": "v1.0.0", "assets": [)"
        R"({"name": "gsx-integrator-installer-1.0.0.exe", "browser_download_url": "https://example.com/installer.exe"},)"
        R"({"name": "gsx-integrator-installer-1.0.0.exe.sha256", "browser_download_url": "https://example.com/installer.exe.sha256"}]})";
    const ReleaseInfo info =
        ParseLatestRelease(json, QStringLiteral("gsx-integrator-installer"), QStringLiteral(".exe"));

    QVERIFY(info.valid);
    QCOMPARE(info.version, QStringLiteral("1.0.0"));
    QCOMPARE(info.zipName, QStringLiteral("gsx-integrator-installer-1.0.0.exe"));
    QCOMPARE(info.zipUrl, QUrl(QStringLiteral("https://example.com/installer.exe")));
    QCOMPARE(info.shaUrl, QUrl(QStringLiteral("https://example.com/installer.exe.sha256")));
}

void ReleaseParserTest::missingShaIsInvalid()
{
    const QByteArray json =
        R"({"tag_name": "v0.2.0", "assets": [)"
        R"({"name": "gsx-integrator-client-0.2.0.zip", "browser_download_url": "https://example.com/client.zip"}]})";
    const ReleaseInfo info =
        ParseLatestRelease(json, QStringLiteral("gsx-integrator-client"));

    QVERIFY(!info.valid);
    QVERIFY(!info.error.isEmpty());
}

void ReleaseParserTest::garbageIsInvalid()
{
    QVERIFY(!ParseLatestRelease("[]", QStringLiteral("x")).valid);
    QVERIFY(!ParseLatestRelease("garbage", QStringLiteral("x")).valid);
    QVERIFY(!ParseLatestRelease(R"({"tag_name": "pre-release"})", QStringLiteral("x")).valid);
}

QTEST_APPLESS_MAIN(ReleaseParserTest)

#include "tst_release_parser.moc"
