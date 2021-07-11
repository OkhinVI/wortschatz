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

private:
    Ui::TestSettings *ui;
    QListViewGlossaryTemaModel *modelFirst;
    QListViewGlossaryTemaModel *modelLast;
    GlossaryDe::SelectSettings &glSelSet;
};

#endif // TESTSETTINGS_H
