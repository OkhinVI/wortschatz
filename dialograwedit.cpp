#include "dialograwedit.h"
#include "ui_dialograwedit.h"
#include "dialogtypewort.h"
#include <QTextBlock>
#include <QMessageBox>
#include <QFileDialog>
#include <fstream>
#include "string_utf8.h"
#include "SerializeString.h"
#include "utilQtTypes.h"
#include "linesramstream.h"
#include "utility.h"

static WortDe::BlockNumType CurrBlock = WortDe::creatBlock(1, 1, 1, 0);

DialogRawEdit::DialogRawEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRawEdit)
{
    ui->setupUi(this);
    setBlockNum(WortDe::preIncrementBlock(CurrBlock));
}

DialogRawEdit::~DialogRawEdit()
{
    delete ui;
}

// const int count = ui->plainTextEdit->document()->blockCount();
// auto tk = ui->plainTextEdit->textCursor();
// tk.position()
// tk.blockNumber()
// tk.columnNumber()
// QString str2 = ui->plainTextEdit->document()->findBlockByLineNumber(blokNum).text();

void DialogRawEdit::on_pushButton_clicked()
{
    unsigned int num3 = ui->lineEdit_6->text().toInt();
    ui->lineEdit_7->setText(QString::number(1));
    ++num3;
    if (num3 > 0xFF)
    {
        num3 = 0xFF;
    }
    ui->lineEdit_6->setText(QString::number(num3));
}

void DialogRawEdit::on_plainTextEdit_blockCountChanged(int newBlockCount)
{
    QString str = "Deutsch";
    ui->label->setText(str + " (" + QString::number(newBlockCount) + ")");

    const int countAndern = ui->plainTextEdit_2->document()->blockCount();
    if (countAndern != newBlockCount)
        ui->label_2->setStyleSheet("color: rgb(255, 0, 0)");
    else
        ui->label_2->setStyleSheet("color: rgb(0, 0, 0)");

    plainTextEditShow(ui->plainTextEdit);
}

void DialogRawEdit::on_plainTextEdit_2_blockCountChanged(int newBlockCount)
{
    QString str = "Russisch";
    ui->label_2->setText(str + " (" + QString::number(newBlockCount) + ")");

    const int countAndern = ui->plainTextEdit->document()->blockCount();
    if (countAndern != newBlockCount)
        ui->label_2->setStyleSheet("color: rgb(255, 0, 0)");
    else
        ui->label_2->setStyleSheet("color: rgb(0, 0, 0)");

    plainTextEditShow(ui->plainTextEdit_2);
}


void DialogRawEdit::on_plainTextEdit_cursorPositionChanged()
{
    plainTextEditShow(ui->plainTextEdit);
}

void DialogRawEdit::on_plainTextEdit_2_cursorPositionChanged()
{
    plainTextEditShow(ui->plainTextEdit_2);
}

void DialogRawEdit::plainTextEditShow(QPlainTextEdit * const plainTextEdit)
{
    static bool loockEventAnder = false;
    if (loockEventAnder)
        return;
    loockEventAnder = true;

    QPlainTextEdit *plainTextEditAnder = plainTextEdit == ui->plainTextEdit ? ui->plainTextEdit_2 : ui->plainTextEdit;

    auto tk = plainTextEdit->textCursor();
    auto tkAnder = plainTextEditAnder->textCursor();
    int diffPos = tkAnder.blockNumber() - tk.blockNumber();
    if (diffPos > 0)
    {
        for (int i = 0; i < diffPos; ++i)
            tkAnder.movePosition(QTextCursor::Up);
        plainTextEditAnder->setTextCursor(tkAnder);
        //ui->plainTextEdit->setFocus();
    } else if (diffPos < 0)
    {
        diffPos = -diffPos;
        for (int i = 0; i < diffPos; ++i)
            tkAnder.movePosition(QTextCursor::Down);
        plainTextEditAnder->setTextCursor(tkAnder);
        //ui->plainTextEdit->setFocus();
    }

    const int blokNum = tk.blockNumber();
    QString str1 = ui->plainTextEdit->document()->findBlockByLineNumber(blokNum).text();
    ui->lineEdit->setText(str1);
    ui->lineEdit->setCursorPosition(0);
    QString str2 = ui->plainTextEdit_2->document()->findBlockByLineNumber(blokNum).text();
    ui->lineEdit_2->setText(str2);
    ui->lineEdit_2->setCursorPosition(0);

    ui->lineEdit_3->setText(QString::number(blokNum + 1) + ":");
    loockEventAnder = false;
}

