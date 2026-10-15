#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_RELEASEPACKAGE_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_RELEASEPACKAGE_H

#include <QtCore/QByteArray>
#include <QtCore/QString>

enum class ExtractStatus
{
    Success = 0,
    OpenFailed,
    InvalidArchive,
    UnsafeEntry,
    WriteFailed,
};

struct ExtractResult
{
    ExtractStatus status = ExtractStatus::Success;
    QString detail;

    [[nodiscard]] bool ok() const { return status == ExtractStatus::Success; }
};

[[nodiscard]] ExtractResult ExtractZip(const QString& zipPath, const QString& destDir);

[[nodiscard]] QString ParseSha256(const QByteArray& content);

[[nodiscard]] QString ReadSha256File(const QString& shaFilePath);

[[nodiscard]] QString Sha256File(const QString& path);

[[nodiscard]] bool Sha256Matches(const QString& filePath, const QString& shaFilePath);

#endif
