#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "testwindow.h"
#include "dialograwedit.h"
#include "dialogtypewort.h"
#include "utility.h"
#include <fstream>
#include <QFileDialog>
#include <QSettings>
#include <QDesktopServices>
#include <QMessageBox>
#include <QKeyEvent>
#include <QWheelEvent>
#include "qlistviewglossarydemodel.h"
#include "string_utf8.h"
#include <sstream>
#include "SerializeString.h"
#include "utilQtTypes.h"
#include "soundofwords.h"

static const char * const SettingsFirma = "OchinWassili";
static const char * const SettingsApp = "LernenDe";
static const char * const SettingsDictionaryPathSfx = "/dictionaryPath";

static const char * const FileEditLog = "trLog.html";

MainWindow::MainWindow(const char *aAccName, QWidget *parent)
    : QMainWindow(parent)
    , accName(aAccName)
    , settingsDictionaryPath(accName + SettingsDictionaryPathSfx)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSettings settings(SettingsFirma, SettingsApp);
    if (settings.contains(QString::fromStdString(settingsDictionaryPath)))
        pathDic = settings.value(QString::fromStdString(settingsDictionaryPath)).toString().toUtf8().toStdString();
    else
        pathDic = "../FrencDic";
    dicDe.setPath(pathDic);
    dicDe.load();
    this->setWindowTitle(QString::fromStdString(accName + " - " + pathDic + " - " + SettingsApp));

    testWin = new TestWindow(dicDe, this, this);

    model = new QListViewGlossaryDeModel(dicDe, this);
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

    keyAltL = new QShortcut(this);
    keyAltL->setKey(Qt::ALT + Qt::Key_L);
    connect(keyAltL, SIGNAL(activated()), this, SLOT(slotShortcutAltL()));

    ui->lineEdit_8->setStyleSheet("color: rgb(0, 0, 255)");
    ui->label_OptionStat_2->setStyleSheet("color: rgb(0, 0, 255)");

    ui->lineEdit_7->installEventFilter(this);
    ui->lineEdit_8->installEventFilter(this);
    ui->label_OptionStat->installEventFilter(this);
    ui->label_OptionStat_2->installEventFilter(this);
    ui->listView->installEventFilter(this);
    ui->textLog->installEventFilter(this);

    QFile file(QString::fromStdString(pathDic + "/" + FileEditLog));
    if (file.exists() && file.open(QIODevice::ReadOnly))
    {
        ui->textLog->setText(file.readAll());
        file.close();
    }

    soundWords = new SoundOfWords(pathDic, this);
}

MainWindow::~MainWindow()
{
    dicDe.save();

    QFile file(QString::fromStdString(pathDic + "/" + FileEditLog));
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(ui->textLog->toHtml().toUtf8());
        file.close();
    }

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
            std::string str = AreaUtf8(deVector[idx]).trim().toString();
            std::string strTr = AreaUtf8(trVector[idx]).trim().toString();
            if (str.empty())
                continue;
            if (str[0] == '#')
            {
                str = AreaUtf8(str).subArea(1).trim().toString();
                unsigned int A_C = 0;
                unsigned int book = 0;
                unsigned int kapitel = 0;
                unsigned int teil = 0;
                std::stringstream ss(str);
                multiScanFromStream(ss, A_C, book, kapitel, teil);
                block_Num = WortDe::creatBlock(A_C, book, kapitel, teil);
                tema = util::getline(ss);
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
    model->upDate();
}

void MainWindow::on_actionOpenDir_triggered()
{
    checkChangesCurrWd();
    std::string str = QFileDialog::getExistingDirectory(0, "Directory dictionary", QString::fromStdString(pathDic)).toUtf8().toStdString();
    if (str.empty())
        return;
    origIndex = -1; // clear index for new glossary

    pathDic = str;
    dicDe.saveClear();
    dicDe.setPath(pathDic);
    dicDe.load();
    model->upDate();
    if (dicDe.size() > 0)
        setNewIndex(0);

    QSettings settings(SettingsFirma, SettingsApp);
    settings.setValue(QString::fromStdString(settingsDictionaryPath), QString::fromStdString(pathDic));
    this->setWindowTitle(QString::fromStdString(accName + " - " + pathDic + " - " + SettingsApp));
}

static bool FingVerbOpt(WortDe &wd, AreaUtf8 &au8, const AreaUtf8::SymbolType symDelimeter)
{
    if (au8.find([symDelimeter](AreaUtf8::SymbolType sym) { return symDelimeter == sym; }))
    {
        wd.setNewPrasens3f(au8.subArea(0, au8.tellg()).trim().toString());
        ++au8;
        size_t pos2 = au8.tellg();
        if (au8.find([symDelimeter](AreaUtf8::SymbolType sym) { return symDelimeter == sym; }))
        {
            wd.setNewPrateritum(au8.subArea(pos2, au8.tellg() - pos2).trim().toString());
            ++au8;
            wd.setNewPerfect(au8.getRestArea().trim().toString());
        }
        else
        {
            wd.setNewPrateritum(au8.getRestArea().trim().toString());
            wd.setNewPerfect("");
        }
        return true;
    }
    return false;
}

