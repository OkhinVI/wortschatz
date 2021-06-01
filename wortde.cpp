#include "wortde.h"
#include <string>
#include <vector>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <ctime>
#include "SerializeString.h"
#include "utility.h"
#include "string_utf8.h"
#include "linesramstream.h"

static inline SerializeWrapperEnum<WortDe::TypeWort> SerializeWrapper(WortDe::TypeWort &en) { return SerializeWrapperEnum<WortDe::TypeWort>(en); }
static inline SerializeWrapperEnum<WortDe::TypeArtikel> SerializeWrapper(WortDe::TypeArtikel &en) { return SerializeWrapperEnum<WortDe::TypeArtikel>(en); }
static inline SerializeWrapperEnum<WortDe::TypePerfect> SerializeWrapper(WortDe::TypePerfect &en) { return SerializeWrapperEnum<WortDe::TypePerfect>(en); }
static inline SerializeWrapperString SerializeWrapper(std::string &_str) { return SerializeWrapperString(_str); }
static inline SerializeWrapperNum<int> SerializeWrapper(int &_num) { return SerializeWrapperNum<int>(_num); }
static inline SerializeWrapperNum<unsigned int> SerializeWrapper(unsigned int &_num) { return SerializeWrapperNum<unsigned int>(_num); }
static inline SerializeWrapperNum<bool> SerializeWrapper(bool &_num) { return SerializeWrapperNum<bool>(_num); }
static inline SerializeWrapperAny<LearningWort> SerializeWrapper(LearningWort &_val) { return SerializeWrapperAny<LearningWort>(_val); }


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
                   SerializeWrapper(w_frequency),\
                   SerializeWrapper(w_accent),\
                   SerializeWrapper(s_raw),\
                   SerializeWrapper(s_translation),\
                   SerializeWrapper(s_phrasePrefix),\
                   SerializeWrapper(s_phraseEnd),\
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

size_t WortDe::countSaveLines()
{
    return 1 + util::NumberOfFunctionArguments(WortDeOptionsSerialize);
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
        s_wort = str.substr(1); // removing the key '@'
        break;
    }

    if (ils.eof())
        return false;
    const std::string &str = ils.get();
    const size_t countLines = std::stoul(str);
    const size_t neadCountLines = util::NumberOfFunctionArguments(WortDeOptionsSerialize);
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

    if (s_wort.empty()) // s_wort must not empty
        s_wort = s_raw;

    return true;
}

WortDe::WortDe()
{

}

WortDe::~WortDe()
{

}

bool WortDe::operator==(const WortDe& wd2) const
{
    return  s_wort == wd2.s_wort &&
            w_type == wd2.w_type &&
            w_block == wd2.w_block &&
            w_frequency == wd2.w_frequency &&
            w_accent == wd2.w_accent &&
            s_raw == wd2.s_raw &&
            s_translation == wd2.s_translation &&
            s_phrasePrefix == wd2.s_phrasePrefix &&
            s_phraseEnd == wd2.s_phraseEnd &&
            s_example == wd2.s_example &&
            n_wortPl == wd2.n_wortPl &&
            n_artikel == wd2.n_artikel &&
            v_sich == wd2.v_sich &&
            v_prasens3f == wd2.v_prasens3f &&
            v_perfect == wd2.v_perfect &&
            v_prateritum == wd2.v_prateritum &&
            v_trennbar == wd2.v_trennbar &&
            v_Pretexts == wd2.v_Pretexts &&
            v_Cases == wd2.v_Cases &&
            v_TypePerfect == wd2.v_TypePerfect;

}

void WortDe::parseRawLine(const std::string &rawDeStr, const std::string &rawTrStr, unsigned int _block, TypeWort tw)
{
    s_raw = AreaUtf8(rawDeStr).trim().toString();
    util::replaceEndLines(s_raw);

    s_translation = AreaUtf8(rawTrStr).trim().toString();
    util::replaceEndLines(s_translation);

    parseRawDe(tw);
    w_block = _block;
}

void WortDe::setNewTranslation(const std::string &str)
{
    s_translation = AreaUtf8(str).trim().toString();
    util::replaceEndLines(s_translation);
}

void WortDe::setNewWort(const std::string &str)
{
    s_wort = AreaUtf8(str).trim().toString();
    util::replaceEndLines(s_wort);
}

void WortDe::setNewPrefix(const std::string &str)
{
    s_phrasePrefix = AreaUtf8(str).trim().toString();
    util::replaceEndLines(s_phrasePrefix);
}

void WortDe::setNewPlural(const std::string &str)
{
    n_wortPl = AreaUtf8(str).trim().toString();
    util::replaceEndLines(n_wortPl);
}

void WortDe::setNewExample(const std::string &str)
{
    s_example = AreaUtf8(str).trim().toString();
    util::replaceEndLines(s_example);
}

