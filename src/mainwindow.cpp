#include "mainwindow.h"
#include "../forms/ui_mainwindow.h"
#include "lsl_cpp.h"
#include <QComboBox>
#include <QDateTime>
#include <QFile>
#include <QGraphicsEllipseItem>

#include <fstream>
#include <regex>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // set the timer to 60 seconds
    ui->lcdNumber_timer->display(m_timeMax / 1000);

    // set the number of trial to 5
    ui->spinBox_nbTrial->setValue(5);
    m_nbTrial = ui->spinBox_nbTrial->value();

    //set the user name to "user"
    ui->lineEdit_userName->setText("user");

    //add a column to the treeWidget_mat and set the header (Name , Description, trial)
    ui->treeWidget_mat->setColumnCount(3);
    ui->treeWidget_mat->setHeaderLabels(QStringList() << "Name"
                                                      << "Comments"
                                                      << "Trial");
    // set the size of the column
    ui->treeWidget_mat->setColumnWidth(0, 100);
    ui->treeWidget_mat->setColumnWidth(1, 120);
    ui->treeWidget_mat->setColumnWidth(2, 50);

    pushButton_load_released();

    //get the icon path of the three stream button (kistler, loadcells, finger_pos) to "user-offline"
    ui->pushButton_kistler->setIcon(QIcon::fromTheme("user-offline"));
    ui->pushButton_loadcells->setIcon(QIcon::fromTheme("user-offline"));
    ui->pushButton_finger_pos->setIcon(QIcon::fromTheme("user-offline"));

    //disable all button exept the button_userName
    ui->pushButton_nbTrial->setEnabled(false);
    ui->pushButton_matRandom->setEnabled(false);
    ui->pushButton_matSelect->setEnabled(false);
    ui->pushButton_startTrial->setEnabled(false);
    ui->pushButton_comment->setEnabled(false);
    ui->pushButton_endTrial->setEnabled(false);

    // scan the avalable lsl stream and display. if the stream is available, the icon is set to "user-available"
    // get the list of available lsl stream
    std::vector<lsl::stream_info> results = lsl::resolve_streams();
    if(results.size() > 0)
    {
        for(int i = 0; i < results.size(); i++)
        {
            std::cout << "stream name: " << results[i].name() << std::endl;
            if(results[i].name() == "Kistler")
            {
                ui->pushButton_kistler->setIcon(
                    QIcon::fromTheme("user-available"));
                m_availableStreams.push_back(results[i].name());
            }
            if(results[i].name() == "FT_sensor")
            {
                ui->pushButton_loadcells->setIcon(
                    QIcon::fromTheme("user-available"));
                m_inlet_loadcells =
                    new lsl::stream_inlet(results[i], 2, 3, false);
                m_availableStreams.push_back(results[i].name());
            }
            if(results[i].name() == "positions")
            {
                ui->pushButton_finger_pos->setIcon(
                    QIcon::fromTheme("user-available"));
                m_inlet_finger_pos =
                    new lsl::stream_inlet(results[i], 2, 1, false);
                m_availableStreams.push_back(results[i].name());
            }
        }
    }
    else
    {
        std::cout << "No stream found" << std::endl;
    }

    //create a trigger outlet lsl stream with name "trigger" and format cf_string with 2 channels
    lsl::stream_info info = lsl::stream_info(
        "trigger", "command", 2, 0, lsl::cf_string, "myuniquesourceid23443");
    m_outlet_trigger = new lsl::stream_outlet(info);

    // link the signal for each button to the corresponding slot
    connect(ui->pushButton_userName, SIGNAL(released()), this,
            SLOT(pushButton_userName_released()));
    connect(ui->pushButton_userName, SIGNAL(released()), this,
            SLOT(pushButton_userName_released()));
    connect(ui->pushButton_matLoad, SIGNAL(released()), this,
            SLOT(pushButton_load_released()));
    connect(ui->pushButton_log_load, SIGNAL(released()), this,
            SLOT(pushButton_load_log_released()));
    connect(ui->pushButton_comment, SIGNAL(released()), this,
            SLOT(pushButton_comment_released()));
    connect(ui->pushButton_matSelect, SIGNAL(released()), this,
            SLOT(pushButton_matSelect_released()));
    connect(ui->pushButton_matRandom, SIGNAL(released()), this,
            SLOT(pushButton_matRandom_released()));
    connect(ui->pushButton_nbTrial, SIGNAL(released()), this,
            SLOT(pushButton_nbTrial_released()));
    connect(ui->pushButton_startTrial, SIGNAL(released()), this,
            SLOT(pushButton_startTrial_released()));
    connect(ui->pushButton_endTrial, SIGNAL(released()), this,
            SLOT(pushButton_endTrial_released()));
    connect(ui->pushButton_tare, SIGNAL(released()), this,
            SLOT(pushButton_tare_released()));

    //add a circle to the graphicsView_motion
    m_scene = new QGraphicsScene();
    m_scene->addEllipse(0, 0, 100, 100);
    ui->graphicsView_motion->setScene(m_scene);

    //new scene for graphicsView_gauge
    m_scene_gauge = new QGraphicsScene();
    //hide the scrollbars
    ui->graphicsView_gauge->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);
    ui->graphicsView_gauge->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // set the timer to 60 seconds
    ui->lcdNumber_timer->display(m_timeMax / 1000);
    ui->label_motion->setText("none");
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTimer()));

    m_timer->start(m_timeStep);
}

