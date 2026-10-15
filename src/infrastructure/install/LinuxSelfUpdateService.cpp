#include "LinuxSelfUpdateService.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QScopeGuard>

#include "../github/GithubDownloader.h"
#include "../system/ChildEnvironment.h"
#include "../system/LinuxInstallLocations.h"
#include "AppImageFiles.h"
#include "ReleasePackage.h"

namespace
{
    constexpr auto kRelaunchScript =
        "i=0; while kill -0 \"$1\" 2>/dev/null && [ \"$i\" -lt 150 ]; do i=$((i+1)); sleep 0.2; done; "
        "exec \"$0\"";
}

void AppImageUpdateWorker::Run(const ReleaseInfo& release)
{
    const AppImageTarget target = ResolveAppImageTarget(RunningAppImagePath());
    if (target.status == AppImageTargetStatus::NotAnAppImage)
    {
        emit failed(SelfUpdateError::NotAnAppImage, {});
        return;
    }

    if (target.status == AppImageTargetStatus::DirNotWritable)
    {
        emit failed(SelfUpdateError::AppImageDirNotWritable, target.dir);
        return;
    }

    const QString stagingDir = QDir::tempPath() + QStringLiteral("/gsx-integrator-self-update");
    QDir(stagingDir).removeRecursively();
    (void)QDir().mkpath(stagingDir);
    const auto cleanup = qScopeGuard([&stagingDir] { QDir(stagingDir).removeRecursively(); });

    const QString shaPath = stagingDir + u'/' + release.zipName + QStringLiteral(".sha256");
    if (const QString error = DownloadFile(release.shaUrl, shaPath); !error.isEmpty())
    {
        emit failed(SelfUpdateError::DownloadFailed, error);
        return;
    }

    const QString partPath = target.dir + QStringLiteral("/.") + release.zipName + QStringLiteral(".part");
    const QString downloadError = DownloadFile(release.zipUrl, partPath,
                                               [this](const qint64 received, const qint64 total)
                                               {
                                                   emit progress(total > 0
                                                                     ? static_cast<double>(received)
                                                                         / static_cast<double>(total)
                                                                     : -1.0);
                                               });
    if (!downloadError.isEmpty())
    {
        emit failed(SelfUpdateError::DownloadFailed, downloadError);
        return;
    }

    if (!Sha256Matches(partPath, shaPath))
    {
        QFile::remove(partPath);
        emit failed(SelfUpdateError::ChecksumMismatch, release.zipName);
        return;
    }

    if (!ReplaceFile(partPath, target.path))
    {
        QFile::remove(partPath);
        emit failed(SelfUpdateError::SwapFailed, target.path);
        return;
    }

    emit replaced(target.path);
}

LinuxSelfUpdateService::LinuxSelfUpdateService(QObject* parent) : QObject(parent)
{
    qRegisterMetaType<ReleaseInfo>();
    qRegisterMetaType<SelfUpdateError>();

    worker_ = new AppImageUpdateWorker;
    worker_->moveToThread(&workerThread_);

    connect(&workerThread_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(this, &LinuxSelfUpdateService::UpdateRequested, worker_, &AppImageUpdateWorker::Run);
    connect(worker_, &AppImageUpdateWorker::progress, this, &LinuxSelfUpdateService::OnProgress);
    connect(worker_, &AppImageUpdateWorker::replaced, this, &LinuxSelfUpdateService::OnReplaced);
    connect(worker_, &AppImageUpdateWorker::failed, this, &LinuxSelfUpdateService::OnFailed);

    workerThread_.start();
}

LinuxSelfUpdateService::~LinuxSelfUpdateService()
{
    workerThread_.quit();
    workerThread_.wait();
}

void LinuxSelfUpdateService::SetObserver(SelfUpdateObserver* observer)
{
    observer_ = observer;
}

void LinuxSelfUpdateService::StartUpdate(const ReleaseInfo& release)
{
    emit UpdateRequested(release);
}

void LinuxSelfUpdateService::OnProgress(const double fraction) const
{
    if (observer_)
    {
        observer_->OnSelfUpdateProgress(fraction);
    }
}

void LinuxSelfUpdateService::OnReplaced(const QString& appImagePath) const
{
    QProcess relaunch;
    relaunch.setProgram(QStringLiteral("/bin/sh"));
    relaunch.setArguments({
        QStringLiteral("-c"), QLatin1String(kRelaunchScript), appImagePath,
        QString::number(QCoreApplication::applicationPid())
    });
    relaunch.setProcessEnvironment(ChildProcessEnvironment(QProcessEnvironment::systemEnvironment()));

    if (!relaunch.startDetached())
    {
        OnFailed(SelfUpdateError::RelaunchFailed, appImagePath);
        return;
    }

    QCoreApplication::exit(0);
}

void LinuxSelfUpdateService::OnFailed(const SelfUpdateError kind, const QString& detail) const
{
    if (observer_)
    {
        observer_->OnSelfUpdateFailed(kind, detail);
    }
}
