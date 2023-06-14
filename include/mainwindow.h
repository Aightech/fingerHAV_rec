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
#include <QSettings>
#include <QTreeWidgetItem>

#include "lsl_cpp.h"
#include <QGraphicsScene>
#include <QTimer>
#include <chrono>
#include <iomanip> // std::setprecision
#include <iostream>
#include <numeric>
#include <pthread.h>
#include <tuple>
#include <utility>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
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
    void
    pushButton_userName_released(); // set the user name, disable the button
    void
    pushButton_nbTrial_released(); // set the number of trial for each material, disable the button

    /**
     * @brief Select the material selected in the treeWidget_mat.
     *        The selected material is highlighted in green.
     *        Change the label_matName and label_trialID.
     *        The material is not selected if it has already been selected more than the number of trial.
     */
    void
    pushButton_matSelect_released();

    /**
     * @brief Select a random material in the treeWidget_mat.
     *        The selected material is highlighted in green.
     *        Change the label_matName and label_trialID.
     *        The material is not selected if it has already been selected more than the number of trial.
     */
    void
    pushButton_matRandom_released();

    /**
     * @brief add a comment to the selected trial in the treeWidget_mat
     */
    void
    pushButton_comment_released();

    /**
     * @brief Disable the buttons: pushButton_startTrial, pushButton_matSelect, pushButton_matRandom.
     *        Increment the trial number for the current material.
     *        Start the timer.
     *        Start the acquisition of the data.
     */
    void
    pushButton_startTrial_released();

    /**
     * @brief Stop the acquisition of the data.
     *        Stop the timer.
     *        Enable the buttons: pushButton_startTrial, pushButton_matSelect, pushButton_matRandom.
     */
    void
    pushButton_endTrial_released();

    /**
     * @brief Update the time and motion during the trial.
     */
    void
    updateTimer();

    /**
     * @brief Load the material configuration file.
     */
  void pushButton_load_released();

    private:
    void
    unselect_trial();
    void
    select_trial(int mat, int trial);
    void
    addLog(std::string log, bool timestamp = false);

    Ui::MainWindow *ui;
    //vector of material name
    std::vector<std::string> m_matName;
    //user name
    std::string m_userName;
    //number of trial for each material
    int m_nbTrial;
    //vector of trial for each material [trialID, comment, nbTrial]
    std::vector<std::vector<std::tuple<std::string, std::string, int>>>
        m_matTrial;

    //current material
    int m_mat;
    //current trial
    int m_trial;

    //timer to update the time and motion during the trial
    QTimer *m_timer;
    int m_time;
    int m_timeMax = 60000; //in miliseconds
    int m_timeStep = 100;   //in miliseconds
    bool m_trialInProgress = false;

    std::vector<std::string> m_availableStreams;
    //LSL stream toeceive finger_pos data
    lsl::stream_inlet *m_inlet_finger_pos = nullptr;
    //LSL stream toeceive loadcells data
    lsl::stream_inlet *m_inlet_loadcells = nullptr;

    lsl::stream_outlet *m_outlet_trigger = nullptr;

    std::string m_commentLog_fileName;

    //scene to display the motion
    QGraphicsScene *m_scene;
};
#endif // MAINWINDOW_H
