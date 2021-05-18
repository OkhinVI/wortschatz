#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialograwedit.h"
#include "dialogtypewort.h"
#include "utility.h"
#include <fstream>
#include <QFileDialog>
#include <QSettings>
#include <QDesktopServices>
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
    currWd.setNewTranslation(AreaUtf8( ui->lineEdit_2->text().toUtf8().toStdString() ).trim().toString());
    currWd.setNewWort(AreaUtf8( ui->lineEdit->text().toUtf8().toStdString() ).trim().toString());
    if (currWd.type() == WortDe::TypeWort::Noun)
        currWd.setNewPlural(AreaUtf8( ui->lineEdit_5->text().toUtf8().toStdString() ).trim().toString());

    std::string prfxStr = AreaUtf8( ui->lineEdit_3->text().toUtf8().toStdString() ).trim().toString();
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
    ui->lineEdit->setText(QString::fromStdString(wd.wort()));
    ui->lineEdit_2->setText(QString::fromStdString(wd.translation()));
    ui->lineEdit_3->setText(QString::fromStdString(wd.prefix()));
    ui->lineEdit_4->setText(QString::fromStdString(wd.raw()));
    ui->lineEdit_6->setText(QString::fromStdString(wd.blockToStr() + " = " + dicDe.tema(wd.block())));

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

    ui->pushButton_13->setEnabled(wd.type() == WortDe::TypeWort::Verb);

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
            getWortDeToCurrWd();
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

    getWortDeToCurrWd();
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
    wortTranslate(beginUrl, endUrl, ui->lineEdit->text().toUtf8().toStdString());
}

void MainWindow::on_pushButton_9_clicked()
{
    const std::string beginUrl = "https://translate.yandex.ru/?utm_source=wizard&text=";
    const std::string endUrl = "&lang=de-ru";
    wortTranslate(beginUrl, endUrl, ui->lineEdit->text().toUtf8().toStdString());
}

void MainWindow::on_pushButton_10_clicked()
{
    const std::string beginUrl = "https://translate.google.de/?hl=ru&tab=TT&sl=de&tl=ru&text=";
    const std::string endUrl = "&op=translate";
    wortTranslate(beginUrl, endUrl, ui->lineEdit->text().toUtf8().toStdString());
}

void MainWindow::on_pushButton_11_clicked()
{
    const std::string rawStr = ui->lineEdit_4->text().toUtf8().toStdString();
    const std::string trStr = ui->lineEdit_2->text().toUtf8().toStdString();
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
    wortTranslate(beginUrl, endUrl, ui->lineEdit->text().toUtf8().toStdString());
}

void MainWindow::on_pushButton_13_clicked()
{
    const std::string beginUrl = "https://www.verbformen.ru/sprjazhenie/";
    const std::string endUrl = ".htm";
    wortTranslate(beginUrl, endUrl, ui->lineEdit->text().toUtf8().toStdString());
}

void MainWindow::on_pushButton_14_clicked()
{
    const std::string beginUrl = "https://ru.pons.com/%D0%BF%D0%B5%D1%80%D0%B5%D0%B2%D0%BE%D0%B4/%D0%BD%D0%B5%D0%BC%D0%B5%D1%86%D0%BA%D0%B8%D0%B9-%D1%80%D1%83%D1%81%D1%81%D0%BA%D0%B8%D0%B9/";
    const std::string endUrl = "";
    wortTranslate(beginUrl, endUrl, ui->lineEdit->text().toUtf8().toStdString());
}

void MainWindow::on_pushButton_15_clicked()
{
    const std::string beginUrl = "https://translate.yandex.ru/?utm_source=wizard&text=";
    const std::string endUrl = "&lang=de-ru";
    if (ui->checkBox_2->checkState() == Qt::Checked)
        CombinationTranslate(beginUrl, endUrl, ui->lineEdit_4->text().toUtf8().toStdString());
    else
        CombinationTranslate(beginUrl, endUrl, ui->lineEdit->text().toUtf8().toStdString());
}

void MainWindow::on_pushButton_16_clicked()
{
    const std::string beginUrl = "https://translate.google.de/?hl=ru&tab=TT&sl=de&tl=ru&text=";
    const std::string endUrl = "&op=translate";
    if (ui->checkBox_2->checkState() == Qt::Checked)
        CombinationTranslate(beginUrl, endUrl, ui->lineEdit_4->text().toUtf8().toStdString());
    else
        CombinationTranslate(beginUrl, endUrl, ui->lineEdit->text().toUtf8().toStdString());
}
