#include "wortde.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "glossaryde.h"
#include "linesramstream.h"
#include "string_utf8.h"

static const std::string GlossaryDePrefix = "GlossaryDe:";
static WortDe NullWortDe;

GlossaryDe::GlossaryDe()
{
    filePath = "";
    fileName = "main_dic_de.dic";
}

void GlossaryDe::setPath(const std::string &aPath)
{
    filePath = (aPath.empty() || aPath.back() == '\\' || aPath.back() == '/') ? aPath : aPath + '/';
    notLoaded = false;
}

void GlossaryDe::setFile(const std::string &name)
{
    fileName = name;
    notLoaded = false;
}

std::string GlossaryDe::tema(const unsigned int blockNum)
{
    auto it = themes.find(blockNum);
    if (it == themes.end())
        return std::string();
    return it->second;
}

const GlossaryDe::Tema &GlossaryDe::getTemaByIndex(size_t idx) const
{
    static const Tema nullTema(0, "");
    return idx < themesVector.size() ? themesVector[idx] : nullTema;
}

void GlossaryDe::loadThemes(const std::string &fileName)
{
    themes.clear();
    std::ifstream is;
    is.open(fileName);

    std::string str;
    unsigned int blockNum = 0;
    while (!is.eof())
    {
        std::getline(is, str);
        if (str.empty())
            continue;
        AreaUtf8 ut8(str);
        const std::string blockNumStr = ut8.getToken(" ").toString();
        if (blockNumStr.empty() || blockNumStr.size() > 8)
            continue;
        try {
            blockNum = stoul(blockNumStr, nullptr, 16);
            themes[blockNum] = ut8.getRestArea().trim().toString();
        }  catch (...) {
            continue;
        }
    }
    is.close();

    themesVector.clear();
    for (auto it = themes.begin(); it != themes.end(); it++)
        themesVector.push_back(Tema(it->first, it->second));
}

void GlossaryDe::calcNextUserNumBlock()
{
    WortDe::BlockNumType maxUserNum = 0;
    for (size_t i = 0; i < dictionary.size(); ++i)
    {
        if (dictionary[i].block() > maxUserNum)
            maxUserNum = dictionary[i].block();
    }

    unsigned int h1, h2, h3, h4;
    WortDe::blockToUint_4(maxUserNum, h1, h2, h3, h4);
    if (h1 < uint32_t(WortDe::TypeLevel::User))
        nextUserNumBlock = WortDe::creatBlock(uint32_t(WortDe::TypeLevel::User), 1, 1, 1);
    else
        nextUserNumBlock = WortDe::preIncrementBlock(maxUserNum);
}

void GlossaryDe::load()
{
    notLoaded = false;
    LinesRamIStream ils;
    const bool isOpened = ils.loadFromFile(filePath + fileName);

    // TODO: add data and time into log
    std::ofstream logDebug;
    logDebug.open(filePath + "LoadWort.log"); // TODO: only for debug
    std::ofstream logError;
    logError.open(filePath + "ErrorLoad.log", std::ios_base::app); // TODO: open only if nead

    if (!isOpened)
    {
        logError << "Error open file: " << filePath + fileName << std::endl;
        notLoaded = true;
        return;
    }

    const std::string headGlossary = ils.get();

    // check head file
    std::istringstream ss(headGlossary);
    std::string prefix;
    ss >> prefix;
    if (prefix != GlossaryDePrefix)
    {
        logError << "Error: file \"" << filePath + fileName << "\" is bad: '" << prefix << "' != '" << GlossaryDePrefix << "'" << std::endl;
        notLoaded = true;
        return;
    }

    size_t loadedCountWd = 0;
    size_t loadedSizeWd = 0;
    size_t loadedBeginUserWort = 0;
    ss >> loadedCountWd >> loadedSizeWd >> loadedBeginUserWort;
    if (loadedSizeWd != WortDe().countSaveLines())
    {
        logError << "Warning: file \"" << filePath + fileName << "\" the number of lines for a Wort has changed: '"
                 << loadedSizeWd << "' != '" << WortDe().countSaveLines() << "'" << std::endl;
    }

    while(!ils.eof())
    {
        WortDe wd;
        if (wd.load(ils, logError))
        {
            wd.debugPrint(logDebug);
            add(wd);
        }
    }

    if (loadedCountWd != size())
    {
        logError << "Warning: file \"" << filePath + fileName << "\" count Wort changed: '"
                 << loadedCountWd << "' != '" << size() << "'" << std::endl;
    }

    if (loadedBeginUserWort > size())
    {
        logError << "Warning: file \"" << filePath + fileName << "\" begin user Wort exceeds the size of the dictionary: '"
                 << loadedBeginUserWort << "' > '" << size() << "'" << std::endl;
        loadedBeginUserWort = size();
    }
    beginUserWort = loadedBeginUserWort;
    calcNextUserNumBlock();

    loadThemes(filePath + "logTema.txt");
}

void GlossaryDe::save()
{
    if (notLoaded)
        return;
    std::ofstream os;
    os.open(filePath + fileName);
    os << GlossaryDePrefix << " " << dictionary.size() << " " << WortDe().countSaveLines() << " " << beginUserWort << std::endl;
    for (size_t i = 0; i < dictionary.size(); ++i)
        dictionary[i].save(os);
}

void GlossaryDe::saveClear()
{
    save();
    dictionary.clear();
    notLoaded = true;
}

void GlossaryDe::fixMainDic()
{
    beginUserWort = dictionary.size();
}

void GlossaryDe::add(const WortDe &wd)
{
    dictionary.push_back(wd);
    notLoaded = false;
}

const WortDe &GlossaryDe::at(size_t idx) const
{
    return idx < dictionary.size() ? dictionary[idx] : NullWortDe;
}

WortDe &GlossaryDe::at(size_t idx)
{
    return idx < dictionary.size() ? dictionary[idx] : NullWortDe;
}

size_t GlossaryDe::find(const std::string &str, size_t pos)
{
    if (str.empty())
        return 0;

    for (; pos < dictionary.size(); ++pos)
    {
        if (AreaUtf8(dictionary[pos].wort()).findCase(str))
            return pos;

        if (AreaUtf8(dictionary[pos].translation()).findCase(str))
            return pos;
    }
    return dictionary.size();
}

// class GlossaryDe::Tema

std::string GlossaryDe::Tema::asString() const
{
    unsigned int h1, h2, h3, h4;
    WortDe::blockToUint_4(blockNum, h1, h2, h3, h4);
    return WortDe::blockH1ToStr(h1) + "." +
            std::to_string(h2) + "  " +
            std::to_string(h3) + "." +
            std::to_string(h4) + " - " +
            blockStr;
}

// class GlossaryDe::WordTestSettings

GlossaryDe::SelectSettings::SelectSettings(const GlossaryDe &aGlDe)
    : lastIdxTema(aGlDe.themesSize() > 0 ? aGlDe.themesSize() - 1 : 0)
    , glDe(aGlDe)
{
}

bool GlossaryDe::SelectSettings::testWort(const WortDe &wd) const
{
    if (wd.block() < glDe.getTemaByIndex(startIdxTema).blockNum)
        return false;
    unsigned int last = glDe.getTemaByIndex(lastIdxTema).blockNum;
    if ((last & 0xff))
        last |= 0xff;
    return wd.block() <= last;
}
