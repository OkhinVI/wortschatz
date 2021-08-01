#ifndef STATWORDS_H
#define STATWORDS_H

#include <vector>
#include <string>

// String255: 1byte - size, size bytes - string, 1byte - '\0'
// Example "Word": { 0x4, 'W', 'o', 'r', 'd', 0x0 }
class String255
{
public:
    explicit String255(const uint8_t *aBuf): buf(aBuf) {}
    uint8_t size() const { return buf[0]; }
    const char *c_str() const { return reinterpret_cast<const char*>(buf + 1); }
    const char *begin() const { return reinterpret_cast<const char*>(buf + 1); }
    const char *end() const { return reinterpret_cast<const char*>(buf + 1 + size()); }
    bool empty() const { return !buf || !buf[0]; }
    bool valid() const { return buf != nullptr; }

    const uint8_t *raw() const { return buf; }
    String255 nextString() const;

    char operator[](size_t idx) const { return static_cast<char>(buf[1 + idx]); }
    bool operator==(const String255 &aStr) const;
    bool operator==(const char *aCStr) const;
    bool operator==(const std::string &aStr) const;

private:
    const uint8_t *buf = nullptr;
};

class String255Iterator
{
public:
    explicit String255Iterator(const String255 &aStr, uint32_t aIdx): str(aStr), idx(aIdx) {}
    bool operator==(const String255Iterator &it) const { return str.raw() == it.str.raw(); }
    bool operator!=(const String255Iterator &it) const { return str.raw() != it.str.raw(); }
    const String255 &operator*() const { return str; }
    const String255 *operator->() const { return &str; }
    String255Iterator &operator++();
    uint32_t getIdx() const { return idx; }

private:
    String255 str;
    uint32_t idx = 0;
};


class WordsStore
{
public:
    typedef uint32_t PosStringType;

public:
    uint32_t addString(const std::string &str, uint8_t option);
    void clear() { stringStore.clear(); idxsStore.clear(); optionsStore.clear(); }
    void save(const std::string &fileName);
    bool load(const std::string &fileName);

    String255 operator[](size_t idx) const { return String255(&stringStore[idxsStore[idx]]); }
    size_t size() const { return idxsStore.size(); }
    String255Iterator begin() const { return String255Iterator(String255(&stringStore[0]), 0); }
    String255Iterator end() const { return String255Iterator(String255(&stringStore[stringStore.size()]), stringStore.size()); }

    String255 get(const uint32_t idx, uint8_t &option) const;
    uint8_t getOption(const uint32_t idx) { return optionsStore[idx]; }
    String255Iterator findIdx(const std::string &str, uint32_t startIdx = 0) const;
    String255Iterator findFirstPartStrIdx(const std::string &str, uint32_t startIdx = 0) const;
    String255Iterator getIterator(size_t idx) const;

private:
    uint32_t getStrSize(PosStringType &offset) const { return stringStore[offset++]; };

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
    bool load(const std::string &path);
    String255Iterator findStrIdx(const std::string &str, size_t pos, bool firstPart, uint8_t &option);

private:
    WordsStore dicWortIdx;
    bool changed = false;
};

class FormWordIdx
{
public:
    FormWordIdx();
    void exportFromFileIdx(const std::string &fileName);
    bool load(const std::string &path);

private:
    WordsStore formWortIdx;
    bool changed = false;
};

class StatWords
{
public:
    StatWords();
    bool load(const std::string &path);
    String255Iterator findDicStrIdx(const std::string &str, size_t pos, bool firstPart, uint8_t &option);

private:
    DicWordIdx dicWorts;
    FormWordIdx formWords;
};

#endif // STATWORDS_H
