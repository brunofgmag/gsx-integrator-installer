#include "Versions.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

QVersionNumber ParseVersion(const QString& tagOrVersion)
{
    QString text = tagOrVersion.trimmed();
    if (text.startsWith(u'v') || text.startsWith(u'V'))
    {
        text.remove(0, 1);
    }

    qsizetype suffixIndex = -1;
    QVersionNumber version = QVersionNumber::fromString(text, &suffixIndex);
    if (version.isNull() || suffixIndex == 0)
    {
        return {};
    }

    return version;
}

bool IsNewer(const QString& candidate, const QString& installed)
{
    const QVersionNumber candidateVersion = ParseVersion(candidate);
    if (candidateVersion.isNull())
    {
        return false;
    }

    const QVersionNumber installedVersion = ParseVersion(installed);
    if (installedVersion.isNull())
    {
        return true;
    }

    return candidateVersion > installedVersion;
}

QString ParsePackageVersion(const QByteArray& manifestJson)
{
    const QJsonDocument document = QJsonDocument::fromJson(manifestJson);
    if (!document.isObject())
    {
        return {};
    }

    return document.object().value(QStringLiteral("package_version")).toString();
}
