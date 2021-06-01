#include "testsettings.h"
#include "ui_testsettings.h"
#include "glossaryde.h"

TestSettings::TestSettings(GlossaryDe::SelectSettings &aGlSelSet, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestSettings),
    glSelSet(aGlSelSet)
{
    ui->setupUi(this);
    modelFirst = new QListViewGlossaryTemaModel(glSelSet);
    ui->comboBox->setModel(modelFirst);
    ui->comboBox->setCurrentIndex(glSelSet.startIdxTema);

    modelLast = new QListViewGlossaryTemaModel(glSelSet);
    ui->comboBox_2->setModel(modelLast);
    ui->comboBox_2->setCurrentIndex(glSelSet.lastIdxTema);
}

TestSettings::~TestSettings()
{
    delete ui;
}

void TestSettings::on_buttonBox_accepted()
{
    glSelSet.startIdxTema = ui->comboBox->currentIndex();
    glSelSet.lastIdxTema = ui->comboBox_2->currentIndex();
}

