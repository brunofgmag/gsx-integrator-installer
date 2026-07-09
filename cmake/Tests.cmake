function(configure_gsxi_test TARGET_NAME TEST_NAME)
    target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Core Qt6::Test)
    target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
    add_test(NAME ${TEST_NAME} COMMAND ${TARGET_NAME})

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:Qt6::Core>"
            "$<TARGET_FILE:Qt6::Test>"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
            VERBATIM)
endfunction()

function(gsxi_add_qt_test TARGET_NAME TEST_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
    configure_gsxi_test(${TARGET_NAME} ${TEST_NAME})
endfunction()

gsxi_add_qt_test(gsxi-sim-detection-tests sim-detection
        tests/tst_sim_detection.cpp
        src/infrastructure/system/SimDetection.cpp
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
add_custom_command(TARGET gsxi-exe-xml-tests POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_FILE:Qt6::Xml>"
        "$<TARGET_FILE_DIR:gsxi-exe-xml-tests>"
        VERBATIM)

gsxi_add_qt_test(gsxi-removal-script-tests removal-script
        tests/tst_removal_script.cpp
        src/infrastructure/install/RemovalScript.cpp
        src/infrastructure/install/RemovalScript.h)

gsxi_add_qt_test(gsxi-dir-copy-tests dir-copy
        tests/tst_dir_copy.cpp
        src/infrastructure/install/DirCopy.cpp
        src/infrastructure/install/DirCopy.h)

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
add_custom_command(TARGET gsxi-translations-tests POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_FILE:Qt6::Xml>"
        "$<TARGET_FILE_DIR:gsxi-translations-tests>"
        VERBATIM)

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

get_target_property(GSXI_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
get_filename_component(GSXI_QT_BIN_DIR "${GSXI_QMAKE_EXECUTABLE}" DIRECTORY)
find_program(GSXI_LUPDATE_EXECUTABLE
        NAMES lupdate
        HINTS "${GSXI_QT_BIN_DIR}"
        NO_DEFAULT_PATH
        REQUIRED)

add_test(NAME translations-up-to-date
        COMMAND powershell -NoProfile -ExecutionPolicy Bypass
        -File "${CMAKE_SOURCE_DIR}/tools/check-translations.ps1"
        -Lupdate "${GSXI_LUPDATE_EXECUTABLE}"
        -SourceDir "${CMAKE_SOURCE_DIR}/src"
        -I18nDir "${CMAKE_SOURCE_DIR}/i18n")
