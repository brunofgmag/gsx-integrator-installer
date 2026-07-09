#include "GithubDownloader.h"

#include <chrono>

#include <QtCore/QCryptographicHash>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace
{
    QNetworkRequest MakeRequest(const QUrl& url)
    {
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("gsx-integrator-installer"));
        request.setRawHeader("Accept", "application/vnd.github+json");
        request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
        request.setTransferTimeout(std::chrono::milliseconds(30000));

        return request;
    }
}

QString DownloadFile(const QUrl& url, const QString& destPath, const std::function<void(qint64, qint64)>& progress)
{
    QFile file(destPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return QStringLiteral("Cannot write to %1").arg(destPath);
    }

    QNetworkAccessManager nam;
    QNetworkReply* reply = nam.get(MakeRequest(url));

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::readyRead, &loop,
                     [reply, &file] { file.write(reply->readAll()); });
    if (progress)
    {
        QObject::connect(reply, &QNetworkReply::downloadProgress, &loop,
                         [&progress](const qint64 received, const qint64 total) { progress(received, total); });
    }

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    file.write(reply->readAll());
    file.close();

    const bool failed = reply->error() != QNetworkReply::NoError;
    const QString error = failed ? reply->errorString() : QString();
    reply->deleteLater();
    if (failed)
    {
        QFile::remove(destPath);
    }

    return error;
}

QString Sha256File(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file))
    {
        return {};
    }

    return QString::fromLatin1(hash.result().toHex());
}
