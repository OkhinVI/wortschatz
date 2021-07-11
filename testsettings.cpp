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
    ui->checkBoxThemes->setCheckState(glSelSet.useRangeTema ? Qt::Checked : Qt::Unchecked);
    ui->comboBox->setCurrentIndex(glSelSet.startIdxTema);
    ui->comboBox_2->setCurrentIndex(glSelSet.lastIdxTema);

    ui->checkBoxFreqIdx->setCheckState(glSelSet.useRangeFreq ? Qt::Checked : Qt::Unchecked);
    ui->spinBoxStartFreq->setValue(glSelSet.startFreqIdx);
    ui->spinBoxEndFreq->setValue(glSelSet.endFreqIdx);

    ui->horizontalSlider->setValue(glSelSet.posIgnoringStatistics);
}

TestSettings::~TestSettings()
{
    delete ui;
}

void TestSettings::on_buttonBox_accepted()
{
    glSelSet.useRangeTema = ui->checkBoxThemes->checkState() == Qt::Checked;
    glSelSet.startIdxTema = ui->comboBox->currentIndex();
    glSelSet.lastIdxTema = ui->comboBox_2->currentIndex();

    glSelSet.useRangeFreq = ui->checkBoxFreqIdx->checkState() == Qt::Checked;
    glSelSet.startFreqIdx = ui->spinBoxStartFreq->value();
    glSelSet.endFreqIdx = ui->spinBoxEndFreq->value();

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


void TestSettings::on_spinBoxStartFreq_editingFinished()
{

}

