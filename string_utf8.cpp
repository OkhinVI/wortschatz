#include "string_utf8.h"
#include <cctype>

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

bool AreaUtf8::isUtf8Pos(size_t aPos) const
{
    return aPos + 1 < size() && IS_UTF8_PREFIX2(raw(aPos)) && IS_UTF8_RAW_SYMBOL(raw(aPos + 1));
}

int AreaUtf8::SymboToInt(SymbolType sym)
{
    if (!IS_VALID_UTF8_2BYTE_SYMBOL(sym)) // sym <= 0x7F or sym is not 2 byte utf8
        return sym & 0xFF;

    return UTF8_2BYTE_SYMBOL_TO_INT(sym);
}

AreaUtf8::SymbolType AreaUtf8::IntToSymbo(int sym)
{
    if (sym < 0 || sym > UTF8_MAX_INT_SYMBOL)
        return 0;
    if (sym < 0x100)
        return sym;
    return UTF8_INT_TO_2BYTE_SYMBOL(sym);
}

std::string &AreaUtf8::StringAddSym(std::string &str, SymbolType sym)
{
    if ((sym & ~0xFF) != 0)
        str = str + char((unsigned char)(sym >> 8)) + char((unsigned char)(sym & 0xFF));
    else
        str = str + char((unsigned char)(sym & 0xFF));
    return str;
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

// if Umlaut return 'u', 'U', 'a', 'A', 'o', 'O' or if Eszett then return 's', else return 0
char AreaUtf8::symbolDeUmlaut(SymbolType sym)
{
    switch (sym)
    {
    case UTF8_STRING_TO_SYMBOL("Ü"):
        return 'U';
    case UTF8_STRING_TO_SYMBOL("ü"):
        return 'u';
    case UTF8_STRING_TO_SYMBOL("Ä"):
        return 'A';
    case UTF8_STRING_TO_SYMBOL("ä"):
        return 'a';
    case UTF8_STRING_TO_SYMBOL("Ö"):
        return 'O';
    case UTF8_STRING_TO_SYMBOL("ö"):
        return 'o';
    case UTF8_STRING_TO_SYMBOL("ß"):
        return 's';
    case 0:
        return 0;
    }
    return 0;
}

AreaUtf8::SymbolType AreaUtf8::tolowerU8(SymbolType sym)
{
    if (sym <= 0x7F)
        return ::tolower(static_cast<char>(sym));
    const int intSym = SymboToInt(sym);
    if ((intSym >= UTF8_STRING_TO_INT_SYMBOL("А") && intSym <= UTF8_STRING_TO_INT_SYMBOL("Я")))
        return IntToSymbo(intSym + UTF8_STRING_TO_INT_SYMBOL("а") - UTF8_STRING_TO_INT_SYMBOL("А"));

    switch (sym)
    {
    case       UTF8_STRING_TO_SYMBOL("Ü"):
        return UTF8_STRING_TO_SYMBOL("ü");
    case       UTF8_STRING_TO_SYMBOL("Ä"):
        return UTF8_STRING_TO_SYMBOL("ä");
    case       UTF8_STRING_TO_SYMBOL("Ö"):
        return UTF8_STRING_TO_SYMBOL("ö");
    case       UTF8_STRING_TO_SYMBOL("Ё"):
        return UTF8_STRING_TO_SYMBOL("ё");
    case 0:
        return 0;
    }
    return sym;
}

bool AreaUtf8::islowerU8(SymbolType sym)
{
    if (sym <= 0x7F)
        return ::islower(static_cast<char>(sym));
    const int intSym = SymboToInt(sym);
    return (intSym >= UTF8_STRING_TO_INT_SYMBOL("а")
            && intSym <= UTF8_STRING_TO_INT_SYMBOL("я"))
            || sym == UTF8_STRING_TO_SYMBOL("ü")
            || sym == UTF8_STRING_TO_SYMBOL("ä")
            || sym == UTF8_STRING_TO_SYMBOL("ö")
            || sym == UTF8_STRING_TO_SYMBOL("ß")
            || sym == UTF8_STRING_TO_SYMBOL("ё");
}

bool AreaUtf8::isupperU8(SymbolType sym)
{
    if (sym <= 0x7F)
        return ::isupper(static_cast<char>(sym));
    const int intSym = SymboToInt(sym);
    return (intSym >= UTF8_STRING_TO_INT_SYMBOL("А")
            && intSym <= UTF8_STRING_TO_INT_SYMBOL("Я"))
            || sym == UTF8_STRING_TO_SYMBOL("Ü")
            || sym == UTF8_STRING_TO_SYMBOL("Ä")
            || sym == UTF8_STRING_TO_SYMBOL("Ö")
            || sym == UTF8_STRING_TO_SYMBOL("Ё");
}

bool AreaUtf8::islowerDe(SymbolType sym)
{
    if (sym <= 0x7F)
        return ::islower(static_cast<char>(sym));
    return sym == UTF8_STRING_TO_SYMBOL("ü")
            || sym == UTF8_STRING_TO_SYMBOL("ä")
            || sym == UTF8_STRING_TO_SYMBOL("ö")
            || sym == UTF8_STRING_TO_SYMBOL("ß");
}

bool AreaUtf8::isupperDe(SymbolType sym)
{
    if (sym <= 0x7F)
        return ::isupper(static_cast<char>(sym));
    return sym == UTF8_STRING_TO_SYMBOL("Ü")
            || sym == UTF8_STRING_TO_SYMBOL("Ä")
            || sym == UTF8_STRING_TO_SYMBOL("Ö");
}

bool AreaUtf8::islowerRu(SymbolType sym)
{
    const int intSym = SymboToInt(sym);
    return (intSym >= UTF8_STRING_TO_INT_SYMBOL("а")
            && intSym <= UTF8_STRING_TO_INT_SYMBOL("я"))
            || sym == UTF8_STRING_TO_SYMBOL("ё");
}

bool AreaUtf8::isupperRu(SymbolType sym)
{
    const int intSym = SymboToInt(sym);
    return (intSym >= UTF8_STRING_TO_INT_SYMBOL("А")
            && intSym <= UTF8_STRING_TO_INT_SYMBOL("Я"))
            || sym == UTF8_STRING_TO_SYMBOL("Ё");
}

bool AreaUtf8::findCase(const std::string str)
{
    if (str.empty())
        return false;

    const size_t startPos = tellg();
    AreaUtf8 au8(str);
    const SymbolType firstSym = tolowerU8(au8.getSymbol());
    const size_t secondAu8Pos = au8.tellg();

    while (find([firstSym](AreaUtf8::SymbolType sym) { return firstSym == AreaUtf8::tolowerU8(sym); })) {
        au8.seekg(secondAu8Pos);
        const size_t findPos = tellg();
        getSymbol();
        bool finded = true;
        while(!au8.eof())
        {
            if (eof() || tolowerU8(getSymbol()) != tolowerU8(au8.getSymbol()))
            {
                finded = false;
                break;
            }
        }
        if (finded)
        {
            seekg(findPos);
            return true;
        }
    }

    seekg(startPos);
    return false;
}

bool AreaUtf8::isSymbolDe(SymbolType sym)
{
    if (isSymbolEn(sym) || symbolDeUmlaut(sym))
        return true;

    return false;
}

bool AreaUtf8::isSymbolRu(SymbolType sym)
{
    const int intSym = SymboToInt(sym);
    return (intSym >= UTF8_STRING_TO_INT_SYMBOL("А") && intSym <= UTF8_STRING_TO_INT_SYMBOL("я"))
            || intSym == UTF8_STRING_TO_INT_SYMBOL("Ё") || intSym == UTF8_STRING_TO_INT_SYMBOL("ё");
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

/*  // search example
    const std::string str = " Hallo Alle!";
    AreaUtf8 cit(str);
    cit.find([](AreaUtf8::SymbolType sym) { return AreaUtf8::isSymbolDe(sym); });
*/
