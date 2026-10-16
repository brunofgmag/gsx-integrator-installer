#include <QtTest/QTest>

#include <QtCore/QCoreApplication>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>

#include "../src/infrastructure/settings/QSettingsRepository.h"

class SettingsRepositoryTest final : public QObject
{
    Q_OBJECT

private slots:
    static void roundTripsAllFields();
    static void readsTheClientMenuShortcutSavedUnderTheOldKey();
};

void SettingsRepositoryTest::roundTripsAllFields()
{
    const QTemporaryDir sandbox;

    QVERIFY(sandbox.isValid());

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, sandbox.path());
    QCoreApplication::setOrganizationName(QStringLiteral("gsxi-test"));
    QCoreApplication::setApplicationName(QStringLiteral("gsxi-test"));

    QSettingsRepository repository;
    const InstallerSettings defaults = repository.Load();

    QCOMPARE(defaults.language, QStringLiteral("system"));
    QVERIFY(defaults.desktopShortcut);
    QVERIFY(defaults.clientMenuShortcut);
    QVERIFY(defaults.setupShortcut);
    QVERIFY(defaults.disabledCommbusSims.isEmpty());

    InstallerSettings settings;
    settings.language = QStringLiteral("pt_BR");
    settings.desktopShortcut = false;
    settings.clientMenuShortcut = false;
    settings.setupShortcut = true;
    settings.disabledCommbusSims = {QStringLiteral("fs2020-steam")};
    repository.Save(settings);

    const InstallerSettings loaded = repository.Load();

    QCOMPARE(loaded.language, QStringLiteral("pt_BR"));
    QVERIFY(!loaded.desktopShortcut);
    QVERIFY(!loaded.clientMenuShortcut);
    QVERIFY(loaded.setupShortcut);
    QCOMPARE(loaded.disabledCommbusSims, QStringList{QStringLiteral("fs2020-steam")});
}

void SettingsRepositoryTest::readsTheClientMenuShortcutSavedUnderTheOldKey()
{
    const QTemporaryDir sandbox;

    QVERIFY(sandbox.isValid());

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, sandbox.path());
    QCoreApplication::setOrganizationName(QStringLiteral("gsxi-test-keys"));
    QCoreApplication::setApplicationName(QStringLiteral("gsxi-test-keys"));

    QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("gsxi-test-keys"),
              QStringLiteral("gsxi-test-keys"))
        .setValue(QStringLiteral("install/clientStartMenuShortcut"), false);

    QVERIFY(!QSettingsRepository().Load().clientMenuShortcut);

    InstallerSettings settings;
    settings.clientMenuShortcut = true;
    QSettingsRepository().Save(settings);

    const QSettings store(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("gsxi-test-keys"),
                          QStringLiteral("gsxi-test-keys"));
    QVERIFY(store.value(QStringLiteral("install/clientStartMenuShortcut")).toBool());
}

QTEST_GUILESS_MAIN(SettingsRepositoryTest)

#include "tst_settings_repository.moc"
