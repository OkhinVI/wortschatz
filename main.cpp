#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    const char *accountName = "User";
    if (argc >= 2 && argv[1][0] != '\0')
        accountName = argv[1];

    MainWindow w(accountName);
    w.show();
    return a.exec();
}
