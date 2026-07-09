#include "SetupViewModel.h"

#include <QtCore/QVariantMap>
#include "domain/Versions.h"
#include "domain/model/InstallOutcome.h"
#include "domain/model/InstallRequest.h"

SetupViewModel::SetupViewModel(SettingsRepository* settings,
                               ReleaseProvider* releases,
                               SystemInspector* inspector,
                               InstallService* install,
                               SelfUpdateService* selfUpdate,
                               QObject* parent)
    : QObject(parent),
      settings_(settings),
      releases_(releases),
      inspector_(inspector),
      install_(install),
      selfUpdate_(selfUpdate)
{
    language_ = settings_->Load().language;
    releases_->SetObserver(this);
    install_->SetObserver(this);
    selfUpdate_->SetObserver(this);

    runningPollTimer_.setInterval(3000);
    connect(&runningPollTimer_, &QTimer::timeout, this, &SetupViewModel::PollRunningProcesses);
    runningPollTimer_.start();
}

SetupViewModel::~SetupViewModel()
{
    releases_->SetObserver(nullptr);
    install_->SetObserver(nullptr);
    selfUpdate_->SetObserver(nullptr);
}

void SetupViewModel::refresh()
{
    state_ = QStringLiteral("checking");
    errorText_ = nullptr;
    clientRelease_ = {};
    commbusRelease_ = {};
    installerRelease_ = {};
    installerUpdateState_ = QStringLiteral("idle");
    installerUpdateError_ = nullptr;

    clientInstalled_ = inspector_->InstalledClientVersion();
    clientRunning_ = inspector_->IsClientRunning();

    const InstallerSettings saved = settings_->Load();
    desktopShortcut_ = saved.desktopShortcut;
    clientStartMenuShortcut_ = saved.clientStartMenuShortcut;
    setupShortcut_ = saved.setupShortcut;

    sims_.clear();
    for (const SimInstall& sim : inspector_->DetectSims())
    {
        SimEntry entry;
        entry.sim = sim;
        entry.commbusInstalled = inspector_->InstalledCommbusVersion(sim.communityPath);
        entry.running = inspector_->IsSimRunning(sim.processName);
        entry.selected = !saved.disabledCommbusSims.contains(sim.id);
        sims_.append(entry);
    }

    emit Changed();

    releases_->FetchLatest();
}

void SetupViewModel::RefreshInstalledVersions()
{
    clientInstalled_ = inspector_->InstalledClientVersion();
    for (SimEntry& entry : sims_)
    {
        entry.commbusInstalled = inspector_->InstalledCommbusVersion(entry.sim.communityPath);
    }
}

void SetupViewModel::OnReleasesFetched(const ReleaseInfo& client, const ReleaseInfo& commbus,
                                       const ReleaseInfo& installer)
{
    clientRelease_ = client;
    commbusRelease_ = commbus;
    installerRelease_ = installer;

    if (!clientRelease_.valid || !commbusRelease_.valid)
    {
        const ReleaseInfo& bad = !clientRelease_.valid ? clientRelease_ : commbusRelease_;
        const ReleaseError kind = bad.errorKind;
        state_ = QStringLiteral("error");
        errorText_ = [kind] {
            return tr("Could not read the latest releases from GitHub. %1")
                .arg(ReleaseErrorText(kind));
        };
    }
    else
    {
        state_ = QStringLiteral("ready");
    }

    emit Changed();
}

QString SetupViewModel::ReleaseErrorText(const ReleaseError kind)
{
    switch (kind)
    {
    case ReleaseError::Network:
        return tr("Could not reach GitHub.");
    case ReleaseError::BadResponse:
        return tr("GitHub returned an unexpected response.");
    case ReleaseError::NoVersionTag:
        return tr("The latest release has no readable version.");
    case ReleaseError::MissingAsset:
        return tr("The latest release is missing its download.");
    case ReleaseError::None:
        break;
    }

    return {};
}

bool SetupViewModel::GetClientNeedsInstall() const
{
    return clientRelease_.valid && (forceReinstall_ || IsNewer(clientRelease_.version, clientInstalled_));
}

bool SetupViewModel::GetInstallerUpdateAvailable() const
{
    return installerRelease_.valid && IsNewer(installerRelease_.version, GetInstallerVersion());
}

