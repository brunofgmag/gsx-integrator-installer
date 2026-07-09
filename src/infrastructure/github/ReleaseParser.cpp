#include "ReleaseParser.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>
#include <QtCore/QVersionNumber>
#include "domain/Versions.h"

ReleaseInfo ParseLatestRelease(const QByteArray& json, const QString& assetPrefix, const QString& extension)
{
    ReleaseInfo info;

    const QJsonDocument document = QJsonDocument::fromJson(json);
    if (!document.isObject())
    {
        info.errorKind = ReleaseError::BadResponse;
        info.error = QStringLiteral("Unexpected response from GitHub.");
        return info;
    }

    const QJsonObject release = document.object();

    const QVersionNumber version = ParseVersion(release.value(QStringLiteral("tag_name")).toString());
    if (version.isNull())
    {
        info.errorKind = ReleaseError::NoVersionTag;
        info.error = QStringLiteral("Release has no readable version tag.");
        return info;
    }
    info.version = version.toString();

    const QString zipName = assetPrefix + u'-' + info.version + extension;
    for (const QJsonValue value : release.value(QStringLiteral("assets")).toArray())
    {
        const QJsonObject asset = value.toObject();
        const QString name = asset.value(QStringLiteral("name")).toString();
        const QUrl url(asset.value(QStringLiteral("browser_download_url")).toString());
        if (name == zipName)
        {
            info.zipName = name;
            info.zipUrl = url;
        }
        else if (name == zipName + QStringLiteral(".sha256"))
        {
            info.shaUrl = url;
        }
    }

    if (info.zipUrl.isEmpty() || info.shaUrl.isEmpty())
    {
        info.errorKind = ReleaseError::MissingAsset;
        info.error = QStringLiteral("Release %1 is missing the %2 asset.").arg(info.version, zipName);

        return info;
    }

    info.valid = true;

    return info;
}
