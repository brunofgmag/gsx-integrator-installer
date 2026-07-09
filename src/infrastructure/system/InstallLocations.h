#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_INSTALLLOCATIONS_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_INSTALLLOCATIONS_H

#include <QtCore/QString>

inline constexpr auto kUninstallKey =
    R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Uninstall\gsx-integrator-client)";
inline constexpr auto kCommbusPackageName = "gsx-integrator-commbus";
inline constexpr auto kShortcutName = "GSX Integrator.lnk";
inline constexpr auto kSetupShortcutName = "GSX Integrator Installer.lnk";
inline constexpr auto kClientExeName = "gsx-integrator-client.exe";
inline constexpr auto kInstallerExeName = "gsx-integrator-installer.exe";
inline constexpr auto kAppDisplayName = "GSX Integrator";

QString ClientInstallDir();
QString ClientExePath();
QString InstalledClientVersion();
bool IsProcessRunning(const QString& exeName);

QString StartMenuShortcutPath();
QString SetupStartMenuShortcutPath();
QString DesktopShortcutPath();

#endif
