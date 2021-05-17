#include "string_utf8.h"


// class AreaString

void AreaString::seekg(size_t aPos)
{
    pos = aPos < size() ? aPos : size();
}

AreaString& AreaString::operator++()
{
    if (pos < size())
        ++pos;
    return *this;
}

AreaString& AreaString::operator--()
{
    if (pos > 0)
        --pos;
    return *this;
}

AreaString AreaString::subArea(size_t pos, size_t len) const
{
    if (pos >= size())
        return AreaString(str, beginPos + size(), 0);
    if (len > size() || pos + len > size())
        return AreaString(str, beginPos + pos, size() - pos);

    return AreaString(str, beginPos + pos, len);
}

bool AreaString::find(const char *s)
{
    const std::string::size_type p = str.find(s, beginPos + pos, sizeStr - pos);
    if (p != std::string::npos)
    {
        pos = p - sizeStr;
        return true;
    }
    return false;
}

AreaString &AreaString::trim()
{
    pos = 0;
    for (; sizeStr > 0; --sizeStr)
    {
        if (!::isspace(str[beginPos + sizeStr - 1]))
            break;
    }
    for (; sizeStr > 0; ++beginPos, --sizeStr)
    {
        if (!::isspace(str[beginPos]))
            break;
    }
    return *this;
}


// class AreaUtf8

const char * const AreaUtf8::defaulDelimiters = " ,\t";


AreaUtf8::SymbolType AreaUtf8::symbol(uint8_t &symSize) const
{
    return symbolFromPos(pos, symSize);
}

AreaUtf8::SymbolType AreaUtf8::symbolFromPos(size_t aPos, uint8_t &symSize) const
{
    symSize = 0;
    if (aPos >= size())
        return 0;

    const uint8_t byte = raw(aPos);
    if (isUtf8Pos(aPos))
    {
        symSize = 2;
        return (static_cast<SymbolType>(byte) << 8) + static_cast<uint8_t>(raw(aPos + 1));
    }
    symSize = 1;
    return byte;
}

#define UTF8_PREFIX_MIN 0xC0
#define UTF8_RAW_SYMBOL_MIN 0x80
#define UTF8_RAW_SYMBOL_MAX 0xBF

bool AreaUtf8::isUtf8Pos(size_t aPos) const
{
    return aPos + 1 < size() &&
           static_cast<uint8_t>(raw(aPos)) >= UTF8_PREFIX_MIN &&
           static_cast<uint8_t>(raw(aPos + 1)) >= UTF8_RAW_SYMBOL_MIN &&
           static_cast<uint8_t>(raw(aPos + 1)) <= UTF8_RAW_SYMBOL_MAX;
}

bool AreaUtf8::isSymbolEn(SymbolType sym)
{
    return (sym >= 'A' && sym <= 'Z') || (sym >= 'a' && sym <= 'z');
}

bool AreaUtf8::isDelimiter(SymbolType sym, const char *delimiters)
{
    size_t idx = 0;
    while(delimiters[idx] != 0)
    {
        if (sym == delimiters[idx++])
            return true;
    }
    return false;
}

#define UTF8_SYMBOL_DE_U_Umlaut 0xC39C
#define UTF8_SYMBOL_DE_u_Umlaut 0xC3BC
#define UTF8_SYMBOL_DE_A_Umlaut 0xC384
#define UTF8_SYMBOL_DE_a_Umlaut 0xC3A4
#define UTF8_SYMBOL_DE_O_Umlaut 0xC396
#define UTF8_SYMBOL_DE_o_Umlaut 0xC3B6
#define UTF8_SYMBOL_DE_Eszett   0xC39F

// if Umlaut return 'u', 'U', 'a', 'A', 'o', 'O' or if Eszett then return 's', else return 0
char AreaUtf8::symbolDeUmlaut(SymbolType sym)
{
    switch (sym)
    {
    case UTF8_SYMBOL_DE_U_Umlaut:
        return 'U';
    case UTF8_SYMBOL_DE_u_Umlaut:
        return 'u';
    case UTF8_SYMBOL_DE_A_Umlaut:
        return 'A';
    case UTF8_SYMBOL_DE_a_Umlaut:
        return 'a';
    case UTF8_SYMBOL_DE_O_Umlaut:
        return 'O';
    case UTF8_SYMBOL_DE_o_Umlaut:
        return 'o';
    case UTF8_SYMBOL_DE_Eszett:
        return 's';
    }
    return 0;
}

bool AreaUtf8::isSymbolDe(SymbolType sym)
{
    if (isSymbolEn(sym))
        return true;

    if (symbolDeUmlaut(sym))
        return true;

    return false;
}

