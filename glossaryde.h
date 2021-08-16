#ifndef GLOSSARYDE_H
#define GLOSSARYDE_H

#include "wortde.h"
#include "statwords.h"
#include <vector>
#include <map>
#include <limits>
#include <random>

class FoundItemsIdx
{
public:
    void clear() { wordIdxs.clear(); types.clear(); strings.clear(); pos = 0; glossaryIndex = -1; }
    void add(uint32_t wordIdx, uint8_t type, const std::string str)
    {
        wordIdxs.push_back(wordIdx);
        types.push_back(type);
        strings.push_back(str);
    }
    size_t size() const { return wordIdxs.size(); }
    bool empty() const { return wordIdxs.empty(); }

    const std::string &getStr() const { return strings[pos]; }
    uint32_t getWordIdx() const { return wordIdxs[pos]; }
    uint8_t getType() const { return types[pos]; }
    size_t getPos() const { return pos; }
    void setPos(size_t aPos) { pos = aPos < size() ? aPos : (!empty() ? size() - 1 : 0);  glossaryIndex = -1;}
    FoundItemsIdx &operator++() { if (pos + 1 < size()) {++pos; glossaryIndex = -1;} return *this; }
    FoundItemsIdx &operator--() { if (pos > 0) {--pos; glossaryIndex = -1;} return *this; }
    void setGlossaryIndex(int aGlossaryIndex) { glossaryIndex = aGlossaryIndex; }
    int getGlossaryIndex() const { return glossaryIndex; }

protected:
    std::vector<uint32_t> wordIdxs;
    std::vector<uint8_t> types;
    std::vector<std::string> strings;
    size_t pos = 0;
    int glossaryIndex = -1;
};

class FoundFormDicIdx: public FoundItemsIdx
{
public:
    void clear() { formWordIdxs.clear(); formTypes.clear(); formStrings.clear(); FoundItemsIdx::clear(); }
    void add(uint32_t wordIdx, uint8_t type, const std::string str,
             uint32_t formWordIdx, uint8_t formType, const std::string formStr)
    {
        FoundItemsIdx::add(wordIdx, type, str);
        formWordIdxs.push_back(formWordIdx);
        formTypes.push_back(formType);
        formStrings.push_back(formStr);
    }
    const std::string &getFormStr() const { return formStrings[pos]; }
    uint32_t getFormWordIdx() const { return formWordIdxs[pos]; }
    uint8_t getFormType() const { return formTypes[pos]; }

protected:
    std::vector<uint32_t> formWordIdxs;
    std::vector<uint8_t> formTypes;
    std::vector<std::string> formStrings;
};

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
        bool useRangeTema = false;
        size_t startIdxTema = 0;
        size_t lastIdxTema = 0;

        bool useRangeFreq = false;
        uint32_t startFreqIdx = 0;
        uint32_t endFreqIdx = 100;

        size_t posIgnoringStatistics = 10;

    private:
        const GlossaryDe &glDe;
    };

public:
    GlossaryDe();
    void setPath(const std::string &path);
    void setFile(const std::string &name);
    void load(const bool saveDbg = true);
    void loadThemes(const std::string &fileName);
    void save();
    void saveClear();
    void fixMainDic();

    void clearAllStatistic();
    size_t calcTestWortIdx(const SelectSettings &selSet); // returns the index of the next word to learn
    int selectVariantsTr(std::vector<size_t> &vecIdxTr);

    void add(const WortDe &wd);
    void insert(size_t idx, const WortDe &wd);
    void importTr(const GlossaryDe &impGloss);
    void importStat(const GlossaryDe &impGloss);

    const WortDe &at(size_t idx) const;
    WortDe &at(size_t idx);
    WortDe &operator[] (size_t idx) { return at(idx); }
    const WortDe &operator[] (size_t idx) const { return at(idx); }
    size_t size() const { return dictionary.size(); }
    bool empty() const { return dictionary.empty(); }

    size_t find(const std::string &str, size_t pos = 0);
    size_t findByWordIdx(const uint32_t wordIdx, size_t pos = 0);
    String255Iterator findStatDic(const std::string &str, size_t pos, uint8_t &option);
    String255Iterator findStatForm(const std::string &str, size_t pos, uint8_t &option, uint32_t &idxDic);
    String255 atStatDic(size_t idx, uint8_t &option) { return statWords.atDicStrIdx(idx, option); }
    String255 atStatForm(size_t idx, uint8_t &option, uint32_t &idxDic) { return statWords.atFormStrIdx(idx, option, idxDic); }

    std::string tema(const unsigned int blockNum);
    const Tema &getTemaByIndex(size_t idx) const;
    size_t themesSize() const { return themesVector.size(); }
    static WortDe::BlockNumType userBlockNum();

    template< typename Func >
    size_t selectIdxFilter(Func func, std::vector<size_t> &selectionIdxs, const SelectSettings &selSet);

private:
    DictionaryDe dictionary;
    StatWords statWords;
    std::string fileName;
    std::string filePath;
    bool notLoaded = false;
    std::map<unsigned int, std::string> themes;
    std::vector<Tema> themesVector;
    size_t beginUserWort = 0;

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
