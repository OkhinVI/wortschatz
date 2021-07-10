#ifndef WORTDE_H
#define WORTDE_H

#include <string>
#include <iostream>

class LinesRamIStream;

class LearningWort
{
public:
    std::string serialize() const;
    void deserialize(const std::string &_str);
    void clear() { *this = LearningWort(); }

public:
    uint32_t startLearning = 0; // Начало изучения слова
    uint32_t lastCorrectAnswer = 0; // Время в секундах последнего правильного ответа
    uint32_t lastWrongtAnswer = 0; // Время в секундах последнего неправильного ответа
    uint32_t numberCorrectAnswers = 0; // количество правильных ответов
    uint32_t numberWrongtAnswers = 0; // количество неправильных ответов
    uint32_t maskCorrectAndWrongtAnswers = 0; // битовая маска последних 32-х ответов (старший бит - последний ответ)
};

class WortDe
{
public:
    typedef uint32_t BlockNumType;

    enum class TypeWort : int // Wortarten
    {
        None = 0,
        Combination,  // сочитание слов  // Wortverbindung
        Noun,         // существительное // das Nomen
        Verb,         // глагол          // das Verb
        Adjective,    // прилагательное  // das Adjektiv
        Pronoun,      // местоимение     // das Pronomen
        Numeral,      // числительное    // das Numerale
        Pretext,      // предлог         // die Präposition
        Conjunction,  // союз            // die Konjunktion
        Particle,     // частица         // die Partikel
        Artikel,      // артикль         // der Artikel
        Interjection, // междометие      // die Interjektion
        Adverb,       // наречие         // Adverb
        _last_one
    };

    enum class TypeArtikel : int
    {
        None = 0,
        Der,
        Das,
        Die,
        Pl,
        ProperNoun,
        Der_Das,
        Der_Die,
        _last_one
    };

    enum class TypePerfect : int
    {
        None = 0,
        Haben = 1,
        Sein = 2,
        SeinHaben = 3,
        _last_one
    };

    enum class TypeLevel: unsigned int
    {
        None = 0,
        A1,
        A2,
        B1,
        B2,
        C1,
        C2,
        User,
        _last_one
    };

public:
    WortDe();
    ~WortDe();

    const std::string& raw() const { return s_raw; }
    const std::string& translation() const { return s_translation; }
    const std::string& wort() const { return s_wort; }
    const std::string& wortPl() const { return n_wortPl; }
    const std::string& vPrasens3f() const { return v_prasens3f; }
    const std::string& vPrateritum() const { return v_prateritum; }
    const std::string& vPerfect() const { return v_perfect; }
    const std::string& example() const { return s_example; }
    TypeArtikel artikel() const { return n_artikel; }
    TypeWort type() const { return w_type; }
    bool hasSich() const { return v_sich; }
    uint32_t freqIdx() const { return w_freqIdx; }

    std::string rawPrefix() const { return s_phrasePrefix; }
    std::string prefix() const;
    std::string blockToStr() const;
    unsigned int block() const { return w_block; }
    std::string prefixAndWort() const;

    bool save(std::ostream &os, const bool clearStat = false);
    std::string exportToStr(const bool clearStat = false);
    bool load(LinesRamIStream &ils, std::ostream &osErr);
    size_t countSaveLines();

    bool operator==(const WortDe& wd2) const;

    static std::string TypeWortToString(TypeWort tw, const char *local = "de");
    static std::string TypeArtikeltToString(TypeArtikel ta, const bool forLabel = false);
    static std::string blockH1ToStr(unsigned int h1);
    static void blockToUint_4(const BlockNumType block, unsigned int &h1, unsigned int &h2, unsigned int &h3, unsigned int &h4);
    static BlockNumType creatBlock(unsigned int h1, unsigned int h2, unsigned int h3, unsigned int h4);
    static BlockNumType &preIncrementBlock(BlockNumType &block);

    void parseRawLine(const std::string &rawDeStr, const std::string &rawTrStr, unsigned int _block, TypeWort tw = TypeWort::None);
    void debugPrint(std::ostream &os);

    bool setNewTypeWort(const TypeWort tw);
    void setNewArtikel(const TypeArtikel art) { n_artikel = art; }
    void setNewSich(const bool sich) { v_sich = sich; }
    void setNewTranslation(const std::string &str);
    void setNewWort(const std::string &str);
    void setNewPrefix(const std::string &str);
    void setNewPlural(const std::string &str);
    void setNewExample(const std::string &str);
    void setNewPrasens3f(const std::string &str);
    void setNewPrateritum(const std::string &str);
    void setNewPerfect(const std::string &str);
    void setNewFreqIdx(uint32_t fIdx) { w_freqIdx = fIdx; }
    void setNewBlock(const BlockNumType aBlock) { w_block = aBlock; }

    void addAnswer(const bool ans);
    void clearStatistic() { l_statistic.clear(); }
    const LearningWort& getStatistic() { return l_statistic; }

private:
    void parseRawDe(TypeWort tw = TypeWort::None);
    bool parseRawDeNoun();
    bool parseRawDeVerb();
    void clearOptions();

private:
    std::string s_wort; // только само слово в словарной форме (для TypeWort::None должно быть то же что и s_raw)
    TypeWort w_type = TypeWort::None;
    BlockNumType w_block = 0; // Указание на уровень слова: (A1, A2, B1, D2, C1, C2):8bit - (номер учебника):8bit - (номер главы):8bit - (номер раздела в главе):8bit
    uint32_t w_freqIdx = 0; // Как часто слово встречается в текстах: позиция 1 - самое используемое слово.
    int w_accent = -1; // Позиция ударной буквы
    std::string s_raw; // как есть, но без перевода (он в s_translation)
    std::string s_translation; // перевод как есть
    std::string s_phrasePrefix; // начало фразы со смысловым словом, например "etwas", которое в предложении скорее будет заменено на другие слова
    std::string s_phraseEnd; // дополнение в скобках после слова, например "(auf + A.)" или "(an +D.)", которое в предложении будет заменено на другие слова
    std::string s_example; // примеры использования
    // Noun
    std::string n_wortPl; // Форма множественного числа
    TypeArtikel n_artikel = TypeArtikel::None; // Артикль слова
    // Verb
    bool v_sich = false; // возвратный глагол с sich
    std::string v_prasens3f;
    std::string v_perfect;
    std::string v_prateritum;
    std::string v_trennbar; // отделяемая приставка
    std::string v_Pretexts; // используемые предлоги (с примерами) - разделитель: '\t'
    std::string v_Cases; // используемые падежи (с примерами) - разделитель: '\t'
    TypePerfect v_TypePerfect = TypePerfect::None; // вспомогательный глагол для образования перфекта
    LearningWort l_statistic; // Статистика изучения слова
};

#endif // WORTDE_H
