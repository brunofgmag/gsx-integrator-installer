set(DOMAIN_SOURCES
        src/domain/model/ReleaseInfo.h
        src/domain/model/SimInstall.h
        src/domain/model/InstallRequest.h
        src/domain/model/InstallOutcome.h
        src/domain/model/UninstallOutcome.h
        src/domain/Versions.cpp
        src/domain/Versions.h
)

set(APPLICATION_SOURCES
        src/application/model/InstallerSettings.h
        src/application/ports/SettingsRepository.h
        src/application/ports/ReleaseProvider.h
        src/application/ports/SelfUpdateService.h
        src/application/ports/SystemInspector.h
        src/application/ports/InstallService.h
        src/application/ports/InstallerGateway.h
        src/application/install/InstallOrchestrator.cpp
        src/application/install/InstallOrchestrator.h
)

set(INFRASTRUCTURE_SOURCES
        src/infrastructure/settings/QSettingsRepository.cpp
        src/infrastructure/settings/QSettingsRepository.h
        src/infrastructure/github/GithubDownloader.cpp
        src/infrastructure/github/GithubDownloader.h
        src/infrastructure/github/GithubReleaseProvider.cpp
        src/infrastructure/github/GithubReleaseProvider.h
        src/infrastructure/github/ReleaseParser.cpp
        src/infrastructure/github/ReleaseParser.h
        src/infrastructure/system/UserCfg.cpp
        src/infrastructure/system/SimDetection.h
        src/infrastructure/system/InstallLocations.h
        src/infrastructure/install/DirCopy.cpp
        src/infrastructure/install/DirCopy.h
        src/infrastructure/install/ReleasePackage.cpp
        src/infrastructure/install/ReleasePackage.h
        src/infrastructure/install/PackageFiles.cpp
        src/infrastructure/install/PackageFiles.h
        src/infrastructure/install/EngineInstallService.cpp
        src/infrastructure/install/EngineInstallService.h
        src/infrastructure/platform/PlatformTypes.h
)

set(WINDOWS_INFRASTRUCTURE_SOURCES
        src/infrastructure/system/SimDetection.cpp
        src/infrastructure/system/InstallLocations.cpp
        src/infrastructure/system/WindowsSystemInspector.cpp
        src/infrastructure/system/WindowsSystemInspector.h
        src/infrastructure/install/ExeXml.cpp
        src/infrastructure/install/ExeXml.h
        src/infrastructure/install/RemovalScript.cpp
        src/infrastructure/install/RemovalScript.h
        src/infrastructure/install/SelfUpdateScript.cpp
        src/infrastructure/install/SelfUpdateScript.h
        src/infrastructure/install/WindowsInstallerGateway.cpp
        src/infrastructure/install/WindowsInstallerGateway.h
        src/infrastructure/install/WindowsSelfUpdateService.cpp
        src/infrastructure/install/WindowsSelfUpdateService.h
        src/infrastructure/platform/WindowsTitleBar.h
)

set(LINUX_INFRASTRUCTURE_SOURCES
        src/infrastructure/system/ChildEnvironment.cpp
        src/infrastructure/system/ChildEnvironment.h
        src/infrastructure/system/LinuxInstallLocations.cpp
        src/infrastructure/system/LinuxInstallLocations.h
        src/infrastructure/system/LinuxSimDetection.cpp
        src/infrastructure/system/LinuxSimDetection.h
        src/infrastructure/system/LinuxSystemInspector.cpp
        src/infrastructure/system/LinuxSystemInspector.h
        src/infrastructure/system/ProcessScan.cpp
        src/infrastructure/system/ProcessScan.h
        src/infrastructure/system/ProtonPrefix.cpp
        src/infrastructure/system/ProtonPrefix.h
        src/infrastructure/install/AppImageFiles.cpp
        src/infrastructure/install/AppImageFiles.h
        src/infrastructure/install/DesktopEntry.cpp
        src/infrastructure/install/DesktopEntry.h
        src/infrastructure/install/LinuxInstallerGateway.cpp
        src/infrastructure/install/LinuxInstallerGateway.h
        src/infrastructure/install/LinuxSelfUpdateService.cpp
        src/infrastructure/install/LinuxSelfUpdateService.h
        src/infrastructure/install/SimulatorWatcher.cpp
        src/infrastructure/install/SimulatorWatcher.h
)

if (WIN32)
    list(APPEND INFRASTRUCTURE_SOURCES ${WINDOWS_INFRASTRUCTURE_SOURCES})
else ()
    list(APPEND INFRASTRUCTURE_SOURCES ${LINUX_INFRASTRUCTURE_SOURCES})
endif ()

set(VIEWMODEL_SOURCES
        src/viewmodel/SetupViewModel.cpp
        src/viewmodel/SetupViewModel.h
)
