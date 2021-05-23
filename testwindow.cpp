#include "testwindow.h"
#include "ui_testwindow.h"

TestWindow::TestWindow(GlossaryDe &aDicDe, QWidget *parent) :
    QMainWindow(parent),
    dicDe(aDicDe),
    ui(new Ui::TestWindow)
{
    ui->setupUi(this);
}

TestWindow::~TestWindow()
{
    delete ui;
}