bool SetupViewModel::SimNeedsInstall(const SimEntry& entry) const
{
    return commbusRelease_.valid && (forceReinstall_ || IsNewer(commbusRelease_.version, entry.commbusInstalled));
}

bool SetupViewModel::UpdateRunningProcesses()
{
    bool changed = false;

    const bool clientRunning = inspector_->IsClientRunning();
    if (clientRunning != clientRunning_)
    {
        clientRunning_ = clientRunning;
        changed = true;
    }

    for (SimEntry& entry : sims_)
    {
        const bool running = inspector_->IsSimRunning(entry.sim.processName);
        if (running != entry.running)
        {
            entry.running = running;
            changed = true;
        }
    }

    return changed;
}

void SetupViewModel::PollRunningProcesses()
{
    if (state_ != QLatin1String("ready"))
    {
        return;
    }
    if (UpdateRunningProcesses())
    {
        emit Changed();
    }
}

bool SetupViewModel::GetAnySimRunning() const
{
    return std::ranges::any_of(sims_, [this](const SimEntry& entry)
    {
        return entry.running && entry.selected && SimNeedsInstall(entry);
    });
}

QVariantList SetupViewModel::GetSims() const
{
    QVariantList list;
    for (const SimEntry& entry : sims_)
    {
        QVariantMap map;
        map.insert(QStringLiteral("label"), entry.sim.label);
        map.insert(QStringLiteral("installedVersion"), entry.commbusInstalled);
        map.insert(QStringLiteral("running"), entry.running);
        map.insert(QStringLiteral("selected"), entry.selected);
        map.insert(QStringLiteral("needsInstall"), SimNeedsInstall(entry));
        list.append(map);
    }

    return list;
}

bool SetupViewModel::HasInstallWork() const
{
    if (GetClientNeedsInstall())
    {
        return true;
    }

    return std::ranges::any_of(sims_, [this](const SimEntry& entry)
    {
        return entry.selected && SimNeedsInstall(entry);
    });
}

bool SetupViewModel::HasShortcutChanges() const
{
    if (!IsInstalled())
    {
        return false;
    }

    return desktopShortcut_ != inspector_->DesktopShortcutExists()
        || clientStartMenuShortcut_ != inspector_->ClientStartMenuShortcutExists()
        || setupShortcut_ != inspector_->SetupStartMenuShortcutExists();
}

bool SetupViewModel::HasAnythingToDo() const
{
    return HasInstallWork() || autoStartMode_ != kAutoStartNoChange || HasShortcutChanges();
}

QString SetupViewModel::GetActionLabel() const
{
    if (HasInstallWork())
    {
        return IsInstalled() ? tr("Update everything") : tr("Install everything");
    }

    if (autoStartMode_ != kAutoStartNoChange || HasShortcutChanges())
    {
        return tr("Apply changes");
    }

    return tr("Everything up to date");
}

bool SetupViewModel::HasSimGeneration(const int generation) const
{
    return std::ranges::any_of(sims_, [=](const SimEntry& entry)
    {
        return entry.sim.generation == generation;
    });
}

QVariantList SetupViewModel::GetAutoStartOptions() const
{
    QVariantList list;
    const auto add = [&list](const int value, const QString& label)
    {
        QVariantMap map;
        map.insert(QStringLiteral("value"), value);
        map.insert(QStringLiteral("label"), label);
        list.append(map);
    };

    add(kAutoStartNoChange, tr("Keep current configuration"));
    add(kAutoStartDisable, tr("Disable auto-start"));

    const bool has2020 = HasSimGeneration(2020);
    const bool has2024 = HasSimGeneration(2024);
    if (has2020 || has2024)
    {
        add(kAutoStartAllSims, tr("Auto-start with MSFS 2020/2024"));
    }
    if (has2020)
    {
        add(kAutoStart2020Only, tr("Auto-start with MSFS 2020 only"));
    }
    if (has2024)
    {
        add(kAutoStart2024Only, tr("Auto-start with MSFS 2024 only"));
    }

    return list;
}

void SetupViewModel::SetAutoStartMode(const int value)
{
    autoStartMode_ = value;
    emit Changed();
}

void SetupViewModel::setSimSelected(const int index, const bool selected)
{
    if (index >= 0 && index < sims_.size())
    {
        sims_[index].selected = selected;

        InstallerSettings settings = settings_->Load();

        settings.disabledCommbusSims.removeAll(sims_[index].sim.id);

        if (!selected)
        {
            settings.disabledCommbusSims.append(sims_[index].sim.id);
        }

        settings_->Save(settings);

        emit Changed();
    }
}

