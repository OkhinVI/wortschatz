#include "statwords.h"
#include <fstream>
#include <iostream>
#include "string_utf8.h"
#include <string.h>


// class String255

bool String255::operator==(const String255 &aStr) const
{
    if (size()!= aStr.size())
        return false;

    return strcmp(c_str(), aStr.c_str()) == 0;
}

bool String255::operator==(const char *aCStr) const
{
    return strcmp(c_str(), aCStr) == 0;
}

bool String255::operator==(const std::string &aStr) const
{
    if (size()!= aStr.size())
        return false;

    return aStr == c_str();
}

String255 String255::nextString() const
{
    return buf ? String255(buf + 1 + size() + 1) : *this;
}


// class String255Iterator

String255Iterator &String255Iterator::operator++()
{
    ++idx;
    str = str.nextString();
    return *this;
}


// class WordsStore

uint32_t WordsStore::addString(const std::string &str, uint8_t option)
{
    const uint8_t maxStringSize = 0xff - 1;
    const uint32_t currIdx = idxsStore.size();
    auto curOffset = stringStore.size();
    idxsStore.push_back(curOffset);
    optionsStore.push_back(option);
    const size_t prfSize = sizeof(uint8_t);
    size_t strSize = str.size();
    if (strSize > maxStringSize)
    {
        strSize = maxStringSize;
        std::cout << "warning: string size > " << maxStringSize << " = (" << str << "), str: '" << str << "'" << std::endl;
    }
    const auto currEntrySize = prfSize + strSize + 1; // prefix + chars + '\0'
    stringStore.resize(stringStore.size() + currEntrySize);
    stringStore[curOffset++] = strSize;

    for (size_t i = 0; i < strSize; ++i)
        stringStore[curOffset++] = str[i];

    stringStore[curOffset++] = '\0';

    return currIdx;
}

bool WordsStore::load(std::istream &is)
{
    clear();
    uint32_t countWord = 0;
    uint64_t stringStoreSize = 0;
    is.read(reinterpret_cast<char *>(&countWord), 4);
    is.read(reinterpret_cast<char *>(&stringStoreSize), 8);

    if (countWord >= 0xffffff || stringStoreSize > 0xffffffff)
        return false;
    idxsStore.reserve(countWord);
    optionsStore.reserve(countWord);
    stringStore.reserve(stringStoreSize + 8);

    stringStore.resize(stringStoreSize);
    is.read(reinterpret_cast<char *>(&stringStore[0]), stringStore.size());
    PosStringType offset = 0;
    for (uint32_t idx = 0; idx < countWord; ++idx)
    {
        idxsStore.push_back(offset);
        uint32_t strSize = getStrSize(offset);
        if (offset + strSize + 1 > stringStore.size())
            return false;

        offset += strSize;
        optionsStore.push_back(stringStore[offset]);
        stringStore[offset++] = 0;
    }

    return offset == stringStore.size();
}

void WordsStore::save(std::ofstream &os)
{
    const uint32_t countWord = idxsStore.size();
    uint64_t stringStoreSize = 0;
    for (PosStringType offset : idxsStore)
    {
        const auto startOffset = offset;
        stringStoreSize += getStrSize(offset);
        stringStoreSize += offset - startOffset + 1;
    }
    os.write(reinterpret_cast<const char *>(&countWord), 4);
    os.write(reinterpret_cast<const char *>(&stringStoreSize), 8);
    for (uint32_t idx = 0; idx < idxsStore.size(); ++idx)
    {
        auto offset = idxsStore[idx];
        const auto startOffset = offset;
        const auto strSize = getStrSize(offset);
        const auto prefSize = offset - startOffset;
        os.write(reinterpret_cast<const char *>(&stringStore[startOffset]), prefSize + strSize);
        os.write(reinterpret_cast<const char *>(&optionsStore[idx]), 1);
    }
}

String255 WordsStore::get(const uint32_t idx, uint8_t &option) const
{
    if (idx >= idxsStore.size())
    {
        option = 0;
        return String255(nullptr);
    }
    option = optionsStore[idx];
    return String255(&stringStore[idxsStore[idx]]);
}

