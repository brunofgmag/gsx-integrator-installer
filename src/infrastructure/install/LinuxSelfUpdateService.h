#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_LINUXSELFUPDATESERVICE_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_LINUXSELFUPDATESERVICE_H

#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include "../../application/ports/SelfUpdateService.h"

class AppImageUpdateWorker final : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

public slots:
    void Run(const ReleaseInfo& release);

signals:
    void progress(double fraction);
    void replaced(const QString& appImagePath);
    void failed(SelfUpdateError kind, const QString& detail);
};

class LinuxSelfUpdateService final : public QObject, public SelfUpdateService
{
    Q_OBJECT

public:
    explicit LinuxSelfUpdateService(QObject* parent = nullptr);
    ~LinuxSelfUpdateService() override;

    void SetObserver(SelfUpdateObserver* observer) override;
    void StartUpdate(const ReleaseInfo& release) override;

signals:
    void UpdateRequested(const ReleaseInfo& release);

private:
    void OnProgress(double fraction) const;
    void OnReplaced(const QString& appImagePath) const;
    void OnFailed(SelfUpdateError kind, const QString& detail) const;

    SelfUpdateObserver* observer_ = nullptr;
    QThread workerThread_;
    AppImageUpdateWorker* worker_ = nullptr;
};

Q_DECLARE_METATYPE(ReleaseInfo)

Q_DECLARE_METATYPE(SelfUpdateError)

#endif
