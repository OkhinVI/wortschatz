#ifndef STRING_UTF8_H
#define STRING_UTF8_H

#include <string>
#include <limits>
#include <vector>

class AreaString
{
public:
    explicit AreaString(const std::string& _str, size_t aBegin = 0, size_t aSizeStr = std::numeric_limits<size_t>::max()):
        str(_str),
        beginPos(aBegin),
        sizeStr( aSizeStr > _str.size() ? _str.size() : aSizeStr) {}

    char raw(size_t idx) const { return str[idx + beginPos]; }
    bool eof() const { return pos >= size(); }
    size_t tellg() const { return pos; }
    void seekg(size_t aPos);
    AreaString& operator++();
    AreaString& operator--();
    char operator*() const { return raw(pos); };
    explicit operator bool () const { return !eof(); }
    std::string toString() const { return str.substr(beginPos, sizeStr); }
    size_t size() const { return sizeStr; }
    bool empty() const { return size() == 0; }
    AreaString subArea(size_t pos = 0, size_t len = std::numeric_limits<size_t>::max()) const;
    AreaString getRestArea() const { return subArea(pos); }
    bool find(const char *s);
    AreaString &trim();

protected:
    size_t pos = 0;

private:
    const std::string &str;
    size_t beginPos = 0;
    size_t sizeStr = 0;
};

class AreaUtf8: public AreaString
{
public:
    typedef unsigned short int SymbolType;

protected:
    static const char * const defaulDelimiters;

public:
    explicit AreaUtf8(const std::string& _str, size_t aBegin = 0, size_t aSizeStr = std::numeric_limits<size_t>::max()):
        AreaString(_str, aBegin, aSizeStr) {}
    AreaUtf8(const AreaString &as): AreaString(as) {}
    static bool isDelimiter(SymbolType sym, const char *delimiters = defaulDelimiters);
    static bool isSymbolEn(SymbolType sym);
    static bool isSymbolDe(SymbolType sym);
    static bool isSymbolRu(SymbolType sym);
    static bool islowerU8(SymbolType sym);
    static bool isSymbolAnyWord(SymbolType sym) { return isSymbolDe(sym) || isSymbolRu(sym) || sym == '-'; }
    static char symbolDeUmlaut(SymbolType sym); // Umlaut or Eszett
    static SymbolType tolowerU8(SymbolType sym);
    static int SymboToInt(SymbolType sym);
    static SymbolType IntToSymbo(int sym);

    SymbolType peek() const;
    SymbolType getSymbol();
    bool hasAsciiSymbol() const;
    bool hasSymbolEn() const;
    bool hasSymbolDe() const;
    bool hasSymbolRu() const;
    AreaUtf8& operator++();
    AreaUtf8& operator--();
    SymbolType operator*() const { return peek(); };
    bool operator==(const char *cStr) const;
    bool operator!=(const char *cStr) const { return !operator==(cStr); }
    AreaUtf8 subArea(size_t pos = 0, size_t len = std::numeric_limits<size_t>::max()) const { return AreaUtf8(AreaString::subArea(pos, len)); }
    AreaUtf8 getToken(const char *delimiters = defaulDelimiters);
    void getAllTokens(std::vector<AreaUtf8> &tokens, const char *delimiters = defaulDelimiters);

    bool findCase(const std::string str);

    template< typename Func > bool find(Func func);
    bool findWordDe() { return find([](SymbolType sym) { return isSymbolDe(sym); }); }
    bool findWordRu() { return find([](SymbolType sym) { return isSymbolRu(sym); }); }
    bool findDelimiter() { return find([](SymbolType sym) { return isDelimiter(sym); }); }
    bool findNotDelimiter() { return find([](SymbolType sym) { return !isDelimiter(sym); }); }

protected:
    SymbolType symbolFromPos(size_t aPos, uint8_t &symSize) const;
    SymbolType symbol(uint8_t &symSize) const;
    bool isUtf8Pos(size_t aPos) const;
};

