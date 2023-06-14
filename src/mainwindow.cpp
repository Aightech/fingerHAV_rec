#include "mainwindow.h"
#include "../forms/ui_mainwindow.h"
#include "lsl_cpp.h"
#include <QComboBox>
#include <QDateTime>
#include <QFile>

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
            if(results[i].name() == "kistler")
            {
                ui->pushButton_kistler->setIcon(
                    QIcon::fromTheme("user-available"));
                m_availableStreams.push_back(results[i].name());
            }
            if(results[i].name() == "loadcells")
            {
                ui->pushButton_loadcells->setIcon(
                    QIcon::fromTheme("user-available"));
                lsl::stream_inlet inlet(results[i], 360, 0, false);
                m_availableStreams.push_back(results[i].name());
            }
            if(results[i].name() == "positions")
            {
                ui->pushButton_finger_pos->setIcon(
                    QIcon::fromTheme("user-available"));
                m_inlet_finger_pos = new lsl::stream_inlet(results[i], 1);
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

    //add a circle to the graphicsView_motion
    m_scene = new QGraphicsScene();
    m_scene->addEllipse(0, 0, 100, 100);
    ui->graphicsView_motion->setScene(m_scene);

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

        m_matTrial.push_back(
            std::vector<std::tuple<std::string, std::string, int>>());
        QTreeWidgetItem *item = ui->treeWidget_mat->topLevelItem(i);
        for(int j = 0; j < m_nbTrial; j++)
        {
            QTreeWidgetItem *child = new QTreeWidgetItem(item);
            child->setText(0, "trial" + QString::number(j + 1));
            child->setText(1, "");
            m_matTrial[i].push_back(
                std::make_tuple(m_userName + "_" + item->text(0).toStdString() +
                                    "_" + child->text(0).toStdString(),
                                "", 0));
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
}

void
MainWindow::pushButton_comment_released()
{
    std::string comment = ui->textEdit_comment->toPlainText().toStdString();
    // get the comment from the lineEdit_comment
    std::get<1>(m_matTrial[m_mat][m_trial]) = comment;
    //add the comment in the treeWidget_mat
    ui->treeWidget_mat->topLevelItem(m_mat)->child(m_trial)->setText(
        1, QString(comment.c_str()));
    //add the comment in the log file
    addLog("(" + ui->label_trialID->text().toStdString() +
               ") COMMENTS: " + comment,
           true);
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
    ui->pushButton_comment->setEnabled(true);
    ui->pushButton_startTrial->setEnabled(true);
    std::cout << "material: " << m_mat << " trial: " << m_trial << std::endl;
    //set the material in the treeWidget_mat to "user-away"
    unselect_trial();
    if(std::get<2>(m_matTrial[m_mat][m_trial]) == 0)
        item->setIcon(0, QIcon::fromTheme("user-available"));
    else
        item->setIcon(0, QIcon::fromTheme("user-away"));
    ui->label_matName->setText(item->parent()->text(0));
    ui->label_trialID->setText(QString(m_userName.c_str()) + "_" +
                               item->parent()->text(0) + "_" + item->text(0));
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
            if(std::get<2>(m_matTrial[mat][i]) == 0)
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
    //set the trial as done
    std::get<2>(m_matTrial[m_mat][m_trial])++;
    //remove the icon
    ui->treeWidget_mat->topLevelItem(m_mat)->child(m_trial)->setText(
        2, QString::number(std::get<2>(m_matTrial[m_mat][m_trial])));

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
    int d = 100;
    int D = 200;
    int a = 500;
    //motion circle color
    QPen penMotion;
    penMotion.setColor(Qt::red);
    penMotion.setWidth(3);
    QPen penFinger_pos;
    penFinger_pos.setColor(Qt::blue);
    penFinger_pos.setWidth(3);

    m_scene->addRect(-a / 2, -a / 2, a, a);
    if(m_inlet_finger_pos != nullptr)
    {
        //get the finger position
        std::vector<std::vector<float>> finger_pos;
        int n = finger_pos.size() - 1;
        // if(m_inlet_finger_pos->pull_chunk(finger_pos))
        //     m_scene->addEllipse((finger_pos[n][0] - 0.5) * a - d / 2,
        //                         (finger_pos[n][1] - 0.5) * a - d / 2, d, d,
        //                         penFinger_pos);
    }

    if(m_trialInProgress)
    {
        //update the timer
        m_time += m_timeStep;

        ui->lcdNumber_timer->display((m_timeMax - m_time) / 1000);

        //motion of a circle on the graphicsView
        //first 10s circle turn clockwise around the center
        if(m_time < 10000)
        {
            ui->label_motion->setText("circle");
            m_scene->addEllipse(D * cos(m_time / 1000.0) - d / 2,
                                D * sin(m_time / 1000.0) - d / 2, d, d,
                                penMotion);
        }
        //second 10s circle turn counter-clockwise around the center
        else if(m_time < 20000)
        {
            ui->label_motion->setText("circle");
            m_scene->addEllipse(D * cos(-m_time / 1000.0) - d / 2,
                                D * sin(-m_time / 1000.0) - d / 2, d, d,
                                penMotion);
        }
        //third 10s circle move back and forth horizontally
        else if(m_time < 30000)
        {
            ui->label_motion->setText("line");
            m_scene->addEllipse(D * cos(m_time / 1000.0) - d / 2, 0 - d / 2, d,
                                d, penMotion);
        }
        //fourth 10s circle move back and forth vertically
        else if(m_time < 40000)
        {
            ui->label_motion->setText("line");
            m_scene->addEllipse(0 - d / 2, D * sin(m_time / 1000.0) - d / 2, d,
                                d, penMotion);
        }
        else
        {
            ui->label_motion->setText("freestyle");
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
        m_scene->addEllipse(-d / 2, -d / 2, d, d);
    }
}
