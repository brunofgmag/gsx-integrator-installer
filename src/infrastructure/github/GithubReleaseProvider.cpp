#include "GithubReleaseProvider.h"

#include <chrono>

#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include "ReleaseParser.h"

namespace
{
    constexpr auto kClientRepo = "brunofgmag/gsx-integrator-client";
    constexpr auto kCommbusRepo = "brunofgmag/gsx-integrator-commbus";
    constexpr auto kInstallerRepo = "brunofgmag/gsx-integrator-installer";
    constexpr auto kClientAssetPrefix = "gsx-integrator-client";
    constexpr auto kCommbusAssetPrefix = "gsx-integrator-commbus";
    constexpr auto kInstallerAssetPrefix = "gsx-integrator-installer";

    QNetworkRequest MakeRequest(const QUrl& url)
    {
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("gsx-integrator-installer"));
        request.setRawHeader("Accept", "application/vnd.github+json");
        request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");

        return request;
    }
}

GithubReleaseProvider::GithubReleaseProvider(QObject* parent) : QObject(parent)
{
    nam_.setTransferTimeout(std::chrono::milliseconds(15000));
}

void GithubReleaseProvider::SetObserver(ReleaseProviderObserver* observer)
{
    observer_ = observer;
}

void GithubReleaseProvider::FetchLatest()
{
    client_ = {};
    commbus_ = {};
    installer_ = {};
    pending_ = 3;
    FetchOne(&client_, QLatin1String(kClientRepo), QLatin1String(kClientAssetPrefix));
    FetchOne(&commbus_, QLatin1String(kCommbusRepo), QLatin1String(kCommbusAssetPrefix));
    FetchOne(&installer_, QLatin1String(kInstallerRepo), QLatin1String(kInstallerAssetPrefix));
}

void GithubReleaseProvider::FetchOne(ReleaseInfo* target, const QString& repo,
                                     const QString& assetPrefix, const QString& extension)
{
    const QUrl url(QStringLiteral("https://api.github.com/repos/%1/releases/latest").arg(repo));

    QNetworkReply* reply = nam_.get(MakeRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, target, assetPrefix, extension]
    {
        reply->deleteLater();

        ReleaseInfo info;

        if (reply->error() != QNetworkReply::NoError)
        {
            info.errorKind = ReleaseError::Network;
            info.error = reply->errorString();
        }
        else
        {
            info = ParseLatestRelease(reply->readAll(), assetPrefix, extension);
        }

        *target = info;

        if (--pending_ == 0 && observer_)
        {
            observer_->OnReleasesFetched(client_, commbus_, installer_);
        }
    });
}
