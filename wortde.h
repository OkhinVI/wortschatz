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

public:
    unsigned int startLearning = 0; // Начало изучения слова
    unsigned int lastCorrectAnswer = 0; // Время в секундах последнего правильного ответа
    unsigned int lastWrongtAnswer = 0; // Время в секундах последнего неправильного ответа
    unsigned int numberCorrectAnswers = 0; // количество правильных ответов
    unsigned int numberWrongtAnswers = 0; // количество неправильных ответов
    unsigned int maskCorrectAndWrongtAnswers = 0; // битовая маска последних 32-х ответов (старший бит - последний ответ)
};

class WortDe
{
public:
    enum class TypeWort : int
    {
        None = 0,
        Combination, // сочитание слов
        Noun, // существительное
        Verb, // глагол
        Adjective, // прилагательное
        Pronoun, // местоимение
        Numeral, // числительное
        Pretext, // предлог
        Conjunction, // союз
        Particle, // частица
        Artikel, // артикль
        Interjection, // междометие
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

public:
    WortDe();
    ~WortDe();

    const std::string& raw() { return s_raw; }
    const std::string& translation() { return s_translation; }
    const std::string& wort() { return s_wort; }
    TypeArtikel artikel() { return n_artikel; }
    TypeWort type() { return w_type; }

    bool save(std::ostream &os);
    bool load(LinesRamIStream &ils, std::ostream &osErr);

    void parseRawLine(const std::string &rawStr, int _block); // Example rawStr: "das Wort \t a translation of a word"
    void debugPrint(std::ostream &os);

private:
    void parseRawDe();

private:
    std::string s_wort; // только само слово в словарной форме (для TypeWort::None должно быть то же что и s_raw)
    TypeWort w_type = TypeWort::None;
    int w_block = 0; // Указание на уровень слова: (A1, A2, B1, D2, C1, C2):8bit - (номер учебника):8bit - (номер главы):8bit - (номер раздела в главе):8bit
    int w_accent = -1; // Позиция ударной буквы
    std::string s_raw; // как есть, но без перевода (он в s_translation)
    std::string s_translation; // перевод как есть
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