void SetupViewModel::PersistShortcutSettings() const
{
    InstallerSettings settings = settings_->Load();
    settings.desktopShortcut = desktopShortcut_;
    settings.clientStartMenuShortcut = clientStartMenuShortcut_;
    settings.setupShortcut = setupShortcut_;
    settings_->Save(settings);
}

void SetupViewModel::SetDesktopShortcut(const bool value)
{
    desktopShortcut_ = value;

    PersistShortcutSettings();

    emit Changed();
}

void SetupViewModel::SetClientStartMenuShortcut(const bool value)
{
    clientStartMenuShortcut_ = value;

    PersistShortcutSettings();

    emit Changed();
}

void SetupViewModel::SetSetupShortcut(const bool value)
{
    setupShortcut_ = value;

    PersistShortcutSettings();

    emit Changed();
}

void SetupViewModel::SetForceReinstall(const bool value)
{
    forceReinstall_ = value;

    emit Changed();
}

void SetupViewModel::SetLanguage(const QString& language)
{
    if (language == language_)
    {
        return;
    }

    language_ = language;

    InstallerSettings settings = settings_->Load();
    settings.language = language;
    settings_->Save(settings);

    emit LanguageChanged();
}

void SetupViewModel::install()
{
    if (state_ != QLatin1String("ready"))
    {
        return;
    }

    if (installerUpdateState_ == QLatin1String("updating"))
    {
        return;
    }

    errorText_ = nullptr;

    UpdateRunningProcesses();

    if (GetClientNeedsInstall() && clientRunning_)
    {
        errorText_ = [] { return tr("Close GSX Integrator before updating it."); };
        emit Changed();
        return;
    }

    if (GetAnySimRunning())
    {
        errorText_ = [] { return tr("Close the simulator before installing the CommBus module."); };
        emit Changed();
        return;
    }

    InstallRequest request;
    request.installClient = GetClientNeedsInstall();
    request.clientRelease = clientRelease_;
    request.commbusRelease = commbusRelease_;
    request.desktopShortcut = desktopShortcut_;
    request.clientStartMenuShortcut = clientStartMenuShortcut_;
    request.setupShortcut = setupShortcut_;
    for (const SimEntry& entry : sims_)
    {
        if (entry.selected && SimNeedsInstall(entry))
        {
            request.commbusTargets.append(
                {entry.sim.communityPath, entry.sim.processName, entry.sim.label});
        }
    }

    const auto addAutoStartTargets = [&](const int wantedGeneration)
    {
        for (const SimEntry& entry : sims_)
        {
            if (wantedGeneration == 0 || entry.sim.generation == wantedGeneration)
            {
                request.exeXmlAddTargets.append(entry.sim.exeXmlPath);
            }
        }
    };

    switch (autoStartMode_)
    {
    case kAutoStartDisable:
        request.exeXmlRemoveTargets = inspector_->CandidateExeXmlPaths(0);
        break;
    case kAutoStartAllSims:
        addAutoStartTargets(0);
        break;
    case kAutoStart2020Only:
        addAutoStartTargets(2020);
        request.exeXmlRemoveTargets = inspector_->CandidateExeXmlPaths(2024);
        break;
    case kAutoStart2024Only:
        addAutoStartTargets(2024);
        request.exeXmlRemoveTargets = inspector_->CandidateExeXmlPaths(2020);
        break;
    default:
        break;
    }

    state_ = QStringLiteral("installing");
    progressText_ = [] { return tr("Preparing"); };
    progressValue_ = -1.0;

    emit Changed();

    install_->Install(request);
}

void SetupViewModel::OnInstallProgress(const InstallProgress& update)
{
    progressText_ = [update] { return InstallProgressText(update); };
    progressValue_ = update.fraction;

    emit Changed();
}

void SetupViewModel::OnInstallFinished(const InstallOutcome& outcome)
{
    if (outcome.ok())
    {
        state_ = QStringLiteral("done");
        RefreshInstalledVersions();
        autoStartMode_ = kAutoStartNoChange;
    }
    else
    {
        state_ = QStringLiteral("error");
        errorText_ = [outcome] { return InstallErrorText(outcome); };
    }

    emit Changed();
}

