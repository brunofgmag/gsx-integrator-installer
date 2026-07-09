#include "EngineInstallService.h"

#include <QtCore/QProcess>
#include "../system/InstallLocations.h"

void InstallWorker::Run(const InstallRequest& request)
{
    const InstallOutcome outcome = orchestrator_.RunInstall(request, [this](const InstallProgress& update)
    {
        emit progress(update);
    });

    emit finished(outcome);
}

EngineInstallService::EngineInstallService(QObject* parent) : QObject(parent)
{
    qRegisterMetaType<InstallRequest>();
    qRegisterMetaType<InstallProgress>();
    qRegisterMetaType<InstallOutcome>();

    worker_ = new InstallWorker;
    worker_->moveToThread(&workerThread_);

    connect(&workerThread_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(this, &EngineInstallService::InstallRequested, worker_, &InstallWorker::Run);
    connect(worker_, &InstallWorker::progress, this, &EngineInstallService::OnProgress);
    connect(worker_, &InstallWorker::finished, this, &EngineInstallService::OnFinished);

    workerThread_.start();
}

EngineInstallService::~EngineInstallService()
{
    workerThread_.quit();
    workerThread_.wait();
}

void EngineInstallService::SetObserver(InstallServiceObserver* observer)
{
    observer_ = observer;
}

void EngineInstallService::Install(const InstallRequest& request)
{
    emit InstallRequested(request);
}

void EngineInstallService::LaunchClient()
{
    QProcess::startDetached(ClientExePath(), {}, ClientInstallDir());
}

void EngineInstallService::Uninstall()
{
    WindowsInstallerGateway gateway;
    InstallOrchestrator orchestrator(gateway);
    orchestrator.RunUninstall();
}

void EngineInstallService::OnProgress(const InstallProgress& update) const
{
    if (observer_)
    {
        observer_->OnInstallProgress(update);
    }
}

void EngineInstallService::OnFinished(const InstallOutcome& outcome) const
{
    if (observer_)
    {
        observer_->OnInstallFinished(outcome);
    }
}
