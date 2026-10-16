#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_PROCESSSCAN_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_PROCESSSCAN_H

#include <QtCore/QString>
#include <QtCore/QStringList>

inline constexpr auto kProcRoot = "/proc";

[[nodiscard]] QString ArgumentBaseName(const QString& argument);

[[nodiscard]] bool IsAnyProcessRunning(const QString& procRoot, const QStringList& baseNames);

[[nodiscard]] bool IsProgramRunning(const QString& procRoot, const QStringList& baseNames);

#endif
