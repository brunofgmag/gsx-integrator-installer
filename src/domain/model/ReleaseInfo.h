#ifndef GSX_INTEGRATOR_INSTALLER_DOMAIN_MODEL_RELEASEINFO_H
#define GSX_INTEGRATOR_INSTALLER_DOMAIN_MODEL_RELEASEINFO_H

#include <QtCore/QString>
#include <QtCore/QUrl>

enum class ReleaseError
{
    None = 0,
    Network,
    BadResponse,
    NoVersionTag,
    MissingAsset,
};

struct ReleaseInfo
{
    bool valid = false;
    ReleaseError errorKind = ReleaseError::None;
    QString error;
    QString version;
    QString zipName;
    QUrl zipUrl;
    QUrl shaUrl;
};

#endif
