#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QMainWindow>
#include "glossaryde.h"
#include <vector>

class QPushButton;
class MainWindow;
class TestSettings;

namespace Ui {
class TestWindow;
}

class TestWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestWindow(GlossaryDe &aDicDe, MainWindow *mw, QWidget *parent = nullptr);
    ~TestWindow();

    void setNewWort();

private:
    void testSelectTr(size_t idx, bool ignoreResult = false);
    void showLevel();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

private:
    bool eventFilter(QObject *target, QEvent *event);

private:
    GlossaryDe &dicDe;
    size_t currTestGlossaryIdx = 0;
    int currIdxCorrectTr = -1;
    std::vector<QPushButton *> vecButton;
    std::vector<size_t> vecIdxTr;
    GlossaryDe::SelectSettings glSelSet;

private:
    Ui::TestWindow *ui;
    MainWindow *mainWindow;
    TestSettings* settingsWin;
};

#endif // TESTWINDOW_H
