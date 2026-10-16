#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_DESKTOPENTRY_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_DESKTOPENTRY_H

#include <QtCore/QString>

struct DesktopEntry
{
    QString name;
    QString comment;
    QString execPath;
    QString iconPath;
    QString startupWmClass;
    bool tryExec = false;
    bool hidden = false;
};

[[nodiscard]] QString QuoteDesktopExec(const QString& path);

[[nodiscard]] QString BuildDesktopEntry(const DesktopEntry& entry);

#endif
