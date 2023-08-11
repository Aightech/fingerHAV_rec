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
#include <string>
#include <tuple>
#include <utility>

class Trial
{
    public:
    Trial(std::string userName, std::string texture, std::string type, int gid, int matid)
        : m_userName(userName), m_texture(texture), m_type(type), m_gid(gid), m_matid(matid)
    {
        m_name = m_userName + "_" + m_texture + "_" + m_type +  "_" + std::to_string(m_matid);
    }
    ~Trial() {}
    void addComment(std::string comment)
    {
        m_comment = comment;
    }

    std::string getName()
    {
        return m_name;
    }

    bool hasBeenRecorded()
    {
        return m_recorded;
    }

    void setRecorded()
    {
        m_recorded = true;
    }

    std::string m_name;
    std::string m_texture;
    std::string m_type;
    std::string m_comment;
    std::string m_userName;
    bool m_recorded = false;
    int m_gid;
    int m_matid;
};

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

    //functions to call every dt, depending on the phase and type of the trial
    void
    moveTargetCircle(double dt, double speed, double radius);
    void
    moveTargetRectangle(double dt, double speed, double width, double height);
    void
    displayLoadGauge(double load, double targetLoad, double range, double min=0, double max=100);
    void
    displayPatternCircle(double radius, double direction);
    void
    displayPatternRectangle(double width, double height);

    private slots:
    void
    pushButton_userName_released(); // set the user name, disable the button
    void
    pushButton_nbTrial_released(); // set the number of trial for each material, disable the button
    void 
    pushButton_tare_released()
    {
        m_tare = m_load;
    }; // tare the loadcells

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
    void
    pushButton_load_released();

    void 
    pushButton_load_log_released();

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
    std::vector<std::vector<Trial *>> m_matTrial;
    Trial *m_currentTrial;
    int m_currentTrialIndex;
    int m_mat;
    int m_trial;

    //timer to update the time and motion during the trial
    QTimer *m_timer;
    int m_time;
    int m_timeMax = 60000; //in miliseconds
    int m_timeStep = 100;  //in miliseconds
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
    //scene to display the load gauge
    QGraphicsScene *m_scene_gauge;

    QGraphicsEllipseItem* m_arrow;

    QPen m_penFinger;
    QPen m_penTarget;
    int m_sizeTexture = 500;
    int m_sizeTarget = 50;
    double m_last_finger_pos[2];

    double m_tare=0;
    double m_load=0;
};
#endif // MAINWINDOW_H
