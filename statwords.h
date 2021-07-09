#ifndef STATWORDS_H
#define STATWORDS_H

#include <vector>
#include <string>

class WordsStore
{
public:
    typedef uint32_t PosStringType;

public:
    PosStringType addString(const std::string &str, uint8_t option);
    void clear() { stringStore.clear(); idxsStore.clear(); optionsStore.clear(); }
    void save(const std::string &fileName);
    bool load(const std::string &fileName);

    const char *get(const uint32_t idx, uint32_t &strSize, uint8_t &option) const;
    size_t size() const { return idxsStore.size(); }
    uint32_t endIdx() const { return idxsStore.size(); }
    uint32_t findIdx(const std::string &str, uint32_t startIdx = 0) const;
    uint32_t findFirstPartStrIdx(const std::string &str, uint32_t startIdx = 0) const;

private:
    uint32_t getStrSize(PosStringType &offset) const;

private:
    std::vector<uint8_t> stringStore;
    std::vector<PosStringType> idxsStore;
    std::vector<uint8_t> optionsStore;
};

class DicWordIdx
{
public:
    DicWordIdx();
    void exportFromFileIdx(const std::string &fileName);

private:
    WordsStore dicWortIdx;
};

class FormWordIdx
{
public:
    FormWordIdx();
    void exportFromFileIdx(const std::string &fileName);

private:
    WordsStore formWortIdx;
};

class StatWords
{
public:
    StatWords();

private:
    DicWordIdx dicWorts;
    FormWordIdx formWords;
};

#endif // STATWORDS_H
