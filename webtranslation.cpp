#include "webtranslation.h"
#include <QDesktopServices>
#include <QUrl>
#include <sstream>
#include "string_utf8.h"

WebTranslation::WebTranslation()
{
//    setLanguage("en");
    setLanguage("ru");
}

void WebTranslation::wortTranslate(const std::string &str, WebSite ws)
{
    if (str.empty())
        return;
    AreaUtf8 au8(str);
    au8.trim();
    AreaUtf8 wort = au8.getToken();
    if (wort.empty())
        return;
    const UrlSite &currUrl = urls[size_t(ws)];
    QString url = QString::fromStdString(currUrl.firstUrl + wort.toString() + currUrl.lastUrl);
    QDesktopServices::openUrl(QUrl(url));
}

void WebTranslation::allTranslate(const std::string &str, WebSite ws)
{
    if (str.empty())
        return;

    std::stringstream ss;
    for (size_t i = 0; i < str.size(); ++i)
    {
        const char sym = str[i];
        if (sym >= 0x10 && sym < 0x30)
            ss << "%" << std::hex << static_cast<int>(sym);
        else
            ss << sym;
    }
    const UrlSite &currUrl = urls[size_t(ws)];
    QString url = QString::fromStdString(currUrl.firstUrl + ss.str() + currUrl.lastUrl);
    QDesktopServices::openUrl(QUrl(url));
}

bool WebTranslation::setLanguage(const std::string &lg)
{
    if (lg == "ru")
    {
         urls = std::vector<UrlSite>({
         {
         "lingvo",
         "https://www.lingvolive.com/ru-ru/translate/de-ru/",
         ""
         },
         {
         "yandex",
         "https://translate.yandex.ru/?utm_source=wizard&text=",
         "&lang=de-ru"
         },
         {
         "google",
         "https://translate.google.de/?hl=ru&tab=TT&sl=de&tl=ru&text=",
         "&op=translate"
         },
         {
         "dwds",
         "https://www.dwds.de/wb/",
         ""
         },
         {
         "verbF",
         "https://www.verbformen.ru/sprjazhenie/",
         ".htm"
         },
         {
         "pons",
         "https://ru.pons.com/%D0%BF%D0%B5%D1%80%D0%B5%D0%B2%D0%BE%D0%B4/%D0%BD%D0%B5%D0%BC%D0%B5%D1%86%D0%BA%D0%B8%D0%B9-%D1%80%D1%83%D1%81%D1%81%D0%BA%D0%B8%D0%B9/",
         ""
         }
         });
         return true;
    }
    else
    {
         urls = std::vector<UrlSite>({
         {
         "lingvo",
         "https://www.lingvolive.com/en-us/translate/de-en/",
         ""
         },
         {
         "yandex",
         "https://translate.yandex.ru/?lang=de-en&text=",
         ""
         },
         {
         "google",
         "https://translate.google.de/?sl=de&tl=en&text=",
         "&op=translate"
         },
         {
         "dwds",
         "https://www.dwds.de/wb/",
         ""
         },
         {
         "verbF",
         "https://www.verbformen.com/conjugation/?w=",
         ""
         },
         {
         "pons",
         "https://en.pons.com/translate/german-english/",
         ""
         }
         });
         if (lg == "en")
            return true;
    }
    return false;
}

// TODO: add:
// https://de.wiktionary.org/wiki/Wort
// https://www.duden.de/rechtschreibung/Wort
// https://corpora.uni-leipzig.de/de/res?corpusId=deu_newscrawl_2011&word=Wort
// https://de.thefreedictionary.com/Wort
