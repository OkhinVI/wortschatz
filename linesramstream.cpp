#include "linesramstream.h"
#include <iostream>
#include <fstream>

// class LinesRamIStream

static std::string NullString;
static const std::string NullConstString;

void LinesRamIStream::load(std::istream &is, size_t maxLines)
{
    clear();
    while(!is.eof() && vs.size() < maxLines)
    {
        std::string str = util::getline(is);
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
    if (gPos < vs.size())
        return vs[gPos++];
    return NullConstString;
}

const std::string &LinesRamIStream::peek() const
{
    if (gPos < vs.size())
        return vs[gPos];
    return NullConstString;
}

void LinesRamIStream::seekg(size_t pos)
{
    gPos = pos < vs.size() ? pos : vs.size();
}

const std::string &LinesRamIStream::operator[](size_t idx) const
{
    return idx >= vs.size() ? NullConstString : vs[idx];
}

// class LinesRamStream

LinesRamStream::LinesRamStream()
{

}

std::string &LinesRamStream::operator[](size_t idx)
{
    return idx >= vs.size() ? NullString : vs[idx];
}
