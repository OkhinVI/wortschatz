#ifndef WORTDE_H
#define WORTDE_H

#include <string>


class WortDe
{
public:
    enum TypeWort
    {
        tpW_None = 0,
        tpW_Combination, // сочетание слов
        tpW_Noun, // существительное
        tpW_Verb, // глагол
        tpW_Adjective, // прилагательное
        tpW_Pronoun, // местоимение
        tpW_Numeral, // числительное
        tpW_Pretext, // предлог
        tpW_Conjunction, // союз
        tpW_Particle, // частица
        tpW_Artikel, // артикль
        tpW_Interjection // междометие
    };

    enum TypeArtikel
    {
        tpA_None = 0,
        tpA_Der,
        tpA_Das,
        tpA_Die,
        tpA_Pl
    };

    enum TypePerfect
    {
        tpP_None = 0,
        tpP_Haben = 1,
        tpP_Sein = 2,
        tpP_SeinHaben = 3
    };

public:
    WortDe();
    WortDe(const std::string &rawStr);

    const std::string& raw() { return s_raw; }
    const std::string& translation() { return s_translation; }
    const std::string& wort() { return s_wort; }
    TypeArtikel artikel() { return n_artikel; }
    TypeWort type() { return w_type; }

private:
    void parseRaw();

private:
    TypeWort w_type = tpW_None;
    std::string s_raw; // как есть, но без перевода (он в s_translation)
    std::string s_wort; // только само слово в словарной форме (для tp_None поле пустое)
    std::string s_translation; // перевод как есть
    std::string s_block; // Указание на раздел, откуда это слово
    std::string s_example; // примеры использования
    // Noun
    std::string n_wortPl; // Форма множественного числа
    TypeArtikel n_artikel = tpA_None; // Артикль слова
    // Verb
    std::string v_prasens3f;
    std::string v_perfect;
    std::string v_prateritum;
    std::string v_trennbar; // отделяемая приставка
    std::string v_Pretexts; // используемые предлоги (с примерами) - разделитель: конец строки
    std::string v_Cases; // используемые падежи (с примерами) - разделитель: конец строки
    TypePerfect v_TypePerfect = tpP_None; // вспомогательный глагол для образования перфекта
};

#endif // WORTDE_H
