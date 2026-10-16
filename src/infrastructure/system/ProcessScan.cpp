#include "ProcessScan.h"

#include <algorithm>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

namespace
{
    bool IsPid(const QString& name)
    {
        return !name.isEmpty() && std::ranges::all_of(name, [](const QChar c) { return c.isDigit(); });
    }

    bool NameListContains(const QStringList& baseNames, const QString& base)
    {
        return std::ranges::any_of(baseNames, [&base](const QString& name)
        {
            return base.compare(name, Qt::CaseInsensitive) == 0;
        });
    }

    bool ExecutableMatches(const QString& exeLinkPath, const QStringList& baseNames)
    {
        QString target = QFileInfo(exeLinkPath).symLinkTarget();
        if (target.endsWith(QLatin1String(" (deleted)")))
        {
            target.chop(10);
        }

        return !target.isEmpty() && NameListContains(baseNames, ArgumentBaseName(target));
    }

    bool ProgramNameMatches(const QString& cmdlinePath, const QStringList& baseNames)
    {
        QFile file(cmdlinePath);
        if (!file.open(QIODevice::ReadOnly))
        {
            return false;
        }

        const QByteArray first = file.readAll().split('\0').value(0);

        return !first.isEmpty() && NameListContains(baseNames, ArgumentBaseName(QString::fromUtf8(first)));
    }

    bool CommandLineMatches(const QString& cmdlinePath, const QStringList& baseNames)
    {
        QFile file(cmdlinePath);
        if (!file.open(QIODevice::ReadOnly))
        {
            return false;
        }

        const QList<QByteArray> arguments = file.readAll().split('\0');
        for (const QByteArray& raw : arguments)
        {
            if (raw.isEmpty())
            {
                continue;
            }

            if (NameListContains(baseNames, ArgumentBaseName(QString::fromUtf8(raw))))
            {
                return true;
            }
        }

        return false;
    }

    bool ScanProcesses(const QString& procRoot, const QStringList& baseNames, const bool anyArgument)
    {
        if (baseNames.isEmpty())
        {
            return false;
        }

        const QStringList entries = QDir(procRoot).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString& entry : entries)
        {
            if (!IsPid(entry))
            {
                continue;
            }

            const QString pidDir = procRoot + u'/' + entry;
            const QString cmdlinePath = pidDir + QStringLiteral("/cmdline");
            const bool byCommandLine = anyArgument ? CommandLineMatches(cmdlinePath, baseNames)
                                                   : ProgramNameMatches(cmdlinePath, baseNames);
            if (byCommandLine || ExecutableMatches(pidDir + QStringLiteral("/exe"), baseNames))
            {
                return true;
            }
        }

        return false;
    }
}

QString ArgumentBaseName(const QString& argument)
{
    QString text = argument.trimmed();
    if (text.size() >= 2 && text.startsWith(u'"') && text.endsWith(u'"'))
    {
        text = text.mid(1, text.size() - 2);
    }

    const qsizetype cut = qMax(text.lastIndexOf(u'/'), text.lastIndexOf(u'\\'));
    return cut < 0 ? text : text.mid(cut + 1);
}

bool IsAnyProcessRunning(const QString& procRoot, const QStringList& baseNames)
{
    return ScanProcesses(procRoot, baseNames, true);
}

bool IsProgramRunning(const QString& procRoot, const QStringList& baseNames)
{
    return ScanProcesses(procRoot, baseNames, false);
}
