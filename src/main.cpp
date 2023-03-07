#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char **argv)
{
  try
    {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
    }
    catch(std::exception &e)
    {
        std::cerr << "[ERROR] Got an exception: " << e.what() << std::endl;
    }
  catch(std::string str)
    {
        std::cerr << "[ERROR] Got an exception: " << str << std::endl;
    }
}
