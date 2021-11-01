#include "testwindow.h"
#include "ui_testwindow.h"
#include "mainwindow.h"
#include "testsettings.h"
#include <vector>
#include <ctime>
#include <QKeyEvent>
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

    for (size_t i = 0; i < vecButton.size(); ++i)
        vecButton[i]->installEventFilter(this);

    ui->lineEdit->installEventFilter(this);

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
    showLevel();
    ui->pushButton->setEnabled(false);
    for (size_t i = 0; i < vecButton.size(); ++i)
        vecButton[i]->setStyleSheet("text-align: left;");

    currTestGlossaryIdx = dicDe.calcTestWortIdx(glSelSet);
    if (currTestGlossaryIdx >= dicDe.size())
        return;

    vecIdxTr.resize(vecButton.size());
    currIdxCorrectTr = dicDe.selectVariantsTr(vecIdxTr);
    if (currIdxCorrectTr < 0)
        return;

    currWord = dicDe.at(currTestGlossaryIdx);
    {
        const std::string prfx = currWord.prefix();
        std::string wortStr = prfx.empty() ? currWord.wort() : prfx + " " + currWord.wort();
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

    if (ui->checkBoxSound->checkState() == Qt::Checked)
        mainWindow->PlayWord(currWord.wort());
}

void TestWindow::testSelectTr(size_t idx, bool onlyFalsh , bool ignoreResult)
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

    bool needRecorder = false;
    WortDe &wd = dicDe.at(currTestGlossaryIdx);
    if (int(idx) == currIdxCorrectTr)
    {
        vecButton[currIdxCorrectTr]->setStyleSheet("text-align: left; background-color: yellow; color: blue;");
        if (!ignoreResult && !onlyFalsh)
        {
            wd.addAnswer(true, dicDe.calcNextSequenceNumber());
            ++RichteAntwort;
        } else {
            wd.setSequenceNumber(dicDe.calcNextSequenceNumber());
        }
    } else {
        vecButton[currIdxCorrectTr]->setStyleSheet("text-align: left; color: red;");
        vecButton[idx]->setStyleSheet("text-align: left; color: gray ;");
        if (!ignoreResult)
        {
            wd.addAnswer(false, dicDe.calcNextSequenceNumber());
            ++FalscheAntwort;
            needRecorder = true;
        } else {
            wd.setSequenceNumber(dicDe.calcNextSequenceNumber());
        }
    }
    mainWindow->setNewIndex(currTestGlossaryIdx);
    if (needRecorder)
        mainWindow->addWordToList();

    currIdxCorrectTr = -1;

    if (RichteAntwort + FalscheAntwort > 0)
    {
        ui->label->setText("richtige: " + QString::number(RichteAntwort)
            + " / falsche: " + QString::number(FalscheAntwort) + " = "
            + QString::number(RichteAntwort * 100 / (RichteAntwort + FalscheAntwort)) + "%");
    }
    showLevel();
}

void TestWindow::showLevel()
{
    size_t count = 0;
    size_t countMinCorrectAnswers = 0;
    uint32_t minCorrectAnswers = 0;
    size_t allCorrectAnswers;
    size_t allNotCorrectAnswers = 0;
    const double k = dicDe.calcProgress(glSelSet, count, countMinCorrectAnswers, minCorrectAnswers, allCorrectAnswers, allNotCorrectAnswers);
    ui->labelLevel->setText("k: " + QString::number(k) + " (l" + QString::number(minCorrectAnswers) + ": "
        + QString::number(countMinCorrectAnswers) + "/+" + QString::number(count - countMinCorrectAnswers) + ")");
    if (allCorrectAnswers + allNotCorrectAnswers > 0)
        ui->labelLevelSum->setText("All r: " + QString::number(allCorrectAnswers) + " / f: " + QString::number(allNotCorrectAnswers));
    else
        ui->labelLevelSum->setText("0/0");
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
    settingsWin->resetVals();
    if (settingsWin->exec() == QDialog::Accepted)
    {
        setNewWort();
    }
}

bool TestWindow::eventFilter(QObject *target, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        const auto button = mouseEvent->button();
        if (button == Qt::MiddleButton || button == Qt::RightButton)
        {
            for (size_t i = 0; i < vecButton.size(); ++i)
            {
                if (target == vecButton[i])
                {
                    testSelectTr(i, button == Qt::RightButton, button == Qt::MiddleButton);
                    return true;
                }
            }
            if (target == ui->lineEdit && button == Qt::MiddleButton)
            {
                mainWindow->PlayWord(currWord.wort());
                return true;
            }

        }
        else
            return QMainWindow::eventFilter(target, event);
        return true;
    }

    return QMainWindow::eventFilter(target, event);
}
