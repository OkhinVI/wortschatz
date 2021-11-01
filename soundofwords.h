#ifndef SOUNDOFWORDS_H
#define SOUNDOFWORDS_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QMediaPlayer;

class SoundOfWords : public QObject
{
    Q_OBJECT
public:
    SoundOfWords(const std::string &aPathDic, QObject* pobj = 0);
    void play(const std::string &word);

private slots:
    void slotFinished(QNetworkReply*);

private:
    void startReq(const std::string &word);
    std::string getFileName(const std::string &word);

private:
    QNetworkAccessManager* m_pnam = nullptr;
    QMediaPlayer *vokalWort = nullptr;
    std::string pathDic;

    QNetworkReply* currReq = nullptr;
    std::string currWord;
    bool isMp3File = false;
};

#endif // SOUNDOFWORDS_H
