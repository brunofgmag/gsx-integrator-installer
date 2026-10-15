#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_DESKTOPENTRY_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_DESKTOPENTRY_H

#include <QtCore/QString>

struct DesktopEntry
{
    QString name;
    QString comment;
    QString execPath;
    QString iconPath;
};

[[nodiscard]] QString QuoteDesktopExec(const QString& path);

[[nodiscard]] QString BuildDesktopEntry(const DesktopEntry& entry);

#endif
