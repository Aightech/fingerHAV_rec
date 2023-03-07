#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/**
 * @file      mainwindow.h
 * @brief     Header of
 * @date      Thu Jul  7 09:48:51 2022
 * @author    aightech
 * @copyright BSD-3-Clause
 * 
 * This module
 */

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QSettings>



#include <numeric>
#include <chrono>
#include <iostream>
#include <iomanip> // std::setprecision
#include <pthread.h>
#include <lsl_cpp.h>
#include <utility>
 #include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


using clk = std::chrono::system_clock;
using sec = std::chrono::duration<double>;

/**
 * @brief The MainWindow class
 * 
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:

private:
    Ui::MainWindow *ui;


};
#endif // MAINWINDOW_H
