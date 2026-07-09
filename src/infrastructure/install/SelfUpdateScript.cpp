#include "SelfUpdateScript.h"

#include <QtCore/QDir>
#include <QtCore/QFile>

namespace
{
    constexpr auto kScript = R"(@echo off
chcp 65001 >nul
cd /d "%TEMP%"
set tries=0
:retry
ping -n 2 127.0.0.1 >nul
robocopy "@PAYLOAD@" "@APP@" /e /r:0 /w:0 >nul
if %errorlevel% lss 8 goto ok
set /a tries+=1
if %tries% lss 300 goto retry
goto cleanup
:ok
start "" /d "@APP@" "@EXE@"
:cleanup
rmdir /s /q "@CLEANUP@"
del "%~f0"
)";
}

bool WriteSelfUpdateScript(const QString& scriptPath, const QString& payloadDir,
                           const QString& appDir, const QString& exePath,
                           const QString& cleanupDir)
{
    QFile script(scriptPath);

    if (!script.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    QString content = QLatin1String(kScript);
    content.replace(QLatin1String("@PAYLOAD@"), QDir::toNativeSeparators(payloadDir));
    content.replace(QLatin1String("@APP@"), QDir::toNativeSeparators(appDir));
    content.replace(QLatin1String("@EXE@"), QDir::toNativeSeparators(exePath));
    content.replace(QLatin1String("@CLEANUP@"), QDir::toNativeSeparators(cleanupDir));
    content.remove(u'\r');
    content.replace(u'\n', QLatin1String("\r\n"));

    return script.write(content.toUtf8()) != -1;
}
