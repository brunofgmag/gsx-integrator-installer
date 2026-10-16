#include <QtTest/QTest>

#include "../src/infrastructure/install/DesktopEntry.h"

class DesktopEntryTest final : public QObject
{
    Q_OBJECT

private slots:
    static void plainPathIsQuoted();
    static void spacesStayInsideTheQuotes();
    static void reservedCharactersAreEscapedTwice();
    static void percentSignsAreDoubled();
    static void entryHasTheKeysALauncherNeeds();
    static void entryWithoutIconOmitsTheKey();
    static void plainEntryCarriesNoneOfTheLauncherOnlyKeys();
    static void launcherOnlyEntryHidesItselfAndNamesItsWindowClass();
};

void DesktopEntryTest::plainPathIsQuoted()
{
    QCOMPARE(QuoteDesktopExec(QStringLiteral("/home/u/app.AppImage")), QStringLiteral("\"/home/u/app.AppImage\""));
}

void DesktopEntryTest::spacesStayInsideTheQuotes()
{
    QCOMPARE(QuoteDesktopExec(QStringLiteral("/home/u/My Apps/app.AppImage")),
             QStringLiteral("\"/home/u/My Apps/app.AppImage\""));
}

void DesktopEntryTest::reservedCharactersAreEscapedTwice()
{
    QCOMPARE(QuoteDesktopExec(QStringLiteral("/a/$b`c\"d\\e")),
             QStringLiteral("\"/a/\\\\$b\\\\`c\\\\\"d\\\\\\\\e\""));
}

void DesktopEntryTest::percentSignsAreDoubled()
{
    QCOMPARE(QuoteDesktopExec(QStringLiteral("/a/100%/x")), QStringLiteral("\"/a/100%%/x\""));
}

void DesktopEntryTest::entryHasTheKeysALauncherNeeds()
{
    const QString text = BuildDesktopEntry({QStringLiteral("GSX Integrator"), QStringLiteral("Helps"),
                                            QStringLiteral("/home/u/gsx-integrator-client.AppImage"),
                                            QStringLiteral("/home/u/gsx-integrator.png")});

    QVERIFY(text.startsWith(QStringLiteral("[Desktop Entry]\n")));
    QVERIFY(text.contains(QStringLiteral("\nType=Application\n")));
    QVERIFY(text.contains(QStringLiteral("\nName=GSX Integrator\n")));
    QVERIFY(text.contains(QStringLiteral("\nExec=\"/home/u/gsx-integrator-client.AppImage\"\n")));
    QVERIFY(text.contains(QStringLiteral("\nIcon=/home/u/gsx-integrator.png\n")));
    QVERIFY(text.contains(QStringLiteral("\nTerminal=false\n")));
}

void DesktopEntryTest::entryWithoutIconOmitsTheKey()
{
    const QString text = BuildDesktopEntry({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("/x"), {}});

    QVERIFY(!text.contains(QStringLiteral("Icon=")));
}

void DesktopEntryTest::plainEntryCarriesNoneOfTheLauncherOnlyKeys()
{
    const QString text = BuildDesktopEntry({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("/x"), {}});

    QVERIFY(!text.contains(QStringLiteral("TryExec=")));
    QVERIFY(!text.contains(QStringLiteral("StartupWMClass=")));
    QVERIFY(!text.contains(QStringLiteral("NoDisplay=")));
}

void DesktopEntryTest::launcherOnlyEntryHidesItselfAndNamesItsWindowClass()
{
    DesktopEntry entry;
    entry.name = QStringLiteral("A");
    entry.comment = QStringLiteral("B");
    entry.execPath = QStringLiteral("/x y");
    entry.startupWmClass = QStringLiteral("app-id");
    entry.tryExec = true;
    entry.hidden = true;

    const QString text = BuildDesktopEntry(entry);

    QVERIFY(text.contains(QStringLiteral("\nTryExec=/x y\n")));
    QVERIFY(text.contains(QStringLiteral("\nStartupWMClass=app-id\n")));
    QVERIFY(text.contains(QStringLiteral("\nNoDisplay=true\n")));
}

QTEST_APPLESS_MAIN(DesktopEntryTest)

#include "tst_desktop_entry.moc"
