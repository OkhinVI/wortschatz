#include "soundofwords.h"
#include <QMediaPlayer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <iostream>
#include "string_utf8.h"

#define debPrint(a) std::cout << a << std::endl

SoundOfWords::SoundOfWords(QObject* pobj) : QObject(pobj)
{
    m_pnam = new QNetworkAccessManager(this);
    connect(m_pnam, SIGNAL(finished(QNetworkReply*)),
            this,   SLOT(slotFinished(QNetworkReply*))
           );
    vokalWort = new QMediaPlayer;
}

void SoundOfWords::play(const std::string &word)
{
    AreaUtf8 au8(word);
    au8.trim();
    AreaUtf8 wort1 = au8.getToken();
    if (wort1.empty())
        return;

    QUrl url(QString::fromStdString("https://de.thefreedictionary.com/" + wort1.toString()));
    QNetworkRequest request(url);
    // QNetworkReply*  pnr =
            m_pnam->get(request);
//    connect(pnr, SIGNAL(downloadProgress(qint64, qint64)),
//            this, SIGNAL(downloadProgress(qint64, qint64))
//           );
}

void SoundOfWords::slotFinished(QNetworkReply* pnr)
{
    if (pnr->error() != QNetworkReply::NoError) {
        // emit error();
        debPrint("Error: " << pnr->url().toString().toStdString());
    }
    else {
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
            vokalWort->setMedia(QUrl(QString::fromStdString(urlMp3)));
            vokalWort->play();
            debPrint("play: " << pnr->url().toString().toStdString());
        }
    }
    pnr->deleteLater();
}
