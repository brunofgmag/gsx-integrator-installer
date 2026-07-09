#include "SimDetection.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QStandardPaths>

QString ParseInstalledPackagesPath(const QString& userCfgContent)
{
    static const QRegularExpression pattern(QStringLiteral("InstalledPackagesPath\\s+\"([^\"]*)\""));
    const QRegularExpressionMatch match = pattern.match(userCfgContent);
    return match.hasMatch() ? match.captured(1).trimmed() : QString();
}

namespace
{
    struct SimCandidate
    {
        const char* id;
        const char* label;
        const char* userCfgSubPath;
        const char* processName;
        int generation;
    };

    constexpr SimCandidate kCandidates[] = {
        {
            "msfs2020-steam", "MSFS 2020 (Steam)",
            "AppData/Roaming/Microsoft Flight Simulator/UserCfg.opt",
            "FlightSimulator.exe", 2020
        },
        {
            "msfs2020-msstore", "MSFS 2020 (Microsoft Store)",
            "AppData/Local/Packages/Microsoft.FlightSimulator_8wekyb3d8bbwe/LocalCache/UserCfg.opt",
            "FlightSimulator.exe", 2020
        },
        {
            "msfs2024-steam", "MSFS 2024 (Steam)",
            "AppData/Roaming/Microsoft Flight Simulator 2024/UserCfg.opt",
            "FlightSimulator2024.exe", 2024
        },
        {
            "msfs2024-msstore", "MSFS 2024 (Microsoft Store)",
            "AppData/Local/Packages/Microsoft.Limitless_8wekyb3d8bbwe/LocalCache/UserCfg.opt",
            "FlightSimulator2024.exe", 2024
        },
    };

    QString ExeXmlPathFor(const QString& profile, const SimCandidate& candidate)
    {
        const QFileInfo userCfg(profile + u'/' + QLatin1String(candidate.userCfgSubPath));
        return QDir::toNativeSeparators(userCfg.absolutePath() + QStringLiteral("/EXE.xml"));
    }
}

QList<SimInstall> DetectSims()
{
    QList<SimInstall> result;
    const QString profile = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    for (const SimCandidate& candidate : kCandidates)
    {
        QFile userCfg(profile + u'/' + QLatin1String(candidate.userCfgSubPath));
        if (!userCfg.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            continue;
        }

        const QString packagesPath = ParseInstalledPackagesPath(QString::fromUtf8(userCfg.readAll()));
        if (packagesPath.isEmpty())
        {
            continue;
        }

        const QString communityPath = QDir::toNativeSeparators(packagesPath + QStringLiteral("/Community"));
        if (!QDir(communityPath).exists())
        {
            continue;
        }

        result.append({
            QLatin1String(candidate.id), QLatin1String(candidate.label),
            communityPath, QLatin1String(candidate.processName),
            ExeXmlPathFor(profile, candidate), candidate.generation
        });
    }

    return result;
}

QStringList CandidateExeXmlPaths(const int generation)
{
    QStringList result;
    const QString profile = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    for (const SimCandidate& candidate : kCandidates)
    {
        if (generation != 0 && candidate.generation != generation)
        {
            continue;
        }

        const QString exeXmlPath = ExeXmlPathFor(profile, candidate);
        if (QFileInfo(QFileInfo(exeXmlPath).absolutePath()).isDir())
        {
            result.append(exeXmlPath);
        }
    }

    return result;
}