MainWindow::~MainWindow()
{
    std::vector<std::string> cmd = {"stop", "all"};
    m_outlet_trigger->push_sample(cmd);
    delete ui;
}

void
MainWindow::pushButton_userName_released()
{
    // get the user name from the lineEdit_userName
    m_userName = ui->lineEdit_userName->text().toStdString();
    std::cout << "user name: " << m_userName << std::endl;
    //disable the lineEdit_userName and the button_userName
    ui->lineEdit_userName->setEnabled(false);
    ui->pushButton_userName->setEnabled(false);

    if(!ui->pushButton_matLoad->isEnabled())
        ui->pushButton_nbTrial->setEnabled(true);
}

void
MainWindow::pushButton_load_released()
{
    // search for a list of materials in the current directory store in a file .conf
    // the file .conf is read: the materials name is separated by new line. and stored in a list
    // the list is displayed in the treeWidget_mat
    // the user can select a material by clicking on it
    // the user can also request a random material by clicking on the button "Random"

    // search for a .conf file in the current directory:
    QString fileName = ui->lineEdit_matPath->text();
    QFile file(fileName);
    std::cout << "file name: " << fileName.toStdString() << std::endl;
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        std::cout << "Error: file " << fileName.toStdString() << " not found"
                  << std::endl;
        return;
    }
    // read the file and store the material name in a list
    QTextStream in(&file);
    QStringList list;
    while(!in.atEnd()) list << in.readLine();
    file.close();

    // add the materials' name to the treeWidget_mat
    for(int i = 0; i < list.size(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget_mat);
        item->setText(0, list.at(i));
        item->setText(1, "");
        item->setText(2, "0");
    }

    //disable the button_load
    ui->pushButton_matLoad->setEnabled(false);
    ui->lineEdit_matPath->setEnabled(false);

    ui->pushButton_nbTrial->setEnabled(true);
}

