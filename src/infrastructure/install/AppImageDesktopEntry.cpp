#include "AppImageDesktopEntry.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "../system/LinuxInstallLocations.h"
#include "DesktopEntry.h"

namespace
{
    constexpr auto kWindowClass = "gsx-integrator-installer";

    bool WriteFileMakingFolders(const QString& path, const QByteArray& bytes)
    {
        if (!QDir().mkpath(QFileInfo(path).absolutePath()))
        {
            return false;
        }

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            return false;
        }

        return file.write(bytes) == bytes.size();
    }

    QByteArray ReadWholeFile(const QString& path)
    {
        QFile file(path);

        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    }

    bool IsOurHiddenEntry(const QByteArray& content)
    {
        return content.contains("\nNoDisplay=true\n");
    }

    QByteArray BuildWindowIconEntry(const QString& appImagePath, const QString& iconPath)
    {
        DesktopEntry entry;
        entry.name = QStringLiteral("GSX Integrator Installer");
        entry.comment = QStringLiteral("Update or uninstall GSX Integrator");
        entry.execPath = appImagePath;
        entry.iconPath = iconPath;
        entry.startupWmClass = QLatin1String(kWindowClass);
        entry.tryExec = true;
        entry.hidden = true;

        return BuildDesktopEntry(entry).toUtf8();
    }

    bool EnsureIcon(const QString& iconPath, const QByteArray& iconBytes)
    {
        if (ReadWholeFile(iconPath) == iconBytes)
        {
            return true;
        }

        return WriteFileMakingFolders(iconPath, iconBytes);
    }
}

bool EnsureWindowIconEntry(const QString& appImagePath, const QString& dataHome, const QByteArray& iconBytes)
{
    const QString desktopPath =
        dataHome + QStringLiteral("/applications/") + QLatin1String(kSetupDesktopFileName);
    if (iconBytes.isEmpty() || !QFileInfo(appImagePath).isFile())
    {
        return false;
    }

    if (QFile::exists(desktopPath) && !IsOurHiddenEntry(ReadWholeFile(desktopPath)))
    {
        return false;
    }

    const QString iconPath = WindowIconPath(dataHome, QLatin1String(kSetupWindowIconName));
    if (!EnsureIcon(iconPath, iconBytes))
    {
        return false;
    }

    const QByteArray wanted = BuildWindowIconEntry(appImagePath, iconPath);

    return ReadWholeFile(desktopPath) == wanted || WriteFileMakingFolders(desktopPath, wanted);
}
