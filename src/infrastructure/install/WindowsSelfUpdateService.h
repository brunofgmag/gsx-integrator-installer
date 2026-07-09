#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_WINDOWSSELFUPDATESERVICE_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_WINDOWSSELFUPDATESERVICE_H

#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include "../../application/ports/SelfUpdateService.h"

class SelfUpdateWorker final : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

public slots:
    void Run(const ReleaseInfo& release);

signals:
    void progress(double fraction);
    void staged();
    void failed(SelfUpdateError kind, const QString& detail);
};

class WindowsSelfUpdateService final : public QObject, public SelfUpdateService
{
    Q_OBJECT

public:
    explicit WindowsSelfUpdateService(QObject* parent = nullptr);
    ~WindowsSelfUpdateService() override;

    void SetObserver(SelfUpdateObserver* observer) override;
    void StartUpdate(const ReleaseInfo& release) override;

signals:
    void UpdateRequested(const ReleaseInfo& release);

private:
    void OnProgress(double fraction) const;
    void OnStaged() const;
    void OnFailed(SelfUpdateError kind, const QString& detail) const;

    SelfUpdateObserver* observer_ = nullptr;
    QThread workerThread_;
    SelfUpdateWorker* worker_ = nullptr;
};

Q_DECLARE_METATYPE(ReleaseInfo)

Q_DECLARE_METATYPE(SelfUpdateError)

#endif
