#include "wortde.h"
#include <string>
#include <vector>
#include <fstream>
#include <limits>
#include <stdexcept>
#include "SerializeString.h"
#include "utility.h"
#include "linesramstream.h"

static inline SerializeWrapperEnum<WortDe::TypeWort> SerializeWrapper(WortDe::TypeWort &en) { return SerializeWrapperEnum<WortDe::TypeWort>(en); }
static inline SerializeWrapperEnum<WortDe::TypeArtikel> SerializeWrapper(WortDe::TypeArtikel &en) { return SerializeWrapperEnum<WortDe::TypeArtikel>(en); }
static inline SerializeWrapperEnum<WortDe::TypePerfect> SerializeWrapper(WortDe::TypePerfect &en) { return SerializeWrapperEnum<WortDe::TypePerfect>(en); }
static inline SerializeWrapperString SerializeWrapper(std::string &_str) { return SerializeWrapperString(_str); }
static inline SerializeWrapperNum<int> SerializeWrapper(int &_num) { return SerializeWrapperNum<int>(_num); }
// static inline SerializeWrapperNum<unsigned int> SerializeWrapper(unsigned int &_num) { return SerializeWrapperNum<unsigned int>(_num); }
static inline SerializeWrapperNum<bool> SerializeWrapper(bool &_num) { return SerializeWrapperNum<bool>(_num); }
static inline SerializeWrapperAny<LearningWort> SerializeWrapper(LearningWort &_val) { return SerializeWrapperAny<LearningWort>(_val); }

//        getline(is, inStr);

static inline void saveLines(std::ostream &) {}

template<typename T, typename ... Types>
static inline void saveLines(std::ostream &os, T value, const Types&... args)
{
    os << value.serialize() << '\n';
    saveLines(os, args...);
}

template<typename ... Types>
static inline void saveBlockLines(std::ostream &os, const Types&... args)
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
                   SerializeWrapper(v_TypePerfect),\
                   SerializeWrapper(l_statistic)

bool WortDe::save(std::ostream &os)
{
    os << '@' << s_wort << '\n'; // start block
    saveBlockLines(os, WortDeOptionsSerialize);

    return true;
}

// load

static inline void loadLines(LinesRamIStream &, size_t) {}

template<typename T, typename ... Types>
static inline void loadLines(LinesRamIStream &ils, size_t countLines, T value, Types... args)
{
    if (ils.eof() || countLines == 0)
        return;
    const std::string &str = ils.get();
    value.deserialize(str);
    return loadLines(ils, countLines - 1, args...);
}


bool WortDe::load(LinesRamIStream &ils, std::ostream &osErr)
{
    while(1)
    {
        if (ils.eof())
            return false;
        const std::string &str = ils.get();
        if (str.empty() || str[0] != '@')
        {
            osErr << ils.tellg() << " skiped line: " << str << std::endl;
            continue;
        }
        s_wort = str;
        break;
    }

    if (ils.eof())
        return false;
    const std::string &str = ils.get();
    const size_t countLines = std::stoul(str);
    const size_t neadCountLines = NumberOfFunctionArguments(WortDeOptionsSerialize);
    const size_t parseCount = std::min(countLines, neadCountLines);
    const size_t startPos = ils.tellg();

    try {
        loadLines(ils, parseCount, WortDeOptionsSerialize);
    }  catch (...) {
        if (ils.tellg() < startPos + parseCount)
        {
            osErr << ils.tellg() << " skiped " << startPos + parseCount - ils.tellg() << " lines (throw)" << str << std::endl;
            ils.seekg(startPos + parseCount);
        }
        throw;
    }

    return true;
}

WortDe::WortDe()
{

}

WortDe::~WortDe()
{

}

void WortDe::parseRawLine(const std::string &rawStr, int _block)
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
    parseRawDe();
    w_block = _block;
}

void WortDe::parseRawDe()
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

static const char *DerDieDasStr[] =
{
    "None",
    "Der",
    "Das",
    "Die",
    "Pl",
    "ProperNoun",
    "Der_Das",
    "Der_Die"
};

static const char *WTypeStr[] =
{
    "None",
    "Combination",
    "Noun",
    "Verb",
    "Adjective",
    "Pronoun",
    "Numeral",
    "Pretext",
    "Conjunction",
    "Particle",
    "Artikel",
    "Interjection"
};

static const char *WKursStr[] =
{
    "None",
    "A1",
    "A2",
    "B1",
    "B2",
    "C1",
    "C2"
};


void WortDe::debugPrint(std::ostream &os)
{
    os << WKursStr[w_block >> 24] << "." << ((w_block >> 16) & 0xff) << ".k" << ((w_block >> 8) & 0xff) << ".t" << (w_block & 0xff) << "; ";
    if (n_artikel != TypeArtikel::None)
        os << DerDieDasStr[static_cast<int>(n_artikel)] << ";\t";
    if (v_sich)
        os << "sich" << ";\t";
    os << s_wort << "; ";
    if (w_type != TypeWort::None)
       os << WTypeStr[static_cast<int>(w_type)] << "; ";

    os << "RAW: " << s_raw << " = " << s_translation;
    os << std::endl;
}

// class LearningWort

// Serialize LearningWort
#define LearningWortSerialize \
                   startLearning,\
                   lastCorrectAnswer,\
                   lastWrongtAnswer,\
                   numberCorrectAnswers,\
                   numberWrongtAnswers,\
                   maskCorrectAndWrongtAnswers

std::string LearningWort::serialize() const
{
    return multiPrintIntoString(" ", LearningWortSerialize);
}
void LearningWort::deserialize(const std::string &_str)
{
    multiScanFromString(_str, LearningWortSerialize);
}
