#ifndef GSX_INTEGRATOR_INSTALLER_TESTS_FAKESTEAMTREE_H
#define GSX_INTEGRATOR_INSTALLER_TESTS_FAKESTEAMTREE_H

#include <QtCore/QStringList>
#include <QtCore/QTemporaryDir>

#include "FileTree.h"

class FakeSteamTree
{
public:
    FakeSteamTree()
    {
        Build();
    }

    [[nodiscard]] bool IsValid() const { return built_ && root_.isValid(); }
    [[nodiscard]] QString Root() const { return QFileInfo(root_.path()).canonicalFilePath(); }
    [[nodiscard]] QString Home() const { return Root() + QStringLiteral("/home"); }
    [[nodiscard]] QString DefaultSteamapps() const { return Home() + QStringLiteral("/.local/share/Steam/steamapps"); }
    [[nodiscard]] QString LegacySteamapps() const { return Home() + QStringLiteral("/.steam/steam/steamapps"); }
    [[nodiscard]] QString GamesLibrary() const { return Root() + QStringLiteral("/games/SteamLibrary"); }
    [[nodiscard]] QStringList SteamappsRoots() const { return {DefaultSteamapps(), LegacySteamapps()}; }

    [[nodiscard]] QString Prefix2024() const
    {
        return GamesLibrary() + QStringLiteral("/steamapps/compatdata/2537590/pfx");
    }

    [[nodiscard]] QString Prefix2020() const
    {
        return DefaultSteamapps() + QStringLiteral("/compatdata/1250410/pfx");
    }

    [[nodiscard]] QString Packages2024() const { return GamesLibrary() + QStringLiteral("/MSFS2024"); }
    [[nodiscard]] QString Packages2020() const { return DefaultSteamapps() + QStringLiteral("/common/MSFS 2020/Packages"); }
    [[nodiscard]] QString Community2024() const { return Packages2024() + QStringLiteral("/community"); }
    [[nodiscard]] QString Community2020() const { return Packages2020() + QStringLiteral("/Community"); }
    [[nodiscard]] QString UserCfgDir2024() const
    {
        return Prefix2024() + QStringLiteral("/drive_c/users/steamuser/AppData/Roaming/Microsoft Flight Simulator 2024");
    }
    [[nodiscard]] QString UserCfgDir2020() const
    {
        return Prefix2020() + QStringLiteral("/drive_c/users/steamuser/AppData/Roaming/Microsoft Flight Simulator");
    }
    [[nodiscard]] QString ExeXml2024() const { return UserCfgDir2024() + QStringLiteral("/exe.xml"); }
    [[nodiscard]] QString ExeXml2020() const { return UserCfgDir2020() + QStringLiteral("/EXE.xml"); }

private:
    void Build()
    {
        if (!root_.isValid())
        {
            return;
        }

        const QString vdf = QStringLiteral(
            "\"libraryfolders\"\n{\n"
            "\t\"0\"\n\t{\n\t\t\"path\"\t\t\"%1\"\n\t\t\"label\"\t\t\"\"\n\t\t\"apps\"\n\t\t{\n"
            "\t\t\t\"1250410\"\t\t\"1\"\n\t\t}\n\t}\n"
            "\t\"1\"\n\t{\n\t\t\"path\"\t\t\"%2\"\n\t\t\"label\"\t\t\"games\"\n\t\t\"apps\"\n\t\t{\n"
            "\t\t\t\"2537590\"\t\t\"1\"\n\t\t}\n\t}\n}\n")
                                .arg(Home() + QStringLiteral("/.local/share/Steam"), GamesLibrary());

        bool ok = WriteTestFile(DefaultSteamapps() + QStringLiteral("/libraryfolders.vdf"), vdf.toUtf8());
        ok = MakeTestSymlink(Home() + QStringLiteral("/.local/share/Steam"), Home() + QStringLiteral("/.steam/steam")) && ok;

        ok = BuildPrefix(Prefix2024(), QStringLiteral("S"), GamesLibrary(), UserCfgDir2024(),
                         QStringLiteral("exe.xml"), QStringLiteral("S:\\MSFS2024")) && ok;
        ok = BuildPrefix(Prefix2020(), QStringLiteral("s"), DefaultSteamapps(), UserCfgDir2020(),
                         QString(), QStringLiteral("s:\\common\\MSFS 2020\\Packages")) && ok;

        ok = QDir().mkpath(Community2024()) && QDir().mkpath(Packages2024() + QStringLiteral("/Official")) && ok;
        ok = QDir().mkpath(Community2020()) && ok;

        built_ = ok;
    }

    bool BuildPrefix(const QString& prefix, const QString& letter, const QString& driveTarget,
                     const QString& userCfgDir, const QString& exeXmlName, const QString& packagesPath)
    {
        bool ok = WriteTestFile(userCfgDir + QStringLiteral("/UserCfg.opt"),
                                QStringLiteral("{Graphics\n}\nInstalledPackagesPath \"%1\"\n").arg(packagesPath).toUtf8());
        if (!exeXmlName.isEmpty())
        {
            ok = WriteTestFile(userCfgDir + u'/' + exeXmlName, "<SimBase.Document/>") && ok;
        }

        ok = MakeTestSymlink(QStringLiteral("../drive_c"), prefix + QStringLiteral("/dosdevices/c:")) && ok;
        ok = MakeTestSymlink(driveTarget, prefix + QStringLiteral("/dosdevices/") + letter.toLower() + u':') && ok;
        ok = MakeTestSymlink(QStringLiteral("/"), prefix + QStringLiteral("/dosdevices/z:")) && ok;

        return ok;
    }

    QTemporaryDir root_;
    bool built_ = false;
};

#endif
