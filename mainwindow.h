#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "glossaryde.h"
#include <QModelIndexList>
#include <QShortcut>

class QListViewGlossaryDeModel;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionRaw_Text_triggered();

    void on_actionConvert_triggered();

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

private:
    void selectItem(int idx);
    void setWortDe(WortDe wd);

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
    QShortcut       *keyAltW;
    QShortcut       *keyAltN;
    QShortcut       *keyAltV;
    QShortcut       *keyAltA;

};
#endif // MAINWINDOW_H
