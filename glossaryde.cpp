#include "wortde.h"
#include <string>
#include <fstream>
#include "glossaryde.h"
#include "linesramstream.h"
#include "string_utf8.h"

static const std::string GlossaryDePrefix = "GlossaryDe:";

GlossaryDe::GlossaryDe()
{
    filePath = "";
    fileName = "main_dic_de.dic";
}

void GlossaryDe::setPath(const std::string &aPath)
{
    filePath = (aPath.empty() || aPath.back() == '\\') ? aPath : aPath + '\\';
    notLoaded = false;
}

void GlossaryDe::setFile(const std::string &name)
{
    fileName = name;
    notLoaded = false;
}

void GlossaryDe::load()
{
    notLoaded = false;
    LinesRamIStream ils;
    const bool isOpened = ils.loadFromFile(filePath + fileName);

    std::ofstream logDebug;
    logDebug.open(filePath + "LoadWort.log"); // TODO: only for debug
    std::ofstream logError;
    logError.open(filePath + "ErrorLoad.log"); // Open only if nead

    if (!isOpened)
    {
        logError << "Error open file: " << filePath + fileName << std::endl;
        notLoaded = true;
        return;
    }

    const std::string headGlossary = ils.get();

    // check head file
    AreaUtf8 au8(headGlossary);
    bool isDelimeter;
    const std::string prefix = au8.getToken(isDelimeter).toString();
    if (prefix != GlossaryDePrefix)
    {
        logError << "Error: file \"" << filePath + fileName << "\" is bad: '" << prefix << "' != '" << GlossaryDePrefix << "'" << std::endl;
        notLoaded = true;
        return;
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
}

void GlossaryDe::save()
{
    if (notLoaded)
        return;
    std::ofstream os;
    os.open(filePath + fileName);
    WortDe wd;
    os <<GlossaryDePrefix << " " << dictionary.size() << " " << wd.countSaveLines() << std::endl;
    for (size_t i = 0; i < dictionary.size(); ++i)
        dictionary[i].save(os);
}

void GlossaryDe::saveClear()
{
    save();
    dictionary.clear();
    notLoaded = true;
}

void GlossaryDe::add(const WortDe &wd)
{
    dictionary.push_back(wd);
    notLoaded = false;
}

const WortDe &GlossaryDe::at(size_t idx) const
{
    const static WortDe nullWortDe;
    return idx < dictionary.size() ? dictionary[idx] : nullWortDe;
}

WortDe &GlossaryDe::at(size_t idx)
{
    static WortDe nullWortDe;
    return idx < dictionary.size() ? dictionary[idx] : nullWortDe;
}
