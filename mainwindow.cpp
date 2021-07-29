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
#include "qlistviewglossarydemodel.h"
#include "string_utf8.h"
#include <sstream>
#include "SerializeString.h"
#include "utilQtTypes.h"

static const char *SettingsFirma = "OchinWassili";
static const char *SettingsApp = "LernenDe";
static const char *SettingsDictionaryPathSfx = "/dictionaryPath";

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
        pathDic = "../Line1_B2";
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
    }
    else if (wd.type() == WortDe::TypeWort::Verb)
    {
        if (wd.vPrasens3f().empty() && wd.vPrateritum().empty() &&  wd.vPerfect().empty())
            options = "";
        else
            options = wd.vPrasens3f() + " ; " + wd.vPrateritum() + " ; " + wd.vPerfect();
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
            reply = QMessageBox::question(this, "Wortschatzkarte geändert", "Änderungen speichern?",
            QMessageBox::Yes|QMessageBox::No);
            if (reply != QMessageBox::Yes)
                return;
        }
    }

    origWd = currWd;
    if (origIndex >= int(dicDe.size()))
        dicDe.add(origWd);
    else
        dicDe[origIndex] = origWd;
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
        QMessageBox::information(this, utilQt::strToQt(str), "nicht finden", QMessageBox::Yes);
        return;
    }

    setNewIndex(idx);
}

void MainWindow::on_lineEdit_7_textChanged(const QString &arg1)
{
    const std::string str = utilQt::strToStd(arg1);
    if (str.empty())
        return;

    const size_t idx = dicDe.find(str, 0);
    if (idx == dicDe.size())
    {
        ui->lineEdit_7->setStyleSheet("color: rgb(255, 0, 0)");
        return;
    }

    ui->lineEdit_7->setStyleSheet("color: rgb(0, 0, 0)");

    setNewIndex(idx);
}

void MainWindow::setNewIndex(int idx)
{
    if (idx < 0 || size_t(idx) >= dicDe.size())
        return;

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
    if (dicDe.empty())
        return;

    if (origIndex < 0)
        setNewIndex(0);
    else
    if (origIndex >= int(dicDe.size()))
        setNewIndex(dicDe.size() - 1);
    else
        setNewIndex(origIndex + 1);
}

void MainWindow::on_pushButton_30_clicked()
{
    if (dicDe.empty())
        return;

    if (origIndex < 1)
        setNewIndex(0);
    else
    if (origIndex >= int(dicDe.size()))
        setNewIndex(dicDe.size() - 1);
    else
        setNewIndex(origIndex - 1);
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

void MainWindow::on_pushButton_31_clicked()
{
    addNewWortFromSearch();
}

void MainWindow::on_pushButton_32_clicked()
{
    addNewWortFromSearch();
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

