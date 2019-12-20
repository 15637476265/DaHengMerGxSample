#ifndef DATARECVTHREAD_H
#define DATARECVTHREAD_H

#include <QObject>
#include <QThread>
#include "DxImageProc.h"
#include "GxIAPI.h"
#include <QDebug>
#include <QMessageBox>
#include <QImage>
#include <QDebug>


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

#define BYTE unsigned char


typedef struct RECV_BUFFER
{
    int nBufferId;
    GX_FRAME_DATA frameData;     ///< 采集图像参数
    int64_t nPixelColorFilter;
    bool bStatus;
}RECV_BUFFER;

#define  IMAGE_IMPROVE_OPEN  1         ///< 开启图像质量提升



#define VERIFY_STATUS_RET(emStatus) \
    if (emStatus != GX_STATUS_SUCCESS) \
{\
    return emStatus;\
    }\




//class DataRecvThread : public QThread
//{
//public:
//    DataRecvThread();
//    static void ShowErrorString(GX_STATUS error_status)
//    {
//        char*     error_info = NULL;
//        size_t    size        = 0;
//        GX_STATUS status     = GX_STATUS_ERROR;

//        status = GXGetLastError(&error_status, NULL, &size);
//        error_info = new char[size];
//        if (NULL == error_info)
//        {
//            return;
//        }

//        status = GXGetLastError (&error_status, error_info, &size);
//        if (status != GX_STATUS_SUCCESS)
//        {
//            qDebug()<< "Error , GXGetLastError接口调用失败 ! ";;
//        }
//        else
//        {
//            qDebug() <<"Error ,纠正相机 : "<<error_info;;
//        }

//        if (NULL != error_info)
//        {
//            delete[] error_info;
//            error_info = NULL;
//        }
//    }
//};
//    DataRecvThread(GX_DEV_HANDLE &hDevice, QObject *parent = 0);
//    ~DataRecvThread();
//    void run();

//    /// 设置开停采标志
//     void SetImgFlg(bool flag);

//    /// 设置保存图片标志
//    void setSaveImageFlag(bool flag);

//    /// 保存图像
//    void SaveImageFile();

//    /// 显示错误信息
//    void ShowErrorString(GX_STATUS emErrorStatus);

//    /// 获取图像宽
//    int getImageWidth();

//    /// 获取图像高
//    int getImageHeight();

//    /// 设置接收buffer状态
//    void setRecvBuffStatus(int id, bool status);

//    /// 获取PixelColorFilter
//    int64_t getPixelColorFilter();

//    /// 获取接收帧数
//    int getRecvImgCount();


//private:

//    GX_DEV_HANDLE m_hDevice;       ///< 设备句柄
//    GX_FRAME_DATA m_FrameData;     ///< 采集图像参数

//    int             m_IndexPos;           ///< 当前图像缓冲区的位置
//    int             m_nPayLoadSize;       ///< 图像大小
//    int             m_nImageHeight;       ///< 原始图像高
//    int             m_nImageWidth;        ///< 原始图像宽
//    int             m_nWndWidth;          ///< 图像显示窗口的宽
//    int             m_nWndHeight;         ///< 图像显示窗口的高
//    int64_t         m_nPixelFormat;       ///< 数据格式
//    int64_t         m_nPixelColorFilter;  ///< 像素格式
//    float               m_RecvImgCount;              ///< 接收帧数统计

//    bool            m_SaveImageFlag;      ///< 保存图片标志
//    bool            m_bGetImg;            ///< 采集线程是否结束的标志：true 运行；false 退出
//    void            *m_pResultImage;      ///< 当前图像缓冲区指针
//    void            *m_pImageRaw8Buf;     ///< 图像缓冲区指针
//    RECV_BUFFER     m_RecvBuffer[MAX_NUM];///< 接收图像结构体指针
//    QImage          *m_pImage[MAX_NUM];   ///< 图像对象指针


//signals:
//    void            _sigShow(RECV_BUFFER *);  ///< 显示图像信号
//};

#endif // DATARECVTHREAD_H
