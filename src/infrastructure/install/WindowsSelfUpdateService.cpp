#include "WindowsSelfUpdateService.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>

#include "../github/GithubDownloader.h"
#include "../system/InstallLocations.h"
#include "ReleasePackage.h"
#include "SelfUpdateScript.h"

namespace
{
    constexpr auto kPayloadDirName = "gsx-integrator-installer";
}

void SelfUpdateWorker::Run(const ReleaseInfo& release)
{
    const QString stagingDir = QDir::tempPath() + QStringLiteral("/gsx-integrator-self-update");
    QDir(stagingDir).removeRecursively();
    (void)QDir().mkpath(stagingDir);

    const QString zipPath = stagingDir + u'/' + release.assetName;

    const QString downloadError = DownloadFile(release.assetUrl, zipPath,
                                               [this](const qint64 received, const qint64 total)
                                               {
                                                   emit progress(total > 0
                                                                     ? static_cast<double>(received) / static_cast<
                                                                         double>(total)
                                                                     : -1.0);
                                               });
    if (!downloadError.isEmpty())
    {
        emit failed(SelfUpdateError::DownloadFailed, downloadError);
        return;
    }

    const QString shaPath = zipPath + QStringLiteral(".sha256");
    const QString shaError = DownloadFile(release.shaUrl, shaPath);
    if (!shaError.isEmpty())
    {
        emit failed(SelfUpdateError::DownloadFailed, shaError);
        return;
    }

    if (!Sha256Matches(zipPath, shaPath))
    {
        QDir(stagingDir).removeRecursively();
        emit failed(SelfUpdateError::ChecksumMismatch, release.assetName);
        return;
    }

    const ExtractResult extract = ExtractZip(zipPath, stagingDir);

    QFile::remove(zipPath);

    const QString payloadDir = stagingDir + u'/' + QLatin1String(kPayloadDirName);
    const QString payloadExe = payloadDir + u'/' + QLatin1String(kInstallerExeName);

    if (!extract.ok() || !QFile::exists(payloadExe))
    {
        QDir(stagingDir).removeRecursively();
        emit failed(SelfUpdateError::ExtractFailed,
                    extract.ok() ? QDir::toNativeSeparators(payloadExe) : extract.detail);
        return;
    }

    emit staged(stagingDir, payloadDir);
}

WindowsSelfUpdateService::WindowsSelfUpdateService(QObject* parent) : QObject(parent)
{
    qRegisterMetaType<ReleaseInfo>();
    qRegisterMetaType<SelfUpdateError>();

    worker_ = new SelfUpdateWorker;
    worker_->moveToThread(&workerThread_);

    connect(&workerThread_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(this, &WindowsSelfUpdateService::UpdateRequested, worker_, &SelfUpdateWorker::Run);
    connect(worker_, &SelfUpdateWorker::progress, this, &WindowsSelfUpdateService::OnProgress);
    connect(worker_, &SelfUpdateWorker::staged, this, &WindowsSelfUpdateService::OnStaged);
    connect(worker_, &SelfUpdateWorker::failed, this, &WindowsSelfUpdateService::OnFailed);

    workerThread_.start();
}

WindowsSelfUpdateService::~WindowsSelfUpdateService()
{
    workerThread_.quit();
    workerThread_.wait();
}

void WindowsSelfUpdateService::SetObserver(SelfUpdateObserver* observer)
{
    observer_ = observer;
}

void WindowsSelfUpdateService::StartUpdate(const ReleaseInfo& release)
{
    emit UpdateRequested(release);
}

void WindowsSelfUpdateService::OnProgress(const double fraction) const
{
    if (observer_)
    {
        observer_->OnSelfUpdateProgress(fraction);
    }
}

void WindowsSelfUpdateService::OnStaged(const QString& stagingDir, const QString& payloadDir) const
{
    const QString scriptPath =
        QDir::toNativeSeparators(QDir::tempPath() + QStringLiteral("/gsxi-self-update.cmd"));

    if (!WriteSelfUpdateScript(scriptPath, payloadDir, QCoreApplication::applicationDirPath(),
                               QCoreApplication::applicationFilePath(), stagingDir))
    {
        OnFailed(SelfUpdateError::SwapFailed, scriptPath);
        return;
    }

    if (!QProcess::startDetached(QStringLiteral("cmd.exe"),
                                 {QStringLiteral("/d"), QStringLiteral("/c"), scriptPath},
                                 QDir::tempPath()))
    {
        OnFailed(SelfUpdateError::RelaunchFailed, scriptPath);
        return;
    }

    QCoreApplication::exit(0);
}

void WindowsSelfUpdateService::OnFailed(const SelfUpdateError kind, const QString& detail) const
{
    if (observer_)
    {
        observer_->OnSelfUpdateFailed(kind, detail);
    }
}