void MainWindow::getWortDeToCurrWd()
{
    currWd.setNewTranslation(utilQt::lineEditToStdStr(ui->lineEdit_2));
    currWd.setNewWort(utilQt::lineEditToStdStr(ui->lineEdit));
    currWd.setNewExample(utilQt::lineEditToStdStr(ui->lineEdit_6));

    if (currWd.type() == WortDe::TypeWort::Noun || currWd.type() == WortDe::TypeWort::Combination)
        currWd.setNewPlural(utilQt::lineEditToStdStr(ui->lineEdit_5));
    else
    if (currWd.type() == WortDe::TypeWort::Verb)
    {
        std::string str = utilQt::lineEditToStdStr(ui->lineEdit_5);
        AreaUtf8 au8(str);
        au8.trim();
        if (au8.empty() ||
                (!FingVerbOpt(currWd, au8, UTF8_STRING_TO_SYMBOL("·")) && // parse from www.verbformen.ru
                 !FingVerbOpt(currWd, au8, UTF8_STRING_TO_SYMBOL(";"))
                 )
                )
        {
            currWd.setNewPrasens3f("");
            currWd.setNewPrateritum("");
            currWd.setNewPerfect("");
        }
    }
}

void MainWindow::setWortDe(WortDe wd)
{
    utilQt::strToLineEdit(ui->lineEdit, wd.wort());
    utilQt::strToLineEdit(ui->lineEdit_2, wd.translation());
    utilQt::strToLineEdit(ui->lineEdit_3, wd.prefix());
    utilQt::strToLineEdit(ui->lineEdit_4, wd.raw());
    utilQt::strToLineEdit(ui->lineEdit_6, wd.example());

    std::string labelWort;
    if (wd.freqIdx() > 0)
        labelWort = std::to_string(wd.freqIdx());

    ui->label->setText(QString::fromStdString(wd.blockToStr() + " = " + dicDe.tema(wd.block())
        + (wd.freqIdx() > 0 ? " [" + std::to_string(wd.freqIdx()) + "]" : "") ));

    std::string options;
    if (wd.type() == WortDe::TypeWort::Noun || wd.type() == WortDe::TypeWort::Combination)
    {
        options = wd.wortPl();
        ui->label_5->setText("plural:");
        ui->lineEdit_5->setEnabled(true);
    }
    else if (wd.type() == WortDe::TypeWort::Verb)
    {
        if (wd.vPrasens3f().empty() && wd.vPrateritum().empty() &&  wd.vPerfect().empty())
            options = "";
        else
            options = wd.vPrasens3f() + " ; " + wd.vPrateritum() + " ; " + wd.vPerfect();
        ui->label_5->setText("verb forms:");
        ui->lineEdit_5->setEnabled(true);
    } else {
        ui->label_5->setText("");
        ui->lineEdit_5->setEnabled(false);
    }
    utilQt::strToLineEdit(ui->lineEdit_5, options);

    std::string typeStr = WortDe::TypeWortToString(wd.type()) + "/" + WortDe::TypeWortToString(wd.type(), "ru");
    ui->pushButton->setText(QString::fromStdString(typeStr));

    setButtonEnable(wd);
}

void MainWindow::checkChangesCurrWd(const bool saveWithoutAsk)
{
    getWortDeToCurrWd();
    if (currWd == origWd)
        return;

    if (origIndex < 0 || static_cast<size_t>(origIndex) > dicDe.size())
        return;

    if (ui->checkBox->checkState() != Qt::Checked && !saveWithoutAsk)
    {
        if (origIndex == int(dicDe.size())) // add new Wort
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Created new Wort", "Add new Wort into Dic?",
            QMessageBox::Yes|QMessageBox::No);
            if (reply != QMessageBox::Yes)
                return;
        } else {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Vocabulary card changed", "Save changes?",
            QMessageBox::Yes|QMessageBox::No);
            if (reply != QMessageBox::Yes)
                return;
        }
    }

    if (currWd.block() == 0 && origWd.translation().empty() && !currWd.translation().empty())
        currWd.setNewBlock(dicDe.userBlockNum()); // TODO: refresh

    origWd = currWd;
    if (origIndex >= int(dicDe.size()))
    {
        const uint32_t freqIdx = origWd.freqIdx();
        if (freqIdx == 0)
            dicDe.add(origWd);
        else
        {
            uint32_t minFr = 0;
            uint32_t minIdx = 0;
            uint32_t maxFr = 0;
            uint32_t maxIdx = 0;
            for (size_t idx = 0; idx < dicDe.size(); ++idx)
            {
                const WortDe &wd = dicDe.at(idx);
                if (wd.freqIdx() > freqIdx)
                {
                    if (wd.freqIdx() < maxFr || maxFr == 0)
                    {
                        maxFr = wd.freqIdx();
                        maxIdx = idx;
                    }
                } else {
                    if (wd.freqIdx() > minFr)
                    {
                        minFr = wd.freqIdx();
                        minIdx = idx;
                    }
                }
            }

            if (minFr > 0)
            {
                origIndex = minIdx + 1;
                dicDe.insert(origIndex, origWd);
            }
            else if (maxFr > 0)
            {
                origIndex = maxIdx;
                dicDe.insert(origIndex, origWd);
            }
            else
            {
                dicDe.add(origWd);
            }
        }
    }
    else
    {
        dicDe[origIndex] = origWd;
    }
