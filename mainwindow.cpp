#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialograwedit.h"
#include "dialogtypewort.h"
#include "utility.h"
#include <fstream>
#include <QFileDialog>
#include <QSettings>
#include "qlistviewglossarydemodel.h"
#include "string_utf8.h"
#include <sstream>
#include "SerializeString.h"

static const char *SettingsFirma = "OchinWassili";
static const char *SettingsApp = "LernenDe";
static const char *SettingsDictionaryPath = "LernenDe/dictionaryPath";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSettings settings(SettingsFirma, SettingsApp);
    if (settings.contains(SettingsDictionaryPath))
        pathDic = settings.value(SettingsDictionaryPath).toString().toUtf8().toStdString();
    dicDe.setPath(pathDic);
    dicDe.load();

    this->model = new QListViewGlossaryDeModel(dicDe);
    this->ui->listView->setModel(model);

    keyAltW = new QShortcut(this);
    keyAltW->setKey(Qt::ALT + Qt::Key_W);
    connect(keyAltW, SIGNAL(activated()), this, SLOT(slotShortcutAltW()));

    keyAltN = new QShortcut(this);
    keyAltN->setKey(Qt::ALT + Qt::Key_N);
    connect(keyAltN, SIGNAL(activated()), this, SLOT(slotShortcutAltN()));

    keyAltV = new QShortcut(this);
    keyAltV->setKey(Qt::ALT + Qt::Key_V);
    connect(keyAltV, SIGNAL(activated()), this, SLOT(slotShortcutAltV()));

    keyAltA = new QShortcut(this);
    keyAltA->setKey(Qt::ALT + Qt::Key_A);
    connect(keyAltA, SIGNAL(activated()), this, SLOT(slotShortcutAltA()));
}

MainWindow::~MainWindow()
{
    dicDe.save();
    delete ui;
}


void MainWindow::on_actionRaw_Text_triggered()
{
    static WortDe::TypeWort typeWort = WortDe::TypeWort::None;
    DialogRawEdit* pdlg = new DialogRawEdit(this);
    pdlg->setTypeWort(typeWort);
    if (pdlg->exec() == QDialog::Accepted)
    {
        typeWort = pdlg->getTypeWort();

        util::VectorString deVector;
        util::VectorString trVector;

        util::VectorFromString(deVector, pdlg->getDeText().toUtf8().toStdString());
        util::VectorFromString(trVector, pdlg->getTranslationText().toUtf8().toStdString());

        std::string tema;
        unsigned int block_Num = pdlg->getBlockNum(tema);
        unsigned int block_Num_Old = 0;
        std::ofstream osTema;
        osTema.open(pathDic + "\\" + "logTema.txt", std::ios_base::app);

        for (size_t idx = 0; idx < deVector.size(); ++idx)
        {
            std::string str = substrWithoutSideSpaces(deVector[idx]);
            std::string strTr = substrWithoutSideSpaces(trVector[idx]);
            if (str.empty())
                continue;
            if (str[0] == '#')
            {
                str = substrWithoutSideSpaces(str, 1);
                int A_C = 0;
                int book = 0;
                int kapitel = 0;
                int teil = 0;
                std::stringstream ss(str);
                multiScanFromStream(ss, A_C, book, kapitel, teil);
                block_Num = (A_C << 24) + (book << 16) + (kapitel << 8) + teil;
                std::getline(ss, tema);
                osTema << std::hex << block_Num << "\t" << tema << std::endl;
                block_Num_Old = block_Num;
                continue;
            }

            WortDe wd;
            wd.parseRawLine(deVector[idx], idx < trVector.size() ? trVector[idx] : std::string(), block_Num, typeWort);
            if (wd.raw().empty())
                continue;
            if (block_Num_Old != block_Num)
            {
                osTema << std::hex << block_Num << "\t" << tema << std::endl;
                block_Num_Old = block_Num;
            }
            dicDe.add(wd);
        }
        dicDe.save();
    }
    delete pdlg;
}


void MainWindow::on_actionConvert_triggered()
{
// TODO:    ConvertFromTextFile();
}

