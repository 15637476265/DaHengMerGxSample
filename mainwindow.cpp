#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <fcntl.h>
#include <sys/stat.h>


using namespace std;
using namespace cv;

string getTime()
{
    time_t timep;
    time(&timep);
    char tmp[32];
    strftime(tmp, sizeof(tmp), "%H:%M:%S",localtime(&timep) );
    return tmp;
}

string getFullTime()
{
    time_t timep;
    time(&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&timep) );
    return tmp;
}

string getData()
{
    time_t timep;
    time(&timep);
    char tmp[32];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d",localtime(&timep) );
    return tmp;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ::mkdir("pictures/",S_IRWXU);
    ui->tree_device->setHeaderLabel(tr("选择相机"));    //设置标题
    initObject();
    initSignals();
    ui->lay_stream->addWidget(cvWidget);

}

void MainWindow::initObject()
{
    cameraService = new CameraHelper();
    cvWidget = new QCvDisplay(this);
    qTimer = new QTimer();
    saveTimer = new QTimer();
}

void MainWindow::showCvImage(cv::Mat cvImg)
{
    cvWidget->display(cvImg);
}

void MainWindow::showPicture()
{
    cv::Mat _mat = cameraService->getImg();
    showCvImage(_mat);
    allowSave = true;
    ui->btn_SaveImg->setEnabled(allowSave);
}

MainWindow::~MainWindow()
{
    delete ui;
    saveThread.reset(nullptr);
}


void MainWindow::initSignals()
{
    connect(cameraService,&CameraHelper::updateCameraDevice,this,&MainWindow::slot_deviceUpdate);
    connect(cvWidget,&QCvDisplay::resizeWidget,this,&MainWindow::slot_DisplayResize);

}

void MainWindow::slot_deviceUpdate(const QStringList &list)
{
    ui->tree_device->clear();
    QTreeWidgetItem *root = new QTreeWidgetItem(ui->tree_device);
    root->setText(0,"USB 2.0");

    foreach (QString d_item, list) {
        QTreeWidgetItem *pDevItem = new QTreeWidgetItem(root, QStringList(d_item));
        root->addChild(pDevItem);  //将分支插入到root目录下
    }

    ui->tree_device->expandAll();
    qDebug()<<"刷新设备节点完成！"<<list;
}

void MainWindow::slot_DisplayResize(QSize size)
{
    cameraService->setRoiSize(size);

}


void MainWindow::on_btn_Scan_clicked()
{
    cameraService->ScanCamera();
}


void MainWindow::on_tree_device_itemClicked(QTreeWidgetItem *item, int column)
{
    QTreeWidgetItem *pParentItem = item->parent();

    if (NULL == pParentItem) //最顶端的没有父节点，点击是需注意
    {
        ui->btn_OpenCamera->setDisabled(true);
        return ;
    }

    QString info = item->data(0,column).toString();
    QString serialIndex = info.mid(15,12);
    qDebug()<<"Device Type :"<<info.left(14);
    qDebug()<<"Serial :"<<serialIndex;
    qDebug()<<"Device Index :"<<column;


    deviceIndex = column;
    qDebug()<<deviceIndex;
    //使能打开设备按钮
    ui->btn_OpenCamera->setDisabled(false);
}

void MainWindow::on_btn_Capture_clicked()
{
    if(!isCaping)
    {
        QObject::connect(qTimer, SIGNAL(timeout()),this, SLOT(showPicture())) ;
        qTimer->start(100);
        isCaping = true;
        ui->btn_Capture->setText("停止数据流");
    }
    else
    {
        if(isCaping)
        {
            qTimer->stop();
            QObject::disconnect(qTimer, SIGNAL(timeout()),this, SLOT(showPicture())) ;
        }
        isCaping = false;
        ui->btn_Capture->setText("开启数据流");
    }
}


void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    cameraService->setExposure(position);
    ui->lEd_exp->setText(QString::number(position));
}

void MainWindow::on_hsd_gain_sliderMoved(int position)
{
    cameraService->setGain(position);
    ui->lEd_gain->setText(QString::number(position));
}

void MainWindow::on_btn_OpenCamera_clicked(bool checked)
{
    if(checked){
        cameraService->connectDevice( deviceIndex+1 );
        ui->btn_OpenCamera->setText("关闭设备");
    }else {
        cameraService->CloseDevice();
        allowSave = false;
        ui->btn_SaveImg->setEnabled(false);
        ui->btn_OpenCamera->setText("打开设备");
    }

}

void MainWindow::saveImg(const cv::Mat &mat, const std::string &_path, const std::string &_name,const std::string &modify)
{
    string strName ;
    if("" == _path)
        strName = "pictures/" ;
    else
        strName = _path;
    if("" == _name){
        strName += getData()+ "/";
        if("" == modify){
            strName += getTime() + ".png";
        }else {
            strName += getTime() + modify + ".png";
        }
    }else{
        strName += _name;
    }


    if ( !mat.data ) {
        cout<<"No Data:"<<strName<<endl;
    }

    imwrite( strName , mat) ;

}


void MainWindow::on_btn_SaveImg_clicked()
{
    cv::Mat _mat = cameraService->getImg();
    saveImg(_mat);
}



void MainWindow::on_checkBox_clicked(bool checked)
{
    allowSave = checked;
    saveThread.reset(new std::thread([this](){
        static int counts = 0;

        while (isCaping || allowSave) {
            std::this_thread::sleep_for( std::chrono::milliseconds(30000) ) ;
            cv::Mat _mat = cameraService->getImg();
            cv::imwrite("pictures/"+to_string(counts++)+".png",_mat);
        }
    }));
    saveThread.data()->detach();
}

void MainWindow::on_lEd_gain_editingFinished()
{
    cameraService->setGain(ui->lEd_gain->text().toInt());
}

void MainWindow::on_lEd_exp_editingFinished()
{
    cameraService->setExposure(ui->lEd_exp->text().toInt());
}
