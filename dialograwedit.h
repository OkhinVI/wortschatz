#ifndef DIALOGRAWEDIT_H
#define DIALOGRAWEDIT_H

#include <QDialog>
#include "wortde.h"

class QPlainTextEdit;

namespace Ui {
class DialogRawEdit;
}

class DialogRawEdit : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRawEdit(QWidget *parent = nullptr);
    ~DialogRawEdit();
    QString getDeText();
    QString getTranslationText();
    void setBlockNum(unsigned int num);
    unsigned int getBlockNum(std::string &tema);

    WortDe::TypeWort getTypeWort();
    void setTypeWort(WortDe::TypeWort tw);

private:
    void plainTextEditShow(QPlainTextEdit * const plainTextEdit);

private slots:
    void on_pushButton_clicked();

    void on_plainTextEdit_blockCountChanged(int newBlockCount);

    void on_plainTextEdit_2_blockCountChanged(int newBlockCount);

    void on_plainTextEdit_cursorPositionChanged();

    void on_plainTextEdit_2_cursorPositionChanged();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    WortDe::TypeWort currTw;

private:
    Ui::DialogRawEdit *ui;
};

#endif // DIALOGRAWEDIT_H
