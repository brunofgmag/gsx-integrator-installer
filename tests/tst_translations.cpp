#include <QtTest/QTest>

#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QXmlStreamReader>

namespace
{
    struct TsMessage
    {
        QString source;
        QString translation;
        bool unfinished = false;
    };

    QList<TsMessage> ParseTs(const QString& fileName)
    {
        QFile file(QStringLiteral(GSXI_I18N_DIR) + u'/' + fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            return {};
        }

        QXmlStreamReader xml(&file);
        QList<TsMessage> messages;
        QString currentSource;

        while (!xml.atEnd())
        {
            xml.readNext();

            if (!xml.isStartElement())
            {
                continue;
            }

            if (xml.name() == u"source")
            {
                currentSource = xml.readElementText();
            }
            else if (xml.name() == u"translation")
            {
                const bool unfinished =
                    xml.attributes().value(QStringLiteral("type")) == u"unfinished";
                messages.push_back({currentSource, xml.readElementText(), unfinished});
            }
        }

        return messages;
    }

    QStringList SortedSources(const QList<TsMessage>& messages)
    {
        QStringList sources;

        sources.reserve(messages.size());

        for (const TsMessage& message : messages)
        {
            sources.append(message.source);
        }

        sources.sort();

        return sources;
    }
}

class TranslationsTest final : public QObject
{
    Q_OBJECT

private slots:
    static void enAndPtBrCoverTheSameSources();
    static void ptBrIsFullyTranslated();
};

void TranslationsTest::enAndPtBrCoverTheSameSources()
{
    const QList<TsMessage> en = ParseTs(QStringLiteral("app_en.ts"));
    const QList<TsMessage> pt = ParseTs(QStringLiteral("app_pt_BR.ts"));

    QVERIFY2(!en.isEmpty(), "app_en.ts had no messages — wrong GSXI_I18N_DIR?");
    QCOMPARE(SortedSources(en), SortedSources(pt));
}

void TranslationsTest::ptBrIsFullyTranslated()
{
    const QList<TsMessage> pt = ParseTs(QStringLiteral("app_pt_BR.ts"));

    QVERIFY(!pt.isEmpty());

    for (const TsMessage& message : pt)
    {
        QVERIFY2(!message.unfinished, qPrintable(QStringLiteral("Unfinished pt_BR: %1").arg(message.source)));
        QVERIFY2(!message.translation.isEmpty(), qPrintable(QStringLiteral("Empty pt_BR: %1").arg(message.source)));
    }
}

QTEST_APPLESS_MAIN(TranslationsTest)

#include "tst_translations.moc"
