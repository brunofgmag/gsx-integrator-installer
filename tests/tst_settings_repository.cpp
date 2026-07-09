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
    QVERIFY(defaults.clientStartMenuShortcut);
    QVERIFY(defaults.setupShortcut);
    QVERIFY(defaults.disabledCommbusSims.isEmpty());

    InstallerSettings settings;
    settings.language = QStringLiteral("pt_BR");
    settings.desktopShortcut = false;
    settings.clientStartMenuShortcut = false;
    settings.setupShortcut = true;
    settings.disabledCommbusSims = {QStringLiteral("fs2020-steam")};
    repository.Save(settings);

    const InstallerSettings loaded = repository.Load();

    QCOMPARE(loaded.language, QStringLiteral("pt_BR"));
    QVERIFY(!loaded.desktopShortcut);
    QVERIFY(!loaded.clientStartMenuShortcut);
    QVERIFY(loaded.setupShortcut);
    QCOMPARE(loaded.disabledCommbusSims, QStringList{QStringLiteral("fs2020-steam")});
}

QTEST_GUILESS_MAIN(SettingsRepositoryTest)

#include "tst_settings_repository.moc"
