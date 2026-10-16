#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_SIMULATORWATCHER_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_SIMULATORWATCHER_H

#include <QtCore/QString>
#include <QtCore/QStringList>

struct WatcherFiles
{
    QString unit;
    QString script;
};

[[nodiscard]] WatcherFiles BuildWatcherFiles(const QString& scriptPath, const QStringList& processNames);

[[nodiscard]] QStringList ParseWatchedProcesses(const QString& unitText);

class SimulatorWatcher
{
public:
    SimulatorWatcher(QString scriptPath, QString unitDir, QString systemctl = QStringLiteral("systemctl"));

    [[nodiscard]] bool SystemdAvailable() const;
    [[nodiscard]] QStringList WatchedProcesses() const;
    [[nodiscard]] bool Add(const QString& processName);
    [[nodiscard]] bool Remove(const QString& processName);
    [[nodiscard]] bool Disable();

private:
    [[nodiscard]] QString UnitPath() const;
    [[nodiscard]] bool Apply(const QStringList& processNames);
    [[nodiscard]] bool Systemctl(const QStringList& arguments) const;

    QString scriptPath_;
    QString unitDir_;
    QString systemctl_;
};

#endif
