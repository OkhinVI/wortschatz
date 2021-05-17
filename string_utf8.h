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
    static bool isSymbolAnyWord(SymbolType sym) { return isSymbolDe(sym) || isSymbolRu(sym) || sym == '-'; }
    static char symbolDeUmlaut(SymbolType sym); // Umlaut or Eszett

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

bool isWortDe(const char ch);

// use std::string
typedef std::string::size_type size_type;

std::string substrWithoutSideSpaces(const std::string &str, size_type posBegin = 0, size_type n = std::string::npos);
std::string getPrefix(const std::string &str, char &findedDelimiter);


#endif // STRING_UTF8_H
