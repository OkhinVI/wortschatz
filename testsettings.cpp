#include "testsettings.h"
#include "ui_testsettings.h"
#include "glossaryde.h"

TestSettings::TestSettings(GlossaryDe::SelectSettings &aGlSelSet, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestSettings),
    glSelSet(aGlSelSet)
{
    ui->setupUi(this);

    modelFirst = new QListViewGlossaryTemaModel(glSelSet, this);
    ui->comboBox->setModel(modelFirst);
    modelLast = new QListViewGlossaryTemaModel(glSelSet, this);
    ui->comboBox_2->setModel(modelLast);

    resetVals();
}

void TestSettings::resetVals()
{
    ui->comboBox->setCurrentIndex(glSelSet.startIdxTema);
    ui->comboBox_2->setCurrentIndex(glSelSet.lastIdxTema);
    ui->horizontalSlider->setValue(glSelSet.posIgnoringStatistics);
}

TestSettings::~TestSettings()
{
    delete ui;
}

void TestSettings::on_buttonBox_accepted()
{
    glSelSet.startIdxTema = ui->comboBox->currentIndex();
    glSelSet.lastIdxTema = ui->comboBox_2->currentIndex();
    glSelSet.posIgnoringStatistics = ui->horizontalSlider->value();
}


void TestSettings::on_horizontalSlider_valueChanged(int value)
{
    if (value < 0)
        value = 0;
    else if (value > 100)
        value = 100;

    ui->lineEdit->setText(QString::number(value) + " %");
}


void TestSettings::on_pushButton_clicked()
{
    resetVals();
}

