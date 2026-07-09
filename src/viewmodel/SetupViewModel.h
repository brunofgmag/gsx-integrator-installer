#ifndef GSX_INTEGRATOR_INSTALLER_VIEWMODEL_SETUPVIEWMODEL_H
#define GSX_INTEGRATOR_INSTALLER_VIEWMODEL_SETUPVIEWMODEL_H

#include <functional>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QVariantList>
#include "application/ports/InstallService.h"
#include "application/ports/ReleaseProvider.h"
#include "application/ports/SelfUpdateService.h"
#include "application/ports/SettingsRepository.h"
#include "application/ports/SystemInspector.h"
#include "domain/model/ReleaseInfo.h"
#include "domain/model/SimInstall.h"

class SetupViewModel final : public QObject,
                             public ReleaseProviderObserver,
                             public InstallServiceObserver,
                             public SelfUpdateObserver
{
    Q_OBJECT
    Q_PROPERTY(QString state READ GetState NOTIFY Changed)
    Q_PROPERTY(QString errorText READ GetErrorText NOTIFY Changed)
    Q_PROPERTY(QString clientInstalled READ GetClientInstalled NOTIFY Changed)
    Q_PROPERTY(QString clientLatest READ GetClientLatest NOTIFY Changed)
    Q_PROPERTY(QString commbusLatest READ GetCommbusLatest NOTIFY Changed)
    Q_PROPERTY(bool clientNeedsInstall READ GetClientNeedsInstall NOTIFY Changed)
    Q_PROPERTY(bool clientRunning READ GetClientRunning NOTIFY Changed)
    Q_PROPERTY(bool anySimRunning READ GetAnySimRunning NOTIFY Changed)
    Q_PROPERTY(QVariantList sims READ GetSims NOTIFY Changed)
    Q_PROPERTY(bool anythingToDo READ HasAnythingToDo NOTIFY Changed)
    Q_PROPERTY(bool installed READ IsInstalled NOTIFY Changed)
    Q_PROPERTY(QString actionLabel READ GetActionLabel NOTIFY Changed)
    Q_PROPERTY(QString progressText READ GetProgressText NOTIFY Changed)
    Q_PROPERTY(double progressValue READ GetProgressValue NOTIFY Changed)
    Q_PROPERTY(bool desktopShortcut READ GetDesktopShortcut WRITE SetDesktopShortcut NOTIFY Changed)
    Q_PROPERTY(bool clientStartMenuShortcut READ GetClientStartMenuShortcut
        WRITE SetClientStartMenuShortcut NOTIFY Changed)
    Q_PROPERTY(bool setupShortcut READ GetSetupShortcut WRITE SetSetupShortcut NOTIFY Changed)
    Q_PROPERTY(bool forceReinstall READ GetForceReinstall WRITE SetForceReinstall NOTIFY Changed)
    Q_PROPERTY(int autoStartMode READ GetAutoStartMode WRITE SetAutoStartMode NOTIFY Changed)
    Q_PROPERTY(QVariantList autoStartOptions READ GetAutoStartOptions NOTIFY Changed)
    Q_PROPERTY(QString installerVersion READ GetInstallerVersion CONSTANT)
    Q_PROPERTY(bool installerUpdateAvailable READ GetInstallerUpdateAvailable NOTIFY Changed)
    Q_PROPERTY(QString installerLatest READ GetInstallerLatest NOTIFY Changed)
    Q_PROPERTY(QString installerUpdateState READ GetInstallerUpdateState NOTIFY Changed)
    Q_PROPERTY(double installerUpdateProgress READ GetInstallerUpdateProgress NOTIFY Changed)
    Q_PROPERTY(QString installerUpdateError READ GetInstallerUpdateError NOTIFY Changed)
    Q_PROPERTY(QString language READ GetLanguage WRITE SetLanguage NOTIFY LanguageChanged)

public:
    enum AutoStartMode
    {
        kAutoStartNoChange = 0,
        kAutoStartDisable,
        kAutoStartAllSims,
        kAutoStart2020Only,
        kAutoStart2024Only,
    };

    SetupViewModel(SettingsRepository* settings, ReleaseProvider* releases,
                   SystemInspector* inspector, InstallService* install,
                   SelfUpdateService* selfUpdate, QObject* parent = nullptr);
    ~SetupViewModel() override;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void install();
    Q_INVOKABLE void selfUpdate();
    Q_INVOKABLE void setSimSelected(int index, bool selected);
    Q_INVOKABLE void openClient() const;
    Q_INVOKABLE void uninstall();

    [[nodiscard]] QString GetState() const { return state_; }
    [[nodiscard]] QString GetErrorText() const { return errorText_ ? errorText_() : QString(); }
    [[nodiscard]] QString GetClientInstalled() const { return clientInstalled_; }
    [[nodiscard]] QString GetClientLatest() const { return clientRelease_.version; }
    [[nodiscard]] QString GetCommbusLatest() const { return commbusRelease_.version; }
    [[nodiscard]] bool GetClientNeedsInstall() const;
    [[nodiscard]] bool GetClientRunning() const { return clientRunning_; }
    [[nodiscard]] bool GetAnySimRunning() const;
    [[nodiscard]] QVariantList GetSims() const;
    [[nodiscard]] bool HasAnythingToDo() const;
    [[nodiscard]] bool IsInstalled() const { return !clientInstalled_.isEmpty(); }
    [[nodiscard]] QString GetActionLabel() const;
    [[nodiscard]] QString GetProgressText() const { return progressText_ ? progressText_() : QString(); }
    [[nodiscard]] double GetProgressValue() const { return progressValue_; }
    [[nodiscard]] bool GetDesktopShortcut() const { return desktopShortcut_; }
    void SetDesktopShortcut(bool value);
    [[nodiscard]] bool GetClientStartMenuShortcut() const { return clientStartMenuShortcut_; }
    void SetClientStartMenuShortcut(bool value);
    [[nodiscard]] bool GetSetupShortcut() const { return setupShortcut_; }
    void SetSetupShortcut(bool value);
    [[nodiscard]] bool GetForceReinstall() const { return forceReinstall_; }
    void SetForceReinstall(bool value);
    [[nodiscard]] int GetAutoStartMode() const { return autoStartMode_; }
    void SetAutoStartMode(int value);
    [[nodiscard]] QVariantList GetAutoStartOptions() const;
    static QString GetInstallerVersion() { return QStringLiteral(GSXI_INSTALLER_VERSION); }
    [[nodiscard]] bool GetInstallerUpdateAvailable() const;
    [[nodiscard]] QString GetInstallerLatest() const { return installerRelease_.version; }
    [[nodiscard]] QString GetInstallerUpdateState() const { return installerUpdateState_; }
    [[nodiscard]] double GetInstallerUpdateProgress() const { return installerUpdateProgress_; }

    [[nodiscard]] QString GetInstallerUpdateError() const
    {
        return installerUpdateError_ ? installerUpdateError_() : QString();
    }

    [[nodiscard]] QString GetLanguage() const { return language_; }
    void SetLanguage(const QString& language);

    void RefreshTranslations()
    {
        emit Changed();
    }

    void OnReleasesFetched(const ReleaseInfo& client, const ReleaseInfo& commbus,
                           const ReleaseInfo& installer) override;
    void OnInstallProgress(const InstallProgress& update) override;
    void OnInstallFinished(const InstallOutcome& outcome) override;
    void OnSelfUpdateProgress(double fraction) override;
    void OnSelfUpdateFailed(SelfUpdateError kind, const QString& detail) override;

signals:
    void Changed();
    void LanguageChanged();

private:
    struct SimEntry
    {
        SimInstall sim;
        QString commbusInstalled;
        bool running = false;
        bool selected = true;
    };

    [[nodiscard]] bool SimNeedsInstall(const SimEntry& entry) const;
    [[nodiscard]] bool HasInstallWork() const;
    [[nodiscard]] bool HasShortcutChanges() const;
    [[nodiscard]] bool HasSimGeneration(int generation) const;
    bool UpdateRunningProcesses();
    void PollRunningProcesses();
    void RefreshInstalledVersions();
    void PersistShortcutSettings() const;
    static QString ReleaseErrorText(ReleaseError kind);
    static QString InstallProgressText(const InstallProgress& update);
    static QString InstallErrorText(const InstallOutcome& outcome);
    static QString SelfUpdateErrorText(SelfUpdateError kind, const QString& detail);

    SettingsRepository* settings_;
    ReleaseProvider* releases_;
    SystemInspector* inspector_;
    InstallService* install_;
    SelfUpdateService* selfUpdate_;

    QString state_ = QStringLiteral("checking");
    std::function<QString()> errorText_;
    QString clientInstalled_;
    bool clientRunning_ = false;
    ReleaseInfo clientRelease_;
    ReleaseInfo commbusRelease_;
    ReleaseInfo installerRelease_;
    QString installerUpdateState_ = QStringLiteral("idle");
    double installerUpdateProgress_ = -1.0;
    std::function<QString()> installerUpdateError_;
    QList<SimEntry> sims_;
    std::function<QString()> progressText_;
    double progressValue_ = -1.0;
    bool desktopShortcut_ = true;
    bool clientStartMenuShortcut_ = true;
    bool setupShortcut_ = true;
    bool forceReinstall_ = false;
    int autoStartMode_ = kAutoStartNoChange;
    QString language_;

    QTimer runningPollTimer_;
};

#endif
