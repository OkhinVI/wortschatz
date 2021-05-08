#ifndef GLOSSARYDE_H
#define GLOSSARYDE_H

#include "wortde.h"
#include <vector>

class GlossaryDe
{
public:
    typedef std::vector<WortDe> DictionaryDe;

public:
    GlossaryDe();
    void load();
    void save();

private:
    DictionaryDe dictionary;
    std::string fileName;
    std::string filePath;
};

#endif // GLOSSARYDE_H
