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
    void setPath(const std::string &path);
    void setFile(const std::string &name);
    void load();
    void save();
    void saveClear();

    void add(const WortDe &wd);
    const WortDe &at(size_t idx) const;
    WortDe &at(size_t idx);
    const WortDe &operator[] (size_t idx) { return at(idx); }
    size_t size() const { return dictionary.size(); }

private:
    DictionaryDe dictionary;
    std::string fileName;
    std::string filePath;
    bool notLoaded = false;
};

#endif // GLOSSARYDE_H