QString DialogRawEdit::getDeText()
{
    return ui->plainTextEdit->toPlainText();
}

QString DialogRawEdit::getTranslationText()
{
    return ui->plainTextEdit_2->toPlainText();
}

void DialogRawEdit::setBlockNum(WortDe::BlockNumType num)
{
    unsigned int h1, h2, h3, h4;
    WortDe::blockToUint_4(num, h1, h2, h3, h4);
    ui->lineEdit_4->setText(QString::number(h1));
    ui->lineEdit_5->setText(QString::number(h2));
    ui->lineEdit_6->setText(QString::number(h3));
    ui->lineEdit_7->setText(QString::number(h4));
}

WortDe::BlockNumType DialogRawEdit::getBlockNum(std::string &tema)
{
    unsigned int num1 = ui->lineEdit_4->text().toInt();
    unsigned int num2 = ui->lineEdit_5->text().toInt();
    unsigned int num3 = ui->lineEdit_6->text().toInt();
    unsigned int num4 = ui->lineEdit_7->text().toInt();

    CurrBlock = WortDe::creatBlock(num1, num2, num3, num4);
    tema = utilQt::lineEditToStdStr(ui->lineEdit_8);

    return CurrBlock;
}

WortDe::TypeWort DialogRawEdit::getTypeWort()
{
    return currTw;
}

void DialogRawEdit::setTypeWort(WortDe::TypeWort tw)
{
    currTw = tw;
    std::string typeStr = WortDe::TypeWortToString(currTw) + "/" + WortDe::TypeWortToString(currTw, "ru");
    ui->pushButton_2->setText(QString::fromStdString(typeStr));
}

void DialogRawEdit::on_pushButton_2_clicked()
{
    DialogTypeWort* pdlg = new DialogTypeWort(this);
    pdlg->setTypeWort(currTw);
    if (pdlg->exec() == QDialog::Accepted)
    {
        currTw = pdlg->getTypeWort();
        std::string typeStr = WortDe::TypeWortToString(currTw) + "/" + WortDe::TypeWortToString(currTw, "ru");
        ui->pushButton_2->setText(QString::fromStdString(typeStr));
    }
    delete pdlg;
}

void DialogRawEdit::on_pushButton_3_clicked()
{
    if (ui->plainTextEdit->document()->blockCount() != ui->plainTextEdit_2->document()->blockCount())
    {
        QMessageBox::information(this, "Add from file", "The number of rows in both windows must be equal.", QMessageBox::Ok);
        return;
    }

    std::string file = QFileDialog::getOpenFileName(nullptr, "Add raw file", "").toUtf8().toStdString();

    if (file.empty())
        return;

    std::ifstream is_raw;
    is_raw.open(file);

    std::string deAll;
    std::string trAll;

    while(!is_raw.eof())
    {
        std::string str;
        std::string strTr;
        getline(is_raw, str);
        str = AreaUtf8(str).trim().toString();
        if (!str.empty())
        {
            std::string::size_type posTab = str.find('\t');
            if (posTab != std::string::npos)
            {
                strTr = AreaUtf8(str).subArea(posTab).trim().toString();
                str = AreaUtf8(str).subArea(0, posTab).trim().toString();
            }
        }
        deAll = deAll + str + '\n';
        trAll = trAll + strTr + '\n';
    }
    ui->plainTextEdit->appendPlainText(QString::fromStdString(deAll));
    ui->plainTextEdit_2->appendPlainText(QString::fromStdString(trAll));
}

