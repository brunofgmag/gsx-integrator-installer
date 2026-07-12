#include "ExeXml.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtXml/QDomDocument>

namespace
{
    const auto kLaunchAddonTag = QStringLiteral("Launch.Addon");

    bool LoadOrCreate(const QString& path, QDomDocument* doc)
    {
        QFile file(path);
        if (file.exists())
        {
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                return false;
            }
            return static_cast<bool>(doc->setContent(&file));
        }
        if (!QFileInfo(QFileInfo(path).absolutePath()).isDir())
        {
            return false;
        }
        return static_cast<bool>(doc->setContent(QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            "<SimBase.Document Type=\"Launch\" version=\"1,0\">\n"
            "  <Descr>Launch</Descr>\n"
            "  <Filename>EXE.xml</Filename>\n"
            "  <Disabled>False</Disabled>\n"
            "  <Launch.ManualLoad>False</Launch.ManualLoad>\n"
            "</SimBase.Document>\n")));
    }

    bool Save(const QDomDocument& doc, const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            return false;
        }

        return file.write(doc.toByteArray(2)) >= 0;
    }

    void SetChildText(QDomDocument& doc, QDomElement& parent, const QString& tag, const QString& text)
    {
        QDomElement element = parent.firstChildElement(tag);
        if (element.isNull())
        {
            element = doc.createElement(tag);
            parent.appendChild(element);
        }

        while (!element.firstChild().isNull())
        {
            element.removeChild(element.firstChild());
        }

        element.appendChild(doc.createTextNode(text));
    }
}

bool ExeXmlAddUpdate(const QString& exeXmlPath, const QString& exePath, const QString& appName)
{
    QDomDocument doc;
    if (!LoadOrCreate(exeXmlPath, &doc))
    {
        return false;
    }

    QDomElement root = doc.documentElement();
    if (root.isNull())
    {
        return false;
    }

    const QString exeName = QFileInfo(exePath).fileName();
    bool found = false;
    for (QDomElement addon = root.firstChildElement(kLaunchAddonTag); !addon.isNull();
         addon = addon.nextSiblingElement(kLaunchAddonTag))
    {
        if (addon.text().contains(exeName, Qt::CaseInsensitive))
        {
            found = true;
            SetChildText(doc, addon, QStringLiteral("Disabled"), QStringLiteral("False"));
            SetChildText(doc, addon, QStringLiteral("ManualLoad"), QStringLiteral("False"));
            SetChildText(doc, addon, QStringLiteral("Name"), appName);
            SetChildText(doc, addon, QStringLiteral("Path"), exePath);
            SetChildText(doc, addon, QStringLiteral("CommandLine"), QStringLiteral("--tray"));
        }
    }

    if (!found)
    {
        QDomElement addon = doc.createElement(kLaunchAddonTag);
        SetChildText(doc, addon, QStringLiteral("Disabled"), QStringLiteral("False"));
        SetChildText(doc, addon, QStringLiteral("ManualLoad"), QStringLiteral("False"));
        SetChildText(doc, addon, QStringLiteral("Name"), appName);
        SetChildText(doc, addon, QStringLiteral("Path"), exePath);
        SetChildText(doc, addon, QStringLiteral("CommandLine"), QStringLiteral("--tray"));
        root.appendChild(addon);
    }

    return Save(doc, exeXmlPath);
}

bool ExeXmlRemove(const QString& exeXmlPath, const QString& exeName)
{
    QFile file(exeXmlPath);

    if (!file.exists())
    {
        return true;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        return false;
    }

    QDomElement root = doc.documentElement();
    if (root.isNull())
    {
        return false;
    }

    QList<QDomElement> obsolete;
    for (QDomElement addon = root.firstChildElement(kLaunchAddonTag); !addon.isNull();
         addon = addon.nextSiblingElement(kLaunchAddonTag))
    {
        if (addon.text().contains(exeName, Qt::CaseInsensitive))
        {
            obsolete.append(addon);
        }
    }

    if (obsolete.isEmpty())
    {
        return true;
    }

    for (QDomElement& addon : obsolete)
    {
        root.removeChild(addon);
    }

    return Save(doc, exeXmlPath);
}
