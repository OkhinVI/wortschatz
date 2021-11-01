#ifndef WEBTRANSLATION_H
#define WEBTRANSLATION_H

#include <string>
#include <vector>
#include <map>

class WebTranslation
{
public:
    enum class WebSite
    {
        None = 0,
        duden,
        dwds,
        wiktionary,
        leipzig,
        thefreeDic,
        lingvo,
        yandex,
        google,
        VerbF,
        pons,
        woerter,
        _last_one
    };

private:
    class UrlSite
    {
    public:
        std::string name;
        std::string firstUrl;
        std::string lastUrl;
    };

public:
    WebTranslation();

    bool wortTranslate(const std::string &str, WebSite ws);
    bool allTranslate(const std::string &str, WebSite ws);
    bool setLanguage(const std::string &lg);
    const std::string& nameAsString(WebSite ws);

private:
    void setDeWebSites();
    void setDeWebSitesFromLines(const std::vector<std::string> &lines);

public:
    const char* const LangRU = "ru";
    const char* const LangEN = "en";

private:
    std::vector<UrlSite> urls;
    std::map<std::string, WebSite> mapWebSite;
};

#endif // WEBTRANSLATION_H
