#include "soundofwords.h"
#include <QMediaPlayer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QFile>
#include <iostream>
#include "string_utf8.h"
#include "utility.h"
#include "wortde.h"

// #include <fstream>
// std::ofstream debStream("D://Temp/log001.txt");
// #define debPrint(a) debStream << a << std::endl

#define debPrint(a) std::cout << a << std::endl
//#define debPrint(a)

SoundOfWords::SoundOfWords(const std::string &aPathDic, QObject* pobj) : QObject(pobj), pathDic(aPathDic)
{
    m_pnam = new QNetworkAccessManager(this);
    connect(m_pnam, SIGNAL(finished(QNetworkReply*)),
            this,   SLOT(slotFinished(QNetworkReply*))
           );
    vokalWort = new QMediaPlayer;
}

void SoundOfWords::play(const std::string &word)
{
    AreaUtf8 wordAr = AreaUtf8(word).getToken();
    if (wordAr.empty())
        return;

    const std::string fileName = getFileNameMp3(wordAr.toString());
    QFile file(QString::fromStdString(fileName));
    if (file.exists())
    {
        debPrint("play local file: " << fileName);
        vokalWort->setMedia(QUrl(QString::fromStdString(fileName)));
        vokalWort->play();
    }
    else
    {
        startSoundReq(wordAr.toString());
    }
}

void SoundOfWords::startSoundReq(const std::string &word)
{
    const std::string urlRaw = "https://de.thefreedictionary.com/" + word;
    QUrl url(QString::fromStdString(urlRaw));
    QNetworkRequest request(url);
    freedictionaryReq = m_pnam->get(request);
    freedictionaryWord = word;
    debPrint("Start sound request: " << urlRaw);

//    connect(pnr, SIGNAL(downloadProgress(qint64, qint64)),
//            this, SIGNAL(downloadProgress(qint64, qint64))
//           );
}

void SoundOfWords::infoWoerter(const std::string &word)
{
    AreaUtf8 wordAr = AreaUtf8(word).getToken();
    if (wordAr.empty())
        return;

    const std::string fileName = getFileNameWoerter(wordAr.toString());
    QFile file(QString::fromStdString(fileName));
    if (file.exists())
    {
        debPrint("info local file: " << fileName);
        if (file.open(QIODevice::ReadOnly))
        {
            QByteArray data = file.readAll();
            printWoerter(wordAr.toString(), data.toStdString());
            file.close();
        }
    }
    else
    {
        const std::string urlRaw = "https://www.woerter.ru/?w=" + wordAr.toString();
        QUrl url(QString::fromStdString(urlRaw));
        QNetworkRequest request(url);
        woerterReq = m_pnam->get(request);
        woerterReqWord = wordAr.toString();
        debPrint("Start Woerter request: " << urlRaw);
    }
}

SoundOfWords::TypeReply SoundOfWords::calcNetworkReply(QNetworkReply* pnr)
{
    if (pnr == freedictionaryReq)
    {
        freedictionaryReq = nullptr;
        return TypeReply::FreeDic;
    }
    else if (pnr == freedictionaryReqMp3)
    {
        freedictionaryReqMp3 = nullptr;
        return TypeReply::FreeDicMp3;
    }
    else if (pnr == woerterReq)
    {
        woerterReq = nullptr;
        return TypeReply::Woerter;
    }

    debPrint("Error QNetworkReply: " << pnr << " != " << pnr);
    return TypeReply::None;
}

void SoundOfWords::slotFinished(QNetworkReply* pnr)
{
    const TypeReply typeRep = calcNetworkReply(pnr);

    if (pnr->error() != QNetworkReply::NoError) {
        // emit error();
        debPrint("Error (" << int(typeRep) << "): " << pnr->url().toString().toStdString());
        pnr->deleteLater();
        return;
    }

    QByteArray body = pnr->readAll();
    QString url = pnr->url().toString();
    pnr->deleteLater();

    // emit done(pnr->url(), pnr->readAll());
    switch (typeRep)
    {
    case TypeReply::FreeDic:
        parseFreeDic(url, body);
        break;
    case TypeReply::FreeDicMp3:
        parseFreeDicMp3(url, body);
        break;
    case TypeReply::Woerter:
        parseWoerter(url, body);
        break;
    case TypeReply::None:
    case TypeReply::_last_one:
        break;
    }
}

