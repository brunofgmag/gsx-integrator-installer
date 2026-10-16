#include "SimDetection.h"
#include "LinuxSimDetection.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "ProtonPrefix.h"

namespace
{
    struct SimCandidate
    {
        const char* id;
        const char* label;
        int appId;
        const char* roamingDirName;
        const char* processName;
        int generation;
    };

    constexpr SimCandidate kCandidates[] = {
        {
            "msfs2020-steam", "MSFS 2020 (Steam)", kMsfs2020AppId,
            "Microsoft Flight Simulator",
            "FlightSimulator.exe", 2020
        },
        {
            "msfs2024-steam", "MSFS 2024 (Steam)", kMsfs2024AppId,
            "Microsoft Flight Simulator 2024",
            "FlightSimulator2024.exe", 2024
        },
    };

    QString UserCfgDir(const QString& prefix, const SimCandidate& candidate)
    {
        const QString roaming = ProtonKnownFolderPath(prefix, ProtonKnownFolder::AppDataRoaming);
        if (roaming.isEmpty())
        {
            return {};
        }

        const QString dir = ResolveIgnoringCase(roaming, QLatin1String(candidate.roamingDirName));
        return QFileInfo(dir).isDir() ? dir : QString();
    }

    QString ExeXmlPathIn(const QString& userCfgDir)
    {
        const QString existing = ResolveIgnoringCase(userCfgDir, QStringLiteral("EXE.xml"));
        return existing.isEmpty() ? userCfgDir + QStringLiteral("/EXE.xml") : existing;
    }
}

QStringList SteamappsDirsOfCurrentUser()
{
    return DefaultSteamappsDirs(QDir::homePath());
}

QList<SimInstall> DetectSims(const QStringList& steamappsDirs)
{
    QList<SimInstall> result;

    for (const SimCandidate& candidate : kCandidates)
    {
        const QString prefix = FindProtonPrefix(steamappsDirs, candidate.appId);
        const QString userCfgDir = prefix.isEmpty() ? QString() : UserCfgDir(prefix, candidate);
        if (userCfgDir.isEmpty())
        {
            continue;
        }

        QFile userCfg(ResolveIgnoringCase(userCfgDir, QStringLiteral("UserCfg.opt")));
        if (!userCfg.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            continue;
        }

        const QString packagesPath = ParseInstalledPackagesPath(QString::fromUtf8(userCfg.readAll()));
        if (packagesPath.isEmpty())
        {
            continue;
        }

        const QString packagesDir = WindowsPathToHost(prefix, packagesPath);
        const QString communityPath = packagesDir.isEmpty()
                                          ? QString()
                                          : ResolveIgnoringCase(packagesDir, QStringLiteral("Community"));
        if (!QFileInfo(communityPath).isDir())
        {
            continue;
        }

        result.append({
            QLatin1String(candidate.id), QLatin1String(candidate.label),
            communityPath, QLatin1String(candidate.processName),
            ExeXmlPathIn(userCfgDir), candidate.generation
        });
    }

    return result;
}

QStringList CandidateExeXmlPaths(const int generation, const QStringList& steamappsDirs)
{
    QStringList result;

    for (const SimCandidate& candidate : kCandidates)
    {
        if (generation != 0 && candidate.generation != generation)
        {
            continue;
        }

        const QString prefix = FindProtonPrefix(steamappsDirs, candidate.appId);
        const QString userCfgDir = prefix.isEmpty() ? QString() : UserCfgDir(prefix, candidate);
        if (!userCfgDir.isEmpty())
        {
            result.append(ExeXmlPathIn(userCfgDir));
        }
    }

    return result;
}

QString SimProcessNameForExeXml(const QString& exeXmlPath, const QStringList& steamappsDirs)
{
    for (const SimCandidate& candidate : kCandidates)
    {
        if (CandidateExeXmlPaths(candidate.generation, steamappsDirs).contains(exeXmlPath))
        {
            return QLatin1String(candidate.processName);
        }
    }

    return {};
}

QList<SimInstall> DetectSims()
{
    return DetectSims(SteamappsDirsOfCurrentUser());
}

QStringList CandidateExeXmlPaths(const int generation)
{
    return CandidateExeXmlPaths(generation, SteamappsDirsOfCurrentUser());
}
