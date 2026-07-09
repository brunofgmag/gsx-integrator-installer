#include "RemovalScript.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

bool WriteRemovalScript(const QString& scriptPath, const QString& dirToRemove)
{
    QFile script(scriptPath);

    if (!script.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    const QString dir = QDir::toNativeSeparators(dirToRemove);
    QTextStream out(&script);
    out.setEncoding(QStringConverter::Utf8);
    out << "@echo off\r\n";
    out << "chcp 65001 >nul\r\n";
    out << "cd /d \"%TEMP%\"\r\n";
    out << "set tries=0\r\n";
    out << ":retry\r\n";
    out << "ping -n 2 127.0.0.1 >nul\r\n";
    out << "rmdir /s /q \"" << dir << "\"\r\n";
    out << "if not exist \"" << dir << "\" goto done\r\n";
    out << "set /a tries+=1\r\n";
    out << "if %tries% lss 300 goto retry\r\n";
    out << ":done\r\n";
    out << "del \"%~f0\"\r\n";

    return true;
}
