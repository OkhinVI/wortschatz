#include "webtranslation.h"
#include <QDesktopServices>
#include <QUrl>
#include <sstream>
#include "string_utf8.h"

WebTranslation::WebTranslation()
{
    urls.resize(size_t(WebSite::_last_one) + 1);
    for (int i = 1; i < int(WebSite::_last_one); ++i)
        mapWebSite[nameAsString(WebSite(i))] = WebSite(i);

//    setLanguage(LangEN);
    setLanguage(LangRU);
}

bool WebTranslation::wortTranslate(const std::string &str, WebSite ws)
{
    if (str.empty())
        return false;
    AreaUtf8 au8(str);
    au8.trim();
    AreaUtf8 wort = au8.getToken();
    if (wort.empty())
        return false;
    const UrlSite &currUrl = urls[size_t(ws)];
    if (currUrl.name.empty() || currUrl.firstUrl.empty())
        return false;

    QString url = QString::fromStdString(currUrl.firstUrl + wort.toString() + currUrl.lastUrl);
    return QDesktopServices::openUrl(QUrl(url));
}

bool WebTranslation::allTranslate(const std::string &str, WebSite ws)
{
    if (str.empty())
        return false;

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
    if (currUrl.name.empty() || currUrl.firstUrl.empty())
        return false;

    QString url = QString::fromStdString(currUrl.firstUrl + ss.str() + currUrl.lastUrl);
    return QDesktopServices::openUrl(QUrl(url));
}

bool WebTranslation::setLanguage(const std::string &lg)
{
    setDeWebSites();
    std::vector<std::string> urlsStrings;
    if (lg == LangRU)
    {
         urlsStrings = std::vector<std::string>({
         "lingvo",
         "https://www.lingvolive.com/ru-ru/translate/de-ru/",
         "",
         "yandex",
         "https://translate.yandex.ru/?utm_source=wizard&text=",
         "&lang=de-ru",
         "google",
         "https://translate.google.de/?hl=ru&tab=TT&sl=de&tl=ru&text=",
         "&op=translate",
         "VerbF",
         "https://www.verbformen.ru/sprjazhenie/",
         ".htm",
         "pons",
         "https://ru.pons.com/%D0%BF%D0%B5%D1%80%D0%B5%D0%B2%D0%BE%D0%B4/%D0%BD%D0%B5%D0%BC%D0%B5%D1%86%D0%BA%D0%B8%D0%B9-%D1%80%D1%83%D1%81%D1%81%D0%BA%D0%B8%D0%B9/",
         ""
         });
    }
    else if (lg == LangEN)
    {
        urlsStrings = std::vector<std::string>({
         "lingvo",
         "https://www.lingvolive.com/en-us/translate/de-en/",
         "",
         "yandex",
         "https://translate.yandex.ru/?lang=de-en&text=",
         "",
         "google",
         "https://translate.google.de/?sl=de&tl=en&text=",
         "&op=translate",
         "VerbF",
         "https://www.verbformen.com/conjugation/?w=",
         "",
         "pons",
         "https://en.pons.com/translate/german-english/",
         ""
         });
    } else
        return false;

    setDeWebSitesFromLines(urlsStrings);
    return true;
}

void WebTranslation::setDeWebSites()
{
    urls.clear();
    urls.resize(size_t(WebSite::_last_one) + 1);

    std::vector<std::string> deUrlsStrings = {
        "duden",
        "https://www.duden.de/rechtschreibung/",
        "",
        "dwds",
        "https://www.dwds.de/wb/",
        "",
        "wiktionary",
        "https://de.wiktionary.org/wiki/",
        "",
        "leipzig",
        "https://corpora.uni-leipzig.de/de/res?corpusId=deu_newscrawl_2011&word=",
        "",
        "thefreeDic",
        "https://de.thefreedictionary.com/",
        ""
        };

    setDeWebSitesFromLines(deUrlsStrings);
}

void WebTranslation::setDeWebSitesFromLines(const std::vector<std::string> &lines)
{
    if (lines.size() % 3 != 0)
    {
        // TODO: print error
    }
    for (size_t i = 0; i + 2 < lines.size(); i += 3)
    {
        const auto it = mapWebSite.find(lines[i]);
        if (lines[i].empty() || it == mapWebSite.end())
        {
            // TODO: print error
            continue;
        }
        WebSite us = it->second;
        urls[int(us)].name = lines[i];
        urls[int(us)].firstUrl = lines[i + 1];
        urls[int(us)].lastUrl = lines[i + 2];
    }
}

const std::string& WebTranslation::nameAsString(WebSite ws)
{
    static std::string nullName;
    switch (ws)
    {
    case WebSite::None:
        break;
    case WebSite::duden:
    {
        static std::string name("duden");
        return name;
    }
    case WebSite::dwds:
    {
        static std::string name("dwds");
        return name;
    }
    case WebSite::wiktionary:
    {
        static std::string name("wiktionary");
        return name;
    }
    case WebSite::leipzig:
    {
        static std::string name("leipzig");
        return name;
    }
    case WebSite::thefreeDic:
    {
        static std::string name("thefreeDic");
        return name;
    }
    case WebSite::lingvo:
    {
        static std::string name("lingvo");
        return name;
    }
    case WebSite::yandex:
    {
        static std::string name("yandex");
        return name;
    }
    case WebSite::google:
    {
        static std::string name("google");
        return name;
    }
    case WebSite::VerbF:
    {
        static std::string name("VerbF");
        return name;
    }
    case WebSite::pons:
    {
        static std::string name("pons");
        return name;
    }
    case WebSite::_last_one:
        break;
    }
    return nullName;
}


// TODO: add:
// https://www.duden.de/rechtschreibung/Wort
// https://de.wiktionary.org/wiki/Wort
// https://corpora.uni-leipzig.de/de/res?corpusId=deu_newscrawl_2011&word=Wort
// https://de.thefreedictionary.com/Wort