void MainWindow::pushButton_load_log_released()
{
    //load the log file in ui->lineEdit_log_load
    QString fileName = ui->lineEdit_load_log->text();
    QFile file(fileName);
    std::cout << "file name: " << fileName.toStdString() << std::endl;
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        std::cout << "Error: file " << fileName.toStdString() << " not found"
                  << std::endl;
        return;
    }
    // read the file and check if trial were already made
    //a ex: 11:27:05: (alexis_44t_cte_speed_4) END  duration: 60000ms
    //user name: alexis
    //material name: 44t
    //trial type: cte_speed
    //trial number: 4
    //trial duration: 60000ms
    
    //for each trial done with a duration of 60000ms, check if material name is in m_matTrial, if so set as done
    QTextStream in(&file);
    QStringList list;
    while(!in.atEnd()) list << in.readLine();
    file.close();

    for(int i=0; i<list.size(); i++)
    {
        QString line = list.at(i);
        
        std::string line_std = line.toStdString();
        if(line_std.find("END") != std::string::npos)
        {
            QStringList line_split = line.split(" ");
            QString mat_name = line_split.at(1);
            //remove the parenthesis
            mat_name = mat_name.mid(1, mat_name.size()-2);
            QString trial_duration = line_split.at(5);
            if(trial_duration == "60000ms")
            {
                for(int j=0; j<m_matTrial.size(); j++)
                {
                    for(int k=0; k<m_matTrial[j].size(); k++)
                    {
                        if(m_matTrial[j][k]->getName() == mat_name.toStdString())
                        {
                           m_matTrial[j][k]->setRecorded();
                           ui->treeWidget_mat->topLevelItem(j)->child(k)->setText(
        2, QString::number(6000).toStdString().c_str());
                        }
                    }
                }
            }
        }
    }

}

void
MainWindow::pushButton_nbTrial_released()
{
    // get the number of trial from the spinBox_nbTrial
    m_nbTrial = ui->spinBox_nbTrial->value();
    std::cout << "number of trial: " << m_nbTrial << std::endl;
    //disable the spinBox_nbTrial and the button_nbTrial
    ui->spinBox_nbTrial->setEnabled(false);
    ui->pushButton_nbTrial->setEnabled(false);

    // for each material, add a child to the treeWidget_mat
    // the child is the list of the comments for each trial of the material
    for(int i = 0; i < ui->treeWidget_mat->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = ui->treeWidget_mat->topLevelItem(i);
        m_matTrial.push_back(std::vector<Trial *>());
        for(int j = 0; j < m_nbTrial; j++)
        {
            QTreeWidgetItem *child = new QTreeWidgetItem(item);
            child->setText(0, "trial" + QString::number(j + 1));
            child->setText(1, "");
            std::string type = "std";
            if(j == 3)
                type = "cte_force";
            if(j == 4)
                type = "cte_speed";
            m_matTrial[i].push_back(new Trial(m_userName,
                                              item->text(0).toStdString(), type,
                                              i + j * m_nbTrial, j));
            // std::cout << "mat: " << i << " trial: " << j
            //           << " name: " << std::get<0>(m_matTrial[i][j])
            //           << std::endl;
        }
    }

    //generate a name for the log file based on the user name, the date and the time in the format dd_mm_yyyy_hh_mm_ss
    m_commentLog_fileName = "log_" + m_userName + "_" +
                            QDateTime::currentDateTime()
                                .toString("dd_MM_yyyy_hh_mm_ss")
                                .toStdString() +
                            ".txt";
    //replace all space by underscore
    std::replace(m_commentLog_fileName.begin(), m_commentLog_fileName.end(),
                 ' ', '_');

    //add log of the user name and the date and time in the log file and all the material name and the number of trial
    addLog("[HEADER]");
    addLog("user: " + m_userName);
    addLog("date: " + QDateTime::currentDateTime().toString().toStdString());
    addLog("number of trial: " + std::to_string(m_nbTrial));
    std::string matList = "";
    for(int i = 0; i < ui->treeWidget_mat->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = ui->treeWidget_mat->topLevelItem(i);
        matList += item->text(0).toStdString() + " ";
    }
    addLog("materials: [" + matList + "\b]");
    std::string streams = " ";
    for(int i = 0; i < m_availableStreams.size(); i++)
        streams += m_availableStreams[i] + " ";
    addLog("streams: [" + matList + "\b]");
    addLog("[LOGS]");

    //enable the button_matRandom and the button_matSelect
    ui->pushButton_matRandom->setEnabled(true);
    ui->pushButton_matSelect->setEnabled(true);

    m_penTarget.setColor(Qt::red);
    m_penTarget.setWidth(3);

    m_penFinger.setColor(Qt::green);
    m_penFinger.setWidth(3);
}

