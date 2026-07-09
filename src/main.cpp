#include <cstring>

#include <windows.h>

#include <QtCore/QFile>
#include <QtCore/QLocale>
#include <QtCore/QSize>
#include <QtCore/QTranslator>
#include <QtGui/QFont>
#include <QtGui/QGuiApplication>
#include <QtGui/QIcon>
#include <QtGui/QStyleHints>
#include <QtQml/QQmlApplicationEngine>
#include <QtQuick/QQuickWindow>

#include "application/install/InstallOrchestrator.h"
#include "infrastructure/github/GithubReleaseProvider.h"
#include "infrastructure/install/EngineInstallService.h"
#include "infrastructure/install/WindowsInstallerGateway.h"
#include "infrastructure/install/WindowsSelfUpdateService.h"
#include "infrastructure/platform/WindowsTitleBar.h"
#include "infrastructure/settings/QSettingsRepository.h"
#include "infrastructure/system/WindowsSystemInspector.h"
#include "viewmodel/SetupViewModel.h"

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--uninstall") == 0)
        {
            const QCoreApplication app(argc, argv);
            const int choice = MessageBoxW(
                nullptr,
                L"Remove GSX Integrator, the CommBus module and the EXE.xml auto-start entries "
                L"from all simulators?",
                L"GSX Integrator", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
            if (choice != IDYES)
            {
                return 0;
            }

            WindowsInstallerGateway gateway;
            if (gateway.IsClientRunning())
            {
                MessageBoxW(nullptr, L"Close GSX Integrator first, then run the uninstaller again.",
                            L"GSX Integrator", MB_OK | MB_ICONWARNING);
                return 1;
            }

            InstallOrchestrator(gateway).RunUninstall();
            MessageBoxW(nullptr, L"GSX Integrator was removed.", L"GSX Integrator",
                        MB_OK | MB_ICONINFORMATION);
            return 0;
        }
    }

    const QGuiApplication app(argc, argv);

    QGuiApplication::setOrganizationName(QStringLiteral("brunofgmag"));
    QGuiApplication::setApplicationName(QStringLiteral("gsx-integrator-installer"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("GSX Integrator Installer"));
    QGuiApplication::setApplicationVersion(QStringLiteral(GSXI_INSTALLER_VERSION));

    QFont monoFont(QStringLiteral("Cascadia Mono"));
    monoFont.setStyleHint(QFont::Monospace);
    QGuiApplication::setFont(monoFont);

    QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);

    QSettingsRepository settingsRepository;

    QTranslator translator;
    const auto applyLanguage = [&translator, &settingsRepository]
    {
        QCoreApplication::removeTranslator(&translator);
        const QString language = settingsRepository.Load().language;
        const QLocale locale = (language.isEmpty() || language == QLatin1String("system"))
                                   ? QLocale()
                                   : QLocale(language);
        if (translator.load(locale, QStringLiteral("app"), QStringLiteral("_"),
                            QStringLiteral(":/i18n")))
        {
            QCoreApplication::installTranslator(&translator);
        }
    };

    applyLanguage();

    QIcon appIcon;
    for (const int size : {16, 24, 32, 48, 64, 128, 256})
    {
        appIcon.addFile(QStringLiteral(":/icons/app-icon_%1.png").arg(size), QSize(size, size));
    }

    QGuiApplication::setWindowIcon(appIcon);

    WindowsSystemInspector inspector;
    GithubReleaseProvider releaseProvider;
    EngineInstallService installService;
    WindowsSelfUpdateService selfUpdateService;
    SetupViewModel controller(&settingsRepository, &releaseProvider, &inspector, &installService,
                              &selfUpdateService);

    controller.refresh();

    QQmlApplicationEngine engine;
    engine.setInitialProperties({
        {QStringLiteral("controller"), QVariant::fromValue(&controller)},
    });

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);

    QObject::connect(&controller, &SetupViewModel::LanguageChanged, &app,
                     [&applyLanguage, &engine, &controller]
                     {
                         applyLanguage();
                         engine.retranslate();
                         controller.RefreshTranslations();
                     });

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/GsxIntegratorInstaller/src/qml/Main.qml")));
    if (engine.rootObjects().isEmpty())
    {
        return EXIT_FAILURE;
    }

    if (auto* rootWindow = qobject_cast<QWindow*>(engine.rootObjects().first()))
    {
        const auto applyTitleBar = [rootWindow]
        {
            WindowsTitleBar::Apply(
                rootWindow, QGuiApplication::styleHints()->colorScheme() != Qt::ColorScheme::Light);
        };

        applyTitleBar();

        QObject::connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
                         rootWindow, applyTitleBar);
    }

    return QGuiApplication::exec();
}