void WortDe::setNewPrasens3f(const std::string &str)
{
    v_prasens3f = AreaUtf8(str).trim().toString();
    util::replaceEndLines(v_prasens3f);
}

void WortDe::setNewPrateritum(const std::string &str)
{
   v_prateritum = AreaUtf8(str).trim().toString();
   util::replaceEndLines(v_prateritum);
}

void WortDe::setNewPerfect(const std::string &str)
{
   v_perfect = AreaUtf8(str).trim().toString();
   util::replaceEndLines(v_perfect);
}

void WortDe::clearOptions()
{
    s_wort.clear();
    w_type = TypeWort::None;
    w_frequency = 0;
    w_accent = -1;
    s_phrasePrefix.clear();
    s_phraseEnd.clear();
    n_wortPl.clear();
    n_artikel = TypeArtikel::None;
    v_sich = false;
    v_prasens3f.clear();
    v_perfect.clear();
    v_prateritum.clear();
    v_trennbar.clear();
    v_Pretexts.clear();
    v_Cases.clear();
    v_TypePerfect = TypePerfect::None;
}

bool WortDe::parseRawDeVerb()
{
    AreaUtf8 aRaw(s_raw);
    const AreaUtf8 prfxS = aRaw.getToken();
    if (prfxS == "sich" && aRaw.getToken() == " ")
        v_sich = true;
    else
        aRaw.seekg(0); // read again

    const size_t startNextWort = aRaw.tellg();
    const AreaUtf8 prfx = aRaw.getToken();
    if ((prfx == "etwas" || prfx == "jdn." || prfx == "jmd." || prfx == "jdn," || prfx == "jemandem" || prfx == "etwas/jemanden" || prfx == "sich/jemanden" || prfx == "etw." ||
         prfx == "jemanden" || prfx == "jemandem")
         && aRaw.getToken() == " ")
        s_phrasePrefix = prfx.toString();
    else
        aRaw.seekg(startNextWort);

    s_wort = aRaw.getRestArea().toString();
    return true;
}

static void parseRawDeNounPl(AreaUtf8 &aRaw, std::string &s_wort, std::string &n_wortPl, WortDe::TypeArtikel &n_artikel);

static bool IsPluralString(const AreaUtf8 &plural)
{
    return plural == "(PI.)" || plural == "(Pl.)" || plural == "(PL)" ||
           plural == "Pl" || plural == "Pl." || plural == "pl";
}

static bool IsPluralDelimiter(const AreaUtf8 &delimiterPl)
{
    return (delimiterPl == " " || delimiterPl == ", " || delimiterPl == ",");
}

static bool HasSymbol(AreaUtf8 au8, AreaUtf8::SymbolType sym)
{
    while (!au8.eof())
    {
        if (au8.getSymbol() == sym)
            return true;
    }
    return false;
}

bool WortDe::parseRawDeNoun()
{
    AreaUtf8 aRaw(s_raw);
    const AreaUtf8 prfx = aRaw.getToken();
    if (aRaw.getToken() != " " || !aRaw.hasSymbolDe())
    {
        s_wort = s_raw;
        return false;
    }

    if (prfx == "der")
        n_artikel = TypeArtikel::Der;
    else if (prfx == "das")
        n_artikel = TypeArtikel::Das;
    else if (prfx == "die")
        n_artikel = TypeArtikel::Die;
    else if (prfx == "der/die")
        n_artikel = TypeArtikel::Der_Die;
    else if (prfx == "der/das")
        n_artikel = TypeArtikel::Der_Das;
    else
    {
        s_wort = s_raw;
        return false;
    }

    parseRawDeNounPl(aRaw, s_wort, n_wortPl, n_artikel);
    return true;
}