template <typename Func> bool AreaUtf8::find(Func func)
{
    size_t index = pos;
    while (index < size())
    {
        uint8_t symSize;
        const SymbolType sym = symbolFromPos(index, symSize);
        if (func(sym))
        {
            pos = index;
            return true;
        }
        index += symSize;
    }
    return false;
}

#define UTF8_PREFIX2_MIN      uint8_t(0b11000000)
#define UTF8_PREFIX2_MAX      uint8_t(0b11011111)
#define UTF8_RAW_SYMBOL_MIN   uint8_t(0b10000000)
#define UTF8_RAW_SYMBOL_MAX   uint8_t(0b10111111)
#define UTF8_RAW_SYMBOL_COUNT (UTF8_RAW_SYMBOL_MAX - UTF8_RAW_SYMBOL_MIN + 1)
#define UTF8_PREFIX2_COUNT    (UTF8_PREFIX2_MAX - UTF8_PREFIX2_MIN + 1)
#define UTF8_MAX_INT_SYMBOL          (int(UTF8_PREFIX2_COUNT) * UTF8_RAW_SYMBOL_COUNT + 0x100)

#define UTF8_PREFIX2_TEST_MASK    uint8_t(0b11100000)
#define UTF8_PREFIX2_TEST_VAL     uint8_t(0b11000000)
#define UTF8_RAW_SYMBOL_TEST_MASK uint8_t(0b11000000)
#define UTF8_RAW_SYMBOL_TEST_VAL  uint8_t(0b10000000)
#define IS_UTF8_PREFIX2(sym_)     ((uint8_t(sym_) & UTF8_PREFIX2_TEST_MASK) == UTF8_PREFIX2_TEST_VAL)
#define IS_UTF8_RAW_SYMBOL(sym_)  ((uint8_t(sym_) & UTF8_RAW_SYMBOL_TEST_MASK) == UTF8_RAW_SYMBOL_TEST_VAL)

#define UTF8_SYMBOL_TEST_MASK ((AreaUtf8::SymbolType(UTF8_PREFIX2_TEST_MASK) << 8) + UTF8_RAW_SYMBOL_TEST_MASK)
#define UTF8_SYMBOL_TEST_VAL  ((AreaUtf8::SymbolType(UTF8_PREFIX2_TEST_VAL) << 8) + UTF8_RAW_SYMBOL_TEST_VAL)
#define IS_VALID_UTF8_2BYTE_SYMBOL(sym_) ((sym_ & UTF8_SYMBOL_TEST_MASK) == UTF8_SYMBOL_TEST_VAL)

#define UTF8_2BYTE_SYMBOL_TO_INT(sym_) (((sym_ >> 8) - UTF8_PREFIX2_MIN) * UTF8_RAW_SYMBOL_COUNT + (sym_ & 0xFF) - UTF8_RAW_SYMBOL_MIN + 0x100)
#define UTF8_INT_TO_2BYTE_SYMBOL(sym_) ((((sym_ - 0x100) / UTF8_RAW_SYMBOL_COUNT) + UTF8_PREFIX2_MIN) * 0x100 + (((sym_ - 0x100) % UTF8_RAW_SYMBOL_COUNT)) + UTF8_RAW_SYMBOL_MIN)

#define UTF8_STRING_TO_SYMBOL(str_) \
    (sizeof(str_) < 3 \
    ? uint8_t(str_[0]) \
    : ((IS_UTF8_PREFIX2(str_[0]) && IS_UTF8_RAW_SYMBOL(str_[1])) \
      ? (AreaUtf8::SymbolType(uint8_t(str_[0])) << 8) + uint8_t(str_[1]) \
      : 0) \
    )
#define UTF8_STRING_TO_INT_SYMBOL(str_) \
    (sizeof(str_) < 3 \
    ? uint8_t(str_[0]) \
    : ((IS_UTF8_PREFIX2(str_[0]) && IS_UTF8_RAW_SYMBOL(str_[1])) \
      ? ((uint8_t(str_[0]) - UTF8_PREFIX2_MIN) * UTF8_RAW_SYMBOL_COUNT + uint8_t(str_[1]) - UTF8_RAW_SYMBOL_MIN + 0x100) \
      : 0) \
    )


#endif // STRING_UTF8_H
