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
// #define debPrint(a)


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
}

bool SoundOfWords::infoWoerter(const std::string &word)
{
    AreaUtf8 wordAr = AreaUtf8(word).getToken();
    if (wordAr.empty())
        return true;

    const std::string fileName = getFileNameWoerter(wordAr.toString());
    QFile file(QString::fromStdString(fileName));
    if (file.exists())
    {
        if (file.open(QIODevice::ReadOnly))
        {
            QByteArray data = file.readAll();
            printWoerter(wordAr.toString(), data.toStdString());
            file.close();
        }
    }
    else
    {
        static const std::string urlOnlyNoun = "https://www.woerter.ru/sushhestvitelnye/?w=";
        static const std::string urlForAll = "https://www.woerter.ru/?w=";
        const std::string urlRaw = (AreaUtf8::isupperDe(wordAr.peek()) ? urlOnlyNoun : urlForAll)
                + wordAr.toString();
        QUrl url(QString::fromStdString(urlRaw));
        QNetworkRequest request(url);
        woerterReq = m_pnam->get(request);
        woerterReqWord = wordAr.toString();
        return false;
    }
    return true;
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
        if (typeRep == TypeReply::Woerter)
        {
            const int NoAccess = QNetworkReply::UnknownContentError;
            const int NoConnection = QNetworkReply::UnknownNetworkError;
            std::string err;
            if (pnr->error() == NoAccess)
                err = "No access: " + woerterReqWord;
            else if (pnr->error() == NoConnection)
                err = "No connection: " + woerterReqWord;
            else
                err = "Error connection: " + woerterReqWord;
            emit doneWoerterError(woerterReqWord, err);
        }
        debPrint("Error (" << int(typeRep) << "): " << pnr->url().toString().toStdString() << ", err = " << pnr->error());
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
    size_t pos1 = 0;
    if (!beginStr.empty())
    {
        pos1 = str.find(beginStr);
        while (numer > 0 && pos1 != std::string::npos)
        {
            pos1 = str.find(beginStr, pos1 + beginStr.size());
            --numer;
        }
        if (pos1 == std::string::npos)
            return std::string();

        pos1+= beginStr.size();
        if (beginStr[beginStr.size() - 1] != '>')
        {
            pos1 = str.find(">", pos1);
            if (pos1 == std::string::npos)
                return std::string();
            pos1 += 1;
        }
    }
    auto pos2 = !endStr.empty() ? str.find(endStr, pos1) : str.size();
    if (pos2 == std::string::npos)
        return std::string();

    std::string newStr = AreaUtf8(str, pos1, pos2 - pos1).trim().toString();
    util::replaceEndLines(newStr);
    return newStr;
}

static void addNameTag(std::string &str, const std::string &name, const std::string data)
{
    static const std::string startStr = "<p class=\"";
    static const std::string endStr = "</p>";
    str = str + startStr + name + "\"\t>" + data + endStr + '\n';
}

static std::string getNameTag(const std::string &str, const std::string &name)
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

        QUrl url(QString::fromStdString(urlMp3));
        QNetworkRequest request(url);
        freedictionaryReqMp3 = m_pnam->get(request);
        freedictionaryMp3FileName = getFileNameMp3(freedictionaryWord);
    }
}

