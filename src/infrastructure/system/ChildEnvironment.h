#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_CHILDENVIRONMENT_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_SYSTEM_CHILDENVIRONMENT_H

#include <QtCore/QProcessEnvironment>

[[nodiscard]] QProcessEnvironment ChildProcessEnvironment(const QProcessEnvironment& environment);

#endif
