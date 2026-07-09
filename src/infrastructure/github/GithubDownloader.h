#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_GITHUB_GITHUBDOWNLOADER_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_GITHUB_GITHUBDOWNLOADER_H

#include <functional>
#include <QtCore/QString>
#include <QtCore/QUrl>

QString DownloadFile(const QUrl& url,
                     const QString& destPath,
                     const std::function<void(qint64, qint64)>& progress = {});

QString Sha256File(const QString& path);

#endif
