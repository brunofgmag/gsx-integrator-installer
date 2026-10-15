#include "SimDetection.h"

#include <QtCore/QRegularExpression>

QString ParseInstalledPackagesPath(const QString& userCfgContent)
{
    static const QRegularExpression pattern(QStringLiteral("InstalledPackagesPath\\s+\"([^\"]*)\""));
    const QRegularExpressionMatch match = pattern.match(userCfgContent);
    return match.hasMatch() ? match.captured(1).trimmed() : QString();
}
