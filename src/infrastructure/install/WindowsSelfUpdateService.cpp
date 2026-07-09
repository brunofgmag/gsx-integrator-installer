#include "WindowsSelfUpdateService.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QRegularExpression>

#include "../github/GithubDownloader.h"

namespace
{
    QString ReadExpectedSha(const QString& shaFilePath)
    {
        QFile file(shaFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }

        const QString content = QString::fromUtf8(file.readAll()).trimmed();

        static const QRegularExpression whitespace(QStringLiteral("\\s"));

        return content.section(whitespace, 0, 0).toLower();
    }
}

void SelfUpdateWorker::Run(const ReleaseInfo& release)
{
    const QString newPath = QCoreApplication::applicationFilePath() + QStringLiteral(".new");

    const QString downloadError = DownloadFile(release.zipUrl, newPath,
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

    const QString shaPath = newPath + QStringLiteral(".sha256");
    const QString shaError = DownloadFile(release.shaUrl, shaPath);
    if (!shaError.isEmpty())
    {
        emit failed(SelfUpdateError::DownloadFailed, shaError);
        return;
    }

    const QString expected = ReadExpectedSha(shaPath);

    QFile::remove(shaPath);

    if (expected.isEmpty() || Sha256File(newPath) != expected)
    {
        QFile::remove(newPath);
        emit failed(SelfUpdateError::ChecksumMismatch, release.zipName);
        return;
    }

    emit staged();
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

void WindowsSelfUpdateService::OnStaged() const
{
    const QString exePath = QCoreApplication::applicationFilePath();
    const QString newPath = exePath + QStringLiteral(".new");
    const QString oldPath = exePath + QStringLiteral(".old");

    QFile::remove(oldPath);

    if (!QFile::rename(exePath, oldPath))
    {
        OnFailed(SelfUpdateError::SwapFailed, {});
        return;
    }

    if (!QFile::rename(newPath, exePath))
    {
        QFile::rename(oldPath, exePath);
        OnFailed(SelfUpdateError::SwapFailed, {});
        return;
    }

    if (QProcess::startDetached(exePath, {}, QFileInfo(exePath).absolutePath()))
    {
        QCoreApplication::quit();
    }
    else
    {
        OnFailed(SelfUpdateError::RelaunchFailed, {});
    }
}

void WindowsSelfUpdateService::OnFailed(const SelfUpdateError kind, const QString& detail) const
{
    if (observer_)
    {
        observer_->OnSelfUpdateFailed(kind, detail);
    }
}