QString SetupViewModel::InstallProgressText(const InstallProgress& update)
{
    switch (update.phase)
    {
    case InstallPhase::DownloadingClient:
        return tr("Downloading %1 %2").arg(tr("GSX Integrator"), update.detail);
    case InstallPhase::InstallingClient:
        return tr("Installing GSX Integrator %1").arg(update.detail);
    case InstallPhase::DownloadingCommbus:
        return tr("Downloading %1 %2").arg(tr("CommBus module"), update.detail);
    case InstallPhase::InstallingCommbus:
        return tr("Installing CommBus module into %1").arg(update.detail);
    case InstallPhase::ConfiguringAutoStart:
        return tr("Configuring simulator auto-start");
    }

    return {};
}

QString SetupViewModel::InstallErrorText(const InstallOutcome& outcome)
{
    switch (outcome.status)
    {
    case InstallStatus::ClientRunning:
        return tr("Close GSX Integrator before updating it.");
    case InstallStatus::InstallerInsideInstallDir:
        return tr("This copy of the installer lives inside the install folder. "
            "Run a downloaded installer to update the client.");
    case InstallStatus::DownloadFailed:
        return outcome.detail;
    case InstallStatus::ChecksumMismatch:
        return tr("Checksum mismatch for %1. Download corrupted?").arg(outcome.detail);
    case InstallStatus::CleanInstallDirFailed:
        return tr("Could not remove the previous installation at %1.").arg(outcome.detail);
    case InstallStatus::TarMissing:
        return tr("tar.exe not found (Windows 10 or newer is required).");
    case InstallStatus::ExtractFailed:
        return tr("Extraction failed: %1").arg(outcome.detail);
    case InstallStatus::ClientExeMissing:
        return tr("The release package did not contain the expected executable.");
    case InstallStatus::SimRunning:
        return tr("%1 is running. Close the simulator before "
            "installing the CommBus module.").arg(outcome.detail);
    case InstallStatus::CommbusReplaceFailed:
        return tr("Could not replace the CommBus package in %1. "
            "Is the simulator running?").arg(outcome.detail);
    case InstallStatus::AutoStartClientMissing:
        return tr("The GSX Integrator client is not installed, "
            "so auto-start could not be configured.");
    case InstallStatus::ExeXmlUpdateFailed:
        return tr("Could not update %1.").arg(outcome.detail);
    case InstallStatus::Success:
        break;
    }

    return {};
}

void SetupViewModel::selfUpdate()
{
    if (state_ != QLatin1String("ready") || !GetInstallerUpdateAvailable()
        || installerUpdateState_ == QLatin1String("updating"))
    {
        return;
    }

    installerUpdateState_ = QStringLiteral("updating");
    installerUpdateError_ = nullptr;
    installerUpdateProgress_ = -1.0;

    emit Changed();

    selfUpdate_->StartUpdate(installerRelease_);
}

void SetupViewModel::OnSelfUpdateProgress(const double fraction)
{
    installerUpdateProgress_ = fraction;

    emit Changed();
}

void SetupViewModel::OnSelfUpdateFailed(const SelfUpdateError kind, const QString& detail)
{
    installerUpdateState_ = QStringLiteral("error");
    installerUpdateError_ = [kind, detail] { return SelfUpdateErrorText(kind, detail); };

    emit Changed();
}

QString SetupViewModel::SelfUpdateErrorText(const SelfUpdateError kind, const QString& detail)
{
    switch (kind)
    {
    case SelfUpdateError::DownloadFailed:
        return detail;
    case SelfUpdateError::ChecksumMismatch:
        return tr("Checksum mismatch for %1. Download corrupted?").arg(detail);
    case SelfUpdateError::SwapFailed:
        return tr("Could not replace the installer executable.");
    case SelfUpdateError::RelaunchFailed:
        return tr("The update was installed, but the installer could not restart. Open it again manually.");
    }

    return {};
}

void SetupViewModel::openClient() const
{
    install_->LaunchClient();
}

void SetupViewModel::uninstall()
{
    UpdateRunningProcesses();

    if (clientRunning_)
    {
        errorText_ = [] { return tr("Close GSX Integrator before uninstalling it."); };
        emit Changed();
        return;
    }

    errorText_ = nullptr;
    install_->Uninstall();
    state_ = QStringLiteral("uninstalled");

    emit Changed();
}