#ifndef _WIN32
    model->upDate(origIndex, origIndex);
#endif
}

void MainWindow::selectItem(const int idx)
{
    if (idx == origIndex)
        return;

    if (idx < 0 || static_cast<size_t>(idx) >= dicDe.size())
        return;

    checkChangesCurrWd();

    origIndex = idx;
    const WortDe& wd = dicDe[origIndex];
    origWd = wd;
    currWd = wd;
    setWortDe(currWd);

    if (ui->checkBox_AutoPlay->checkState() == Qt::Checked)
        PlayWord(currWd.wort());
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
    getWortDeToCurrWd();
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
    checkChangesCurrWd(true);
}

void MainWindow::on_pushButton_3_clicked()
{
    if (origIndex < 0 || static_cast<size_t>(origIndex) > dicDe.size())
        return;
    currWd = origWd;
    setWortDe(currWd);
}

void MainWindow::on_pushButton_4_clicked()
{
    getWortDeToCurrWd();
    currWd.setNewTypeWort(WortDe::TypeWort::Combination);
    setWortDe(currWd);
}

void MainWindow::on_pushButton_5_clicked()
{
    getWortDeToCurrWd();
    currWd.setNewTypeWort(WortDe::TypeWort::Noun);
    setWortDe(currWd);
}

void MainWindow::on_pushButton_6_clicked()
{
    getWortDeToCurrWd();
    currWd.setNewTypeWort(WortDe::TypeWort::Verb);
    setWortDe(currWd);
}

void MainWindow::on_pushButton_7_clicked()
{
    getWortDeToCurrWd();
    currWd.setNewTypeWort(WortDe::TypeWort::Adjective);
    setWortDe(currWd);
}

void MainWindow::slotShortcutAltW()
{
    getWortDeToCurrWd();
    currWd.setNewTypeWort(WortDe::TypeWort::Combination);
    setWortDe(currWd);
}

void MainWindow::slotShortcutAltN()
{
    getWortDeToCurrWd();
    currWd.setNewTypeWort(WortDe::TypeWort::Noun);
    setWortDe(currWd);
}

void MainWindow::slotShortcutAltV()
{
    getWortDeToCurrWd();
    currWd.setNewTypeWort(WortDe::TypeWort::Verb);
    setWortDe(currWd);
}

void MainWindow::slotShortcutAltA()
{
    getWortDeToCurrWd();
    currWd.setNewTypeWort(WortDe::TypeWort::Adjective);
    setWortDe(currWd);
}

void MainWindow::slotShortcutAltL()
{
    checkChangesCurrWd();
    testWin->show();
}

void MainWindow::on_pushButton_8_clicked()
{
    getWortDeToCurrWd();
    webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::lingvo);
}

void MainWindow::on_pushButton_9_clicked()
{
    getWortDeToCurrWd();
    webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::yandex);
}

void MainWindow::on_pushButton_10_clicked()
{
    getWortDeToCurrWd();
    webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::google);
}

void MainWindow::on_pushButton_11_clicked()
{
    getWortDeToCurrWd();
    std::string raw = utilQt::lineEditToStdStr(ui->lineEdit_4);
    currWd.parseRawLine(raw, currWd.translation(), currWd.block(), currWd.type());
    setWortDe(currWd);
}

void MainWindow::on_actionToRaw_triggered()
{
    // TODO: add file selection
    std::ofstream os;
    os.open(pathDic + "/" + "raw_dic_out.txt");
    unsigned int lastBlockNum = 0;
    for (size_t i = 0; i < dicDe.size(); ++i)
    {
        const WortDe &wd = dicDe[i];
        if (wd.block() != lastBlockNum)
        {
            unsigned int h1, h2, h3, h4;
            WortDe::blockToUint_4(wd.block(), h1, h2, h3, h4);
            os << "\n#" << h1 << " " << h2 << " " << h3 << " " << h4 << " "
                << dicDe.tema(wd.block()) << "\n" << std::endl;
            lastBlockNum = wd.block();
        }
        os << wd.raw() << "\t" << wd.translation() << std::endl;
    }
}

void MainWindow::on_pushButton_12_clicked()
{
    getWortDeToCurrWd();
    webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::dwds);
}

