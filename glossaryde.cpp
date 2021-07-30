#include "wortde.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "glossaryde.h"
#include "linesramstream.h"
#include "string_utf8.h"
#include "utility.h"

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

static const std::string GlossaryDePrefix = "GlossaryDe:";
static WortDe NullWortDe;

GlossaryDe::GlossaryDe()
{
    filePath = "";
    fileName = "main_dic_de.dic";

    genRandom.seed(time(nullptr) + __rdtsc());
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

    unsigned int blockNum = 0;
    while (is.good())
    {
        std::string str = util::getline(is);
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

void GlossaryDe::load(const bool saveDbg)
{
    notLoaded = false;
    LinesRamIStream ils;
    const bool isOpened = ils.loadFromFile(filePath + fileName);

    // TODO: add data and time into log
    std::ofstream logDebug;
    if(saveDbg) logDebug.open(filePath + "LoadWort.log"); // TODO: only for debug
    std::ofstream logError;
    if(saveDbg) logError.open(filePath + "ErrorLoad.log", std::ios_base::app); // TODO: open only if nead

    if (!isOpened)
    {
        if(saveDbg) logError << "Error open file: " << filePath + fileName << std::endl;
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
        if(saveDbg) logError << "Error: file \"" << filePath + fileName << "\" is bad: '" << prefix << "' != '" << GlossaryDePrefix << "'" << std::endl;
        notLoaded = true;
        return;
    }

    size_t loadedCountWd = 0;
    size_t loadedSizeWd = 0;
    size_t loadedBeginUserWort = 0;
    ss >> loadedCountWd >> loadedSizeWd >> loadedBeginUserWort;
    if (loadedSizeWd != WortDe().countSaveLines())
    {
        if(saveDbg) logError << "Warning: file \"" << filePath + fileName << "\" the number of lines for a Wort has changed: '"
                 << loadedSizeWd << "' != '" << WortDe().countSaveLines() << "'" << std::endl;
    }

    while(!ils.eof())
    {
        WortDe wd;
        if (wd.load(ils, saveDbg ? &logError : nullptr))
        {
            // if (wd.wort().substr(0, 3) != "---") { // delete Wort with "---" prefix
            if(saveDbg) wd.debugPrint(logDebug);
            add(wd);
            // }
        }
    }

    if (loadedCountWd != size())
    {
        if(saveDbg) logError << "Warning: file \"" << filePath + fileName << "\" count Wort changed: '"
                 << loadedCountWd << "' != '" << size() << "'" << std::endl;
    }

    if (loadedBeginUserWort > size())
    {
        if(saveDbg) logError << "Warning: file \"" << filePath + fileName << "\" begin user Wort exceeds the size of the dictionary: '"
                 << loadedBeginUserWort << "' > '" << size() << "'" << std::endl;
        loadedBeginUserWort = size();
    }
    beginUserWort = loadedBeginUserWort;
    calcNextUserNumBlock();

    loadThemes(filePath + "logTema.txt");
    statWords.load(filePath);
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

void GlossaryDe::clearAllStatistic()
{
    for (WortDe &wd : dictionary)
        wd.clearStatistic();
}

void GlossaryDe::add(const WortDe &wd)
{
    dictionary.push_back(wd);
    notLoaded = false;
}

void GlossaryDe::insert(size_t idx, const WortDe &wd)
{
    if (idx < dictionary.size())
        dictionary.insert(dictionary.begin() + idx, wd);
    else
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

size_t GlossaryDe::findByWordIdx(const uint32_t wordIdx, size_t pos)
{
    for (; pos < dictionary.size(); ++pos)
    {
        if (dictionary[pos].freqIdx() == wordIdx)
            return pos;
    }
    return dictionary.size();
}

String255Iterator GlossaryDe::findStatDic(const std::string &str, size_t pos, uint8_t &option)
{
    return statWords.findDicStrIdx(str, pos, true, option);
}

size_t GlossaryDe::calcTestWortIdx(const SelectSettings &selSet)
{
    std::vector<size_t> selectionIdxs;
    selectIdxFilter([](const WortDe &de, size_t) { return !de.translation().empty(); }, selectionIdxs, selSet);
    if (selectionIdxs.empty())
    {
        currIdxLearnWordDe = dictionary.size();
        return dictionary.size();
    }

    // in selSet.posIgnoringStatistics % of cases, we ignore the frequency of correct answers.
    bool useStatistic = true;
    if (selSet.posIgnoringStatistics >= 100)
        useStatistic = false;
    else if (selSet.posIgnoringStatistics > 0)
        useStatistic = (genRandom() % 100) >= selSet.posIgnoringStatistics;

    if (useStatistic)
    {
        // select words with minimum correct answers
        uint32_t minCorrectAnswers = std::numeric_limits<uint32_t>::max();
        uint32_t countMinCorrectAnswers = 0;
        for (size_t i = 0; i < selectionIdxs.size(); ++i)
        {
            const LearningWort &lw = dictionary[selectionIdxs[i]].getStatistic();
            if (minCorrectAnswers > lw.numberCorrectAnswers)
            {
                minCorrectAnswers = lw.numberCorrectAnswers;
                selectionIdxs[0] = selectionIdxs[i];
                countMinCorrectAnswers = 1;
            } else if (minCorrectAnswers == lw.numberCorrectAnswers)
            {
                selectionIdxs[countMinCorrectAnswers++] = selectionIdxs[i];
            }
        }
        if (countMinCorrectAnswers > 0)
            selectionIdxs.resize(countMinCorrectAnswers);
    }

    uint64_t taktCPU = __rdtsc();
    uint64_t randNum = genRandom();
    size_t currTestIdxWithTr = (taktCPU + randNum) % selectionIdxs.size();
    currIdxLearnWordDe = selectionIdxs[currTestIdxWithTr];
    return currIdxLearnWordDe;
}

int GlossaryDe::selectVariantsTr(std::vector<size_t> &vecIdxTr)
{
    if (currIdxLearnWordDe >= dictionary.size())
        return -1;

    std::vector<size_t> selectionIdxs;
    const WortDe &wd = dictionary[currIdxLearnWordDe];

    SelectSettings trSelSet(*this);
    const size_t currIdx = currIdxLearnWordDe;
    selectIdxFilter([wd, currIdx](const WortDe &aWd, size_t idx)
        { return !aWd.translation().empty()
                  && (wd.type() == aWd.type())
                  && idx != currIdx
                  && wd.translation() != aWd.translation()
        ; },
        selectionIdxs, trSelSet);

    if (selectionIdxs.size() < vecIdxTr.size())
    {
        selectIdxFilter([wd, currIdx](const WortDe &aWd, size_t idx)
            { return !aWd.translation().empty()
                      && idx != currIdx
            ; },
            selectionIdxs, trSelSet);
    }

    if (selectionIdxs.size() < vecIdxTr.size())
        return -1;

    int currIdxCorrectTr = genRandom() % vecIdxTr.size();

    for (size_t i = 0; i < vecIdxTr.size(); ++i)
    {
        if (currIdxCorrectTr == int(i))
        {
            vecIdxTr[i] = currIdxLearnWordDe;
        } else {
            size_t nextIdx = genRandom() % (selectionIdxs.size() - 1);
            vecIdxTr[i] = selectionIdxs[nextIdx];
            if (nextIdx + 1 < selectionIdxs.size())
                selectionIdxs[nextIdx] = selectionIdxs.back();
            selectionIdxs.pop_back();
        }
    }
    return currIdxCorrectTr;
}

void GlossaryDe::importTr(const GlossaryDe &impGloss)
{
    std::ofstream osLog;
    osLog.open(filePath + "logImport.txt");

    std::vector<std::string> impWort; // first word from import WordDe.
    std::vector<uint32_t> idxImpWort;
    std::vector<uint32_t> outIdxImpWort;
    impWort.reserve(impGloss.dictionary.size());
    idxImpWort.reserve(impGloss.dictionary.size());
    outIdxImpWort.reserve(impGloss.dictionary.size());

    for (size_t i = 0; i < impGloss.dictionary.size(); ++i)
    {
        const WortDe &impWd = impGloss.dictionary[i];
        if (!impWd.translation().empty())
        {
            idxImpWort.push_back(i);
            std::string impWord = impWd.type() == WortDe::TypeWort::Combination
                    ? impWd.wort()
                    : AreaUtf8(impWd.wort()).getToken().toString();
            impWort.push_back(impWord);
        } else
            impWort.push_back(std::string());
    }

    for (size_t idx = 0; idx < dictionary.size(); ++idx)
    {
        WortDe &wd = dictionary[idx];
        if (!wd.translation().empty())
            continue;
        for (size_t i = 0; i < idxImpWort.size(); ++i)
        {
            const auto idxInImpDic = idxImpWort[i];
            const WortDe &impWd = impGloss.dictionary[idxInImpDic];
            if (wd.wort() != impWort[idxInImpDic])
                continue;

            if (wd.type() == WortDe::TypeWort::None)
                wd.setNewTypeWort(impWd.type());

            if (wd.type() != impWd.type())
            {
                osLog << "Skip:\t'" << wd.wort() << "', type " << WortDe::TypeWortToString(wd.type())
                      << " != " <<  WortDe::TypeWortToString(impWd.type()) << "\t" << impWd.translation() << "\n";
                continue;
            }

            wd.setNewTranslation(impWd.translation());
            wd.setNewWort(impWd.wort());
            if (wd.rawPrefix().empty())
                wd.setNewPrefix(impWd.rawPrefix());
            if (wd.example().empty())
                wd.setNewExample(impWd.example());

            if (wd.block() == 0 && impWd.block() != 0)
            {
                wd.setNewBlock(impWd.block());
                auto it = themes.find(wd.block());
                if (it == themes.end())
                {
                    auto itImp = impGloss.themes.find(wd.block());
                    if (itImp != impGloss.themes.end())
                    {
                        themes[wd.block()] = itImp->second;
                        osLog << "Add tema: " << std::hex << wd.block()
                              << std::dec << "\t'" << itImp->second << "'\n";
                    }
                }
            }

            if (wd.type() == WortDe::TypeWort::Noun)
            {
                if (wd.wortPl().empty())
                    wd.setNewPlural(impWd.wortPl());
                if (wd.artikel() == WortDe::TypeArtikel::None)
                    wd.setNewArtikel(impWd.artikel());
            } else if (wd.type() == WortDe::TypeWort::Verb)
            {
                if (wd.vPerfect().empty() && wd.vPrasens3f().empty() && wd.vPrateritum().empty())
                {
                    wd.setNewPerfect(impWd.vPerfect());
                    wd.setNewPrasens3f(impWd.vPrasens3f());
                    wd.setNewPrateritum(impWd.vPrateritum());
                }
                if (!wd.hasSich())
                    wd.setNewSich(impWd.hasSich());
            }

            outIdxImpWort.push_back(idxImpWort[i]);
            idxImpWort.erase(idxImpWort.begin() + i);
            break;
        }
    }

    osLog << "\n\n";
    osLog << "Import:\t" << outIdxImpWort.size() << "\n";
    osLog << "Ignore:\t" << idxImpWort.size() << "\n";

    std::ofstream osIgn;
    osIgn.open(filePath + "IgnoreImport.dic");
    osIgn << GlossaryDePrefix << " " << idxImpWort.size() << " " << WortDe().countSaveLines() << " " << 0 << std::endl;
    for (size_t i = 0; i < idxImpWort.size(); ++i)
    {
        const auto idxInImpDic = idxImpWort[i];
        WortDe impWd = impGloss.dictionary[idxInImpDic];
        impWd.save(osIgn);
        const std::string &currFindWord = impWort[idxInImpDic];
        if (impWd.type() != WortDe::TypeWort::Combination && impWd.type() != WortDe::TypeWort::None)
        {
            bool finded = false;
            for (size_t idxFind = 0; idxFind < outIdxImpWort.size(); ++idxFind)
            {
                auto idxInOutImpDic = outIdxImpWort[idxFind];
                if (currFindWord == impWort[idxInOutImpDic])
                {
                    finded = true;
                    break;
                }
            }
            if (!finded)
                osLog << impWd.wort() << "\t" << impWd.translation() << "\t" << "\n";
        }
    }
}

void GlossaryDe::importStat(const GlossaryDe &impGloss)
{
    std::vector<uint32_t> idxImpWort;
    std::vector<uint32_t> outIdxImpWort;
    idxImpWort.reserve(impGloss.dictionary.size());
    outIdxImpWort.reserve(impGloss.dictionary.size());

    for (size_t i = 0; i < impGloss.dictionary.size(); ++i)
    {
        const WortDe &impWd = impGloss.dictionary[i];
        if (!impWd.translation().empty() && impWd.getStatistic().startLearning != 0)
            idxImpWort.push_back(i);
    }

    for (size_t idx = 0; idx < dictionary.size(); ++idx)
    {
        WortDe &wd = dictionary[idx];
        if (wd.translation().empty())
            continue;
        for (size_t i = 0; i < idxImpWort.size(); ++i)
        {
            const auto idxInImpDic = idxImpWort[i];
            const WortDe &impWd = impGloss.dictionary[idxInImpDic];

            if (wd.wort() != impWd.wort() || wd.type() != impWd.type())
                continue;

            wd.copyStatistic(impWd);

            outIdxImpWort.push_back(idxImpWort[i]);
            idxImpWort.erase(idxImpWort.begin() + i);
            break;
        }
    }
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
    if (useRangeTema)
    {
        if (wd.block() < glDe.getTemaByIndex(startIdxTema).blockNum)
            return false;
        unsigned int last = glDe.getTemaByIndex(lastIdxTema).blockNum;
        if ((last & 0xff) == 0)
            last |= 0xff;
        if (wd.block() > last)
            return false;
    }

    if (useRangeFreq && (wd.freqIdx() < startFreqIdx || wd.freqIdx() > endFreqIdx))
        return false;

    return true;
}
