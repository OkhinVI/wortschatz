#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "glossaryde.h"
#include "webtranslation.h"
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
    MainWindow(const char *aAccName, QWidget *parent = nullptr);
    ~MainWindow();

    void setNewIndex(int idx);
    void addWordToList();

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
    void slotShortcutAltL();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

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

    void on_pushButton_31_clicked();

    void on_pushButton_32_clicked();

    void on_actionToRaw_triggered();

    void on_actionExport_to_text_triggered();

    void on_actionClear_statistic_triggered();

    void on_dudenButton_33_clicked();

    void on_wiktionaryButton_33_clicked();

    void on_leipzigButton_clicked();

    void on_thefreeDicButton_clicked();

    void on_actionimport_options_triggered();

    void on_actionImport_statistic_triggered();

    void on_checkBox_AutoSearch_stateChanged(int arg1);

    void on_checkBoxUseForm_stateChanged(int arg1);

    void on_buttonAddWordToList_clicked();

    void on_checkBox_FullWord_stateChanged(int arg1);

    void on_textLog_selectionChanged();

    void on_pushButton_33_clicked();

private:
    bool eventFilter(QObject *target, QEvent *event);

    void selectItem(const int idx);
    void setButtonEnable(WortDe wd);
    void setWortDe(WortDe wd);
    void getWortDeToCurrWd();
    void checkChangesCurrWd(const bool saveWithoutAsk = false);
    void addNewWortFromSearch();
    void addNewWortFromStatSearch();
    void clearCurrWord();
    void findStatWord(const std::string &str);
    void showFoundStatWord();
    void statWordClear();
    void nextFoundStatWord(int delta);

private:
    std::string accName;
    std::string settingsDictionaryPath;
    std::string pathDic;
    GlossaryDe dicDe;
    QListViewGlossaryDeModel *model;
    int origIndex = -1;
    WortDe origWd;
    WortDe currWd;

    FoundItemsIdx statFound;
    FoundFormDicIdx statFoundForm;

    WebTranslation webTr;

private:
    Ui::MainWindow *ui;
    QShortcut      *keyAltW;
    QShortcut      *keyAltN;
    QShortcut      *keyAltV;
    QShortcut      *keyAltA;
    QShortcut      *keyAltL;
    TestWindow     *testWin;

};
#endif // MAINWINDOW_H
