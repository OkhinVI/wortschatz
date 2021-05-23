#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QMainWindow>
#include "glossaryde.h"

namespace Ui {
class TestWindow;
}

class TestWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestWindow(GlossaryDe &aDicDe, QWidget *parent = nullptr);
    ~TestWindow();

private:
    GlossaryDe &dicDe;

private:
    Ui::TestWindow *ui;
};

#endif // TESTWINDOW_H