void MainWindow::on_pushButton_13_clicked()
{
    getWortDeToCurrWd();
    webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::VerbF);
}

void MainWindow::on_pushButton_14_clicked()
{
    getWortDeToCurrWd();
    webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::pons);
}

void MainWindow::on_pushButton_15_clicked()
{
    getWortDeToCurrWd();
    webTr.allTranslate(currWd.prefixAndWort(), WebTranslation::WebSite::yandex);
}

void MainWindow::on_pushButton_16_clicked()
{
    getWortDeToCurrWd();
    webTr.allTranslate(currWd.prefixAndWort(), WebTranslation::WebSite::google);
}

void MainWindow::on_pushButton_17_clicked()
{
    const std::string str = utilQt::lineEditToStdStr(ui->lineEdit_7);
    const size_t idx = dicDe.find(str, origIndex + 1);
    if (idx == dicDe.size())
    {
        clearCurrWord();
        // QMessageBox::information(this, utilQt::strToQt(str), "nicht finden", QMessageBox::Yes);
        return;
    }

    setNewIndex(idx);
}

void MainWindow::statWordClear()
{
    ui->lineEdit_8->setText("");
    ui->label_OptionStat->setText("");
    ui->label_OptionStat_2->setText("");
    statFound.clear();
    statFoundForm.clear();
}

void MainWindow::findStatWord(const std::string &str)
{
    if (str.empty())
    {
        statWordClear();
        return;
    }

    const bool useFullWord = ui->checkBox_FullWord->checkState() == Qt::Checked;

    statFound.clear();
    uint32_t lastPos = 0;
    for (int i = 0; i < 100; ++i)
    {
        uint8_t option = 0;
        String255Iterator it = dicDe.findStatDic(str, lastPos, useFullWord, option);
        if (!it->valid())
            break;
        statFound.add(it.getIdx(), option, it->c_str());
        lastPos = it.getIdx() + 1;
    }

    statFoundForm.clear();
    lastPos = 0;
    for (int i = 0; i < 100; ++i)
    {
        uint8_t optionForm = 0;
        uint32_t idxDic = 0;
        String255Iterator it = dicDe.findStatForm(str, lastPos, optionForm, useFullWord, idxDic);
        if (!it->valid())
            break;
        uint8_t optionDic = 0;
        std::string strDic = dicDe.atStatDic(idxDic, optionDic).c_str();
        statFoundForm.add(idxDic, optionDic, strDic, it.getIdx(), optionForm, it->c_str());
        lastPos = it.getIdx() + 1;
    }

    showFoundStatWord();
}

void MainWindow::showFoundStatWord()
{
    if (statFound.empty() && statFoundForm.empty())
    {
        statWordClear();
        return;
    }

    const bool useForm = ui->checkBoxUseForm->checkState() == Qt::Checked;
    FoundItemsIdx &currStatFound = useForm ? statFoundForm : statFound;

    if (!currStatFound.empty())
    {
        ui->lineEdit_8->setText(QString::fromStdString(currStatFound.getStr()));
        std::string strOption;
        if (!useForm)
            strOption = std::to_string(currStatFound.getPos() + 1) + '/'
                    + (currStatFound.size() > 99 ? "99+" : std::to_string(currStatFound.size())) + " ";

        strOption = strOption + "= " + std::to_string(currStatFound.getWordIdx()) + " - "
                + WortDe::TypeWortToString(static_cast<WortDe::TypeWort>(currStatFound.getType()));
        ui->label_OptionStat->setText(QString::fromStdString(strOption));

        const size_t idx = dicDe.findByWordIdx(currStatFound.getWordIdx(), 0);
        currStatFound.setGlossaryIndex(idx == dicDe.size() ? -1 : idx);

        ui->lineEdit_8->setStyleSheet(currStatFound.getGlossaryIndex() < 0 ? "color: rgb(0, 0, 255)" : "color: rgb(50, 50, 50)");

        if (ui->checkBox_AutoSearch->checkState() == Qt::Checked) {
            if (currStatFound.getGlossaryIndex() < 0)
                clearCurrWord();
            else
                setNewIndex(currStatFound.getGlossaryIndex());
        }
    } else {
        ui->lineEdit_8->setText("");
        ui->label_OptionStat->setText("");
    }

    if (!statFoundForm.empty())
    {
        std::string strFormOption = std::to_string(statFoundForm.getPos() + 1) + '/'
                + (statFoundForm.size() > 99 ? "99+" : std::to_string(statFoundForm.size()))
                + " - " + statFoundForm.getFormStr() + " = "
                + std::to_string(statFoundForm.getFormWordIdx()) + " - "
                + StatWords::formTypeToCStr(statFoundForm.getFormType());
        if (!useForm)
        {
            strFormOption = strFormOption + " ["
                + statFoundForm.getStr() + "/" + std::to_string(statFoundForm.getWordIdx()) + "]";
        }
        ui->label_OptionStat_2->setText(QString::fromStdString(strFormOption));
    } else {
        ui->label_OptionStat_2->setText("");
    }
}

