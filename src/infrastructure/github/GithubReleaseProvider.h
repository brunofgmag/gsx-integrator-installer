#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_GITHUB_GITHUBRELEASEPROVIDER_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_GITHUB_GITHUBRELEASEPROVIDER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtNetwork/QNetworkAccessManager>
#include "../../application/ports/ReleaseProvider.h"

class GithubReleaseProvider final : public QObject, public ReleaseProvider
{
public:
    explicit GithubReleaseProvider(QObject* parent = nullptr);

    void SetObserver(ReleaseProviderObserver* observer) override;
    void FetchLatest() override;

private:
    void FetchOne(ReleaseInfo* target, const QString& repo, const QString& assetPrefix,
                  const QString& extension = QStringLiteral(".zip"));

    QNetworkAccessManager nam_;
    ReleaseProviderObserver* observer_ = nullptr;
    ReleaseInfo client_;
    ReleaseInfo commbus_;
    ReleaseInfo installer_;
    int pending_ = 0;
};

#endif
