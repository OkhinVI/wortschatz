#include "statwords.h"
#include <fstream>
#include <iostream>
#include "string_utf8.h"


// class WordsStore

WordsStore::PosStringType WordsStore::addString(const std::string &str, uint8_t option)
{
    const auto currIdx = idxsStore.size();
    auto curOffset = stringStore.size();
    idxsStore.push_back(curOffset);
    optionsStore.push_back(option);
    const size_t prfSize = str.size() < 0xff ? sizeof(uint8_t) : sizeof(uint8_t) + sizeof(uint32_t);
    const auto currEntrySize = prfSize + str.size() + 1; // prefix + chars + '\0'
    stringStore.resize(stringStore.size() + currEntrySize);

    if (prfSize == sizeof(uint8_t))
    {
        stringStore[curOffset++] = str.size();
    } else {
        stringStore[curOffset++] = 0xff;
        const uint32_t sizeStr = str.size();
        stringStore[curOffset++] = (sizeStr) & 0xff;
        stringStore[curOffset++] = (sizeStr >> 8) & 0xff;
        stringStore[curOffset++] = (sizeStr >> 16) & 0xff;
        stringStore[curOffset++] = (sizeStr >> 24) & 0xff;
    }

    for (char sym : str)
        stringStore[curOffset++] = sym;

    stringStore[curOffset++] = '\0';

    return currIdx;
}

bool WordsStore::load(const std::string &fileName)
{
    clear();
    std::ifstream is;
    is.open(fileName, std::ios::binary);
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

void WordsStore::save(const std::string &fileName)
{
    std::ofstream os;
    os.open(fileName, std::ios::binary);
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

const char *WordsStore::get(const uint32_t idx, uint32_t &strSize, uint8_t &option) const
{
    if (idx >= idxsStore.size())
    {
        strSize = 0;
        option = 0;
        return nullptr;
    }
    option = optionsStore[idx];
    auto curOffset = idxsStore[idx];
    strSize = getStrSize(curOffset);
    return reinterpret_cast<const char *>(&stringStore[curOffset]);
}

uint32_t WordsStore::getStrSize(PosStringType &offset) const
{
    uint32_t strSize = stringStore[offset++];
    if (strSize == 0xff)
    {
        strSize = stringStore[offset + 3];
        strSize <<= 8;
        strSize |= stringStore[offset + 2];
        strSize <<= 8;
        strSize |= stringStore[offset + 1];
        strSize <<= 8;
        strSize |= stringStore[offset];
        offset += 4;
    }
    return strSize;
}

uint32_t WordsStore::findIdx(const std::string &str, uint32_t startIdx) const
{
    for (uint32_t idx = startIdx; idx < idxsStore.size(); ++idx)
    {
        PosStringType offset = idxsStore[idx];
        const auto currStrSize = getStrSize(offset);
        if (currStrSize != str.size())
            continue;
        if (str == reinterpret_cast<const char *>(&stringStore[offset]))
            return idx;
    }
    return endIdx();
}

uint32_t WordsStore::findFirstPartStrIdx(const std::string &str, uint32_t startIdx) const
{
    for (uint32_t idx = startIdx; idx < idxsStore.size(); ++idx)
    {
        PosStringType offset = idxsStore[idx];
        const auto currStrSize = getStrSize(offset);
        if (currStrSize < str.size())
            continue;
        const char *currCStr = reinterpret_cast<const char *>(&stringStore[offset]);
        if (str.compare(0, str.size(), currCStr, str.size()) == 0)
            return idx;
    }
    return endIdx();
}

// class DicWordIdx

DicWordIdx::DicWordIdx()
{
}

void DicWordIdx::exportFromFileIdx(const std::string &fileName)
{
    dicWortIdx.clear();
    dicWortIdx.addString("", 0); // position from 1
    std::ifstream is;
    is.open(fileName);
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
}

// class FormWordIdx

FormWordIdx::FormWordIdx()
{
}

void FormWordIdx::exportFromFileIdx(const std::string &fileName)
{
    formWortIdx.clear();
    formWortIdx.addString("", 0); // position from 1
    std::ifstream is;
    is.open(fileName);
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
        if (newIdx != idxWord) {
            std::cout << "Error: " << newIdx << " != " << idxWord << ", word = "
                << word << ", frenc = " << frenc << ", dicWordIdx = " << dicWordIdx
                << ", sourWordIdx = " << sourWordIdx << std::endl;
        }
    }
}

// class StatWords

StatWords::StatWords()
{

}