void
MainWindow::pushButton_comment_released()
{
    std::string comment = ui->textEdit_comment->toPlainText().toStdString();
    // get the comment from the lineEdit_comment
    m_currentTrial->addComment(comment);
    //add the comment in the treeWidget_mat
    ui->treeWidget_mat->topLevelItem(m_mat)->child(m_trial)->setText(
        1, QString(comment.c_str()));
    //add the comment in the log file
    addLog("(" + m_currentTrial->getName() + ") COMMENTS: " + comment, true);
}

void
MainWindow::select_trial(int mat, int trial)
{

    QTreeWidgetItem *item = ui->treeWidget_mat->topLevelItem(mat)->child(trial);
    if(item->parent() == NULL)
    {
        std::cout << "Error: select a trial" << std::endl;
        return;
    }
    m_mat = ui->treeWidget_mat->indexFromItem(item->parent()).row();
    m_trial = ui->treeWidget_mat->indexFromItem(item).row();
    m_currentTrial = m_matTrial[m_mat][m_trial];
    ui->pushButton_comment->setEnabled(true);
    ui->pushButton_startTrial->setEnabled(true);
    std::cout << "material: " << m_mat << " trial: " << m_trial << std::endl;
    //set the material in the treeWidget_mat to "user-away"
    unselect_trial();
    if(m_matTrial[m_mat][m_trial]->hasBeenRecorded())
        item->setIcon(0, QIcon::fromTheme("user-available"));
    else
        item->setIcon(0, QIcon::fromTheme("user-away"));
    ui->label_matName->setText(item->parent()->text(0));
    ui->label_trialID->setText(QString(m_currentTrial->getName().c_str()));
    ui->treeWidget_mat->setCurrentItem(item);
    //add to the log the selected material and trial
    addLog("SELECT: " + item->parent()->text(0).toStdString() + " | " +
               item->text(0).toStdString(),
           true);
}

void
MainWindow::addLog(std::string log, bool timestamp)
{
    //open the log file in append mode
    std::ofstream commentLog_file(m_commentLog_fileName, std::ofstream::app);
    commentLog_file << (timestamp ? QDateTime::currentDateTime()
                                            .toString("hh:mm:ss")
                                            .toStdString() +
                                        ": "
                                  : "")
                    << log << std::endl;
    commentLog_file.close();
}

void
MainWindow::unselect_trial()
{
    //remove all trial icon
    for(int i = 0; i < ui->treeWidget_mat->topLevelItemCount(); i++)
    {
        for(int j = 0; j < ui->treeWidget_mat->topLevelItem(i)->childCount();
            j++)
        {
            //remove icon
            ui->treeWidget_mat->topLevelItem(i)->child(j)->setIcon(0, QIcon());
        }
    }
}

void
MainWindow::pushButton_matSelect_released()
{
    // get the selected material in the treeWidget_mat
    QTreeWidgetItem *item = ui->treeWidget_mat->currentItem();
    if(item->parent() == NULL)
    {
        std::cout << "Error: select a trial" << std::endl;
        return;
    }

    select_trial(ui->treeWidget_mat->indexFromItem(item->parent()).row(),
                 ui->treeWidget_mat->indexFromItem(item).row());
}

void
MainWindow::pushButton_matRandom_released()
{
    // get a random material from the treeWidget_mat
    int mat = -1;
    //get the smallest trial number with no trial done
    int trial = -1;
    int loop = 0;
    while(trial == -1 && loop < 100)
    {
        mat = rand() % ui->treeWidget_mat->topLevelItemCount();
        for(int i = 0; i < ui->treeWidget_mat->topLevelItem(mat)->childCount();
            i++)
        {
            if(m_matTrial[mat][i]->hasBeenRecorded() == 0)
            {
                trial = i;
                break;
            }
        }
        loop++;
    }
    if(trial == -1)
    {
        std::cout << "Error: no trial available" << std::endl;
        return;
    }
    select_trial(mat, trial);
}

