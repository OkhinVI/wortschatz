#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wortde.h"
#include <fstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    QString str1 = "Привет = Hello - 2021";
//    std::string str8 = str1.toUtf8().toStdString();

    WortDe wd(" der Tisch \t\t Стул ");
    WortDe wd2(" sich beschränken (auf + A.) \t\t ограничиваться (на ...) ");

    std::ofstream os;
    os.open("D:\\Temp\\dic_de.txt");
    wd.save(os);
    wd2.save(os);
    os.close();

}

MainWindow::~MainWindow()
{
    delete ui;
}

