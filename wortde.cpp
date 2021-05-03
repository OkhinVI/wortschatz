#include "wortde.h"
#include <string>

typedef std::string::size_type size_type;

static std::string substrWithoutSideSpaces(const std::string &str, size_type posBegin = 0, size_type n = std::string::npos);
static std::string getPrefix(const std::string &str, char &findedDelimiter);

static inline
bool isWort(const char ch)
{
    const unsigned char chU = ch;
    return (chU > 127 || chU == '-' || ::isalpha(chU));
}

WortDe::WortDe()
{

}

WortDe::WortDe(const std::string &rawStr)
{
    size_type posTab = rawStr.find('\t');
    if (posTab != std::string::npos)
    {
        s_translation = substrWithoutSideSpaces(rawStr, posTab);
        s_raw = substrWithoutSideSpaces(rawStr, 0, posTab);
    } else
    {
        s_raw = substrWithoutSideSpaces(rawStr, 0);
    }
    parseRaw();
}

void WortDe::parseRaw()
{
    char delimiter = 0;
    std::string prfx = getPrefix(s_raw, delimiter);
    if (delimiter == ' ')
    {
        TypeArtikel artikel = tpA_None;
        if (prfx == "der")
            artikel = tpA_Der;
        else if (prfx == "das")
            artikel = tpA_Das;
        else if (prfx == "die")
            artikel = tpA_Die;
        else if (prfx == "sich")
            w_type = tpW_Verb; // TODO: test Combination

        if (artikel)
        {
            // TODO: test Combination
            w_type = tpW_Noun;
            n_artikel = artikel;
            s_wort = substrWithoutSideSpaces(s_raw, prfx.size());
        }
    }
}

static std::string substrWithoutSideSpaces(const std::string &str, size_type posBegin, size_type n)
{
    size_type posEnd = (n == std::string::npos || posBegin + n > str.size()) ?
        str.size() : posBegin + n;
    if (posBegin >= posEnd)
        return std::string();
    for (; posBegin < posEnd; ++posBegin)
    {
        if (!::isspace(str[posBegin]))
            break;
    }
    for (; posBegin < posEnd; --posEnd)
    {
        if (!::isspace(str[posEnd - 1]))
            break;
    }
    return str.substr(posBegin, posEnd - posBegin);
}

static std::string getPrefix(const std::string &str, char &findedDelimiter)
{
    for (size_type idx = 0; idx < str.size(); ++idx)
    {
        if (!isWort(str[idx]))
        {
            if (idx < 1)
                break;
            findedDelimiter = str[idx];
            return str.substr(0, idx);
        }
    }
    return std::string();
}