void SoundOfWords::parseFreeDicMp3(QString &, QByteArray &bodyAr)
{
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
    std::string body = bodyAr.toStdString();

    WortDe::TypeWort tw = WortDe::TypeWort::None;
    std::string paragraf = getHtmlTag(body, "<h1 class=\"rClear\"", "</p>");
    std::string typeWord;
    if (!(typeWord = getHtmlTag(paragraf, "<a href=\"/glagoly/", "</a>")).empty())
        tw = WortDe::TypeWort::Verb;
    else if (!(typeWord = getHtmlTag(paragraf, "<a href=\"/sushhestvitelnye/", "</a>")).empty())
        tw = WortDe::TypeWort::Noun;
    AreaUtf8 typeWordAr(typeWord);
    typeWordAr.getToken(); typeWordAr.getToken();
    typeWordAr.getToken(); typeWordAr.getToken();
    AreaUtf8 webWord = typeWordAr.getToken();
    if (webWord.toString() != woerterReqWord)
    {
        if (webWord.empty())
        {
            const std::string err = "Word not found: " + woerterReqWord;
            emit doneWoerterError(woerterReqWord, err);
            debPrint("Word not found: " << webWord.toString() << " / " << url.toStdString());
        } else {
            const std::string err = "Word not found: " + woerterReqWord + " (found only another word: " + webWord.toString() + ")";
            emit doneWoerterError(woerterReqWord, err);
            debPrint("Falsches Wort: " << webWord.toString() << " / " << url.toStdString());
        }
        return;
    }

    paragraf = getHtmlTag(body, "<span class=\"rInf\">", "<p class=");
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
    std::string opt0 = getHtmlTag(body, "<p class=\"rInf r1Zeile rU3px rO0px\">", "</p>", 0);
    std::string opt1 = getHtmlTag(body, "<p class=\"rInf r1Zeile rU3px rO0px\">", "</p>", 1);
    std::string opt0i = getHtmlTag(opt0, "<i", "</i>");
    if (!opt0.empty() && opt0i.empty() && opt1.empty())
        opt1 = opt0;
    opt0 = opt0i;
    std::string opt2 = getHtmlTag(body, "<p class=\"rInf r1Zeile rU3px rO0px\" onclick=", "</p>");

    if (tw != WortDe::TypeWort::Noun && tw != WortDe::TypeWort::Verb)
    {
        const std::string err = "Error parsing word: " + woerterReqWord;
        emit doneWoerterError(woerterReqWord, err);
        debPrint("Error type word: '" << typeWord << "'");
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
}

static std::string ParsePlural(const std::string &pl, const std::string &word)
{
    AreaUtf8 plAll(pl);
    plAll.getToken(","); plAll.getToken(",");
    std::string plStr = plAll.getToken(",").trim().toString();
    if (plStr.size() > 1 && plStr[plStr.size() - 1] == '-' && plStr != "¨-")
        plStr = plStr.substr(0, plStr.size() - 1);

    if (!word.empty() && word[word.size() - 1] == 'e')
    {
        if (plStr == "-en")
            plStr = "-n";
        else if (plStr == "¨-en")
            plStr = "¨-n";
        else if (plStr == "-er")
            plStr = "¨-r";
        else if (plStr == "¨-er")
            plStr = "¨-r";
    }
    return plStr;
}

static std::string ParseTrans(const std::string &trans)
{
    AreaUtf8 trAll(trans);
    std::vector<std::string> vecStr;
    while (!trAll.eof())
    {
        AreaUtf8 token = trAll.getToken(",").trim(); trAll.getToken(",");
        std::string tokenStr;
        while (!token.eof())
        {
            AreaUtf8::SymbolType sym = token.getSymbol();
            if (sym != UTF8_STRING_TO_SYMBOL("́"))
                AreaUtf8::StringAddSym(tokenStr, sym);
        }

        vecStr.push_back(tokenStr);
    }

    std::string result;
    for (size_t i = 0; i < vecStr.size(); ++i)
    {
        const std::string &tokenStr = vecStr[i];
        bool overlapping = false;
        for (size_t j = 0; j < i; ++j)
        {
            if (tokenStr == vecStr[j])
            {
                overlapping = true;
                break;
            }
        }
        if (overlapping)
            continue;

        if (result.empty())
            result = tokenStr;
        else
            result = result + ", " + tokenStr;
    }
    return result;
}

void SoundOfWords::printWoerter(const std::string &parseWort, const std::string &data)
{
    static const std::string colorDer = "<span style=\" color:#0000A0;\">";
    static const std::string colorDie = "<span style=\" color:#A00000;\">";
    static const std::string colorDas = "<span style=\" color:#00A000;\">";
    static const std::string colorDef = "<span style=\" color:#000000;\">";
    static const std::string colorVebForm = "<span style=\" color:#0000D0;\">";
    static const std::string colorVebUse = "<span style=\" color:#8000B0;\">";
    static const std::string colorDeBedeutung = "<span style=\" color:#505050;\">";
    static const std::string colorGray = "<span style=\" color:#808080;\">";
    static const std::string colorBlue = "<span style=\" color:#0000D0;\">";
    static const std::string colorGreen = "<span style=\" color:#00B000;\">";
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
    WortDe de;
    const std::string translationNew = ParseTrans(translation);
    if (twStr == "Noun")
    {
        const std::string art = getNameTag(data, "art");
        const std::string plural = getNameTag(data, "plural");

        de.parseRawLine(parseWort, translationNew, 0, WortDe::TypeWort::Noun);
        de.setNewPlural(ParsePlural(plural, parseWort));
        std::string colorArt;
        if (art == "der")
        {
            colorArt = colorDer;
            de.setNewArtikel(WortDe::TypeArtikel::Der);
        }
        else if (art == "die")
        {
            colorArt = colorDie;
            de.setNewArtikel(WortDe::TypeArtikel::Die);
        }
        else if (art == "das")
        {
            colorArt = colorDas;
            de.setNewArtikel(WortDe::TypeArtikel::Das);
        }
        else
        {
            colorArt = colorDef;
        }

        result = startP + colorGray + level + ": " + endColor + colorArt + art + endColor + " " + wort + " <i>[" + plural + "]</i>" + endP + '\n'
            + startP + "<b>" + translation + "</b>" + endP + '\n'
            + startP + colorGreen + "<i>" + "Gen·Pl: " + "</i>" + endColor + colorBlue + opt2 + endColor + endP + '\n'
            + startP + colorDeBedeutung + opt0 + endColor + endP;
    } else if (twStr == "Verb")
    {
        const std::string opt1 = getNameTag(data, "opt1");
        de.parseRawLine(parseWort, translationNew, 0, WortDe::TypeWort::Verb);

        result = startP + colorGray + level + ": " + endColor + wort + endP + '\n'
                + startP + "<b>" + translation + "</b>" + endP + '\n'
                + startP + colorVebForm + opt2 + endColor + endP + '\n'
                + startP + colorVebUse + opt1 + endColor  + endP + '\n'
                + startP + colorDeBedeutung + opt0 + endColor  + endP;
    } else {
        const std::string err = "Error loaded type word: '" + twStr + "' - " + parseWort;
        emit doneWoerterError(parseWort, err);
        debPrint(err);
        return;
    }

    emit doneWoerterInfo(parseWort, result, de);
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
