#ifndef GLOSSARYDE_H
#define GLOSSARYDE_H

#include "wortde.h"
#include <vector>
#include <map>

class GlossaryDe
{
public:
    typedef std::vector<WortDe> DictionaryDe;

public:
    GlossaryDe();
    void setPath(const std::string &path);
    void setFile(const std::string &name);
    void load();
    void loadThemes(const std::string &fileName);
    void save();
    void saveClear();

    void add(const WortDe &wd);
    const WortDe &at(size_t idx) const;
    WortDe &at(size_t idx);
    const WortDe &operator[] (size_t idx) { return at(idx); }
    size_t size() const { return dictionary.size(); }

    std::string tema(const unsigned int blockNum);

private:
    DictionaryDe dictionary;
    std::string fileName;
    std::string filePath;
    bool notLoaded = false;
    std::map<unsigned int, std::string> themes;
};

#endif // GLOSSARYDE_H
