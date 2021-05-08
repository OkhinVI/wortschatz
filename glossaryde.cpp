#include "wortde.h"
#include <string>
#include <fstream>
#include "glossaryde.h"
#include "linesramstream.h"

GlossaryDe::GlossaryDe()
{
    filePath = "D:\\Qt\\Prj\\LernenDeData\\";
    fileName = "dic_de.txt";
}

void GlossaryDe::load()
{
    LinesRamIStream ils;
    const bool isOpened = ils.loadFromFile(filePath + fileName);

    std::ofstream logDebug;
    logDebug.open(filePath + "LoadWort.log");
    std::ofstream logError;
    logError.open(filePath + "ErrorLoad.log");

    if (!isOpened)
    {
        logError << "Error open file: " << filePath + fileName << std::endl;
        return;
    }

    while(!ils.eof())
    {
        WortDe wd;
        if (wd.load(ils, logError))
        {
            wd.debugPrint(logDebug);
            dictionary.push_back(wd);
        }
    }
}
