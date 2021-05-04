#include "wortde.h"
#include <string>
#include <fstream>
#include <limits>
#include <stdexcept>
#include "SerializeString.h"
#include "utility.h"

static inline SerializeWrapperEnum<WortDe::TypeWort> SerializeWrapper(WortDe::TypeWort &en) { return SerializeWrapperEnum<WortDe::TypeWort>(en); }
static inline SerializeWrapperEnum<WortDe::TypeArtikel> SerializeWrapper(WortDe::TypeArtikel &en) { return SerializeWrapperEnum<WortDe::TypeArtikel>(en); }
static inline SerializeWrapperEnum<WortDe::TypePerfect> SerializeWrapper(WortDe::TypePerfect &en) { return SerializeWrapperEnum<WortDe::TypePerfect>(en); }
static inline SerializeWrapperString SerializeWrapper(std::string &_str) { return SerializeWrapperString(_str); }
static inline SerializeWrapperNum<int> SerializeWrapper(int &_num) { return SerializeWrapperNum<int>(_num); }
// static inline SerializeWrapperNum<unsigned int> SerializeWrapper(unsigned int &_num) { return SerializeWrapperNum<unsigned int>(_num); }
static inline SerializeWrapperNum<bool> SerializeWrapper(bool &_num) { return SerializeWrapperNum<bool>(_num); }

//        getline(is, inStr);

static inline void saveLines(std::ostream &) {}

template<typename T, typename ... Types>
void saveLines(std::ostream &os, T value, const Types&... args)
{
    os << value.serialize() << '\n';
    saveLines(os, args...);
}

template<typename ... Types>
void saveBlockLines(std::ostream &os, const Types&... args)
{
    const size_t number = sizeof...(args);
    os << number << '\n';
    saveLines(os, args...);
}

// Serialize options for s_wort (without s_wort)
#define WortDeOptionsSerialize \
                   SerializeWrapper(w_type),\
                   SerializeWrapper(w_block),\
                   SerializeWrapper(w_accent),\
                   SerializeWrapper(s_raw),\
                   SerializeWrapper(s_translation),\
                   SerializeWrapper(s_example),\
                   SerializeWrapper(n_wortPl),\
                   SerializeWrapper(n_artikel),\
                   SerializeWrapper(v_sich),\
                   SerializeWrapper(v_prasens3f),\
                   SerializeWrapper(v_perfect),\
                   SerializeWrapper(v_prateritum),\
                   SerializeWrapper(v_trennbar),\
                   SerializeWrapper(v_Pretexts),\
                   SerializeWrapper(v_Cases),\
                   SerializeWrapper(v_TypePerfect)

bool WortDe::save(std::ostream &os)
{
    os << '@' << s_wort << '\n'; // start block
    saveBlockLines(os, WortDeOptionsSerialize);

    return true;
}

bool WortDe::load(std::istream &is)
{
    (void) is; // TODO: implementation
    return true;
}

WortDe::WortDe()
{

}

WortDe::~WortDe()
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
        TypeArtikel artikel = TypeArtikel::None;
        if (prfx == "der")
        {
            artikel = TypeArtikel::Der;
        }
        else if (prfx == "das")
        {
            artikel = TypeArtikel::Das;
        }
        else if (prfx == "die")
        {
            artikel = TypeArtikel::Die;
        }
        else if (prfx == "sich")
        {
            w_type = TypeWort::Verb; // TODO: test Combination
            v_sich = true;
        }
        else
            prfx.clear();

        if (artikel != TypeArtikel::None)
        {
            // TODO: test Combination
            w_type = TypeWort::Noun;
            n_artikel = artikel;
        }
        if (!prfx.empty())
            s_wort = substrWithoutSideSpaces(s_raw, prfx.size());
        else
            s_wort = s_raw;
    }
}
