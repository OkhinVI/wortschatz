#include "testwindow.h"
#include "ui_testwindow.h"
#include "mainwindow.h"
#include "testsettings.h"
#include <vector>
#include <ctime>
#include "utilQtTypes.h"

static size_t RichteAntwort = 0;
static size_t FalscheAntwort = 0;

TestWindow::TestWindow(GlossaryDe &aDicDe, MainWindow *mw, QWidget *parent) :
    QMainWindow(parent),
    dicDe(aDicDe),
    glSelSet(aDicDe),
    ui(new Ui::TestWindow),
    mainWindow(mw)
{
    ui->setupUi(this);
    settingsWin = new TestSettings(glSelSet, this);

    vecButton.push_back(ui->pushButton_2);
    vecButton.push_back(ui->pushButton_3);
    vecButton.push_back(ui->pushButton_4);
    vecButton.push_back(ui->pushButton_5);
    vecButton.push_back(ui->pushButton_6);
    vecButton.push_back(ui->pushButton_9);
    vecButton.push_back(ui->pushButton_10);

    ui->lineEdit->setStyleSheet("background-color: yellow; color: blue;");
}

TestWindow::~TestWindow()
{
    delete ui;
}

void TestWindow::on_pushButton_clicked()
{
    setNewWort();
}

void TestWindow::setNewWort()
{
    ui->pushButton->setEnabled(false);
    for (size_t i = 0; i < vecButton.size(); ++i)
        vecButton[i]->setStyleSheet("text-align: left;");

    currTestGlossaryIdx = dicDe.calcTestWortIdx(glSelSet);
    vecIdxTr.resize(vecButton.size());
    currIdxCorrectTr = dicDe.selectVariantsTr(vecIdxTr);
    if (currIdxCorrectTr < 0)
        return;

    {
        WortDe &wd = dicDe.at(currTestGlossaryIdx);
        const std::string prfx = wd.prefix();
        std::string wortStr = prfx.empty() ? wd.wort() : prfx + " " + wd.wort();
        const auto posWortNotEqul = wortStr.find("(≠");
        if (posWortNotEqul != std::string::npos)
            wortStr = wortStr.substr(0, posWortNotEqul);
        utilQt::strToLineEdit(ui->lineEdit, wortStr);
        ui->lineEdit->setCursorPosition(0);
    }

    for (size_t i = 0; i < vecButton.size(); ++i)
    {
        std::string wortTr = dicDe[vecIdxTr[i]].translation();
        const auto posWortNotEqul = wortTr.find("(≠");
        if (posWortNotEqul != std::string::npos)
            wortTr = wortTr.substr(0, posWortNotEqul);
        vecButton[i]->setText(QString::fromStdString(wortTr));
    }
}

void TestWindow::testSelectTr(size_t idx)
{
    ui->pushButton->setEnabled(true);
    if (idx >= vecButton.size())
        return;

    if (currIdxCorrectTr < 0)
    {
        if (idx < vecIdxTr.size())
            mainWindow->setNewIndex(vecIdxTr[idx]);
        return;
    }

    WortDe &wd = dicDe.at(currTestGlossaryIdx);
    if (int(idx) == currIdxCorrectTr)
    {
        vecButton[currIdxCorrectTr]->setStyleSheet("text-align: left; background-color: yellow; color: blue;");
        wd.addAnswer(true);
        ++RichteAntwort;
    } else {
        vecButton[currIdxCorrectTr]->setStyleSheet("text-align: left; color: red;");
        vecButton[idx]->setStyleSheet("text-align: left; color: gray ;");
        wd.addAnswer(false);
        ++FalscheAntwort;
    }
    mainWindow->setNewIndex(currTestGlossaryIdx);
    currIdxCorrectTr = -1;

    ui->label->setText("richtige: " + QString::number(RichteAntwort) + " / falsche: "  + QString::number(FalscheAntwort) +
                       " = " + QString::number(RichteAntwort * 100 / (RichteAntwort + FalscheAntwort)) + "%");
}

void TestWindow::on_pushButton_2_clicked()
{
    testSelectTr(0);
}

void TestWindow::on_pushButton_3_clicked()
{
    testSelectTr(1);
}

void TestWindow::on_pushButton_4_clicked()
{
    testSelectTr(2);
}

void TestWindow::on_pushButton_5_clicked()
{
    testSelectTr(3);
}

void TestWindow::on_pushButton_6_clicked()
{
    testSelectTr(4);
}

void TestWindow::on_pushButton_7_clicked()
{
    mainWindow->setNewIndex(currTestGlossaryIdx);
    mainWindow->show();
    this->close();
}

void TestWindow::on_pushButton_8_clicked()
{
    this->close();
}

void TestWindow::on_pushButton_9_clicked()
{
    testSelectTr(5);
}

void TestWindow::on_pushButton_10_clicked()
{
    testSelectTr(6);
}

void TestWindow::on_pushButton_11_clicked()
{
    if (settingsWin->exec() == QDialog::Accepted)
    {

    }
}
