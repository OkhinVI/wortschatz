#include "mainwindow.h"
#include "ui_mainwindow.h"
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
    model->upDate();
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

void MainWindow::getWortDeToCurrWd()
{
    currWd.setNewTranslation(utilQt::lineEditToStdStr(ui->lineEdit_2));
    currWd.setNewWort(utilQt::lineEditToStdStr(ui->lineEdit));
    if (currWd.type() == WortDe::TypeWort::Noun)
        currWd.setNewPlural(utilQt::lineEditToStdStr(ui->lineEdit_5));

    std::string prfxStr = utilQt::lineEditToStdStr(ui->lineEdit_3);
    AreaUtf8 prfx(prfxStr);
    AreaUtf8 erstWort = prfx.getToken(" ");
    prfx.getToken(" ");
    if (currWd.type() == WortDe::TypeWort::Noun)
    {
        if (erstWort == "der")
            currWd.setNewArtikel(WortDe::TypeArtikel::Der);
        else if (erstWort == "das")
            currWd.setNewArtikel(WortDe::TypeArtikel::Das);
        else if (erstWort == "die")
        {
            if (currWd.artikel() != WortDe::TypeArtikel::Pl)
                currWd.setNewArtikel(WortDe::TypeArtikel::Die);
        }
        else if (erstWort == "der/das")
            currWd.setNewArtikel(WortDe::TypeArtikel::Der_Das);
        else if (erstWort == "der/die")
            currWd.setNewArtikel(WortDe::TypeArtikel::Der_Die);
        else
            prfx.seekg(0);
    }
    else if(currWd.type() == WortDe::TypeWort::Verb)
    {
        if (erstWort == "sich")
        {
            currWd.setNewSich(true);
        }
        else
        {
            currWd.setNewSich(false);
            prfx.seekg(0);
        }
    }
    else
        prfx.seekg(0);

    currWd.setNewPrefix(prfx.getRestArea().trim().toString());
}

void MainWindow::setWortDe(WortDe wd)
{
    utilQt::strToLineEdit(ui->lineEdit, wd.wort());
    utilQt::strToLineEdit(ui->lineEdit_2, wd.translation());
    utilQt::strToLineEdit(ui->lineEdit_3, wd.prefix());
    utilQt::strToLineEdit(ui->lineEdit_4, wd.raw());
    utilQt::strToLineEdit(ui->lineEdit_6, wd.blockToStr() + " = " + dicDe.tema(wd.block()));

    std::string options;
    if (wd.type() == WortDe::TypeWort::Noun)
    {
        options = wd.wortPl();
    }
    else if (wd.type() == WortDe::TypeWort::Verb)
    {
        options = wd.vPrasens3f() + "; " + wd.vPrateritum() + "; " + wd.vPerfect();
    }
    utilQt::strToLineEdit(ui->lineEdit_5, options);

    std::string typeStr = WortDe::TypeWortToString(wd.type()) + "/" + WortDe::TypeWortToString(wd.type(), "ru");
    ui->pushButton->setText(QString::fromStdString(typeStr));

    setButtonEnable(wd);
}

void MainWindow::checkChangesCurrWd(const bool saveWithoutAsk)
{
    if (origIndex < 0 || static_cast<size_t>(origIndex) >= dicDe.size())
        return;

    getWortDeToCurrWd();
    if (currWd == origWd)
        return;

    if (ui->checkBox->checkState() != Qt::Checked && !saveWithoutAsk)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Wortschatzkarte geändert", "Änderungen speichern?",
        QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes)
            return;
    }

    origWd = currWd;
    dicDe.at(origIndex) = origWd;
    model->upDate(origIndex, origIndex);
}

