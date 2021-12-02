#ifndef SOUNDOFWORDS_H
#define SOUNDOFWORDS_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QMediaPlayer;
class WortDe;

class SoundOfWords : public QObject
{
    Q_OBJECT
public:
    enum class TypeReply
    {
        None = 0,
        FreeDic,
        FreeDicMp3,
        Woerter,
        _last_one
    };

public:
    SoundOfWords(const std::string &aPathDic, QObject* pobj = 0);
    void play(const std::string &word);
    bool infoWoerter(const std::string &word); // true - is in the cache / false - waiting for server response

signals:
    void doneWoerterInfo(const std::string &word, const std::string &info, const WortDe &de);
    void doneWoerterError(const std::string &word, const std::string &error);

private slots:
    void slotFinished(QNetworkReply*);

private:
    void startSoundReq(const std::string &word);
    std::string getFileNameMp3(const std::string &word);
    std::string getFileNameWoerter(const std::string &word);
    TypeReply calcNetworkReply(QNetworkReply* pnr);
    void parseFreeDic(QString &url, QByteArray &bodyAr);
    void parseFreeDicMp3(QString &url, QByteArray &bodyAr);
    void parseWoerter(QString &url, QByteArray &bodyAr);
    void printWoerter(const std::string &parseWort, const std::string &data);

private:
    QNetworkAccessManager* m_pnam = nullptr;
    QMediaPlayer *vokalWort = nullptr;
    std::string pathDic;

    QNetworkReply* freedictionaryReq = nullptr;
    std::string freedictionaryWord;
    QNetworkReply* freedictionaryReqMp3 = nullptr;
    std::string freedictionaryMp3FileName;

    QNetworkReply* woerterReq = nullptr;
    std::string woerterReqWord;
};

#endif // SOUNDOFWORDS_H