String255Iterator WordsStore::findIdx(const std::string &str, uint32_t startIdx) const
{
    for (uint32_t idx = startIdx; idx < idxsStore.size(); ++idx)
    {
        if (String255(&stringStore[idxsStore[idx]]) == str)
            return getIterator(idx);
    }
    return String255Iterator(String255(nullptr), 0);
}

String255Iterator WordsStore::findFirstPartStrIdx(const std::string &str, uint32_t startIdx) const
{
    for (uint32_t idx = startIdx; idx < idxsStore.size(); ++idx)
    {
        String255 currStr(&stringStore[idxsStore[idx]]);
        const char * const c_str = currStr.c_str();
        const size_t len = str.size();
        if (currStr.size() >= str.size()
            && str.compare(0, len, c_str, len) == 0)
            return getIterator(idx);
    }
    return String255Iterator(String255(nullptr), 0);
}

String255Iterator WordsStore::getIterator(size_t idx) const
{
    return (idx < idxsStore.size())
            ? String255Iterator(String255(&stringStore[idxsStore[idx]]), idx)
            : String255Iterator(String255(nullptr), 0);
}


// class VectorUint24

bool VectorUint24::save(std::ostream &os)
{
    const uint32_t countItem = size();
    const size_t countByte = countItem * size_ofItem;
    os.write(reinterpret_cast<const char *>(&countItem), 4);
    os.write(reinterpret_cast<const char *>(&rawVector[0]), countByte);
    return true;
}

bool VectorUint24::load(std::istream &is)
{
    rawVector.clear();
    uint32_t countItem = 0;
    is.read(reinterpret_cast<char *>(&countItem), 4);
    if (countItem >= 0xffffff || countItem == 0)
        return false;

    const size_t countByte = countItem * size_ofItem;
    rawVector.resize(countByte);
    is.read(reinterpret_cast<char *>(&rawVector[0]), rawVector.size());
    if (is.fail())
    {
        rawVector.clear();
        return false;
    }
    return true;
}


// class DicWordIdx

DicWordIdx::DicWordIdx()
{
}

bool DicWordIdx::load(const std::string &path)
{
    changed = false;
    fileName = path + "dwi.dat";
    std::ifstream is;
    is.open(fileName, std::ios::binary);
    const bool result = dicWortIdx.load(is);
    if (!result)
        dicWortIdx.clear();
    return result;
}

bool DicWordIdx::save()
{
    if (!changed || fileName.empty())
        return false;

    std::ofstream os;
    os.open(fileName, std::ios::binary);
    dicWortIdx.save(os);
    return true;
}


void DicWordIdx::exportFromFileIdx(const std::string &fileName)
{
    changed = false;
    dicWortIdx.clear();
    dicWortIdx.addString("", 0); // position from 1
    std::ifstream is;
    is.open(fileName);
    if (!is.good())
        return;
    std::string line;
    while(!is.eof())
    {
        std::getline(is, line);
        if (line.empty())
            continue;
        AreaUtf8 au8(line);
        std::string word = au8.getToken("\t").toString();
        au8.getToken("\t");
        uint32_t idxWord = std::stoul(au8.getToken("\t").toString());
        au8.getToken("\t");
        uint32_t frenc = std::stoul(au8.getToken("\t").toString());
        au8.getToken("\t");
        uint32_t typeNum = std::stoul(au8.getToken("\t").getToken(" ").toString());
        uint32_t newIdx = dicWortIdx.addString(word, typeNum);
        if (newIdx != idxWord)
            std::cout << "Error: " << newIdx << " != " << idxWord << ", word = " << word << ", frenc = " << frenc << std::endl;
    }
    if (dicWortIdx.size() > 1)
        changed = true;
}

