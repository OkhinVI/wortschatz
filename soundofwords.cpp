#include "soundofwords.h"
#include <QMediaPlayer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QFile>
#include <iostream>
#include "string_utf8.h"

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
    if (word.empty())
        return;

    const std::string fileName = getFileName(word);
    QFile file(QString::fromStdString(fileName));
    if (file.exists())
    {
        debPrint("play local file: " << fileName);
        vokalWort->setMedia(QUrl(QString::fromStdString(fileName)));
        vokalWort->play();
    }
    else
    {
        startReq(word);
    }
}

void SoundOfWords::startReq(const std::string &word)
{
    const std::string urlRaw = "https://de.thefreedictionary.com/" + word;
    QUrl url(QString::fromStdString(urlRaw));
    QNetworkRequest request(url);
    currReq = m_pnam->get(request);
    currWord = word;
    isMp3File = false;
    debPrint("Start request: " << urlRaw);

//    connect(pnr, SIGNAL(downloadProgress(qint64, qint64)),
//            this, SIGNAL(downloadProgress(qint64, qint64))
//           );
}

void SoundOfWords::slotFinished(QNetworkReply* pnr)
{
    if (pnr != currReq)
    {
        pnr->deleteLater();
        debPrint("Error QNetworkReply: " << pnr << " != " << currReq);
        return;
    }
    currReq = nullptr;

    if (pnr->error() != QNetworkReply::NoError) {
        // emit error();
        debPrint("Error: " << pnr->url().toString().toStdString());
    } else if (isMp3File)
    {
        isMp3File = false;
        const std::string fileName = getFileName(currWord);
        debPrint("save file: " << fileName);
        QFile file(QString::fromStdString(fileName));
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(pnr->readAll());
            file.close();
            vokalWort->setMedia(QUrl(QString::fromStdString(fileName)));
            vokalWort->play();
        }
    }
    else
    {
        // emit done(pnr->url(), pnr->readAll());
        std::string body = pnr->readAll().toStdString();
        const std::string findString = "data-snd=\"";
        auto pos1 = body.find(findString);
        if (pos1 == std::string::npos)
        {
            debPrint("data-snd not found: " << pnr->url().toString().toStdString());
        } else {
            pos1 += findString.size();
            auto pos2 = body.find("\"", pos1);
            const std::string urlMp3 = "https://img2.tfd.com/pron/mp3/" + body.substr(pos1, pos2 - pos1) + ".mp3";

            debPrint("download: " << urlMp3);
            QUrl url(QString::fromStdString(urlMp3));
            QNetworkRequest request(url);
            currReq = m_pnam->get(request);
            isMp3File = true;
        }
    }
    pnr->deleteLater();
}

std::string SoundOfWords::getFileName(const std::string &word)
{
    const std::string fullPath = pathDic + "/CacheSound";
    const std::string sufix = AreaUtf8::islowerU8(AreaUtf8(word).getSymbol()) ? "" : "_";
    return fullPath + "/" + sufix + word + ".mp3";
}
