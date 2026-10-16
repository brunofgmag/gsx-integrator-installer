#include "PackageFiles.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "../../domain/Versions.h"
#include "../github/GithubDownloader.h"
#include "../system/InstallLocations.h"
#include "../system/SimDetection.h"
#include "ReleasePackage.h"

DownloadResult DownloadVerifiedRelease(const ReleaseInfo& release,
                                       const std::function<void(double)>& onFraction)
{
    const QString tempDir = QDir::tempPath() + QStringLiteral("/gsx-integrator-installer");
    (void)QDir().mkpath(tempDir);
    const QString packagePath = tempDir + u'/' + release.assetName;

    QString error = DownloadFile(release.assetUrl, packagePath,
                                 [&](const qint64 received, const qint64 total)
                                 {
                                     if (onFraction && total > 0)
                                     {
                                         onFraction(static_cast<double>(received) / static_cast<double>(total));
                                     }
                                 });
    if (!error.isEmpty())
    {
        return {InstallStatus::DownloadFailed, error, {}};
    }

    const QString shaPath = packagePath + QStringLiteral(".sha256");
    error = DownloadFile(release.shaUrl, shaPath);
    if (!error.isEmpty())
    {
        return {InstallStatus::DownloadFailed, error, {}};
    }

    if (!Sha256Matches(packagePath, shaPath))
    {
        return {InstallStatus::ChecksumMismatch, release.assetName, {}};
    }

    return {InstallStatus::Success, {}, packagePath};
}

InstallOutcome ExtractZipTo(const QString& zipPath, const QString& destDir)
{
    const ExtractResult result = ExtractZip(zipPath, destDir);
    switch (result.status)
    {
    case ExtractStatus::Success:
        return {};
    case ExtractStatus::UnsafeEntry:
        return {InstallStatus::ExtractUnsafeEntry, result.detail};
    case ExtractStatus::WriteFailed:
        return {InstallStatus::ExtractWriteFailed, result.detail};
    case ExtractStatus::OpenFailed:
    case ExtractStatus::InvalidArchive:
        break;
    }

    return {InstallStatus::ExtractFailed, result.detail};
}

bool RemoveExistingPackageDir(const QString& packageDir)
{
    const QFileInfo entry(packageDir);
    if (entry.isJunction() || entry.isSymbolicLink())
    {
#ifdef Q_OS_WIN
        return QDir().rmdir(packageDir);
#else
        return QFile::remove(packageDir);
#endif
    }

    if (!QDir(packageDir).exists())
    {
        return true;
    }

    return QDir(packageDir).removeRecursively();
}

void WriteCommbusVersionMarker(const QString& packageDir, const QString& version)
{
    QFile marker(packageDir + u'/' + QLatin1String(kCommbusVersionMarker));
    if (marker.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        marker.write(version.toUtf8());
    }
}

QString ReadInstalledCommbusVersion(const QString& communityPath)
{
    const QString packageDir = communityPath + u'/' + QLatin1String(kCommbusPackageName);

    QFile marker(packageDir + u'/' + QLatin1String(kCommbusVersionMarker));
    if (marker.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        const QString version = QString::fromUtf8(marker.readAll()).trimmed();
        if (!version.isEmpty())
        {
            return version;
        }
    }

    QFile file(packageDir + QStringLiteral("/manifest.json"));

    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }

    return ParsePackageVersion(file.readAll());
}

QList<CommbusPackage> FindInstalledCommbusPackages()
{
    QList<CommbusPackage> packages;
    for (const SimInstall& sim : DetectSims())
    {
        const QString packageDir = sim.communityPath + u'/' + QLatin1String(kCommbusPackageName);
        if (QDir(packageDir).exists())
        {
            packages.append({packageDir, sim.processName, sim.label});
        }
    }

    return packages;
}