void MainWindow::on_actionOpenDir_triggered()
{
    std::string str = QFileDialog::getExistingDirectory(0, "Directory dictionary", QString::fromStdString(pathDic)).toUtf8().toStdString();
    if (str.empty())
        return;

    pathDic = str;
    dicDe.saveClear();
    dicDe.setPath(pathDic);
    dicDe.load();
    // TODO: show

    QSettings settings(SettingsFirma, SettingsApp);
    settings.setValue(SettingsDictionaryPath, QString::fromStdString(pathDic));
}

void MainWindow::setWortDe(WortDe wd)
{
    ui->lineEdit->setText(QString::fromStdString(wd.wort()));
    ui->lineEdit_2->setText(QString::fromStdString(wd.translation()));
    ui->lineEdit_3->setText(QString::fromStdString(wd.prefix()));
    ui->lineEdit_4->setText(QString::fromStdString(wd.raw()));

    std::string options;
    if (wd.type() == WortDe::TypeWort::Noun)
    {
        options = wd.wortPl();
    }
    else if (wd.type() == WortDe::TypeWort::Verb)
    {
        options = wd.vPrasens3f() + "; " + wd.vPrateritum() + "; " + wd.vPerfect();
    }
    ui->lineEdit_5->setText(QString::fromStdString(options));

    std::string typeStr = WortDe::TypeWortToString(wd.type()) + "/" + WortDe::TypeWortToString(wd.type(), "ru");
    ui->pushButton->setText(QString::fromStdString(typeStr));
}

void MainWindow::selectItem(int idx)
{
    if (currIndex == idx)
        return;
    currIndex = idx;
    if (idx < 0 || static_cast<size_t>(idx) >= dicDe.size())
        return;

    // TODO: check currWd changes
    if (ui->checkBox->checkState() == Qt::Checked)
    {
        if (origIndex >= 0 || static_cast<size_t>(origIndex) < dicDe.size())
        {
            origWd = currWd;
            dicDe.at(origIndex) = origWd;
        }
    }

    WortDe wd = dicDe[idx];
    setWortDe(wd);
    currWd = wd;
    origIndex = currIndex;
    origWd = wd;
}

void MainWindow::on_listView_activated(const QModelIndex &index)
{
    selectItem(index.row());
}

void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    selectItem(index.row());
}

void MainWindow::on_pushButton_clicked()
{
    DialogTypeWort* pdlg = new DialogTypeWort(this);
    pdlg->setTypeWort(currWd.type());
    if (pdlg->exec() == QDialog::Accepted)
    {
        currWd.setNewTypeWort(pdlg->getTypeWort());
        setWortDe(currWd);
    }
    delete pdlg;
}

void MainWindow::on_pushButton_2_clicked()
{
    if (origIndex < 0 || static_cast<size_t>(origIndex) >= dicDe.size())
        return;
    origWd = currWd;
    dicDe.at(origIndex) = origWd;
}

void MainWindow::on_pushButton_3_clicked()
{
    if (origIndex < 0 || static_cast<size_t>(origIndex) >= dicDe.size())
        return;
    currWd = origWd;
    setWortDe(currWd);
}

void MainWindow::on_pushButton_4_clicked()
{
    currWd.setNewTypeWort(WortDe::TypeWort::Combination);
    setWortDe(currWd);
}

void MainWindow::on_pushButton_5_clicked()
{
    currWd.setNewTypeWort(WortDe::TypeWort::Noun);
    setWortDe(currWd);
}

void MainWindow::on_pushButton_6_clicked()
{
    currWd.setNewTypeWort(WortDe::TypeWort::Verb);
    setWortDe(currWd);
}

void MainWindow::on_pushButton_7_clicked()
{
    currWd.setNewTypeWort(WortDe::TypeWort::Adjective);
    setWortDe(currWd);
}

void MainWindow::slotShortcutAltW()
{
    currWd.setNewTypeWort(WortDe::TypeWort::Combination);
    setWortDe(currWd);
}

void MainWindow::slotShortcutAltN()
{
    currWd.setNewTypeWort(WortDe::TypeWort::Noun);
    setWortDe(currWd);
}

void MainWindow::slotShortcutAltV()
{
    currWd.setNewTypeWort(WortDe::TypeWort::Verb);
    setWortDe(currWd);
}

void MainWindow::slotShortcutAltA()
{
    currWd.setNewTypeWort(WortDe::TypeWort::Adjective);
    setWortDe(currWd);
}

