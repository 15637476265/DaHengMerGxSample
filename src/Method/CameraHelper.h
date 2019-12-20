#ifndef CAMERAHELPER_H
#define CAMERAHELPER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QMessageBox>
#include <QWaitCondition>
#include <QComboBox>
#include <QTreeWidgetItem>
#include "DxImageProc.h"
#include "GxIAPI.h"
#include <opencv2/opencv.hpp>
#include "DataRecvThread.h"
#include <stdio.h>
#include <stdlib.h>
#include <mutex>

using namespace std ;

#define  GX_VERIFY(emStatus) \
    if (emStatus != GX_STATUS_SUCCESS)\
{\
    ShowErrorString(emStatus); \
    }  ///< 错误提示函数宏定义

class CameraHelper : public QObject
{
    Q_OBJECT
public:
    CameraHelper();
    ~CameraHelper();

public:
    static void EnumDevice(uint32_t *devCount);

    void connectDevice(int32_t deviceIndex);
    void disconnectDevice();

    void ScanCamera();
    cv::Mat getImg();


    void OpenDevice(int32_t deviceIndex);
    void CloseDevice();


    void setRoiSize(QSize);
    void setRoi( float offsetX, float offsetY,int width,int height);
    void setGain( float gain ) ;
    void setExposure( int exposure ) ; //设置曝光



private:
    void preConnectDevice();
    void initGXLib();
    GX_STATUS InitDevice();
    GX_STATUS GetDeviceInitParam();
    GX_STATUS SetPixelFormat8bit();
    cv::Mat GetStream();            //存储当前图像数据

    void UpDateDevList(int nDevNum);

    void PrepareForShowImg();           // 为显示图像准备资源
    bool PrepareForShowMonoImg();       // 为显示Gray图像准备资源
    void UnPrepareForShowImg();         //释放为图像显示准备资源

signals:
    void updateCameraDevice(QStringList);





private:
    ///相机参数
    GX_DEV_HANDLE       m_hDevice;               ///< 设备句柄
    GX_FRAME_DATA       m_FrameData;             ///< 采集图像参数
    GX_DEVICE_BASE_INFO *m_pBaseinfo;            ///< 设备信息结构提指针

    QImage              *m_ShowImage;             ///< 显示图像对象
    QTimer              *m_Timer;                ///< 定时器

    void                *m_result_image;
    bool                m_bIsSnaping;           ///< 是否正在连续采集
    bool                m_bOpenFlag;            ///< 打开设备标志
    bool                device_isOpenFlag;      //设备是否已经打开
    GX_FRAME_DATA       m_frame_data;


/*
 * 线程安全
*/
private:
    std::recursive_mutex _mtx;
    mutable std::recursive_mutex _mtxMat;



public:
    int64_t m_image_width;
    int64_t m_image_height;
    int64_t m_payload_size;
    int64_t m_pixel_color;

    int32_t deviceIndex;
    QImage* m_show_image;


    cv::Mat _mat;
    bool m_color_filter;
    int m_time_out;




};

#endif // CAMERAHELPER_H
