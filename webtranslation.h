#ifndef WEBTRANSLATION_H
#define WEBTRANSLATION_H

#include <string>
#include <vector>

class WebTranslation
{
public:
    enum class WebSite
    {
        lingvo = 0,
        yandex,
        google,
        dwds,
        VebF,
        pons,
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

    void wortTranslate(const std::string &str, WebSite ws);
    void allTranslate(const std::string &str, WebSite ws);
    bool setLanguage(const std::string &lg);

private:
    std::vector<UrlSite> urls;
};

#endif // WEBTRANSLATION_H
