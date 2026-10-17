#include "SimulatorWatcher.h"

#include <algorithm>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QRegularExpression>

#include "../system/LinuxInstallLocations.h"

namespace
{
    constexpr int kSystemctlTimeoutMs = 30000;

    constexpr auto kScriptTemplate = R"script(#!/usr/bin/env bash
set -u
shopt -s nocasematch

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
client="${GSXI_CLIENT:-$here/@CLIENT@}"
proc_root="${GSXI_PROC_ROOT:-/proc}"
poll="${GSXI_POLL_SECONDS:-2}"
read -r -a simulators <<<"${GSXI_SIMULATORS:-}"
client_names=(gsx-integrator-client @CLIENT@)

if [[ ${#simulators[@]} -eq 0 ]]; then
    echo "GSXI_SIMULATORS is empty" >&2
    exit 1
fi

running() {
    local names pattern hits
    names=$(IFS='|'; echo "$*")
    pattern='^"?(.*[/\\])?('"${names//./\\.}"')"?$'
    hits=$(grep -s -z -l -i -E -- "$pattern" "$proc_root"/[0-9]*/cmdline)
    hits=${hits//"$proc_root/$$/cmdline"/}
    [[ $hits == *[![:space:]]* ]]
}

program_running() {
    local cmdline pid arg base name
    for cmdline in "$proc_root"/[0-9]*/cmdline; do
        pid="${cmdline%/cmdline}"
        pid="${pid##*/}"
        [[ $pid == "$$" ]] && continue
        arg=""
        IFS= read -r -d '' arg 2>/dev/null <"$cmdline"
        arg="${arg//\"/}"
        base="${arg##*[/\\]}"
        for name in "$@"; do
            [[ $base == "$name" ]] && return 0
        done
    done
    return 1
}

client_running() {
    program_running "${client_names[@]}" && return 0
    [[ -n $(find "$proc_root" -maxdepth 2 -path "$proc_root/[0-9]*/exe" -ilname '*/gsx-integrator-client' -print -quit 2>/dev/null) ]]
}

echo "Watching for ${simulators[*]}"
client_pid=""
active=0
while true; do
    if running "${simulators[@]}"; then
        if (( ! active )); then
            active=1
            if [[ ! -x $client ]]; then
                echo "Simulator started, but the client at $client is not executable"
            elif client_running; then
                echo "Simulator started and the client is already running"
            else
                setsid "$client" --tray >/dev/null 2>&1 &
                client_pid=$!
                echo "Simulator started, started the client as pid $client_pid"
            fi
        fi
    elif (( active )); then
        active=0
        if [[ -n $client_pid ]]; then
            kill -- "-$client_pid" 2>/dev/null || kill "$client_pid" 2>/dev/null
            echo "Simulator closed, stopped the client at pid $client_pid"
            client_pid=""
        else
            echo "Simulator closed"
        fi
    fi
    sleep "$poll"
done
)script";

    constexpr auto kUnitTemplate = R"unit([Unit]
Description=Start GSX Integrator with the simulator

[Service]
Type=simple
Environment="GSXI_SIMULATORS=@SIMULATORS@"
ExecStart="@SCRIPT@"
Restart=on-failure
RestartSec=5

[Install]
WantedBy=default.target
)unit";

    QString EscapeUnitValue(const QString& value)
    {
        QString escaped = value;
        escaped.replace(u'%', QLatin1String("%%"));
        escaped.replace(u'$', QLatin1String("$$"));
        escaped.replace(u'\\', QLatin1String("\\\\"));
        escaped.replace(u'"', QLatin1String("\\\""));
        return escaped;
    }

    bool IsProcessName(const QString& name)
    {
        static const QRegularExpression pattern(QStringLiteral(R"(^[A-Za-z0-9._-]+$)"));
        return pattern.match(name).hasMatch();
    }

    bool WriteIfChanged(const QString& path, const QString& content, const QFileDevice::Permissions permissions)
    {
        QFile existing(path);
        if (existing.open(QIODevice::ReadOnly) && QString::fromUtf8(existing.readAll()) == content
            && existing.permissions() == permissions)
        {
            return true;
        }
        existing.close();

        if (!QDir().mkpath(QFileInfo(path).absolutePath()))
        {
            return false;
        }

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            return false;
        }

        if (file.write(content.toUtf8()) < 0)
        {
            return false;
        }

        file.close();

        return QFile::setPermissions(path, permissions);
    }

    bool DiffersFrom(const QString& path, const QString& content)
    {
        QFile existing(path);
        return !existing.open(QIODevice::ReadOnly) || QString::fromUtf8(existing.readAll()) != content;
    }

    constexpr QFileDevice::Permissions kScriptPermissions =
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
        | QFileDevice::ReadGroup | QFileDevice::ExeGroup | QFileDevice::ReadOther | QFileDevice::ExeOther;

    constexpr QFileDevice::Permissions kUnitPermissions =
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther;
}

WatcherFiles BuildWatcherFiles(const QString& scriptPath, const QStringList& processNames)
{
    WatcherFiles files;
    files.script = QString::fromLatin1(kScriptTemplate).replace(QLatin1String("@CLIENT@"),
                                                                 QLatin1String(kClientAppImageName));
    files.unit = QString::fromLatin1(kUnitTemplate)
                     .replace(QLatin1String("@SIMULATORS@"), EscapeUnitValue(processNames.join(u' ')))
                     .replace(QLatin1String("@SCRIPT@"), EscapeUnitValue(scriptPath));
    return files;
}

QStringList ParseWatchedProcesses(const QString& unitText)
{
    static const QRegularExpression pattern(
        QStringLiteral(R"re(^Environment="GSXI_SIMULATORS=([^"]*)"$)re"),
        QRegularExpression::MultilineOption);

    const QRegularExpressionMatch match = pattern.match(unitText);
    if (!match.hasMatch())
    {
        return {};
    }

    return match.captured(1).split(u' ', Qt::SkipEmptyParts);
}

SimulatorWatcher::SimulatorWatcher(QString scriptPath, QString unitDir, QString systemctl)
    : scriptPath_(std::move(scriptPath)), unitDir_(std::move(unitDir)), systemctl_(std::move(systemctl))
{
}

QString SimulatorWatcher::UnitPath() const
{
    return unitDir_ + u'/' + QLatin1String(kWatcherUnitName);
}

bool SimulatorWatcher::Systemctl(const QStringList& arguments) const
{
    QProcess process;
    process.start(systemctl_, QStringList{QStringLiteral("--user")} + arguments);
    if (!process.waitForFinished(kSystemctlTimeoutMs))
    {
        process.kill();
        process.waitForFinished();
        return false;
    }

    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

bool SimulatorWatcher::SystemdAvailable() const
{
    return Systemctl({QStringLiteral("show-environment")});
}

QStringList SimulatorWatcher::WatchedProcesses() const
{
    QFile unit(UnitPath());
    if (!unit.open(QIODevice::ReadOnly))
    {
        return {};
    }

    return ParseWatchedProcesses(QString::fromUtf8(unit.readAll()));
}

bool SimulatorWatcher::Add(const QString& processName)
{
    if (!IsProcessName(processName))
    {
        return false;
    }

    QStringList names = WatchedProcesses();
    if (!names.contains(processName))
    {
        names.append(processName);
    }
    names.sort();

    return Apply(names);
}

bool SimulatorWatcher::Remove(const QString& processName)
{
    QStringList names = WatchedProcesses();
    if (!names.contains(processName))
    {
        return true;
    }

    names.removeAll(processName);

    return names.isEmpty() ? Disable() : Apply(names);
}

bool SimulatorWatcher::Apply(const QStringList& processNames)
{
    if (!SystemdAvailable())
    {
        return false;
    }

    const WatcherFiles files = BuildWatcherFiles(scriptPath_, processNames);

    const bool unitExisted = QFile::exists(UnitPath());
    const bool filesChanged = DiffersFrom(UnitPath(), files.unit) || DiffersFrom(scriptPath_, files.script);

    if (!WriteIfChanged(scriptPath_, files.script, kScriptPermissions)
        || !WriteIfChanged(UnitPath(), files.unit, kUnitPermissions))
    {
        return false;
    }

    const QString unitName = QLatin1String(kWatcherUnitName);
    if (!Systemctl({QStringLiteral("daemon-reload")}) || !Systemctl({QStringLiteral("enable"), QStringLiteral("--now"), unitName}))
    {
        return false;
    }

    return !(unitExisted && filesChanged) || Systemctl({QStringLiteral("restart"), unitName});
}

bool SimulatorWatcher::Disable()
{
    const bool unitExists = QFile::exists(UnitPath());
    const bool scriptExists = QFile::exists(scriptPath_);
    if (!unitExists && !scriptExists)
    {
        return true;
    }

    const bool available = SystemdAvailable();
    if (unitExists && available
        && !Systemctl({QStringLiteral("disable"), QStringLiteral("--now"), QLatin1String(kWatcherUnitName)}))
    {
        return false;
    }

    const bool removed = (!unitExists || QFile::remove(UnitPath())) && (!scriptExists || QFile::remove(scriptPath_));
    if (available)
    {
        (void)Systemctl({QStringLiteral("daemon-reload")});
    }

    return removed;
}
