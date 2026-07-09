qt_add_executable(${APP_NAME}
        assets/app.rc
        assets/resources.qrc
        src/main.cpp
        ${DOMAIN_SOURCES}
        ${APPLICATION_SOURCES}
        ${INFRASTRUCTURE_SOURCES}
        ${VIEWMODEL_SOURCES}
)

set_source_files_properties(src/qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(${APP_NAME}
        URI GsxIntegratorInstaller
        VERSION 1.0
        QML_FILES
        src/qml/Main.qml
        src/qml/Theme.qml
        src/qml/components/ActionButton.qml
        src/qml/components/CheckRow.qml
        src/qml/components/LanguagePicker.qml
        src/qml/components/LinkText.qml
        src/qml/components/ProgressTrack.qml
        src/qml/components/RadioRow.qml
        src/qml/components/StatusRow.qml
        src/qml/components/UpdateBanner.qml
)

qt_add_translations(${APP_NAME}
        TS_FILES i18n/app_en.ts i18n/app_pt_BR.ts
        RESOURCE_PREFIX "/i18n"
)

target_include_directories(${APP_NAME} PRIVATE src)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/VERSION.txt")
file(READ "${CMAKE_SOURCE_DIR}/VERSION.txt" GSXI_INSTALLER_VERSION)
string(STRIP "${GSXI_INSTALLER_VERSION}" GSXI_INSTALLER_VERSION)

target_compile_definitions(${APP_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        GSXI_INSTALLER_VERSION="${GSXI_INSTALLER_VERSION}"
        $<$<CONFIG:Release>:NDEBUG>
)

if (MSVC)
    target_compile_options(${APP_NAME} PRIVATE /W4 /permissive- /Zc:preprocessor /external:W0)
endif ()

target_link_libraries(${APP_NAME} PRIVATE
        Qt6::Quick
        Qt6::Network
        Qt6::Xml
        dwmapi
        ole32
        uuid
        advapi32
)
