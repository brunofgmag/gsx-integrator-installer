#ifndef GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_PACKAGEFILES_H
#define GSX_INTEGRATOR_INSTALLER_INFRASTRUCTURE_INSTALL_PACKAGEFILES_H

#include <functional>
#include <QtCore/QList>
#include <QtCore/QString>
#include "../../application/ports/InstallerGateway.h"
#include "../../domain/model/InstallOutcome.h"
#include "../../domain/model/ReleaseInfo.h"

[[nodiscard]] DownloadResult DownloadVerifiedRelease(const ReleaseInfo& release,
                                                     const std::function<void(double)>& onFraction);

[[nodiscard]] InstallOutcome ExtractZipTo(const QString& zipPath, const QString& destDir);

[[nodiscard]] bool RemoveExistingPackageDir(const QString& packageDir);

void WriteCommbusVersionMarker(const QString& packageDir, const QString& version);

[[nodiscard]] QString ReadInstalledCommbusVersion(const QString& communityPath);

[[nodiscard]] QList<CommbusPackage> FindInstalledCommbusPackages();

#endif
