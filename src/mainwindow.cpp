#include "mainwindow.h"
#include "../forms/ui_mainwindow.h"
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
ui->setupUi(this);
}

MainWindow::~MainWindow()
{

    delete ui;
}
