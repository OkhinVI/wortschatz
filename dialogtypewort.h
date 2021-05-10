#ifndef DIALOGTYPEWORT_H
#define DIALOGTYPEWORT_H

#include <QDialog>
#include "wortde.h"

namespace Ui {
class DialogTypeWort;
}

class DialogTypeWort : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTypeWort(QWidget *parent = nullptr);
    ~DialogTypeWort();

    WortDe::TypeWort getTypeWort();
    void setTypeWort(WortDe::TypeWort tw);

private:
    Ui::DialogTypeWort *ui;
};

#endif // DIALOGTYPEWORT_H
