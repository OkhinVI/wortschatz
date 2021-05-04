#include "utility.h"


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
        if (!isWort(str[idx]))
        {
            if (idx < 1)
                break;
            findedDelimiter = str[idx];
            return str.substr(0, idx);
        }
    }
    return std::string();
}