void DialogRawEdit::on_pushButton_4_clicked()
{
    if (ui->plainTextEdit->document()->blockCount() != ui->plainTextEdit_2->document()->blockCount())
    {
        QMessageBox::information(this, "Import1 from file", "The number of rows in both windows must be equal.", QMessageBox::Ok);
        return;
    }

    std::string file = QFileDialog::getOpenFileName(nullptr, "Import1 raw file", "").toUtf8().toStdString();

    if (file.empty())
        return;

    LinesRamStream ls;
    if (!ls.loadFromFile(file))
    {
        QMessageBox::information(this, "Import1 from file", "Error open file", QMessageBox::Ok);
        return;
    }

    if (ls.empty())
        return;

    std::ofstream osLog;
    osLog.open(file + ".err.log");

    for (auto& line : ls)
        line = AreaUtf8(line).trim().toString();

    unsigned int num1 = ui->lineEdit_4->text().toInt();
    unsigned int num2 = ui->lineEdit_5->text().toInt();

    std::string deAll;
    std::string trAll;
    std::string tema;
    while (!ls.eof())
    {
        std::string line = ls.get();
        if (line.empty())
        {
            deAll = deAll + '\n';
            trAll = trAll + '\n';
            continue;
        }

        if (line[0] == '#')
        {
            if (line.size() < 3 || line[1] != ' ' || line[2] < '0' || line[2] > '9')
            {
                tema = AreaUtf8(line).subArea(1).trim().toString();
                continue;
            }
            AreaUtf8 au8(line);
            au8.seekg(2);
            const int kap = std::stoul(au8.getToken(".").toString());
            au8.getToken(".");
            const int subTems = std::stoul(au8.getToken(".").toString());
            au8.seekg(2);
            const std::string subTema = "#" + std::to_string(num1) + " " + std::to_string(num2) + " " + std::to_string(kap) + " " + std::to_string(subTems) + " " + au8.getRestArea().toString();

            if (!tema.empty())
            {
                deAll = deAll + "#" + std::to_string(num1) + " " + std::to_string(num2) + " " + std::to_string(kap) + " " + std::to_string(0) + " " + tema  + '\n';
                trAll = trAll + '\n';
                tema.clear();
            }
            deAll = deAll + subTema + '\n' + '\n';
            trAll = trAll + '\n' + '\n';
            if (ls.peek().empty())
                ls.get();
            else
            if (ls.peek()[0] == '#')
                continue;

            util::VectorString deVec;
            util::VectorString trVec;
            const size_t lineNum = ls.tellg() + 1;

            while (!(line = ls.get()).empty())
            {
                if (line[0] == '#')
                {
                    osLog << lineNum << ": error: de ended with #, size = " << deVec.size() << " - skiped tema: " << tema << std::endl;
                    ls.seekg(ls.tellg() - 1);
                    continue;
                }
                deVec.push_back(line);
            }

            while (!(line = ls.get()).empty())
            {
                if (line[0] == '#')
                {
                    osLog << lineNum << ": error: tr ended with #, size = " << trVec.size() << " - skiped tema: " << tema << std::endl;
                    ls.seekg(ls.tellg() - 1);
                    continue;
                }
                trVec.push_back(line);
            }

            if (deVec.size() != trVec.size())
            {
                osLog << lineNum << ": error: de size = " << deVec.size() << ", tr size = " << trVec.size() << " - skiped tema: " << tema << std::endl;
                continue;
            }

            for (size_t i = 0; i < deVec.size(); ++i)
            {
                deAll = deAll + deVec[i] + '\n';
                trAll = trAll + trVec[i] + '\n';
            }
            deAll = deAll + '\n';
            trAll = trAll + '\n';
        }
    }
    ui->plainTextEdit->appendPlainText(QString::fromStdString(deAll));
    ui->plainTextEdit_2->appendPlainText(QString::fromStdString(trAll));
}