String255Iterator DicWordIdx::findStrIdx(const std::string &str, size_t pos, bool firstPart, uint8_t &option)
{
    String255Iterator it = firstPart ? dicWortIdx.findFirstPartStrIdx(str, pos) : dicWortIdx.findIdx(str, pos);
    if (it->valid())
        option = dicWortIdx.getOption(it.getIdx());
    return it;
}


// class FormWordIdx

FormWordIdx::FormWordIdx()
{
}

bool FormWordIdx::load(const std::string &path)
{
    changed = false;
    fileName = path + "fwi.dat";
    std::ifstream is;
    is.open(fileName, std::ios::binary);
    if (!formWortIdx.load(is) || !idx24Dics.load(is))
    {
        formWortIdx.clear();
        idx24Dics.clear();
        return false;
    }

    return true;
}

bool FormWordIdx::save()
{
    if (!changed || fileName.empty())
        return false;

    std::ofstream os;
    os.open(fileName, std::ios::binary);
    formWortIdx.save(os);
    idx24Dics.save(os);
    return true;
}

void FormWordIdx::exportFromFileIdx(const std::string &fileName)
{
    changed = false;
    formWortIdx.clear();
    formWortIdx.addString("", 0); // position from 1
    idx24Dics.push_back(0);

    std::ifstream is;
    is.open(fileName);
    if (!is.good())
        return;
    std::string line;
    while(!is.eof())
    {
        std::getline(is, line);
        if (line.empty())
            continue;
        AreaUtf8 au8(line);
        std::string word = au8.getToken("\t").toString();
        au8.getToken("\t");
        uint32_t idxWord = std::stoul(au8.getToken("\t").toString());
        au8.getToken("\t");
        uint32_t typeNum = std::stoul(au8.getToken("\t").getToken(" ").toString());
        au8.getToken("\t");
        uint32_t dicWordIdx = std::stoul(au8.getToken("\t").getToken(" ").toString());
        au8.getToken("\t");
        uint32_t sourWordIdx = std::stoul(au8.getToken("\t").getToken(" ").toString());
        au8.getToken("\t");
        uint32_t frenc = std::stoul(au8.getToken("\t").toString());
        au8.getToken("\t");

        uint32_t newIdx = formWortIdx.addString(word, typeNum);
        idx24Dics.push_back(dicWordIdx);
        if (newIdx != idxWord) {
            std::cout << "Error: " << newIdx << " != " << idxWord << ", word = "
                << word << ", frenc = " << frenc << ", dicWordIdx = " << dicWordIdx
                << ", sourWordIdx = " << sourWordIdx << std::endl;
        }
        if (idx24Dics.size() - 1 != idxWord) {
            std::cout << "Error dicWordIdx: " << idxWord << " != " << idx24Dics.size() - 1 << ", word = "
                << word << ", frenc = " << frenc << ", dicWordIdx = " << dicWordIdx
                << ", sourWordIdx = " << sourWordIdx << std::endl;
        }
    }
    if (formWortIdx.size() > 1)
        changed = true;
}

String255Iterator FormWordIdx::findStrIdx(const std::string &str, size_t pos, bool firstPart, uint8_t &option, uint32_t &idxDic)
{
    String255Iterator it = firstPart ? formWortIdx.findFirstPartStrIdx(str, pos) : formWortIdx.findIdx(str, pos);
    if (it->valid()) {
        option = formWortIdx.getOption(it.getIdx());
        idxDic = idx24Dics[it.getIdx()];
    }
    return it;
}


// class StatWords

StatWords::StatWords()
{

}

bool StatWords::load(const std::string &path)
{
    if (!dicWorts.load(path))
        return false;
    formWords.load(path);

    return true;
}

String255Iterator StatWords::findDicStrIdx(const std::string &str, size_t pos, bool firstPart, uint8_t &option)
{
    return dicWorts.findStrIdx(str, pos, firstPart, option);
}

String255Iterator StatWords::findFormStrIdx(const std::string &str, size_t pos, bool firstPart, uint8_t &option, uint32_t &idxDic)
{
    return formWords.findStrIdx(str, pos, firstPart, option, idxDic);
}
