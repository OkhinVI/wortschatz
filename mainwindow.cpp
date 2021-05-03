#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wortde.h"
#include <stdio.h> // Nur for Debug

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    QString str1 = "Привет = Hello - 2021";
//    std::string str8 = str1.toUtf8().toStdString();

    WortDe wd(" der Tisch \t\t Стул ");
//    WortDe wd(" sich beschränken (auf + A.) \t\t ограничиваться (на ...) ");
    FILE *Out = fopen("D:\\Temp\\testWortDe.txt", "w");
    fprintf(Out,"%s\n%s\n%s\n%d\n%d\n", wd.raw().c_str(), wd.translation().c_str(), wd.wort().c_str(),
            wd.type(), wd.artikel());
    fclose(Out);
}

MainWindow::~MainWindow()
{
    delete ui;
}