#define UTF8_SYMBOL_RU_A  0xD090
#define UTF8_SYMBOL_RU_p  0xD0BF
#define UTF8_SYMBOL_RU_r  0xD180
#define UTF8_SYMBOL_RU_ja 0xD18F
#define UTF8_SYMBOL_RU_JO 0xD081
#define UTF8_SYMBOL_RU_jo 0xD191

bool AreaUtf8::isSymbolRu(SymbolType sym)
{
    return (sym >= UTF8_SYMBOL_RU_A && sym <= UTF8_SYMBOL_RU_p) ||
            (sym >= UTF8_SYMBOL_RU_r && sym <= UTF8_SYMBOL_RU_ja) ||
            (sym == UTF8_SYMBOL_RU_JO || sym == UTF8_SYMBOL_RU_jo);
}


AreaUtf8::SymbolType AreaUtf8::getSymbol()
{
    uint8_t symSyze;
    const SymbolType sym = symbol(symSyze);
    pos += symSyze;
    return sym;
}

AreaUtf8::SymbolType AreaUtf8::peek() const
{
    uint8_t symSyze;
    return symbol(symSyze);
}

bool AreaUtf8::hasAsciiSymbol() const
{
    return eof() ? false : static_cast<uint8_t>(raw(pos)) <= 0x7F;
}

bool AreaUtf8::hasSymbolEn() const
{
    uint8_t symSyze;
    const SymbolType sym = symbol(symSyze);
    return symSyze > 0 && isSymbolEn(sym);
}

bool AreaUtf8::hasSymbolDe() const
{
    uint8_t symSyze;
    const SymbolType sym = symbol(symSyze);
    return symSyze > 0 && isSymbolDe(sym);
}

bool AreaUtf8::hasSymbolRu() const
{
    uint8_t symSyze;
    const SymbolType sym = symbol(symSyze);
    return symSyze > 0 && isSymbolRu(sym);
}

AreaUtf8& AreaUtf8::operator++()
{
    if (pos < size())
    {
        if (isUtf8Pos(pos))
            pos += 2;
        else
            ++pos;
    }
    return *this;
}

AreaUtf8& AreaUtf8::operator--()
{
    if (pos >= 1)
    {
        if (isUtf8Pos(pos - 2))
            pos -= 2;
        else
            --pos;
    }
    return *this;
}

bool AreaUtf8::operator==(const char *cStr) const
{
    for (size_t i = 0; i < size(); ++i)
    {
        if(cStr[i] == 0 || raw(i) != cStr[i])
            return false;
    }
    return cStr[size()] == 0;
}

AreaUtf8 AreaUtf8::getToken(const char *delimiters)
{
    if (eof())
        return subArea(size(), 0);

    const size_t startPos = tellg();
    uint8_t symSyze;
    const SymbolType startSym = symbol(symSyze);
    const bool delimiterToken = isDelimiter(startSym, delimiters);
    bool result;

    if (delimiterToken)
        result = find([delimiters](SymbolType sym) { return !isDelimiter(sym, delimiters); });
    else
        result = find([delimiters](SymbolType sym) { return isDelimiter(sym, delimiters); });

    if (!result)
    {
        seekg(size()); // end data
        return subArea(startPos, size() - startPos);
    }

    return subArea(startPos, tellg() - startPos);
}

void AreaUtf8::getAllTokens(std::vector<AreaUtf8> &tokens, const char *delimiters)
{
    tokens.clear();
    while (AreaUtf8 tok = getToken(delimiters))
        tokens.push_back(tok);
}

bool isWortDe(const char ch)
{
    const unsigned char chU = ch;
    return (chU > 127 || chU == '-' || ::isalpha(chU));
}

std::string substrWithoutSideSpaces(const std::string &str, size_type posBegin, size_type n)
{
    size_type posEnd = (n == std::string::npos || posBegin + n > str.size()) ?
        str.size() : posBegin + n;
    if (posBegin >= posEnd)
        return std::string();
    for (; posBegin < posEnd; ++posBegin)
    {
        if (!::isspace(str[posBegin]))
            break;
    }
    for (; posBegin < posEnd; --posEnd)
    {
        if (!::isspace(str[posEnd - 1]))
            break;
    }
    return str.substr(posBegin, posEnd - posBegin);
}

std::string getPrefix(const std::string &str, char &findedDelimiter)
{
    for (size_type idx = 0; idx < str.size(); ++idx)
    {
        if (!isWortDe(str[idx]))
        {
            if (idx < 1)
                break;
            findedDelimiter = str[idx];
            return str.substr(0, idx);
        }
    }
    return std::string();
}

/*  // search example
    const std::string str = " Hallo Alle!";
    AreaUtf8 cit(str);
    cit.find([](AreaUtf8::SymbolType sym) { return AreaUtf8::isSymbolDe(sym); });
*/
