#ifndef LINESRAMSTREAM_H
#define LINESRAMSTREAM_H

#include <string>
#include <vector>
#include <limits>
#include "utility.h"

class LinesRamIStream
{
public:
    void load(std::istream &is, size_t maxLines = std::numeric_limits<size_t>::max());
    bool loadFromFile(const std::string &fileName, size_t maxLines = std::numeric_limits<size_t>::max());
    const std::string &get();
    size_t remainderOfLines() const { return vs.size() - gPos; }
    bool eof() const { return gPos >= vs.size(); }
    size_t tellg() const { return gPos; }
    void seekg(size_t pos);
    const std::string &peek() const;

    const std::string &operator[](size_t idx) const;
    bool empty() const { return vs.empty(); }
    bool size() const { return vs.size(); }

protected:
    void clear() { vs.clear(); gPos = 0; }

protected:
    util::VectorString vs;
    size_t gPos = 0;
};



class LinesRamStream: public LinesRamIStream
{
public:
    LinesRamStream();

    std::string &operator[](size_t idx);

protected:
    void clear() { LinesRamIStream::clear(); pPos = 0; }

protected:
    size_t pPos = 0;
};

#endif // LINESRAMSTREAM_H
