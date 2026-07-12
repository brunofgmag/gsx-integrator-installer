#include <QtTest/QTest>

#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>

#include "../src/infrastructure/install/ExeXml.h"

namespace
{
    QString ReadAll(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }

        return QString::fromUtf8(file.readAll());
    }

    void WriteAll(const QString& path, const QString& content)
    {
        QFile file(path);

        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));

        file.write(content.toUtf8());
    }
}

class ExeXmlTest final : public QObject
{
    Q_OBJECT

private slots:
    static void addUpdateCreatesFileWithEntry();
    static void addUpdateUpdatesInsteadOfDuplicating();
    static void addUpdatePreservesOtherAddons();
    static void removeRemovesOnlyOurEntry();
    static void removeMissingFileIsSuccess();
};

void ExeXmlTest::addUpdateCreatesFileWithEntry()
{
    const QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("EXE.xml"));

    QVERIFY(ExeXmlAddUpdate(path, QStringLiteral("C:\\Apps\\gsx-integrator-client.exe"),
        QStringLiteral("GSX Integrator")));

    const QString content = ReadAll(path);

    QVERIFY(content.contains(QStringLiteral("SimBase.Document")));
    QVERIFY(content.contains(QStringLiteral("Launch.Addon")));
    QVERIFY(content.contains(QStringLiteral("gsx-integrator-client.exe")));
    QVERIFY(content.contains(QStringLiteral("GSX Integrator")));
    QVERIFY(content.contains(QStringLiteral("<CommandLine>--tray</CommandLine>")));
}

void ExeXmlTest::addUpdateUpdatesInsteadOfDuplicating()
{
    const QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("EXE.xml"));

    QVERIFY(ExeXmlAddUpdate(path, QStringLiteral("C:\\Old\\gsx-integrator-client.exe"),
        QStringLiteral("GSX Integrator")));
    QVERIFY(ExeXmlAddUpdate(path, QStringLiteral("C:\\New\\gsx-integrator-client.exe"),
        QStringLiteral("GSX Integrator")));

    const QString content = ReadAll(path);

    QCOMPARE(static_cast<int>(content.count(QStringLiteral("<Launch.Addon>"))), 1);
    QVERIFY(content.contains(QStringLiteral("C:\\New\\gsx-integrator-client.exe")));
    QVERIFY(!content.contains(QStringLiteral("C:\\Old\\gsx-integrator-client.exe")));
    QCOMPARE(static_cast<int>(content.count(QStringLiteral("<CommandLine>--tray</CommandLine>"))), 1);
}

void ExeXmlTest::addUpdatePreservesOtherAddons()
{
    const QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("EXE.xml"));

    WriteAll(path, QStringLiteral(
                 "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                 "<SimBase.Document Type=\"Launch\" version=\"1,0\">\n"
                 "  <Descr>Launch</Descr>\n"
                 "  <Launch.Addon>\n"
                 "    <Name>Other Tool</Name>\n"
                 "    <Path>C:\\Other\\other-tool.exe</Path>\n"
                 "  </Launch.Addon>\n"
                 "</SimBase.Document>\n"));

    QVERIFY(ExeXmlAddUpdate(path, QStringLiteral("C:\\Apps\\gsx-integrator-client.exe"),
        QStringLiteral("GSX Integrator")));

    const QString content = ReadAll(path);

    QVERIFY(content.contains(QStringLiteral("other-tool.exe")));
    QVERIFY(content.contains(QStringLiteral("gsx-integrator-client.exe")));
    QCOMPARE(static_cast<int>(content.count(QStringLiteral("<Launch.Addon>"))), 2);
}

void ExeXmlTest::removeRemovesOnlyOurEntry()
{
    const QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("EXE.xml"));

    WriteAll(path, QStringLiteral(
                 "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                 "<SimBase.Document Type=\"Launch\" version=\"1,0\">\n"
                 "  <Launch.Addon>\n"
                 "    <Name>Other Tool</Name>\n"
                 "    <Path>C:\\Other\\other-tool.exe</Path>\n"
                 "  </Launch.Addon>\n"
                 "  <Launch.Addon>\n"
                 "    <Name>GSX Integrator</Name>\n"
                 "    <Path>C:\\Apps\\gsx-integrator-client.exe</Path>\n"
                 "  </Launch.Addon>\n"
                 "</SimBase.Document>\n"));

    QVERIFY(ExeXmlRemove(path, QStringLiteral("gsx-integrator-client.exe")));

    const QString content = ReadAll(path);

    QVERIFY(content.contains(QStringLiteral("other-tool.exe")));
    QVERIFY(!content.contains(QStringLiteral("gsx-integrator-client.exe")));
    QCOMPARE(static_cast<int>(content.count(QStringLiteral("<Launch.Addon>"))), 1);
}

void ExeXmlTest::removeMissingFileIsSuccess()
{
    const QTemporaryDir dir;

    QVERIFY(ExeXmlRemove(dir.filePath(QStringLiteral("EXE.xml")),
        QStringLiteral("gsx-integrator-client.exe")));
}

QTEST_APPLESS_MAIN(ExeXmlTest)

#include "tst_exe_xml.moc"
