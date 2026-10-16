#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_PROTONPREFIX_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_PROTONPREFIX_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QStringList>

inline constexpr int kMsfs2020AppId = 1250410;
inline constexpr int kMsfs2024AppId = 2537590;
inline constexpr auto kProtonUserName = "steamuser";

enum class ProtonKnownFolder
{
    Profile,
    AppDataRoaming,
    AppDataLocal,
};

[[nodiscard]] QStringList DefaultSteamappsDirs(const QString& home);

[[nodiscard]] QStringList ParseLibraryFolders(const QByteArray& libraryFoldersVdf);

[[nodiscard]] QStringList SteamLibrarySteamappsDirs(const QStringList& steamappsDirs);

[[nodiscard]] QString FindProtonPrefix(const QStringList& steamappsDirs, int appId);

[[nodiscard]] QString ResolveIgnoringCase(const QString& baseDir, const QString& relativePath);

[[nodiscard]] QString WindowsPathToHost(const QString& prefixDir, const QString& windowsPath);

[[nodiscard]] QString ProtonKnownFolderPath(const QString& prefixDir, ProtonKnownFolder folder);

#endif
