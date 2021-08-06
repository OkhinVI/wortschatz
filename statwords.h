#ifndef STATWORDS_H
#define STATWORDS_H

#include <vector>
#include <string>
#include <type_traits>

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
    void save(std::ofstream &os);
    bool load(std::istream &is);

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


template<class TypeUint8>
class Uint24IteratorTemplate
{
public:
    static const size_t size_ofItem = 3;

public:
    explicit Uint24IteratorTemplate(TypeUint8 *aBuf): buf(aBuf) {}
    uint32_t get() const { return (uint32_t(buf[2]) << 16) + (uint32_t(buf[1]) << 8) + buf[0]; }

    typename std::enable_if<!std::is_const<TypeUint8 *>::value, void>::type
    set(uint32_t num) { buf[0] = num; buf[1] = num >> 8; buf[2] = num >> 16; }

    static Uint24IteratorTemplate AddToVector(std::vector<uint8_t> &vec, uint32_t num) {
        vec.push_back(num); vec.push_back(num >> 8); vec.push_back(num >> 16);
        return Uint24IteratorTemplate(&vec[vec.size() - size_ofItem]);
    }

    bool operator==(const Uint24IteratorTemplate &it) const { return buf == it.buf; }
    bool operator!=(const Uint24IteratorTemplate &it) const { return buf != it.buf; }
    Uint24IteratorTemplate &operator++() { buf += size_ofItem; return *this; }
    Uint24IteratorTemplate &operator--() { buf -= size_ofItem; return *this; }
    Uint24IteratorTemplate &operator+=(size_t n) { buf += size_ofItem * n; return *this; }
    Uint24IteratorTemplate &operator-=(size_t n) { buf -= size_ofItem * n; return *this; }
    Uint24IteratorTemplate operator+(size_t n) { return Uint24IteratorTemplate(buf + size_ofItem * n); }
    Uint24IteratorTemplate operator-(size_t n) { return Uint24IteratorTemplate(buf - size_ofItem * n); }
    ssize_t operator-(const Uint24IteratorTemplate &it) { return (buf - it.buf) / size_ofItem; }
    uint32_t operator*() const { return get(); }

protected:
    TypeUint8 *buf = nullptr;
};

typedef Uint24IteratorTemplate<const uint8_t> Uint24ConstIterator;
typedef Uint24IteratorTemplate<uint8_t> Uint24Iterator;

class VectorUint24
{
public:
    static const size_t size_ofItem = Uint24ConstIterator::size_ofItem;

public:
    size_t size() const { return rawVector.size() / size_ofItem; }
    bool empty() const { return rawVector.empty(); }
    void clear() { rawVector.clear(); }
    void push_back(uint32_t num) { Uint24Iterator::AddToVector(rawVector, num); }
    uint32_t operator[](size_t idx) const { return *(Uint24ConstIterator(&rawVector[0]) + idx); }
    Uint24Iterator begin() { return Uint24Iterator(&rawVector[0]); }
    Uint24Iterator end() { return Uint24Iterator(&rawVector[rawVector.size()]); }
    Uint24ConstIterator begin() const { return Uint24ConstIterator(&rawVector[0]); }
    Uint24ConstIterator end() const { return Uint24ConstIterator(&rawVector[rawVector.size()]); }

    Uint24Iterator getIterator(size_t idx) { return Uint24Iterator(&rawVector[0])+= idx; }

    bool save(std::ostream &os);
    bool load(std::istream &is);

private:
    std::vector<uint8_t> rawVector;
};


class DicWordIdx
{
public:
    DicWordIdx();
    void exportFromFileIdx(const std::string &fileName);
    bool load(const std::string &path);
    bool save();
    String255Iterator findStrIdx(const std::string &str, size_t pos, bool firstPart, uint8_t &option);
    String255 at(size_t idx, uint8_t &option) { return dicWortIdx.get(idx, option); }

private:
    WordsStore dicWortIdx;
    std::string fileName;
    bool changed = false;
};

class FormWordIdx
{
public:
    FormWordIdx();
    void exportFromFileIdx(const std::string &fileName);
    bool load(const std::string &path);
    bool save();
    String255Iterator findStrIdx(const std::string &str, size_t pos, bool firstPart, uint8_t &option, uint32_t &idxDic);
    String255 at(size_t idx, uint8_t &option, uint32_t &idxDic) { idxDic = idx24Dics[idx]; return formWortIdx.get(idx, option); }

private:
    WordsStore formWortIdx;
    VectorUint24 idx24Dics;
    std::string fileName;
    bool changed = false;
};

class StatWords
{
public:
    StatWords();
    bool load(const std::string &path);
    String255Iterator findDicStrIdx(const std::string &str, size_t pos, bool firstPart, uint8_t &option);
    String255Iterator findFormStrIdx(const std::string &str, size_t pos, bool firstPart, uint8_t &option, uint32_t &idxDic);
    String255 atDicStrIdx(size_t idx, uint8_t &option) { return dicWorts.at(idx, option); }
    String255 atFormStrIdx(size_t idx, uint8_t &option, uint32_t &idxDic) { return formWords.at(idx, option, idxDic); }

private:
    DicWordIdx dicWorts;
    FormWordIdx formWords;
};

#endif // STATWORDS_H