static std::string getHtmlTag(const std::string &str, const std::string &beginStr, const std::string &endStr, size_t numer = 0)
{
    auto pos1 = str.find(beginStr);
    while (numer > 0 && pos1 != std::string::npos)
    {
        pos1 = str.find(beginStr, pos1 + beginStr.size());
        --numer;
    }
    if (pos1 == std::string::npos)
        return std::string();

    pos1+= beginStr.size();
    pos1 = str.find(">", pos1);
    if (pos1 == std::string::npos)
        return std::string();

    pos1 += 1;
    auto pos2 = !endStr.empty() ? str.find(endStr, pos1) : str.size();
    if (pos2 == std::string::npos)
        return std::string();

    std::string newStr = AreaUtf8(str, pos1, pos2 - pos1).trim().toString();
    util::replaceEndLines(newStr);
    return newStr;
}

void addNameTag(std::string &str, const std::string &name, const std::string data)
{
    static const std::string startStr = "<p class=\"";
    static const std::string endStr = "</p>";
    str = str + startStr + name + "\"\t>" + data + endStr + '\n';
}

std::string getNameTag(const std::string &str, const std::string &name)
{
    static const std::string startStr = "<p class=\"";
    static const std::string endStr = "</p>";
    return getHtmlTag(str, startStr + name + "\"", endStr);
}

void SoundOfWords::parseFreeDic(QString &url, QByteArray &bodyAr)
{
    std::string body = bodyAr.toStdString();
    const std::string findString = "data-snd=\"";
    auto pos1 = body.find(findString);
    if (pos1 == std::string::npos)
    {
        debPrint("data-snd not found: " << url.toStdString());
    } else {
        pos1 += findString.size();
        auto pos2 = body.find("\"", pos1);
        const std::string urlMp3 = "https://img2.tfd.com/pron/mp3/" + body.substr(pos1, pos2 - pos1) + ".mp3";

        debPrint("download: " << urlMp3);
        QUrl url(QString::fromStdString(urlMp3));
        QNetworkRequest request(url);
        freedictionaryReqMp3 = m_pnam->get(request);
        freedictionaryMp3FileName = getFileNameMp3(freedictionaryWord);
    }
}

void SoundOfWords::parseFreeDicMp3(QString &, QByteArray &bodyAr)
{
    debPrint("save file: " << freedictionaryMp3FileName);
    QFile file(QString::fromStdString(freedictionaryMp3FileName));
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(bodyAr);
        file.close();
        vokalWort->setMedia(QUrl(QString::fromStdString(freedictionaryMp3FileName)));
        vokalWort->play();
    }
}