void MainWindow::selectItem(int idx)
{
    if (currIndex == idx)
        return;
    currIndex = idx;
    if (idx < 0 || static_cast<size_t>(idx) >= dicDe.size())
        return;

    checkChangesCurrWd();

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
    if (origIndex < 0 || static_cast<size_t>(origIndex) >= dicDe.size())
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


void MainWindow::wortTranslate(const std::string &beginUrl, const std::string &endUrl, const std::string &str)
{
    if (str.empty())
        return;
    AreaUtf8 au8(str);
    au8.trim();
    AreaUtf8 wort = au8.getToken();
    if (wort.empty())
        return;
    QString url = QString::fromStdString(beginUrl + wort.toString() + endUrl);
    ui->label->setText(url);
    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::CombinationTranslate(const std::string &beginUrl, const std::string &endUrl, const std::string &str)
{
    if (str.empty())
        return;
    std::stringstream ss;
    for (size_t i = 0; i < str.size(); ++i)
    {
        const char sym = str[i];
        if (sym >= 0x10 && sym < 0x30)
            ss << "%" << std::hex << static_cast<int>(sym);
        else
            ss << sym;
    }
    QString url = QString::fromStdString(beginUrl + ss.str() + endUrl);
    ui->label->setText(url);
    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::on_pushButton_8_clicked()
{
    const std::string beginUrl = "https://www.lingvolive.com/ru-ru/translate/de-ru/";
    const std::string endUrl = "";
    wortTranslate(beginUrl, endUrl, utilQt::lineEditToStdStr(ui->lineEdit));
}

void MainWindow::on_pushButton_9_clicked()
{
    const std::string beginUrl = "https://translate.yandex.ru/?utm_source=wizard&text=";
    const std::string endUrl = "&lang=de-ru";
    wortTranslate(beginUrl, endUrl, utilQt::lineEditToStdStr(ui->lineEdit));
}

void MainWindow::on_pushButton_10_clicked()
{
    const std::string beginUrl = "https://translate.google.de/?hl=ru&tab=TT&sl=de&tl=ru&text=";
    const std::string endUrl = "&op=translate";
    wortTranslate(beginUrl, endUrl, utilQt::lineEditToStdStr(ui->lineEdit));
}

void MainWindow::on_pushButton_11_clicked()
{
    const std::string rawStr = utilQt::lineEditToStdStr(ui->lineEdit_4);
    const std::string trStr = utilQt::lineEditToStdStr(ui->lineEdit_2);
    currWd.parseRawLine(rawStr, trStr, currWd.block(), currWd.type());
    setWortDe(currWd);
}

void MainWindow::on_actionSave_as_raw_triggered()
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
            os << "\n#" << (wd.block() >> 24) << " " << ((wd.block() >> 16) & 0xFF) << " " << ((wd.block() >> 8) & 0xFF) << " " << (wd.block() & 0xFF) << " "
                << dicDe.tema(wd.block()) << "\n" << std::endl;
            lastBlockNum = wd.block();
        }
        os << wd.raw() << "\t" << wd.translation() << std::endl;
    }
}

void MainWindow::on_pushButton_12_clicked()
{
    const std::string beginUrl = "https://www.dwds.de/wb/";
    const std::string endUrl = "";
    wortTranslate(beginUrl, endUrl, utilQt::lineEditToStdStr(ui->lineEdit));
}

void MainWindow::on_pushButton_13_clicked()
{
    const std::string beginUrl = "https://www.verbformen.ru/sprjazhenie/";
    const std::string endUrl = ".htm";
    wortTranslate(beginUrl, endUrl, utilQt::lineEditToStdStr(ui->lineEdit));
}

void MainWindow::on_pushButton_14_clicked()
{
    const std::string beginUrl = "https://ru.pons.com/%D0%BF%D0%B5%D1%80%D0%B5%D0%B2%D0%BE%D0%B4/%D0%BD%D0%B5%D0%BC%D0%B5%D1%86%D0%BA%D0%B8%D0%B9-%D1%80%D1%83%D1%81%D1%81%D0%BA%D0%B8%D0%B9/";
    const std::string endUrl = "";
    wortTranslate(beginUrl, endUrl, utilQt::lineEditToStdStr(ui->lineEdit));
}

void MainWindow::on_pushButton_15_clicked()
{
    const std::string beginUrl = "https://translate.yandex.ru/?utm_source=wizard&text=";
    const std::string endUrl = "&lang=de-ru";
    if (ui->checkBox_2->checkState() == Qt::Checked)
        CombinationTranslate(beginUrl, endUrl, utilQt::lineEditToStdStr(ui->lineEdit_4));
    else
        CombinationTranslate(beginUrl, endUrl, utilQt::lineEditToStdStr(ui->lineEdit));
}

void MainWindow::on_pushButton_16_clicked()
{
    const std::string beginUrl = "https://translate.google.de/?hl=ru&tab=TT&sl=de&tl=ru&text=";
    const std::string endUrl = "&op=translate";
    if (ui->checkBox_2->checkState() == Qt::Checked)
        CombinationTranslate(beginUrl, endUrl, utilQt::lineEditToStdStr(ui->lineEdit_4));
    else
        CombinationTranslate(beginUrl, endUrl, utilQt::lineEditToStdStr(ui->lineEdit));
}

void MainWindow::on_pushButton_17_clicked()
{
    const std::string str = utilQt::lineEditToStdStr(ui->lineEdit_7);
    const size_t idx = dicDe.find(str, currIndex + 1);
    if (idx == dicDe.size())
    {
        QMessageBox::information(this, utilQt::strToQt(str), "nicht finden", QMessageBox::Yes);
        return;
    }

    QModelIndex index = model->creatNewIndex(idx);
    if ( index.isValid() ) {
        ui->listView->setCurrentIndex(index);
        selectItem(idx);
    }
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

    QModelIndex index = model->creatNewIndex(idx);
    if ( index.isValid() ) {
        ui->listView->setCurrentIndex(index);
        selectItem(idx);
    }
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
