#include "QSettingsRepository.h"

#include <QtCore/QSettings>

namespace
{
    constexpr auto kLanguageKey = "ui/language";
    constexpr auto kSystem = "system";
    constexpr auto kDesktopShortcutKey = "install/desktopShortcut";
    constexpr auto kClientStartMenuShortcutKey = "install/clientStartMenuShortcut";
    constexpr auto kSetupShortcutKey = "install/setupShortcut";
    constexpr auto kDisabledCommbusSimsKey = "install/disabledCommbusSims";
}

InstallerSettings QSettingsRepository::Load() const
{
    const QSettings store;
    InstallerSettings settings;
    settings.language =
        store.value(QLatin1String(kLanguageKey), QLatin1String(kSystem)).toString();
    settings.desktopShortcut =
        store.value(QLatin1String(kDesktopShortcutKey), settings.desktopShortcut).toBool();
    settings.clientStartMenuShortcut =
        store.value(QLatin1String(kClientStartMenuShortcutKey), settings.clientStartMenuShortcut)
            .toBool();
    settings.setupShortcut =
        store.value(QLatin1String(kSetupShortcutKey), settings.setupShortcut).toBool();
    settings.disabledCommbusSims =
        store.value(QLatin1String(kDisabledCommbusSimsKey)).toStringList();

    return settings;
}

void QSettingsRepository::Save(const InstallerSettings& settings)
{
    QSettings store;
    store.setValue(QLatin1String(kLanguageKey), settings.language);
    store.setValue(QLatin1String(kDesktopShortcutKey), settings.desktopShortcut);
    store.setValue(QLatin1String(kClientStartMenuShortcutKey), settings.clientStartMenuShortcut);
    store.setValue(QLatin1String(kSetupShortcutKey), settings.setupShortcut);
    store.setValue(QLatin1String(kDisabledCommbusSimsKey), settings.disabledCommbusSims);
}
