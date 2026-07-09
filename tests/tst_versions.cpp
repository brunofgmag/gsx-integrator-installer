#include <QtTest/QTest>

#include <QtCore/QVersionNumber>

#include "../src/domain/Versions.h"

class VersionsTest final : public QObject
{
    Q_OBJECT

private slots:
    static void parseVersionAcceptsTagPrefix();
    static void isNewerComparesAndTreatsEmptyAsNotInstalled();
    static void parsePackageVersionReadsManifest();
};

void VersionsTest::parseVersionAcceptsTagPrefix()
{
    QCOMPARE(ParseVersion(QStringLiteral("v1.2.3")), QVersionNumber(1, 2, 3));
    QCOMPARE(ParseVersion(QStringLiteral("1.2.3")), QVersionNumber(1, 2, 3));
    QVERIFY(ParseVersion(QStringLiteral("banana")).isNull());
    QVERIFY(ParseVersion(QString()).isNull());
}

void VersionsTest::isNewerComparesAndTreatsEmptyAsNotInstalled()
{
    QVERIFY(IsNewer(QStringLiteral("0.2.0"), QStringLiteral("0.1.1")));
    QVERIFY(!IsNewer(QStringLiteral("0.1.1"), QStringLiteral("0.1.1")));
    QVERIFY(!IsNewer(QStringLiteral("0.1.0"), QStringLiteral("0.1.1")));
    QVERIFY(IsNewer(QStringLiteral("0.0.1"), QString()));
    QVERIFY(!IsNewer(QString(), QStringLiteral("0.1.0")));
    QVERIFY(IsNewer(QStringLiteral("0.10.0"), QStringLiteral("0.9.9")));
}

void VersionsTest::parsePackageVersionReadsManifest()
{
    const QByteArray manifest = R"({"title": "x", "package_version": "0.1.0"})";

    QCOMPARE(ParsePackageVersion(manifest), QStringLiteral("0.1.0"));
    QVERIFY(ParsePackageVersion("not json").isEmpty());
    QVERIFY(ParsePackageVersion(R"({"title": "x"})").isEmpty());
}

QTEST_APPLESS_MAIN(VersionsTest)

#include "tst_versions.moc"
