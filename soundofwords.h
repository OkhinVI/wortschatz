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
    SoundOfWords(QObject* pobj = 0);
    void play(const std::string &word);

private slots:
    void slotFinished(QNetworkReply*);

private:
    QNetworkAccessManager* m_pnam;
    QMediaPlayer *vokalWort;

};

#endif // SOUNDOFWORDS_H
