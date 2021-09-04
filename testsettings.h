#ifndef TESTSETTINGS_H
#define TESTSETTINGS_H

#include <QDialog>
#include "qlistviewglossarydemodel.h"

namespace Ui {
class TestSettings;
}

class TestSettings : public QDialog
{
    Q_OBJECT

public:
    explicit TestSettings(GlossaryDe::SelectSettings &aGlSelSet, QWidget *parent = nullptr);
    ~TestSettings();

    void resetVals();

private slots:
    void on_buttonBox_accepted();

    void on_horizontalSlider_valueChanged(int value);

    void on_pushButton_clicked();

    void on_comboBox_currentIndexChanged(int index);

    void on_comboBox_2_currentIndexChanged(int index);

    void on_spinBoxStartFreq_valueChanged(int arg1);

    void on_spinBoxEndFreq_valueChanged(int arg1);

private:
    Ui::TestSettings *ui;
    QListViewGlossaryTemaModel *modelFirst;
    QListViewGlossaryTemaModel *modelLast;
    GlossaryDe::SelectSettings &glSelSet;
};

#endif // TESTSETTINGS_H
