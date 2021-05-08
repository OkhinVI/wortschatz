#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "glossaryde.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    QString str1 = "Привет = Hello - 2021";
//    std::string str8 = str1.toUtf8().toStdString();

    GlossaryDe dicDe;
    dicDe.load();

}

MainWindow::~MainWindow()
{
    delete ui;
}

