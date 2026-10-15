function(configure_gsxi_test TARGET_NAME TEST_NAME)
    target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Core Qt6::Test)
    target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
    add_test(NAME ${TEST_NAME} COMMAND ${TARGET_NAME})

    if (WIN32)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_FILE:Qt6::Core>"
                "$<TARGET_FILE:Qt6::Test>"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                VERBATIM)
    endif ()
endfunction()

function(gsxi_copy_qt_xml TARGET_NAME)
    if (WIN32)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_FILE:Qt6::Xml>"
                "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                VERBATIM)
    endif ()
endfunction()

function(gsxi_add_qt_test TARGET_NAME TEST_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
    configure_gsxi_test(${TARGET_NAME} ${TEST_NAME})
endfunction()

gsxi_add_qt_test(gsxi-sim-detection-tests sim-detection
        tests/tst_sim_detection.cpp
        src/infrastructure/system/UserCfg.cpp
        src/infrastructure/system/SimDetection.h)

gsxi_add_qt_test(gsxi-versions-tests versions
        tests/tst_versions.cpp
        src/domain/Versions.cpp
        src/domain/Versions.h)

gsxi_add_qt_test(gsxi-release-parser-tests release-parser
        tests/tst_release_parser.cpp
        src/infrastructure/github/ReleaseParser.cpp
        src/infrastructure/github/ReleaseParser.h
        src/domain/Versions.cpp
        src/domain/Versions.h)

gsxi_add_qt_test(gsxi-exe-xml-tests exe-xml
        tests/tst_exe_xml.cpp
        src/infrastructure/install/ExeXml.cpp
        src/infrastructure/install/ExeXml.h)
target_link_libraries(gsxi-exe-xml-tests PRIVATE Qt6::Xml)
gsxi_copy_qt_xml(gsxi-exe-xml-tests)

if (WIN32)
    gsxi_add_qt_test(gsxi-removal-script-tests removal-script
            tests/tst_removal_script.cpp
            src/infrastructure/install/RemovalScript.cpp
            src/infrastructure/install/RemovalScript.h)

    gsxi_add_qt_test(gsxi-self-update-script-tests self-update-script
            tests/tst_self_update_script.cpp
            src/infrastructure/install/SelfUpdateScript.cpp
            src/infrastructure/install/SelfUpdateScript.h)
endif ()

gsxi_add_qt_test(gsxi-dir-copy-tests dir-copy
        tests/tst_dir_copy.cpp
        src/infrastructure/install/DirCopy.cpp
        src/infrastructure/install/DirCopy.h)

gsxi_add_qt_test(gsxi-release-package-tests release-package
        tests/tst_release_package.cpp
        src/infrastructure/install/ReleasePackage.cpp
        src/infrastructure/install/ReleasePackage.h)
target_link_libraries(gsxi-release-package-tests PRIVATE miniz)

gsxi_add_qt_test(gsxi-install-orchestrator-tests install-orchestrator
        tests/doubles/FakeInstallerGateway.h
        tests/tst_install_orchestrator.cpp
        src/application/install/InstallOrchestrator.cpp
        src/application/install/InstallOrchestrator.h)

gsxi_add_qt_test(gsxi-settings-repository-tests settings-repository
        tests/tst_settings_repository.cpp
        src/infrastructure/settings/QSettingsRepository.cpp
        src/infrastructure/settings/QSettingsRepository.h)

gsxi_add_qt_test(gsxi-translations-tests translations
        tests/tst_translations.cpp)
target_link_libraries(gsxi-translations-tests PRIVATE Qt6::Xml)
target_compile_definitions(gsxi-translations-tests PRIVATE
        GSXI_I18N_DIR="${CMAKE_SOURCE_DIR}/i18n")
gsxi_copy_qt_xml(gsxi-translations-tests)

gsxi_add_qt_test(gsxi-setup-viewmodel-tests setup-viewmodel
        tests/doubles/FakeInstallService.h
        tests/doubles/FakeReleaseProvider.h
        tests/doubles/FakeSelfUpdateService.h
        tests/doubles/FakeSettingsRepository.h
        tests/doubles/FakeSystemInspector.h
        tests/tst_setup_viewmodel.cpp
        src/viewmodel/SetupViewModel.cpp
        src/viewmodel/SetupViewModel.h
        src/domain/Versions.cpp
        src/domain/Versions.h)
target_compile_definitions(gsxi-setup-viewmodel-tests PRIVATE GSXI_INSTALLER_VERSION="test")

if (NOT WIN32)
    set(GSXI_LINUX_PROTON_SOURCES
            src/infrastructure/system/ProtonPrefix.cpp
            src/infrastructure/system/ProtonPrefix.h)

    set(GSXI_LINUX_PLATFORM_SOURCES
            ${GSXI_LINUX_PROTON_SOURCES}
            src/application/install/InstallOrchestrator.cpp
            src/application/install/InstallOrchestrator.h
            src/domain/Versions.cpp
            src/domain/Versions.h
            src/infrastructure/github/GithubDownloader.cpp
            src/infrastructure/github/GithubDownloader.h
            src/infrastructure/install/AppImageFiles.cpp
            src/infrastructure/install/AppImageFiles.h
            src/infrastructure/install/DesktopEntry.cpp
            src/infrastructure/install/DesktopEntry.h
            src/infrastructure/install/LinuxInstallerGateway.cpp
            src/infrastructure/install/LinuxInstallerGateway.h
            src/infrastructure/install/PackageFiles.cpp
            src/infrastructure/install/PackageFiles.h
            src/infrastructure/install/ReleasePackage.cpp
            src/infrastructure/install/ReleasePackage.h
            src/infrastructure/install/SimulatorWatcher.cpp
            src/infrastructure/install/SimulatorWatcher.h
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
            src/infrastructure/system/UserCfg.cpp)

    function(gsxi_add_linux_test TARGET_NAME TEST_NAME)
        gsxi_add_qt_test(${TARGET_NAME} ${TEST_NAME} ${ARGN})
        target_link_libraries(${TARGET_NAME} PRIVATE miniz)
    endfunction()

    gsxi_add_linux_test(gsxi-proton-prefix-tests proton-prefix
            tests/tst_proton_prefix.cpp
            tests/support/FakeSteamTree.h
            tests/support/FileTree.h
            ${GSXI_LINUX_PROTON_SOURCES})

    gsxi_add_linux_test(gsxi-process-scan-tests process-scan
            tests/tst_process_scan.cpp
            tests/support/FileTree.h
            src/infrastructure/system/ProcessScan.cpp
            src/infrastructure/system/ProcessScan.h)

    gsxi_add_linux_test(gsxi-linux-sim-detection-tests linux-sim-detection
            tests/tst_linux_sim_detection.cpp
            tests/support/FakeSteamTree.h
            tests/support/FileTree.h
            ${GSXI_LINUX_PROTON_SOURCES}
            src/infrastructure/system/LinuxSimDetection.cpp
            src/infrastructure/system/LinuxSimDetection.h
            src/infrastructure/system/UserCfg.cpp
            src/infrastructure/system/SimDetection.h)

    gsxi_add_linux_test(gsxi-child-environment-tests child-environment
            tests/tst_child_environment.cpp
            tests/support/FileTree.h
            src/infrastructure/system/ChildEnvironment.cpp
            src/infrastructure/system/ChildEnvironment.h)

    gsxi_add_linux_test(gsxi-desktop-entry-tests desktop-entry
            tests/tst_desktop_entry.cpp
            tests/support/FileTree.h
            src/infrastructure/install/DesktopEntry.cpp
            src/infrastructure/install/DesktopEntry.h)

    gsxi_add_linux_test(gsxi-app-image-files-tests app-image-files
            tests/tst_app_image_files.cpp
            tests/support/FileTree.h
            src/infrastructure/install/AppImageFiles.cpp
            src/infrastructure/install/AppImageFiles.h)

    gsxi_add_linux_test(gsxi-simulator-watcher-tests simulator-watcher
            tests/tst_simulator_watcher.cpp
            tests/support/FileTree.h
            src/infrastructure/install/SimulatorWatcher.cpp
            src/infrastructure/install/SimulatorWatcher.h
            src/infrastructure/system/LinuxInstallLocations.h)

    gsxi_add_linux_test(gsxi-linux-installer-gateway-tests linux-installer-gateway
            tests/tst_linux_installer_gateway.cpp
            tests/support/FakeSteamTree.h
            tests/support/FileTree.h
            ${GSXI_LINUX_PLATFORM_SOURCES})
    target_link_libraries(gsxi-linux-installer-gateway-tests PRIVATE Qt6::Network)
endif ()

get_target_property(GSXI_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
get_filename_component(GSXI_QT_BIN_DIR "${GSXI_QMAKE_EXECUTABLE}" DIRECTORY)
find_program(GSXI_LUPDATE_EXECUTABLE
        NAMES lupdate
        HINTS "${GSXI_QT_BIN_DIR}"
        NO_DEFAULT_PATH
        REQUIRED)

if (WIN32)
    add_test(NAME translations-up-to-date
            COMMAND powershell -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/check-translations.ps1"
            -Lupdate "${GSXI_LUPDATE_EXECUTABLE}"
            -SourceDir "${CMAKE_SOURCE_DIR}/src"
            -I18nDir "${CMAKE_SOURCE_DIR}/i18n")
else ()
    add_test(NAME translations-up-to-date
            COMMAND "${CMAKE_COMMAND}"
            -DLUPDATE=${GSXI_LUPDATE_EXECUTABLE}
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}/src
            -DI18N_DIR=${CMAKE_SOURCE_DIR}/i18n
            -P "${CMAKE_SOURCE_DIR}/tools/check-translations.cmake")
endif ()
