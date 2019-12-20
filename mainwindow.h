#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "src/Method/CameraHelper.h"
#include <QTimer>
#include <thread>
#include "src/Widget/QCvDisplay.h"
#include <QScopedPointer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void initObject();
    void initSignals();
    void showCvImage(cv::Mat cvImg);
    void saveImg(const cv::Mat &mat, const std::string &_path = "", const std::string &_name = "",const std::string &modify = "");
public slots:
    void slot_deviceUpdate(const QStringList&);
    void slot_DisplayResize(QSize);

    void showPicture();
private slots:

    void on_btn_Scan_clicked();

    void on_tree_device_itemClicked(QTreeWidgetItem *item, int column);

    void on_btn_Capture_clicked();

    void on_horizontalSlider_sliderMoved(int position);

    void on_hsd_gain_sliderMoved(int position);

    void on_btn_OpenCamera_clicked(bool checked);

    void on_btn_SaveImg_clicked();

    void on_checkBox_clicked(bool checked);

    void on_lEd_gain_editingFinished();

    void on_lEd_exp_editingFinished();

private:
    Ui::MainWindow *ui;
    CameraHelper *cameraService;
    int32_t deviceIndex;        //当前设备ID
    QMutex   m_Mutex;                 ///< 互斥锁
    bool isCaping;
    QTimer* qTimer;         //刷新图像的定时器
    QTimer* saveTimer;         //刷新图像的定时器
    QCvDisplay *cvWidget;

    QScopedPointer<std::thread>  saveThread;

    bool allowSave;

};

#endif // MAINWINDOW_H
