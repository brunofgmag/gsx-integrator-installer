#include "DesktopEntry.h"

QString QuoteDesktopExec(const QString& path)
{
    QString escaped;
    escaped.reserve(path.size() + 2);
    for (const QChar c : path)
    {
        if (c == u'"' || c == u'`' || c == u'$' || c == u'\\')
        {
            escaped.append(u'\\');
        }
        escaped.append(c);
    }

    escaped.replace(u'\\', QLatin1String("\\\\"));
    escaped.replace(u'%', QLatin1String("%%"));

    return u'"' + escaped + u'"';
}

QString BuildDesktopEntry(const DesktopEntry& entry)
{
    QString text = QStringLiteral("[Desktop Entry]\nType=Application\nVersion=1.0\n");
    text += QStringLiteral("Name=") + entry.name + u'\n';
    text += QStringLiteral("Comment=") + entry.comment + u'\n';
    text += QStringLiteral("Exec=") + QuoteDesktopExec(entry.execPath) + u'\n';
    if (!entry.iconPath.isEmpty())
    {
        text += QStringLiteral("Icon=") + entry.iconPath + u'\n';
    }
    text += QStringLiteral("Terminal=false\nCategories=Utility;\n");

    return text;
}
