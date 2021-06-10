#include "utility.h"
#include <iostream>

void util::VectorFromString(VectorString &vec, const std::string &str)
{
    vec.clear();
    for (size_t pos = 0; pos < str.size();)
    {
        std::string line;
        const size_t newPos = str.find('\n', pos);
        if (newPos == std::string::npos)
        {
            if (pos < str.size())
                line = str.substr(pos);
            else
                break;
            pos = str.size();
        } else
        {
            line = str.substr(pos, newPos - pos);
            pos = newPos + 1;
        }
        vec.push_back(line);
    }
}

void util::replaceEndLines(std::string &str)
{
    for(size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '\n' || str[i] == '\r')
            str[i] = ' ';
    }
}

std::string util::getline(std::istream &is)
{
    std::string line;
    std::getline(is, line);
    size_t pos;
    for (pos = line.size(); pos > 0; --pos)
    {
        const char sym = line[pos - 1];
        if (sym != '\n' && sym != '\r')  // fix DOS endline "\r\n" for *unix systems
            break;
    }
    if (pos < line.size())
        line.resize(pos);
    return line;
}