void SoundOfWords::parseWoerter(QString &url, QByteArray &bodyAr)
{
    {
        QFile file(QString::fromStdString("D://Temp/WoerterCache/" + woerterReqWord + ".html"));
        debPrint("getted: " << url.toStdString());
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(bodyAr);
            file.close();
        }
    }

    std::string body = bodyAr.toStdString();

    WortDe::TypeWort tw = WortDe::TypeWort::None;
    std::string paragraf = getHtmlTag(body, "<h1 class=\"rClear\"", "</p>");
    std::string typeWord;
    if (!(typeWord = getHtmlTag(paragraf, "<a href=\"/glagoly/", "</a>")).empty())
        tw = WortDe::TypeWort::Verb;
    else if (!(typeWord = getHtmlTag(paragraf, "<a href=\"/sushhestvitelnye/", "</a>")).empty())
        tw = WortDe::TypeWort::Noun;
    AreaUtf8 typeWordAr(typeWord);
    typeWordAr.getToken();typeWordAr.getToken();
    typeWordAr.getToken();typeWordAr.getToken();
    AreaUtf8 webWord = typeWordAr.getToken();
    if (webWord.toString() != woerterReqWord)
    {
        return; // Falsches Wort
    }

    paragraf = getHtmlTag(body, "<span class=\"rInf\"", "<p class=");
    std::string plural;
    if (tw == WortDe::TypeWort::Noun)
    {
        plural = getHtmlTag(paragraf, "<span title=\"родительный", "</span>");
        if (plural.empty())
            plural = getHtmlTag(paragraf, "<span title=\"без окончания", "</span>");
        if (plural.empty())
            plural = getHtmlTag(paragraf, "<span title=\"", "</span>", 2);
    }
    std::string level = getHtmlTag(body, "<span class=\"bZrt\"", "</span>");
    std::string wortArt =  getHtmlTag(body, "<p class=\"rCntr rClear\"", "</p>");
    std::string wort = getHtmlTag(wortArt, "<span class=\"bHf\"", "</span>");
    wort = getHtmlTag(wort, "<q", "</q>");
    std::string art = tw == WortDe::TypeWort::Noun ? getHtmlTag(wortArt, "<span title=\"", "</span>") : "";
    std::string translation = getHtmlTag(body, "<span lang=\"ru\"", "</span>");
    translation = getHtmlTag(translation, "<img ", "");
    const std::string nbsp = "&nbsp;";
    if (translation.substr(0, nbsp.size()) == nbsp)
        translation = translation.substr(nbsp.size());
    std::string opt0 = getHtmlTag(body, "<p class=\"rInf r1Zeile rU3px rO0px\"", "</p>", 0);
    opt0 = getHtmlTag(opt0, "<i", "</i>");
    std::string opt1 = getHtmlTag(body, "<p class=\"rInf r1Zeile rU3px rO0px\"", "</p>", 1);
    if (opt1.size() < 2 || opt1[0] != '(' || opt1[opt1.size() - 1] != ')')
        opt1.clear();
    std::string opt2 = getHtmlTag(body, "<p class=\"rInf r1Zeile rU3px rO0px\" onclick=", "</p>");

    std::string result;
    static const std::string colorDer = "<span style=\" color:#0000A0;\">";
    static const std::string colorDie = "<span style=\" color:#A00000;\">";
    static const std::string colorDas = "<span style=\" color:#00A000;\">";
    static const std::string colorDef = "<span style=\" color:#000000;\">";
    static const std::string colorGray = "<span style=\" color:#808080;\">";
    static const std::string endColor = "</span>";
    static const std::string startP = "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
    static const std::string endP = "</p>";
    if (tw == WortDe::TypeWort::Noun)
    {
        std::string colorArt;
        if (art == "der")
            colorArt = colorDer;
        else if (art == "die")
            colorArt = colorDie;
        else if (art == "das")
            colorArt = colorDas;
        else
            colorArt = colorDef;

        result = startP + colorGray + level + ": " + endColor + colorArt + art + endColor + " " + wort + " <i>[" + plural + "]</i>" + endP + '\n'
            + startP + "<b>" + translation + "</b>" + endP + '\n'
            + startP + colorGray + "Gen,Pl: " + endColor + opt2 + endP + '\n'
            + startP + "<i>" + opt0 + "</i>" + endP;
    } else if (tw == WortDe::TypeWort::Verb)
    {
        result = startP + colorGray + level + ": " + endColor + wort + endP + '\n'
                + startP + "<b>" + translation + "</b>" + endP + '\n'
                + startP + opt2  + endP + '\n'
                + startP + opt1  + endP + '\n'
                + startP + "<i>" + opt0 + "</i>"  + endP;
    } else {
        debPrint("Level: '" << level << "'");
        debPrint("Type: '"<< (tw == WortDe::TypeWort::Verb ? "Verb: " : tw == WortDe::TypeWort::Noun ? "Noun: " : "????: ") << typeWord << "'");
        debPrint("wort: '" << wort << "'");
        debPrint("art: '" << art << "'");
        debPrint("plural: '" << plural << "'");
        debPrint("translation: '" << translation << "'");
        debPrint("opt0: '" << opt0 << "'");
        debPrint("opt1: '" << opt1 << "'");
        debPrint("opt2: '" << opt2 << "'");
    }

    if (tw == WortDe::TypeWort::None)
    {
        return; // Error parsing
    }

    std::string resData;
    addNameTag(resData, "level", level);
    addNameTag(resData, "WortDe::TypeWort", tw == WortDe::TypeWort::Verb ? "Verb" : tw == WortDe::TypeWort::Noun ? "Noun" : "");
    addNameTag(resData, "typeWord", typeWord);
    addNameTag(resData, "wort", wort);
    addNameTag(resData, "art", art);
    addNameTag(resData, "plural", plural);
    addNameTag(resData, "translation", translation);
    addNameTag(resData, "opt0", opt0);
    addNameTag(resData, "opt1", opt1);
    addNameTag(resData, "opt2", opt2);

    QFile file(QString::fromStdString(getFileNameWoerter(woerterReqWord)));
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(resData.data(), resData.size());
        file.close();
    }

    printWoerter(woerterReqWord, resData);
    debPrint(result);
}

