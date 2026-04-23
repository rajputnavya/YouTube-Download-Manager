#include "widget.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget mainWindow;
    mainWindow.setWindowTitle("YouTube Donwload Manager");
    mainWindow.resize(600,400);
    mainWindow.show();

    return a.exec();
}