void
MainWindow::pushButton_startTrial_released()
{
    //disable the button_startTrial
    ui->pushButton_startTrial->setEnabled(false);
    ui->pushButton_matSelect->setEnabled(false);
    ui->pushButton_matRandom->setEnabled(false);

    //enable the button_endTrial
    ui->pushButton_endTrial->setEnabled(true);

    //send trigger command on the m_outlet_trigger
    std::vector<std::string> cmd = {"start",
                                    ui->label_trialID->text().toStdString()};
    m_outlet_trigger->push_sample(cmd);
    //display "wait"
    //ui->label_trialID->setText("Wait");
    //ui->label_motion->setText("Wait");
    //show
    ui->label_motion->show();
    //tare 
    pushButton_tare_released();
    //wait for 1s
    usleep(1000000);

    m_time = 0;
    m_trialInProgress = true;
    //add to the log the start of the trial
    addLog("(" + ui->label_trialID->text().toStdString() + ") START", true);
}

void
MainWindow::pushButton_endTrial_released()
{
    addLog("(" + ui->label_trialID->text().toStdString() + ") END  duration: " +
               QString::number(m_time).toStdString() + "ms",
           true);
    ui->treeWidget_mat->topLevelItem(m_mat)->child(m_trial)->setText(
        2, QString::number(m_time).toStdString().c_str());

    //set the trial as done
    m_matTrial[m_mat][m_trial]->setRecorded();

    //count number of trial done
    int trialDone = 0;
    for(int i = 0; i < m_matTrial.size(); i++)
    {
        for(int j = 0; j < m_matTrial[i].size(); j++)
        {
            if(m_matTrial[i][j]->hasBeenRecorded() == 1)
                trialDone++;
        }
    }
    //add log saying the number of trial done
    addLog("TRIAL DONE: " + QString::number(trialDone).toStdString() + "/" +
               QString::number(m_matTrial.size() * m_matTrial[0].size())
                   .toStdString(),
           true);

    m_trialInProgress = false;
    m_time = 0;
    //disable the button_endTrial
    ui->pushButton_endTrial->setEnabled(false);
    //enable the button_startTrial
    ui->pushButton_startTrial->setEnabled(true);
    ui->pushButton_matSelect->setEnabled(true);
    ui->pushButton_matRandom->setEnabled(true);

    //set back the lcdNumber_timer
    ui->lcdNumber_timer->display(m_timeMax / 1000);
    //set back the label_motion
    ui->label_motion->setText("none");
    //add to the log the end of the trial
    std::vector<std::string> cmd = {"pause",
                                    ui->label_trialID->text().toStdString()};
    m_outlet_trigger->push_sample(cmd);
}

