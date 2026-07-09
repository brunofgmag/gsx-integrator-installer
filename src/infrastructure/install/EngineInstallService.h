#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_ENGINEINSTALLSERVICE_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_ENGINEINSTALLSERVICE_H

#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include "../../application/install/InstallOrchestrator.h"
#include "../../application/ports/InstallService.h"
#include "WindowsInstallerGateway.h"

class InstallWorker final : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

public slots:
    void Run(const InstallRequest& request);

signals:
    void progress(const InstallProgress& update);
    void finished(const InstallOutcome& outcome);

private:
    WindowsInstallerGateway gateway_;
    InstallOrchestrator orchestrator_{gateway_};
};

class EngineInstallService final : public QObject, public InstallService
{
    Q_OBJECT

public:
    explicit EngineInstallService(QObject* parent = nullptr);
    ~EngineInstallService() override;

    void SetObserver(InstallServiceObserver* observer) override;
    void Install(const InstallRequest& request) override;
    void LaunchClient() override;
    void Uninstall() override;

signals:
    void InstallRequested(const InstallRequest& request);

private:
    void OnProgress(const InstallProgress& update) const;
    void OnFinished(const InstallOutcome& outcome) const;

    InstallServiceObserver* observer_ = nullptr;
    QThread workerThread_;
    InstallWorker* worker_ = nullptr;
};

Q_DECLARE_METATYPE(InstallRequest)
Q_DECLARE_METATYPE(InstallProgress)
Q_DECLARE_METATYPE(InstallOutcome)

#endif
