#ifndef CAMERAHELPER_H
#define CAMERAHELPER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QMutex>
#include <QMessageBox>
#include <QWaitCondition>


#include "DxImageProc.h"
#include "GxIAPI.h"
#include <opencv2/opencv.hpp>


#define SHOW_IMAGE          ///< 显示图像宏
#define MAX_NUM 2           ///< 接收图像缓冲区个数
#define GET_IMAGE_TIME_OUT   100
typedef qint8 int8_t;
typedef qint16 int16_t;
typedef qint32 int32_t;


typedef  quint8  uint8_t;
typedef  quint16 uint16_t;
typedef  quint32 uint32_t;

#define UPDATE_TIME_OUT 1000

/// 如果结果为false，则跳出循环，否则不处理
#define UI_CHECK_BOOL_RESULT(result) \
    if(!result)\
{\
    break;\
    }

/// 如果结果为NULL，则跳出循环，否则不处理
#define UI_CHECK_NEW_MEMORY(new_memory)\
    if(NULL == new_memory)\
{\
    break;\
    }

#define  GX_VERIFY(emStatus) \
    if (emStatus != GX_STATUS_SUCCESS)\
{\
    ShowErrorString(emStatus); \
    }  ///< 错误提示函数宏定义

#define VERIFY_STATUS_RET(emStatus) \
    if (emStatus != GX_STATUS_SUCCESS) \
{\
    return emStatus;\
    }\

#define BYTE unsigned char


typedef struct RECV_BUFFER
{
    int nBufferId;
    GX_FRAME_DATA frameData;     ///< 采集图像参数
    int64_t nPixelColorFilter;
    bool bStatus;
}RECV_BUFFER;

#define  IMAGE_IMPROVE_OPEN  1         ///< 开启图像质量提升
#define  GX_VERIFY(emStatus) \
         if(emStatus != GX_STATUS_SUCCESS) \
         { \
            qDebug() << "emStatus = " << emStatus; \
            ShowErrorString(emStatus); \
            return;\
         }  ///< 错误提示函数宏定义



class CameraHelper : public QThread
{
    Q_OBJECT
public:
    CameraHelper(GX_DEV_HANDLE &hDevice, QObject *parent = nullptr);
public:

    ~CameraHelper();
    GX_STATUS OpenDevice();

    GX_STATUS CloseDevice();

    GX_STATUS InitDevice();

    GX_STATUS SetPixelFormat8bit();

    GX_STATUS GetDeviceInitParam();

    cv::Mat getImg();

    cv::Mat takePicture();

    void connectDevice();

    void disconnectDevice();

    void ShowErrorString(GX_STATUS );

    void setRoi( float offsetX, float offsetY,int width,int height);

    void setGain( float gain ) ;

    void setExposure( int exposure ) ;
    /// 为显示图像准备资源
    bool PrepareForShowImg();
    /// 为彩色图像显示准备资源,分配Buffer
    bool PrepareForShowColorImg();
    /// 为黑白图像显示准备资源,分配Buffer
    bool PrepareForShowMonoImg();
    /// 释放为显示图像准备的资源
    void UnPrepareForShowImg();

    GX_FRAME_DATA getFrameDate();

private:
    GX_DEV_HANDLE  m_device_handle;
    GX_FRAME_DATA m_frame_data;
    int64_t m_pixel_color;
    std::recursive_mutex _mtx;
    mutable std::recursive_mutex _mtxMat;
    cv::Mat _mat;
    bool m_is_open;
    bool m_is_snap;
    bool m_color_filter;
    int m_time_out;

public:
    int64_t m_image_width;
    int64_t m_image_height;
    int64_t m_payload_size;
    QImage* m_show_image;
    void* m_result_image;
};

#endif // CAMERAHELPER_H