void
MainWindow::updateTimer()
{
    // clear the scene
    m_scene->clear();

    double speed = 0.3;
    double targetLoad = -1.3;
    double rangeLoad = 0.5;
    double max_load = 0;
    double min_load = -6;

    m_scene->addRect(-m_sizeTexture / 2, -m_sizeTexture / 2, m_sizeTexture,
                     m_sizeTexture, QPen(Qt::black), QBrush(Qt::white));

    if(m_inlet_loadcells != nullptr)
    {
        //get the loadcells data
        std::vector<std::vector<float>> loadcells;
        if(m_inlet_loadcells->pull_chunk(loadcells))
        {
            //get the load
            m_load = loadcells[0][2];
        }
    }
    // displayLoadGauge(m_load - m_tare, targetLoad, rangeLoad, max_load,
    //                  min_load);

    if(m_trialInProgress)
    {
        //update the timer
        m_time += m_timeStep;

        ui->lcdNumber_timer->display((m_timeMax - m_time) / 1000);

        //phase1: free motion
        //phase2: circle motion clockwise
        //phase3: circle motion counterclockwise
        //phase4: linear motion horizontal
        //phase5: linear motion vertical

        //if trial number is < 3, do pattern display
        //if trial number is == 3, do pattern display and gauge display
        //if trial number is == 4, do target display

        if(m_trial != 3 || m_time < 16000)
            ui->graphicsView_gauge->hide();

        if(m_time < 16000)
        {
            ui->label_motion->setText("freestyle");
        }
        else if(m_time < 38000)
        {
            if(m_time < 27000)
                speed = -speed;
            if(m_trial < 4)
            {
                displayPatternCircle(0.8 * m_sizeTexture / 2, speed);
                if(m_trial == 3)
                    displayLoadGauge(m_load - m_tare, targetLoad, rangeLoad,
                                     max_load, min_load);
            }
            if(m_trial == 4)
                moveTargetCircle(m_time, speed, 0.8 * m_sizeTexture / 2);
        }
        else
        {
            int w = 0.8 * m_sizeTexture;
            int h = 0.8 * m_sizeTexture;
            if(m_time < 49000)
                w = 0;
            else
                h = 0;

            if(m_trial < 4)
            {
                displayPatternRectangle(w, h);
                if(m_trial == 3)
                    displayLoadGauge(m_load - m_tare, targetLoad, rangeLoad,
                                     max_load, min_load);
            }
            if(m_trial == 4)
                moveTargetRectangle(m_time, speed, w, h);
        }
        //stop the timer
        if(m_time >= m_timeMax)
        {
            //trigger the button_endTrial
            pushButton_endTrial_released();
        }
    }
    else
    {
        m_scene->addEllipse(-m_sizeTarget / 2, -m_sizeTarget / 2, m_sizeTarget,
                            m_sizeTarget, m_penTarget);
    }
    if(m_inlet_finger_pos != nullptr)
    {
        //get the finger position
        std::vector<std::vector<float>> finger_pos;
        QBrush brush = QBrush(Qt::red);
        if(m_inlet_finger_pos->pull_chunk(finger_pos))
        {
            if(abs(finger_pos[0][0]) < 1.2 && abs(finger_pos[0][1]) < 1.2)
            {
                m_last_finger_pos[0] = finger_pos[0][0];
                m_last_finger_pos[1] = finger_pos[0][1];
                brush.setColor(Qt::green);
            }
            printf("finger pos: [%d]\n", finger_pos.size());
        }
        double x = (m_last_finger_pos[0] - 0.5) * m_sizeTexture;
        double y = (m_last_finger_pos[1] - 0.5) * m_sizeTexture;

        m_scene->addEllipse(x - m_sizeTarget / 2, y - m_sizeTarget / 2,
                            m_sizeTarget * 0.9, m_sizeTarget * 0.9, m_penFinger,
                            brush);
    }
}

void
MainWindow::moveTargetCircle(double dt, double speed, double radius)
{
    ui->label_motion->setText("Follow the target");
    m_scene->addEllipse(
        radius * cos(speed * 2 * M_PI * m_time / 1000.0) - m_sizeTarget / 2,
        radius * sin(speed * 2 * M_PI * m_time / 1000.0) - m_sizeTarget / 2,
        m_sizeTarget, m_sizeTarget, m_penTarget);
}

void
MainWindow::moveTargetRectangle(double dt,
                                double speed,
                                double width,
                                double height)
{
    ui->label_motion->setText("Follow the target");
    //display a circle target moving in a rectangular pattern
    //ratio of the width and height of the rectangle
    double angleTime = speed * 2 * M_PI * m_time / 1000.0;
    double cosAngleTime = cos(angleTime);
    double signCosAngleTime = cosAngleTime / abs(cosAngleTime);

    if(width == 0)
        width = 0.0001;
    if(height == 0)
        height = 0.0001;

    double hyp = sqrt(pow(width / 2, 2) + pow(height / 2, 2));
    double cosAngle = signCosAngleTime * width / (2 * hyp);
    double sinAngle = signCosAngleTime * height / (2 * hyp);

    double x, y;
    if(abs(cosAngleTime) > abs(cosAngle))
    {
        x = hyp * cosAngle;
        y = hyp * sin(angleTime);
    }
    else
    {
        x = hyp * cos(angleTime);
        y = hyp * sinAngle;
    }
    m_scene->addEllipse(x - m_sizeTarget / 2, y - m_sizeTarget / 2,
                        m_sizeTarget, m_sizeTarget, m_penTarget);
}

