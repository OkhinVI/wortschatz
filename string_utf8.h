#ifndef STRING_UTF8_H
#define STRING_UTF8_H

#include <string>
#include <limits>

class AreaString
{
public:
    explicit AreaString(const std::string& _str, size_t aBegin = 0, size_t aSizeStr = std::numeric_limits<size_t>::max()):
        str(_str),
        beginPos(aBegin),
        sizeStr( aSizeStr > _str.size() ? _str.size() : aSizeStr) {}

    char raw(size_t idx) { return str[idx + beginPos]; }
    bool eof() const { return pos >= size(); }
    size_t tellg() const { return pos; }
    void seekg(size_t aPos);
    AreaString& operator++();
    AreaString& operator--();
    char operator*() { return raw(pos); };
    std::string toString() { return str.substr(beginPos, sizeStr); }
    size_t size() const { return sizeStr; }
    bool empty() const { return size() == 0; }
    AreaString subArea(size_t pos = 0, size_t len = std::numeric_limits<size_t>::max());
    bool find(const char *s);
    void trim();

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

public:
    explicit AreaUtf8(const std::string& _str, size_t aBegin = 0, size_t aSizeStr = std::numeric_limits<size_t>::max()):
        AreaString(_str, aBegin, aSizeStr) {}
    AreaUtf8(const AreaString &as): AreaString(as) {}
    static bool isDelimiter(SymbolType sym, const char *delimiters = " ,\t");
    static bool isSymbolEn(SymbolType sym);
    static bool isSymbolDe(SymbolType sym);
    static bool isSymbolRu(SymbolType sym);
    static bool isSymbolAnyWord(SymbolType sym) { return isSymbolDe(sym) || isSymbolRu(sym) || sym == '-'; }
    static char symbolDeUmlaut(SymbolType sym); // Umlaut or Eszett

    SymbolType peek();
    SymbolType getSymbol();
    bool hasAsciiSymbol();
    bool hasSymbolEn();
    bool hasSymbolDe();
    bool hasSymbolRu();
    AreaUtf8& operator++();
    AreaUtf8& operator--();
    SymbolType operator*() { return peek(); };
    AreaUtf8 subArea(size_t pos = 0, size_t len = std::numeric_limits<size_t>::max()) { return AreaUtf8(AreaString::subArea(pos, len)); }
    AreaUtf8 getToken(bool &delimiterToken, const char *delimiters = " ,\t");

    template< typename Func > bool find(Func func);
    bool findWordDe() { return find([](SymbolType sym) { return isSymbolDe(sym); }); }
    bool findWordRu() { return find([](SymbolType sym) { return isSymbolRu(sym); }); }
    bool findDelimiter() { return find([](SymbolType sym) { return isDelimiter(sym); }); }
    bool findNotDelimiter() { return find([](SymbolType sym) { return !isDelimiter(sym); }); }

protected:
    SymbolType symbolFromPos(size_t aPos, uint8_t &symSize);
    SymbolType symbol(uint8_t &symSize);
    bool isUtf8Pos(size_t aPos);
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

bool isWortDe(const char ch);

// use std::string
typedef std::string::size_type size_type;

std::string substrWithoutSideSpaces(const std::string &str, size_type posBegin = 0, size_type n = std::string::npos);
std::string getPrefix(const std::string &str, char &findedDelimiter);


#endif // STRING_UTF8_H