static void parseRawDeNounPl(AreaUtf8 &aRaw, std::string &s_wort, std::string &n_wortPl, WortDe::TypeArtikel &n_artikel)
{
    const size_t startWort = aRaw.tellg();
    const AreaUtf8 wort = aRaw.getToken();
    const AreaUtf8 delimiter1 = aRaw.getToken();
    const AreaUtf8 token1 = aRaw.getToken();
    const AreaUtf8 delimiter2 = aRaw.getToken();
    const AreaUtf8 token2 = aRaw.getToken();

    const bool genetiv = !token2.empty();
    const AreaUtf8 delimiterPl = genetiv ? delimiter2 : delimiter1;
    const AreaUtf8 plural = genetiv ? token2 : token1;

    if (aRaw || (token2.empty() && !delimiter2.empty()))
    {
        aRaw.seekg(startWort);
        s_wort = aRaw.getRestArea().toString();
        return;
    }

    if (genetiv)
    {
        if ((token1 != "-en" && token1 != "-es" && token1 != "-s" && token1 != "-" && token1 != "=") ||
             !HasSymbol(delimiter2, ',') ||
             delimiter1 != " ")
        {
            aRaw.seekg(startWort);
            s_wort = aRaw.getRestArea().toString();
            return;
        }
    }

    if (plural.empty() && delimiterPl == ",")
    {
        s_wort = AreaUtf8(wort).toString();
    }
    else if (IsPluralString(plural) && IsPluralDelimiter(delimiterPl))
    {
        s_wort = AreaUtf8(wort).toString();
        n_wortPl = "Pl.";
        n_artikel = WortDe::TypeArtikel::Pl;
    }
    else if ((plural == "(Sg.)") && IsPluralDelimiter(delimiterPl))
    {
        s_wort = AreaUtf8(wort).toString();
        n_wortPl = "Sg.";
    }
    else if (plural.hasSymbolDe() && HasSymbol(delimiterPl, ','))
    {
        s_wort = AreaUtf8(wort).toString();
        n_wortPl = plural.toString();
    }
    else if ((plural == "-" ||
            plural == "¨" ||
            plural == "-e" ||
            plural == "¨e" ||
            plural == "-er" ||
            plural == "¨er" ||
            plural == "-n" ||
            plural == "-en" ||
            plural == "-s" ||
            plural == "=" ||
            *plural == '-' ||
            plural.subArea(0, 2) == "..") && IsPluralDelimiter(delimiterPl))
    {
        s_wort = AreaUtf8(wort).toString();
        n_wortPl = AreaUtf8(plural).toString();
    } else
    {
        aRaw.seekg(startWort);
        s_wort = aRaw.getRestArea().toString();
    }
}

bool WortDe::setNewTypeWort(const TypeWort tw)
{
    clearOptions();
    w_type = tw;

    if (tw == TypeWort::Noun)
        return parseRawDeNoun();
    else if (tw == TypeWort::Verb)
        return parseRawDeVerb();

    s_wort = s_raw;
    return true;
}

void WortDe::parseRawDe(TypeWort tw)
{
    if (tw != TypeWort::None)
    {
        setNewTypeWort(tw);
        return;
    }

    AreaUtf8 aRaw(s_raw);
    AreaUtf8 prfx = aRaw.getToken();
    if (aRaw.getToken() == " " && aRaw.hasSymbolDe())
    {
        if (prfx == "der" || prfx == "das" || prfx == "die" || prfx == "der/die" || prfx == "der/das")
        {
            setNewTypeWort(TypeWort::Noun);
            return;
        }
        else if (prfx == "sich")
        {
            setNewTypeWort(TypeWort::Verb);
            return;
        }
        else if (prfx == "etwas" || prfx == "jdn." || prfx == "jmd." || prfx == "jdn," || prfx == "jemandem" || prfx == "etwas/jemanden" || prfx == "sich/jemanden" || prfx == "etw." ||
             prfx == "jemanden" || prfx == "jemandem")
        {
            clearOptions();
            s_phrasePrefix = prfx.toString();
            s_wort = aRaw.getRestArea().trim().toString();
            return;
        }
    }
    clearOptions();
    s_wort = s_raw;
}

std::string WortDe::prefix() const
{
    std::string str = s_phrasePrefix;
    if (w_type == TypeWort::Noun)
    {
        str = str.empty() ? TypeArtikeltToString(n_artikel) : TypeArtikeltToString(n_artikel) + " " + str;
    }
    else if (w_type == TypeWort::Verb)
    {
        if (v_sich)
            str = str.empty() ? "sich" : "sich " + str;
    }
    return str;
}

std::string WortDe::prefixAndWort() const
{
    const std::string prfx = prefix();
    return prfx.empty() ? s_wort : prfx + " " + s_wort;
}

