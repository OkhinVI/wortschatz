#ifndef GLOSSARYDE_H
#define GLOSSARYDE_H

#include "wortde.h"
#include <vector>
#include <map>
#include <limits>
#include <random>

class GlossaryDe
{
public:
    typedef std::vector<WortDe> DictionaryDe;

    class Tema
    {
    public:
        Tema(unsigned int aNum, const std::string &str): blockNum(aNum), blockStr(str) {}
        std::string asString() const;

    public:
        unsigned int blockNum = 0;
        std::string blockStr;
    };

    class SelectSettings
    {
    public:
        explicit SelectSettings(const GlossaryDe &aGlDe);

        size_t themesSize() const { return glDe.themesSize(); }
        const Tema &getTemaByIndex(size_t idx) const { return glDe.getTemaByIndex(idx); }
        bool testWort(const WortDe &wd) const;

    public:
        size_t startIdxTema = 0;
        size_t lastIdxTema = 0;

    private:
        const GlossaryDe &glDe;
    };

public:
    GlossaryDe();
    void setPath(const std::string &path);
    void setFile(const std::string &name);
    void load();
    void loadThemes(const std::string &fileName);
    void save();
    void saveClear();
    void fixMainDic();

    void clearAllStatistic();
    size_t calcTestWortIdx(const SelectSettings &selSet); // returns the index of the next word to learn
    int selectVariantsTr(std::vector<size_t> &vecIdxTr);

    void add(const WortDe &wd);
    void import(const GlossaryDe &impGloss);

    const WortDe &at(size_t idx) const;
    WortDe &at(size_t idx);
    WortDe &operator[] (size_t idx) { return at(idx); }
    const WortDe &operator[] (size_t idx) const { return at(idx); }
    size_t size() const { return dictionary.size(); }
    bool empty() const { return dictionary.empty(); }

    size_t find(const std::string &str, size_t pos = 0);

    std::string tema(const unsigned int blockNum);
    const Tema &getTemaByIndex(size_t idx) const;
    size_t themesSize() const { return themesVector.size(); }
    WortDe::BlockNumType userBlockNum() const { return nextUserNumBlock; }

    template< typename Func >
    size_t selectIdxFilter(Func func, std::vector<size_t> &selectionIdxs, const SelectSettings &selSet);

private:
    void calcNextUserNumBlock();

private:
    DictionaryDe dictionary;
    std::string fileName;
    std::string filePath;
    bool notLoaded = false;
    std::map<unsigned int, std::string> themes;
    std::vector<Tema> themesVector;
    size_t beginUserWort = 0;
    WortDe::BlockNumType nextUserNumBlock = 0;

    size_t currIdxLearnWordDe = 0;
    std::mt19937 genRandom;
};

template< typename Func >
size_t GlossaryDe::selectIdxFilter(Func func, std::vector<size_t> &selectionIdxs, const SelectSettings &selSet)
{
    selectionIdxs.clear();
    for (size_t i = 0; i < dictionary.size(); ++i)
    {
        if (func(dictionary[i], i) && selSet.testWort(dictionary[i]))
            selectionIdxs.push_back(i);
    }
    return selectionIdxs.size();
}

#endif // GLOSSARYDE_H
