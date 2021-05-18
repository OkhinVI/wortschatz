#include "utility.h"

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
