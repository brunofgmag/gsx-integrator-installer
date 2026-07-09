#include "RemovalScript.h"

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
rmdir /s /q "@DIR@"
if not exist "@DIR@" goto done
set /a tries+=1
if %tries% lss 300 goto retry
:done
del "%~f0"
)";
}

bool WriteRemovalScript(const QString& scriptPath, const QString& dirToRemove)
{
    QFile script(scriptPath);

    if (!script.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    QString content = QLatin1String(kScript);
    content.replace(QLatin1String("@DIR@"), QDir::toNativeSeparators(dirToRemove));
    content.remove(u'\r');
    content.replace(u'\n', QLatin1String("\r\n"));

    return script.write(content.toUtf8()) != -1;
}
