#include <QtTest/QTest>

#include "../src/infrastructure/system/SimDetection.h"

class SimDetectionTest final : public QObject
{
    Q_OBJECT

private slots:
    static void extractsQuotedValue();
    static void handlesCrLf();
    static void missingEntryGivesEmpty();
};

void SimDetectionTest::extractsQuotedValue()
{
    const auto content = QStringLiteral(
        "{Graphics\n}\nInstalledPackagesPath \"D:\\MSFS 2024\\Packages\"\nSomethingElse 1\n");

    QCOMPARE(ParseInstalledPackagesPath(content), QStringLiteral("D:\\MSFS 2024\\Packages"));
}

void SimDetectionTest::handlesCrLf()
{
    const auto content = QStringLiteral("A 1\r\nInstalledPackagesPath \"C:\\X\"\r\nB 2\r\n");

    QCOMPARE(ParseInstalledPackagesPath(content), QStringLiteral("C:\\X"));
}

void SimDetectionTest::missingEntryGivesEmpty()
{
    QVERIFY(ParseInstalledPackagesPath(QStringLiteral("Nothing here")).isEmpty());
}

QTEST_APPLESS_MAIN(SimDetectionTest)

#include "tst_sim_detection.moc"
