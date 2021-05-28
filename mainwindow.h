#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "glossaryde.h"
#include <QModelIndexList>
#include <QShortcut>

class QListViewGlossaryDeModel;
class TestWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setNewIndex(int idx);

private slots:
    void on_actionRaw_Text_triggered();

    void on_actionOpenDir_triggered();

    void on_listView_activated(const QModelIndex &index);

    void on_listView_clicked(const QModelIndex &index);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void slotShortcutAltW();
    void slotShortcutAltN();
    void slotShortcutAltV();
    void slotShortcutAltA();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

    void on_actionSave_as_raw_triggered();

    void on_pushButton_12_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_14_clicked();

    void on_pushButton_15_clicked();

    void on_pushButton_16_clicked();

    void on_pushButton_17_clicked();

    void on_lineEdit_7_textChanged(const QString &arg1);

    void on_pushButton_18_clicked();

    void on_pushButton_19_clicked();

    void on_pushButton_20_clicked();

    void on_pushButton_21_clicked();

    void on_pushButton_22_clicked();

    void on_pushButton_24_clicked();

    void on_pushButton_25_clicked();

    void on_pushButton_26_clicked();

    void on_pushButton_23_clicked();

    void on_pushButton_27_clicked();

    void on_pushButton_28_clicked();

    void on_actionSave_triggered();

    void on_actionTest_words_triggered();

    void on_pushButton_29_clicked();

    void on_pushButton_30_clicked();

    void on_actionFix_triggered();

private:
    void selectItem(int idx);
    void setButtonEnable(WortDe wd);
    void setWortDe(WortDe wd);
    void getWortDeToCurrWd();
    void wortTranslate(const std::string &beginUrl, const std::string &endUrl, const std::string &str);
    void CombinationTranslate(const std::string &beginUrl, const std::string &endUrl);
    void checkChangesCurrWd(const bool saveWithoutAsk = false);

private:
    std::string pathDic;
    GlossaryDe dicDe;
    QListViewGlossaryDeModel *model;
    int currIndex = -1;
    WortDe currWd;
    int origIndex = -1;
    WortDe origWd;

private:
    Ui::MainWindow *ui;
    QShortcut      *keyAltW;
    QShortcut      *keyAltN;
    QShortcut      *keyAltV;
    QShortcut      *keyAltA;
    TestWindow     *testWin;

};
#endif // MAINWINDOW_H