void MainWindow::on_lineEdit_7_textChanged(const QString &arg1)
{
    const std::string str = utilQt::strToStd(arg1);
    if (str.empty())
    {
        statWordClear();
        return;
    }

    const size_t idx = dicDe.find(str, 0);
    if (idx == dicDe.size())
    {
        ui->lineEdit_7->setStyleSheet("color: rgb(255, 0, 0)");
    } else {
        ui->lineEdit_7->setStyleSheet("color: rgb(0, 0, 0)");
        if (ui->checkBox_AutoSearch->checkState() != Qt::Checked)
            setNewIndex(idx);
    }

    findStatWord(str);
}

void MainWindow::setNewIndex(int idx)
{
    if (dicDe.empty())
    {
        clearCurrWord();
        return;
    }

    if (idx < 0)
        idx = 0;
    else if (idx >= int(dicDe.size()))
        idx = dicDe.size() - 1;

    QModelIndex index = model->creatNewIndex(idx);
    if ( index.isValid() ) {
        ui->listView->setCurrentIndex(index);
        selectItem(idx);
    }
}

void MainWindow::on_pushButton_18_clicked()
{
    const std::string str = utilQt::lineEditToStdStr(ui->lineEdit_7);
    size_t idx = dicDe.find(str, 0);
    if (idx == dicDe.size())
        idx = 0;

    setNewIndex(idx);

    findStatWord(str);
}

void MainWindow::on_pushButton_19_clicked()
{
    getWortDeToCurrWd();
    if (currWd.type() != WortDe::TypeWort::Noun)
        currWd.setNewTypeWort(WortDe::TypeWort::Noun);
    currWd.setNewArtikel(WortDe::TypeArtikel::Der);
    setWortDe(currWd);
}

void MainWindow::on_pushButton_20_clicked()
{
    getWortDeToCurrWd();
    if (currWd.type() != WortDe::TypeWort::Noun)
        currWd.setNewTypeWort(WortDe::TypeWort::Noun);
    currWd.setNewArtikel(WortDe::TypeArtikel::Das);
    setWortDe(currWd);
}

void MainWindow::on_pushButton_21_clicked()
{
    getWortDeToCurrWd();
    if (currWd.type() != WortDe::TypeWort::Noun)
        currWd.setNewTypeWort(WortDe::TypeWort::Noun);
    currWd.setNewArtikel(WortDe::TypeArtikel::Die);
    setWortDe(currWd);
}

void MainWindow::on_pushButton_22_clicked()
{
    getWortDeToCurrWd();
    if (currWd.type() != WortDe::TypeWort::Noun)
        currWd.setNewTypeWort(WortDe::TypeWort::Noun);
    currWd.setNewArtikel(WortDe::TypeArtikel::Pl);
    currWd.setNewPlural("Pl.");
    setWortDe(currWd);
}

void MainWindow::on_pushButton_24_clicked()
{
    getWortDeToCurrWd();
    if (currWd.type() != WortDe::TypeWort::Noun)
        currWd.setNewTypeWort(WortDe::TypeWort::Noun);
    currWd.setNewArtikel(WortDe::TypeArtikel::Der_Die);
    setWortDe(currWd);
}

void MainWindow::on_pushButton_25_clicked()
{
    getWortDeToCurrWd();
    if (currWd.type() != WortDe::TypeWort::Noun)
        currWd.setNewTypeWort(WortDe::TypeWort::Noun);
    currWd.setNewArtikel(WortDe::TypeArtikel::Der_Das);
    setWortDe(currWd);
}

void MainWindow::on_pushButton_26_clicked()
{
    getWortDeToCurrWd();
    if (currWd.type() != WortDe::TypeWort::Noun)
        currWd.setNewTypeWort(WortDe::TypeWort::Noun);
    currWd.setNewArtikel(WortDe::TypeArtikel::ProperNoun);
    setWortDe(currWd);
}

void MainWindow::on_pushButton_23_clicked()
{
    getWortDeToCurrWd();
    if (currWd.type() != WortDe::TypeWort::Verb)
    {
        currWd.setNewTypeWort(WortDe::TypeWort::Verb);
        currWd.setNewSich(true);
    }
    else
        currWd.setNewSich(!currWd.hasSich());
    setWortDe(currWd);
}