void
MainWindow::displayLoadGauge(
    double load, double targetLoad, double lim, double min, double max)
{
    ui->graphicsView_gauge->show();

    int tmax = max;
    max = min;
    min = tmax;

    double scale_range = max - min;
    double scale_load = (load - min) / scale_range;
    double scale_target = (targetLoad - min) / scale_range;
    double scale_lim = lim / scale_range;

    //add a color gradient to the scene
    int h = ui->graphicsView_gauge->height();
    int w = ui->graphicsView_gauge->width();
    int o = 2;
    int l = h * lim;
    int thickness = 2;
    int load_value = scale_load * h;
    QLinearGradient gradient(0, 0, 0, h);
    gradient.setColorAt(0, Qt::red);
    gradient.setColorAt(scale_target - scale_lim, Qt::yellow);
    gradient.setColorAt(scale_target, Qt::green);
    gradient.setColorAt(scale_target + scale_lim, Qt::yellow);
    gradient.setColorAt(1, Qt::red);

    //add a rectangle to the scene withou border and with the gradient
    m_scene_gauge->addRect(0, 0, w - o, h - o, QPen(Qt::NoPen),
                           QBrush(gradient));
    //add two lines to the scene to limit the gauge on the y axis
    m_scene_gauge->addLine(thickness, (scale_target - scale_lim) * h,
                           w - thickness * 2, (scale_target - scale_lim) * h,
                           QPen(Qt::black, thickness));
    m_scene_gauge->addLine(thickness, (scale_target + scale_lim) * h,
                           w - thickness * 2, (scale_target + scale_lim) * h,
                           QPen(Qt::black, thickness));
    //add a circle to the scene to display the load value
    m_scene_gauge->addEllipse(0, load_value - w / 2, w - o, w - o,
                              QPen(Qt::NoPen), QBrush(Qt::black));
    ui->graphicsView_gauge->setScene(m_scene_gauge);
}

void
MainWindow::displayPatternCircle(double radius, double direction)
{
    ui->label_motion->setText("Follow the pattern");
    //display a empty circle with a grey border of thickness m_sizeTarget
    m_scene->addEllipse(-radius, -radius, 2 * radius, 2 * radius,
                        QPen(Qt::gray, m_sizeTarget));
    //add a curved arrow to the scene
    double scale = 0.5;
    int arrowWidth = 10;
    m_arrow = new QGraphicsEllipseItem();
    m_scene->addItem(m_arrow);
    m_arrow->setRect(-radius * scale, -radius * scale, 2 * radius * scale,
                     2 * radius * scale);
    m_arrow->setStartAngle(0);
    m_arrow->setSpanAngle(16 * 90);
    m_arrow->setPen(QPen(Qt::blue, 10));
    //add a filled circle to the scene
    m_scene->addEllipse(
        -radius * scale + arrowWidth / 2, -radius * scale + arrowWidth / 2,
        2 * radius * scale - arrowWidth, 2 * radius * scale - arrowWidth,
        QPen(Qt::NoPen), QBrush(Qt::white));

    //add a polygon in triangle shape to the scene
    //the triangle is pointing in the direction of the movement
    QPolygonF triangle;
    int triangleSize = 10;
    if(direction > 0)
    {
        triangle << QPointF(0, -radius * scale - triangleSize);
        triangle << QPointF(0, -radius * scale + triangleSize);
        triangle << QPointF(-triangleSize, -radius * scale);
    }
    else
    {
        triangle << QPointF(radius * scale - triangleSize, 0);
        triangle << QPointF(radius * scale + triangleSize, 0);
        triangle << QPointF(radius * scale, triangleSize);
    }
    m_scene->addPolygon(triangle, QPen(Qt::blue, 2), QBrush(Qt::blue));
}

void
MainWindow::displayPatternRectangle(double width, double height)
{
    ui->label_motion->setText("Follow the pattern");
    //display a empty rectangle with a grey border of thickness m_sizeTarget
    m_scene->addRect(-width / 2, -height / 2, width, height,
                     QPen(Qt::gray, m_sizeTarget));
}
