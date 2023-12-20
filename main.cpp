//
// Created by elias on 26/11/2023.
//
#include "mainwindow.h"
#include <QApplication>

int main (int argc, char *argv[]){
    // Create UI
    QApplication a(argc, argv);
    MainWindow w;
    // Set the background color using a style sheet
    w.setStyleSheet("background-color: lightblue;");
    w.show();
    return a.exec();
}