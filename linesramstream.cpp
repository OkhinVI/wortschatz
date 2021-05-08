#include "linesramstream.h"
#include <iostream>
#include <fstream>

// class LinesRamIStream

void LinesRamIStream::load(std::istream &is, size_t maxLines)
{
    std::string str;
    clear();
    while(!is.eof() && vs.size() < maxLines)
    {
        getline(is, str);
        if (!is.eof() || !str.empty())
            vs.push_back(str);
    }
}

bool LinesRamIStream::loadFromFile(const std::string &fileName, size_t maxLines)
{
    std::ifstream is;
    clear();
    is.open(fileName);
    if (is.fail())
        return false;

    load(is, maxLines);
    return true;
}

const std::string &LinesRamIStream::get()
{
    static std::string endedString;
    if (gPos < vs.size())
        return vs[gPos++];
    return endedString;
}

void LinesRamIStream::seekg(size_t pos)
{
    gPos = pos < vs.size() ? pos : vs.size();
}

// class LinesRamStream

LinesRamStream::LinesRamStream()
{

}