std::string WortDe::TypeWortToString(TypeWort tw, const char *local)
{
    int index = 2; // de
    const std::string locStr((local == nullptr) ? "de" : local);
    if (locStr == "en")
        index = 0;
    else if (locStr == "ru")
        index = 1;
    else if (locStr == "de")
        index = 2;

    const int countLeng = 3;
    switch (tw) {
    case WortDe::TypeWort::None:
    {
        const char *tws[countLeng] = {"None", "Нет", "None"};
        return tws[index];
    }
    case WortDe::TypeWort::Combination:  // словосочитание  // Wortverbindung
    {
        const char *tws[countLeng] = {"Combination", "словосочитание", "Wortverbindung"};
        return tws[index];
    }
    case WortDe::TypeWort::Noun:         // существительное // das Nomen
    {
        const char *tws[countLeng] = {"Noun", "существительное", "Nomen"};
        return tws[index];
    }
    case WortDe::TypeWort::Verb:         // глагол          // das Verb
    {
        const char *tws[countLeng] = {"Verb", "глагол", "Verb"};
        return tws[index];
    }
    case WortDe::TypeWort::Adjective:    // прилагательное  // das Adjektiv
    {
        const char *tws[countLeng] = {"Adjective", "прилагательное", "Adjektiv"};
        return tws[index];
    }
    case WortDe::TypeWort::Pronoun:      // местоимение     // das Pronomen
    {
        const char *tws[countLeng] = {"Pronoun", "местоимение", "Pronomen"};
        return tws[index];
    }
    case WortDe::TypeWort::Numeral:      // числительное    // das Numerale
    {
        const char *tws[countLeng] = {"Numeral", "числительное", "Numerale"};
        return tws[index];
    }
    case WortDe::TypeWort::Pretext:      // предлог         // die Präposition
    {
        const char *tws[countLeng] = {"Pretext", "предлог", "Präposition"};
        return tws[index];
    }
    case WortDe::TypeWort::Conjunction:  // союз            // die Konjunktion
    {
        const char *tws[countLeng] = {"Conjunction", "союз", "Konjunktion"};
        return tws[index];
    }
    case WortDe::TypeWort::Particle:     // частица         // die Partikel
    {
        const char *tws[countLeng] = {"Particle", "частица", "Partikel"};
        return tws[index];
    }
    case WortDe::TypeWort::Artikel:      // артикль         // der Artikel
    {
        const char *tws[countLeng] = {"Artikel", "артикль", "Artikel"};
        return tws[index];
    }
    case WortDe::TypeWort::Interjection: // междометие      // die Interjektion
    {
        const char *tws[countLeng] = {"Interjection", "междометие", "Interjektion"};
        return tws[index];
    }
    case WortDe::TypeWort::Adverb:       // наречие         // Adverb
    {
        const char *tws[countLeng] = {"Adverb", "наречие", "Adverb"};
        return tws[index];
    }
    case WortDe::TypeWort::_last_one:
        break;
    }
    return "";
}

std::string WortDe::TypeArtikeltToString(TypeArtikel ta, const bool forLabel)
{
    switch (ta) {
    case TypeArtikel::None:
        return forLabel ? "None"       : "";
    case TypeArtikel::Der:
        return forLabel ? "Der"        : "der";
    case TypeArtikel::Das:
        return forLabel ? "Das"        : "das";
    case TypeArtikel::Die:
        return forLabel ? "Die"        : "die";
    case TypeArtikel::Pl:
        return forLabel ? "Pl"         : "die";
    case TypeArtikel::ProperNoun:
        return forLabel ? "ProperNoun" : "";
    case TypeArtikel::Der_Das:
        return forLabel ? "Der/Das"    : "der/das";
    case TypeArtikel::Der_Die:
        return forLabel ? "Der/Die"    : "der/die";
    case TypeArtikel::_last_one:
        break;
    }
    return "";
}

std::string WortDe::blockHeadToStr(BlockNumType block)
{
    const size_t WKursStrSize = 7;
    static const char *WKursStr[WKursStrSize] =
    {
        "None",
        "A1",
        "A2",
        "B1",
        "B2",
        "C1",
        "C2"
    };
    std::string str;
    const size_t headBlock = block >> 24;
    if (headBlock > 0 && headBlock < WKursStrSize)
        str = WKursStr[headBlock];
    else if (headBlock == 0x10)
        str = "Us";
    else
        str = std::to_string(headBlock);
    return str;
}

std::string WortDe::blockToStr() const
{
    return blockHeadToStr(w_block) + "."
           + std::to_string((w_block >> 16) & 0xff)
           + ".k" + std::to_string((w_block >> 8) & 0xff) + ".t"
           + std::to_string(w_block & 0xff);
}

void WortDe::debugPrint(std::ostream &os)
{
    os << blockToStr() << "; ";

    const std::string prfx = prefix();
    if (!prfx.empty())
        os << prfx << ";\t";

    os << s_wort << "; ";
    if (w_type != TypeWort::None)
       os << TypeWortToString(w_type) << "; ";

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

void WortDe::setAnswer(const bool ans)
{
    const auto currTie = time(nullptr);
    if (ans)
    {
        ++l_statistic.numberCorrectAnswers;
        l_statistic.lastCorrectAnswer = currTie;
    } else {
        ++l_statistic.numberWrongtAnswers;
        l_statistic.lastWrongtAnswer = currTie;
    }

    if (l_statistic.startLearning == 0)
        l_statistic.startLearning = currTie;

     unsigned int mask = l_statistic.maskCorrectAndWrongtAnswers >> 1;
     if (ans)
         mask |= 0x80000000;
     l_statistic.maskCorrectAndWrongtAnswers = mask;
}