void SoundOfWords::printWoerter(const std::string &parseWort, const std::string &data)
{
    static const std::string colorDer = "<span style=\" color:#0000A0;\">";
    static const std::string colorDie = "<span style=\" color:#A00000;\">";
    static const std::string colorDas = "<span style=\" color:#00A000;\">";
    static const std::string colorDef = "<span style=\" color:#000000;\">";
    static const std::string colorGray = "<span style=\" color:#808080;\">";
    static const std::string endColor = "</span>";
    static const std::string startP = "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">";
    static const std::string endP = "</p>";
    std::string result;

    const std::string twStr = getNameTag(data, "WortDe::TypeWort");
    const std::string level = getNameTag(data, "level");
    const std::string translation = getNameTag(data, "translation");
    const std::string wort = getNameTag(data, "wort");
    const std::string opt0 = getNameTag(data, "opt0");
    const std::string opt2 = getNameTag(data, "opt2");
    if (twStr == "Noun")
    {
        const std::string art = getNameTag(data, "art");
        const std::string plural = getNameTag(data, "plural");
        std::string colorArt;
        if (art == "der")
            colorArt = colorDer;
        else if (art == "die")
            colorArt = colorDie;
        else if (art == "das")
            colorArt = colorDas;
        else
            colorArt = colorDef;

        result = startP + colorGray + level + ": " + endColor + colorArt + art + endColor + " " + wort + " <i>[" + plural + "]</i>" + endP + '\n'
            + startP + "<b>" + translation + "</b>" + endP + '\n'
            + startP + colorGray + "Gen,Pl: " + endColor + opt2 + endP + '\n'
            + startP + "<i>" + opt0 + "</i>" + endP;
    } else if (twStr == "Verb")
    {
        const std::string opt1 = getNameTag(data, "opt1");
        result = startP + colorGray + level + ": " + endColor + wort + endP + '\n'
                + startP + "<b>" + translation + "</b>" + endP + '\n'
                + startP + opt2  + endP + '\n'
                + startP + opt1  + endP + '\n'
                + startP + "<i>" + opt0 + "</i>"  + endP;
    } else {
        return;
    }
    emit doneWoerterInfo(parseWort, result);
}

std::string SoundOfWords::getFileNameMp3(const std::string &word)
{
    const std::string fullPath = pathDic + "/CacheSound";
    const std::string sufix = AreaUtf8::islowerU8(AreaUtf8(word).getSymbol()) ? "" : "_";
    return fullPath + "/" + sufix + word + ".mp3";
}

std::string SoundOfWords::getFileNameWoerter(const std::string &word)
{
    const std::string fullPath = pathDic + "/CacheWoerter";
    const std::string sufix = AreaUtf8::islowerU8(AreaUtf8(word).getSymbol()) ? "" : "_";
    return fullPath + "/" + sufix + word + ".txt";
}