void MainWindow::setButtonEnable(WortDe wd)
{
    const bool verb = wd.type() == WortDe::TypeWort::Verb || wd.type() == WortDe::TypeWort::None;
    const bool artikl = wd.type() == WortDe::TypeWort::Noun || wd.type() == WortDe::TypeWort::None;

    ui->pushButton_19->setStyleSheet(wd.artikel() == WortDe::TypeArtikel::Der ? "font-weight: bold; color: blue" : "");
    ui->pushButton_20->setStyleSheet(wd.artikel() == WortDe::TypeArtikel::Das ? "font-weight: bold; color: green" : "");
    ui->pushButton_21->setStyleSheet(wd.artikel() == WortDe::TypeArtikel::Die ? "font-weight: bold; color: red" : "");
    ui->pushButton_22->setStyleSheet(wd.artikel() == WortDe::TypeArtikel::Pl  ? "font-weight: bold" : "");
    ui->pushButton_24->setStyleSheet(wd.artikel() == WortDe::TypeArtikel::Der_Die  ? "font-weight: bold" : "");
    ui->pushButton_25->setStyleSheet(wd.artikel() == WortDe::TypeArtikel::Der_Das  ? "font-weight: bold" : "");
    ui->pushButton_26->setStyleSheet(wd.artikel() == WortDe::TypeArtikel::ProperNoun  ? "font-weight: bold" : "");

    ui->pushButton_23->setStyleSheet(wd.hasSich()  ? "font-weight: bold" : "");

    ui->pushButton_13->setEnabled(verb);
    ui->pushButton_23->setEnabled(verb);

    ui->pushButton_19->setEnabled(artikl);
    ui->pushButton_20->setEnabled(artikl);
    ui->pushButton_21->setEnabled(artikl);
    ui->pushButton_22->setEnabled(artikl);
    ui->pushButton_24->setEnabled(artikl);
    ui->pushButton_25->setEnabled(artikl);
    ui->pushButton_26->setEnabled(artikl);
}

void MainWindow::on_pushButton_27_clicked()
{
    getWortDeToCurrWd();
    std::string str = currWd.wort();
    AreaUtf8 au8(str);
    auto prfx = au8.getToken(" ");
    auto newWort = au8.getRestArea().trim();
    if (newWort.empty())
        return;

    currWd.setNewPrefix(currWd.rawPrefix() + " " + prfx.toString());
    currWd.setNewWort(newWort.toString());
    setWortDe(currWd);
}

void MainWindow::on_pushButton_28_clicked()
{
    getWortDeToCurrWd();
    std::string str = currWd.rawPrefix();
    AreaUtf8 au8(str);
    std::vector<AreaUtf8> tokens;
    au8.getAllTokens(tokens, " ");
    if (tokens.empty())
        return;

    auto prfxEnd = tokens.back();
    currWd.setNewPrefix(au8.subArea(0, str.size() - prfxEnd.size()).trim().toString());
    currWd.setNewWort(prfxEnd.toString() + " " +  currWd.wort());
    setWortDe(currWd);
}

void MainWindow::on_actionSave_triggered()
{
    checkChangesCurrWd();
    dicDe.save();
}

void MainWindow::on_actionTest_words_triggered()
{
    checkChangesCurrWd();
    testWin->setNewWort();
    testWin->show();
}

void MainWindow::on_pushButton_29_clicked()
{
    setNewIndex(origIndex + 1);
    ui->listView->setFocus();
}

void MainWindow::on_pushButton_30_clicked()
{
    setNewIndex(origIndex - 1);
    ui->listView->setFocus();
}

void MainWindow::clearCurrWord()
{
    checkChangesCurrWd();
    origIndex = dicDe.size();
    origWd = WortDe();
    currWd = origWd;
    setWortDe(currWd);
}

void MainWindow::addNewWortFromSearch()
{
    checkChangesCurrWd();
    origIndex = dicDe.size();
    origWd = WortDe();
    currWd = origWd;
    currWd.parseRawLine(utilQt::lineEditToStdStr(ui->lineEdit_7), "", dicDe.userBlockNum());
    setWortDe(currWd);
}

void MainWindow::addNewWortFromStatSearch()
{
    FoundItemsIdx &currStatFound = ui->checkBoxUseForm->checkState() == Qt::Checked ? statFoundForm : statFound;
    if (currStatFound.empty())
        return;

    checkChangesCurrWd();
    origIndex = dicDe.size();
    origWd = WortDe();
    currWd = origWd;

    WortDe::TypeWort tw = currStatFound.getType() > uint8_t(WortDe::TypeWort::None)
                       && currStatFound.getType() < uint8_t(WortDe::TypeWort::_last_one)
                       ? WortDe::TypeWort(currStatFound.getType()) : WortDe::TypeWort::None;
    currWd.parseRawLine(currStatFound.getStr(), "", dicDe.userBlockNum(), tw);
    currWd.setNewFreqIdx(currStatFound.getWordIdx());
    setWortDe(currWd);
}

void MainWindow::on_pushButton_31_clicked()
{
    addNewWortFromSearch();
    if (ui->checkBox_NeedTr->checkState() == Qt::Checked)
        webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::lingvo);
}

