#include <cstring>

#include <QtCore/QtGlobal>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <cstdio>

#include <unistd.h>

#include <QtCore/QLockFile>
#endif

#include <QtCore/QDir>
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
#include <QtQuick/QSGRendererInterface>

#include "application/install/InstallOrchestrator.h"
#include "infrastructure/github/GithubReleaseProvider.h"
#include "infrastructure/install/EngineInstallService.h"
#include "infrastructure/platform/PlatformTypes.h"
#include "infrastructure/settings/QSettingsRepository.h"
#include "viewmodel/SetupViewModel.h"

#ifdef Q_OS_WIN
#include "infrastructure/platform/WindowsTitleBar.h"
#endif

namespace
{
    constexpr int kContinueInWindow = -1;

#ifdef Q_OS_WIN
    bool AcquireSingleInstance()
    {
        CreateMutexW(nullptr, FALSE, L"Local\\gsx-integrator-installer.single-instance");
        return GetLastError() != ERROR_ALREADY_EXISTS;
    }

    int RunUninstallCommand(int argc, char* argv[])
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

        const UninstallOutcome outcome = InstallOrchestrator(gateway).RunUninstall();
        if (!outcome.ok())
        {
            MessageBoxW(nullptr, SetupViewModel::UninstallErrorText(outcome).toStdWString().c_str(),
                        L"GSX Integrator", MB_OK | MB_ICONWARNING);
            return 1;
        }

        MessageBoxW(nullptr, L"GSX Integrator was removed.", L"GSX Integrator",
                    MB_OK | MB_ICONINFORMATION);
        return 0;
    }
#else
    QLockFile& InstanceLock()
    {
        static QLockFile lock(
            (qEnvironmentVariableIsEmpty("XDG_RUNTIME_DIR") ? QDir::tempPath()
                                                            : qEnvironmentVariable("XDG_RUNTIME_DIR"))
            + QStringLiteral("/gsx-integrator-installer-%1.lock").arg(getuid()));
        return lock;
    }

    bool AcquireSingleInstance()
    {
        InstanceLock().setStaleLockTime(0);
        return InstanceLock().tryLock(0);
    }

    enum class UninstallAnswer
    {
        Yes,
        No,
        AskInWindow,
    };

    UninstallAnswer ConfirmUninstall(int argc, char* argv[])
    {
        for (int i = 1; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--yes") == 0)
            {
                return UninstallAnswer::Yes;
            }
        }

        if (!isatty(STDIN_FILENO))
        {
            return UninstallAnswer::AskInWindow;
        }

        std::fputs("Remove GSX Integrator, the CommBus module and the auto-start service "
                   "from all simulators? [y/N] ", stdout);
        std::fflush(stdout);

        char buffer[16] = {};
        if (!std::fgets(buffer, sizeof(buffer), stdin))
        {
            return UninstallAnswer::No;
        }

        return buffer[0] == 'y' || buffer[0] == 'Y' ? UninstallAnswer::Yes : UninstallAnswer::No;
    }

    int RunUninstallCommand(int argc, char* argv[])
    {
        switch (ConfirmUninstall(argc, argv))
        {
        case UninstallAnswer::Yes:
            break;
        case UninstallAnswer::No:
            return 0;
        case UninstallAnswer::AskInWindow:
            return kContinueInWindow;
        }

        const QCoreApplication app(argc, argv);
        LinuxInstallerGateway gateway;
        if (gateway.IsClientRunning())
        {
            std::fputs("Close GSX Integrator first, then run the uninstaller again.\n", stderr);
            return 1;
        }

        const UninstallOutcome outcome = InstallOrchestrator(gateway).RunUninstall();
        if (!outcome.ok())
        {
            std::fprintf(stderr, "%s\n", SetupViewModel::UninstallErrorText(outcome).toUtf8().constData());
            return 1;
        }

        std::fputs("GSX Integrator was removed.\n", stdout);
        return 0;
    }
#endif
}

int main(int argc, char* argv[])
{
    if (!AcquireSingleInstance())
    {
        return 0;
    }

    bool confirmUninstallInWindow = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--uninstall") == 0)
        {
            const int exitCode = RunUninstallCommand(argc, argv);
            if (exitCode != kContinueInWindow)
            {
                return exitCode;
            }

            confirmUninstallInWindow = true;
            break;
        }
    }

    const QGuiApplication app(argc, argv);

    QGuiApplication::setOrganizationName(QStringLiteral("brunofgmag"));
    QGuiApplication::setApplicationName(QStringLiteral("gsx-integrator-installer"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("GSX Integrator Installer"));
    QGuiApplication::setApplicationVersion(QStringLiteral(GSXI_INSTALLER_VERSION));
#ifdef Q_OS_LINUX
    QGuiApplication::setDesktopFileName(QStringLiteral("gsx-integrator-installer"));
#endif

    QFont monoFont(QStringLiteral("Cascadia Mono"));
    monoFont.setStyleHint(QFont::Monospace);
    QGuiApplication::setFont(monoFont);

    if (!qEnvironmentVariableIsSet("QSG_RHI_BACKEND")
        && !qEnvironmentVariableIsSet("QT_QUICK_BACKEND"))
    {
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
    }

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

    PlatformSystemInspector inspector;
    GithubReleaseProvider releaseProvider;
    EngineInstallService installService;
    PlatformSelfUpdateService selfUpdateService;
    SetupViewModel controller(&settingsRepository, &releaseProvider, &inspector, &installService,
                              &selfUpdateService);

    if (confirmUninstallInWindow)
    {
        controller.BeginUninstallConfirmation();
    }
    else
    {
        controller.refresh();
    }

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

#ifdef Q_OS_WIN
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
#endif

    return QGuiApplication::exec();
}
