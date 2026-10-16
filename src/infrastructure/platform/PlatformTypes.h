#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_PLATFORM_PLATFORMTYPES_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_PLATFORM_PLATFORMTYPES_H

#include <QtCore/QtGlobal>

#ifdef Q_OS_WIN
#include "../install/WindowsInstallerGateway.h"
#include "../install/WindowsSelfUpdateService.h"
#include "../system/WindowsSystemInspector.h"

using PlatformInstallerGateway = WindowsInstallerGateway;
using PlatformSelfUpdateService = WindowsSelfUpdateService;
using PlatformSystemInspector = WindowsSystemInspector;
#else
#include "../install/LinuxInstallerGateway.h"
#include "../install/LinuxSelfUpdateService.h"
#include "../system/LinuxSystemInspector.h"

using PlatformInstallerGateway = LinuxInstallerGateway;
using PlatformSelfUpdateService = LinuxSelfUpdateService;
using PlatformSystemInspector = LinuxSystemInspector;
#endif

#endif