void MainWindow::on_pushButton_32_clicked()
{
    FoundItemsIdx &currStatFound = ui->checkBoxUseForm->checkState() == Qt::Checked ? statFoundForm : statFound;
    if (currStatFound.empty())
        return;

    if (currStatFound.getGlossaryIndex() >= 0)
    {
        QMessageBox::information(this, utilQt::strToQt(currStatFound.getStr()), "Already in the dictionary", QMessageBox::Yes);
        return;
    }

    addNewWortFromStatSearch();
    if (ui->checkBox_NeedTr->checkState() == Qt::Checked)
        webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::lingvo);
}

void MainWindow::on_actionExport_to_text_triggered()
{
    // TODO: add file selection
    std::ofstream os;
    os.open(pathDic + "/" + "dic_tab.txt");
    for (size_t i = 0; i < dicDe.size(); ++i)
        os << dicDe[i].exportToStr(true)  << std::endl;
    os.close();

    /*
    // search for duplicate words
    os.open(pathDic + "/" + "dic_doubles.txt");
    std::vector<WortDe> newGloss;
    for (size_t i = 0; i < dicDe.size(); ++i)
        newGloss.push_back(dicDe[i]);

    std::string lastWord;
    for (size_t pos = 0; pos < newGloss.size(); ++pos)
    {
        std::string dupl;
        const std::string firstWort = AreaUtf8(newGloss[pos].wort()).getToken().toString();
        for (size_t i = pos + 1; i < newGloss.size(); ++i)
        {
            const std::string nextWort = AreaUtf8(newGloss[i].wort()).getToken().toString();
            if (firstWort == nextWort)
            {
                dupl += newGloss[i].exportToStr(true) + "\n";
                newGloss.erase(newGloss.begin() + i);
                --i;
            }
        }
        if (!dupl.empty())
        {
            os << newGloss[pos].exportToStr(true) << "\n" << dupl;
        }
    }
    */
}

void MainWindow::on_actionClear_statistic_triggered()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clearing statistics.", "Clear word learning statistic?",
    QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
        dicDe.clearAllStatistic();
}

void MainWindow::on_dudenButton_33_clicked()
{
    getWortDeToCurrWd();
    webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::duden);
}


void MainWindow::on_wiktionaryButton_33_clicked()
{
    getWortDeToCurrWd();
    webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::wiktionary);
}


void MainWindow::on_leipzigButton_clicked()
{
    getWortDeToCurrWd();
    webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::leipzig);
}


void MainWindow::on_thefreeDicButton_clicked()
{
    getWortDeToCurrWd();
    webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::thefreeDic);
}


void MainWindow::on_actionimport_options_triggered()
{
    checkChangesCurrWd();
    std::string pathImportDir = QFileDialog::getExistingDirectory(0, "Dictionary directory for importing translations", "").toUtf8().toStdString();
    if (pathImportDir.empty())
        return;

    GlossaryDe impDicDe;
    impDicDe.setPath(pathImportDir);
    impDicDe.load(false);
    dicDe.importTr(impDicDe);

    model->upDate();
}


void MainWindow::on_actionImport_statistic_triggered()
{
    checkChangesCurrWd();
    std::string pathImportDir = QFileDialog::getExistingDirectory(0, "Dictionary directory for importing statistics", "").toUtf8().toStdString();
    if (pathImportDir.empty())
        return;

    GlossaryDe impDicDe;
    impDicDe.setPath(pathImportDir);
    impDicDe.load(false);
    dicDe.importStat(impDicDe);
}


void MainWindow::nextFoundStatWord(int delta)
{
    FoundItemsIdx &currStatFound = ui->checkBoxUseForm->checkState() == Qt::Checked ? statFoundForm : statFound;

    if (currStatFound.empty())
        return;

    if (delta < 0)
        --currStatFound;
    else
        ++currStatFound;
    showFoundStatWord();
}


void MainWindow::on_checkBox_AutoSearch_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        showFoundStatWord();
        ui->pushButton_32->setEnabled(true);
        ui->pushButton_31->setEnabled(false);
        ui->pushButton_17->setEnabled(false);
    } else {
        ui->pushButton_32->setEnabled(false);
        ui->pushButton_31->setEnabled(true);
        ui->pushButton_17->setEnabled(true);
    }
}


bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = (QKeyEvent *)event;
        const auto key = keyEvent->key();
        const auto mod = keyEvent->modifiers();
        const auto maskCtrlAltShift = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier;
        if (target == ui->lineEdit_7)
        {
            if ((key == Qt::Key_Return || key == Qt::Key_Enter) && (mod & maskCtrlAltShift) == Qt::NoModifier)
            {
                FoundItemsIdx &currStatFound = ui->checkBoxUseForm->checkState() == Qt::Checked ? statFoundForm : statFound;
                if (!currStatFound.empty())
                    ui->lineEdit_7->setText(QString::fromStdString(currStatFound.getStr()));
            }
            else if (key == Qt::Key_Space && (mod & maskCtrlAltShift) == Qt::ControlModifier)
            {
                addWordToList();
            }
            else if (key == Qt::Key_Up && (mod & maskCtrlAltShift) == Qt::NoModifier)
                nextFoundStatWord(-1);
            else if (key == Qt::Key_Down && (mod & maskCtrlAltShift) == Qt::NoModifier)
                nextFoundStatWord(+1);
            else
                return QMainWindow::eventFilter(target, event);
            return true;
        }
        else if (target == ui->listView)
        {
            if (key == Qt::Key_Up && (mod & maskCtrlAltShift) == Qt::NoModifier)
                setNewIndex(origIndex - 1);
            else if (key == Qt::Key_Down && (mod & maskCtrlAltShift) == Qt::NoModifier)
                setNewIndex(origIndex + 1);
            else if (key == Qt::Key_Home && (mod & maskCtrlAltShift) == Qt::NoModifier)
                setNewIndex(0);
            else if (key == Qt::Key_End && (mod & maskCtrlAltShift) == Qt::NoModifier)
                setNewIndex(dicDe.empty() ? 0 : dicDe.size() - 1);
            else
                return QMainWindow::eventFilter(target, event);
            return true;
        }
    }
    else if (event->type() == QEvent::Wheel)
    {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
        auto delta = wheelEvent->angleDelta() / 8;

        if (target == ui->lineEdit_8 || target == ui->label_OptionStat
                || (target == ui->lineEdit_7 && ui->checkBox_AutoSearch->checkState() == Qt::Checked))
        {
            if (delta.y() > 0)
                nextFoundStatWord(-1);
            else if (delta.y() < 0)
                nextFoundStatWord(+1);
            else
                return QMainWindow::eventFilter(target, event);
            return true;
        }
        else if (target == ui->label_OptionStat_2)
        {
            if (delta.y() > 0)
            {
                --statFoundForm;
                showFoundStatWord();
            }
            else if (delta.y() < 0)
            {
                ++statFoundForm;
                showFoundStatWord();
            }
            else
                return QMainWindow::eventFilter(target, event);
            return true;
        }
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        const auto button = mouseEvent->button();
        if (button == Qt::MiddleButton)
        {
            if (target == ui->textLog || target == ui->lineEdit_7)
            {
                addWordToList();
                return true;
            }
        }
        else
            return QMainWindow::eventFilter(target, event);
    }

    return QMainWindow::eventFilter(target, event);
}

void MainWindow::addWordToList()
{
    getWortDeToCurrWd();
    if (!currWd.wort().empty() && !currWd.translation().empty())
    {
        // const std::string bold = "<span style=\" font-weight:600;\">";
        // const std::string italic = "<span style=\" font-style:italic;\">";
        const std::string blue = "<span style=\" color:#4000A0;\">";
        const std::string endSpan = "</span>";
        const std::string prefix = currWd.prefix();
        QString value;
        if (prefix.empty())
        {
            value = QString::fromStdString(blue + currWd.wort() + endSpan + " - " + currWd.translation());
        } else {
            value = QString::fromStdString(prefix + " " + blue + currWd.wort() + endSpan + " - " + currWd.translation());
        }
        ui->textLog->append(value);
    }
}

void MainWindow::on_checkBoxUseForm_stateChanged(int)
{
    showFoundStatWord();
}


void MainWindow::on_buttonAddWordToList_clicked()
{
    addWordToList();
}


void MainWindow::on_checkBox_FullWord_stateChanged(int /* arg1 */)
{
    const std::string strFind = utilQt::lineEditToStdStr(ui->lineEdit_7);
    findStatWord(strFind);
}


void MainWindow::on_textLog_selectionChanged()
{
    const QString qstr = ui->textLog->textCursor().selectedText();
    if (!qstr.isEmpty() && qstr != ui->lineEdit_7->text())
    {
        if (ui->checkBox_AutoTranslation->checkState() == Qt::Checked)
        {
            static std::string oldWord;
            ui->lineEdit_7->setText(qstr);
            if (ui->checkBox_AutoWebTranslation->checkState() == Qt::Checked && !currWd.wort().empty() && currWd.wort() != oldWord)
            {
                oldWord = currWd.wort();
                webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::lingvo);
            }
        } else if (ui->checkBox_AutoWebTranslation->checkState() == Qt::Checked)
            webTr.wortTranslate(qstr.toStdString(), WebTranslation::WebSite::lingvo);
    }
}


void MainWindow::on_pushButton_33_clicked()
{
    getWortDeToCurrWd();
    webTr.wortTranslate(currWd.wort(), WebTranslation::WebSite::woerter);
}

void MainWindow::on_pushButton_34_clicked()
{
    getWortDeToCurrWd();
    if (!currWd.wort().empty())
        PlayWord(currWd.wort());
}

void MainWindow::PlayWord(const std::string &word)
{
    AreaUtf8 au8(word);
    au8.trim();
    AreaUtf8 word1 = au8.getToken();
    if (word1.empty())
        return;
    soundWords->play(word1.toString());
}
