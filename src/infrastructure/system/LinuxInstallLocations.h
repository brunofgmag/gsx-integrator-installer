#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_LINUXINSTALLLOCATIONS_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_LINUXINSTALLLOCATIONS_H

#include <QtCore/QString>
#include <QtCore/QStringList>

inline constexpr auto kClientAppImageName = "gsx-integrator-client.AppImage";
inline constexpr auto kInstallerAppImageName = "gsx-integrator-installer.AppImage";
inline constexpr auto kClientInstallDirName = "gsx-integrator-client";
inline constexpr auto kMaintenanceDirName = "maintenance";
inline constexpr auto kInstallMetadataName = "install.ini";
inline constexpr auto kInstalledVersionKey = "DisplayVersion";
inline constexpr auto kWatcherScriptName = "gsx-integrator-watcher.sh";
inline constexpr auto kWatcherUnitName = "gsx-integrator-client.service";
inline constexpr auto kClientDesktopFileName = "gsx-integrator-client.desktop";
inline constexpr auto kSetupDesktopFileName = "gsx-integrator-installer.desktop";
inline constexpr auto kDesktopIconName = "gsx-integrator.png";

[[nodiscard]] QString DataHome();
[[nodiscard]] QString ConfigHome();

[[nodiscard]] QString MaintenanceDir();
[[nodiscard]] QString MaintenanceInstallerPath();
[[nodiscard]] QString InstallMetadataPath();
[[nodiscard]] QString DesktopIconPath();
[[nodiscard]] QString WatcherScriptPath();
[[nodiscard]] QString WatcherUnitDir();

[[nodiscard]] QString RunningAppImagePath();
[[nodiscard]] QStringList ClientProcessNames();

#endif
