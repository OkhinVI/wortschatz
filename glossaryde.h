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
    void fixMainDic();

    void add(const WortDe &wd);
    const WortDe &at(size_t idx) const;
    WortDe &at(size_t idx);
    WortDe &operator[] (size_t idx) { return at(idx); }
    const WortDe &operator[] (size_t idx) const { return at(idx); }
    size_t size() const { return dictionary.size(); }
    bool empty() const { return dictionary.empty(); }

    size_t find(const std::string &str, size_t pos = 0);

    std::string tema(const unsigned int blockNum);

private:
    DictionaryDe dictionary;
    std::string fileName;
    std::string filePath;
    bool notLoaded = false;
    std::map<unsigned int, std::string> themes;
    size_t beginUserWort = 0;
};

#endif // GLOSSARYDE_H
