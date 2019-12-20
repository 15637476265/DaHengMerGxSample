#include "CameraHelper.h"

#include "GxIAPI.h"
#include <iostream>
#include <unistd.h>
#include <chrono>
#include "DxImageProc.h"
#include <string>
#include <thread>
#include <QMessageBox>

using namespace std;
using namespace std::chrono ;

void ShowErrorString(GX_STATUS emErrorStatus)
{
    char      chErrorInof[512] = {0};
    size_t    nSize     = 512;
    GX_STATUS emStatus = GX_STATUS_ERROR;

    emStatus = GXGetLastError (&emErrorStatus, chErrorInof, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        QMessageBox::about(NULL, "About", "GXGetLastError接口调用失败！");
    }
    else
    {
        qDebug() << "error";
        QMessageBox::about(NULL, "About", QObject::tr("%1").arg(QString(QLatin1String(chErrorInof))));
    }
}


CameraHelper::CameraHelper()
    :m_bIsSnaping(false)
    ,m_bOpenFlag(false)
    ,m_color_filter(false)
    ,m_time_out(2000)
    ,m_image_width(0)
    ,m_image_height(0)
{
    m_pBaseinfo      = nullptr;    ///< 设备信息
    m_hDevice        = nullptr;    ///< 设备句柄
    m_ShowImage      = nullptr;    ///< 显示图像指针
    memset(&m_frame_data, 0, sizeof(GX_FRAME_DATA));
    preConnectDevice();
}

CameraHelper::~CameraHelper()
{
    GX_STATUS status = GX_STATUS_ERROR;
    if (device_isOpenFlag)
    {
        CloseDevice();
    }
    if(m_hDevice != nullptr)
    {
        m_hDevice = nullptr;
    }
    status = GXCloseLib();
}


void CameraHelper::ScanCamera()
{
    uint32_t nDevNum = 0;//设备个数
    EnumDevice(&nDevNum);
    //更新设备列表
    UpDateDevList(nDevNum);
}


void CameraHelper::preConnectDevice()
{
    std::lock_guard<std::recursive_mutex> lck(_mtx);

    GX_STATUS status = GX_STATUS_SUCCESS;
    initGXLib();

    if ( (m_hDevice != NULL) && (!m_bOpenFlag) )
    {
        status = GXCloseDevice(m_hDevice);
        GX_VERIFY(status);
        m_hDevice = NULL;
    }
}


void CameraHelper::initGXLib()
{
    GX_STATUS status = GX_STATUS_SUCCESS;
    status = GXInitLib();
    if (status != GX_STATUS_SUCCESS)
    {
        ShowErrorString(status);
        exit(0);
    }
}

void CameraHelper::setExposure(int exposure)
{
    if(m_hDevice == nullptr){
        return;
    }

    GX_STATUS status = GX_STATUS_SUCCESS;
    status = GXSetFloat(m_hDevice, GX_FLOAT_EXPOSURE_TIME,exposure );
    GX_VERIFY(status);
}


void CameraHelper::OpenDevice(int32_t deviceIndex)
{
    GX_STATUS retStatus = GX_STATUS_SUCCESS;
    retStatus = GXOpenDeviceByIndex(deviceIndex,&m_hDevice);
    if (retStatus != GX_STATUS_SUCCESS){
        qDebug() << "open device error";
        ShowErrorString(retStatus);
        return ;
    }

    device_isOpenFlag = true;

}

GX_STATUS CameraHelper::GetDeviceInitParam()
{
    std::lock_guard<std::recursive_mutex> lck(_mtx);

    GX_STATUS status = GX_STATUS_SUCCESS;
    bool      is_implemented = false;
    // 查询当前相机是否支持GX_ENUM_PIXEL_COLOR_FILTER
    status = GXIsImplemented(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &is_implemented);
    VERIFY_STATUS_RET(status);
    m_color_filter = is_implemented;
//    //支持彩色图像
//    if(is_implemented)
//    {
//        status = GXGetEnum(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &m_pixel_color);
//        VERIFY_STATUS_RET(status);
//    }
    //获取图像宽度
    status = GXGetInt(m_hDevice, GX_INT_WIDTH, &m_image_width);
    VERIFY_STATUS_RET(status);
    //获取图像高度
    status = GXGetInt(m_hDevice, GX_INT_HEIGHT, &m_image_height);
    VERIFY_STATUS_RET(status);
    //获取图像大小
    status = GXGetInt(m_hDevice, GX_INT_PAYLOAD_SIZE, &m_payload_size);

    _mat.create(m_image_height,m_image_width,CV_8UC3);

    return status;
}

cv::Mat CameraHelper::getImg()
{
    //std::lock_guard<std::recursive_mutex> lck(_mtx);
    return const_cast<CameraHelper*>(this)->GetStream() ;
}


void CameraHelper::EnumDevice(uint32_t *devCount)
{
    GX_STATUS status = GX_STATUS_SUCCESS;
    status = GXUpdateDeviceList(devCount, UPDATE_TIME_OUT);
    GX_VERIFY(status);
}


void CameraHelper::UpDateDevList(int nDevNum)
{
    QString strDevInfo = "";

    if (m_pBaseinfo != NULL)
    {
        delete []m_pBaseinfo;
        m_pBaseinfo = NULL;
    }
    m_pBaseinfo = new GX_DEVICE_BASE_INFO[nDevNum];

    //获取所有枚举相机信息
    size_t size = nDevNum * sizeof(GX_DEVICE_BASE_INFO);
    GXGetAllDeviceBaseInfo(m_pBaseinfo, &size);

    QStringList device_list;
    qDebug()<< "枚举相机信息完毕!";
    for (int i = 0; i < nDevNum; i++)
    {
        strDevInfo.sprintf("%s", m_pBaseinfo[i].szDisplayName);
        device_list.push_back(strDevInfo);
        //m_mapItemDev[strDevInfo] = i + 1;
    }

    emit updateCameraDevice(device_list);
}


GX_STATUS CameraHelper::SetPixelFormat8bit()
{
    std::lock_guard<std::recursive_mutex> lck(_mtx);

    GX_STATUS status    = GX_STATUS_SUCCESS;
    int64_t   pixel_size  = 0;
    uint32_t  enum_entry  = 0;
    size_t    buffer_size = 0;
    GX_ENUM_DESCRIPTION  *enum_description = NULL;
    // 获取像素点大小
    status = GXGetEnum(m_hDevice, GX_ENUM_PIXEL_SIZE, &pixel_size);
    if (status != GX_STATUS_SUCCESS)
    {
        return status;
    }
    // 判断为8bit时直接返回,否则设置为8bit
    if (pixel_size == GX_PIXEL_SIZE_BPP8)
    {
        return GX_STATUS_SUCCESS;
    }
    else
    {
        // 获取设备支持的像素格式枚举数
        status = GXGetEnumEntryNums(m_hDevice, GX_ENUM_PIXEL_FORMAT, &enum_entry);
        if (status != GX_STATUS_SUCCESS)
        {
            return status;
        }
        // 为获取设备支持的像素格式枚举值准备资源
        buffer_size      = enum_entry * sizeof(GX_ENUM_DESCRIPTION);
        enum_description = new GX_ENUM_DESCRIPTION[enum_entry];
        // 获取支持的枚举值
        status = GXGetEnumDescription(m_hDevice, GX_ENUM_PIXEL_FORMAT, enum_description, &buffer_size);
        if (status != GX_STATUS_SUCCESS)
        {
            if (enum_description != NULL)
            {
                delete []enum_description;
                enum_description = NULL;
            }
            return status;
        }
        // 遍历设备支持的像素格式,设置像素格式为8bit,
        // 如设备支持的像素格式为Mono10和Mono8则设置其为Mono8
        for (uint32_t i = 0; i < enum_entry; i++)
        {
            if ((enum_description[i].nValue & GX_PIXEL_8BIT) == GX_PIXEL_8BIT)
            {
                status = GXSetEnum(m_hDevice, GX_ENUM_PIXEL_FORMAT, enum_description[i].nValue);
                break;
            }
        }
        // 释放资源
        if (enum_description != NULL)
        {
            delete []enum_description;
            enum_description = NULL;
        }
    }
    return status;
}

GX_STATUS CameraHelper::InitDevice()
{
    std::lock_guard<std::recursive_mutex> lck(_mtx);

    GX_STATUS status = GX_STATUS_SUCCESS;
    //设置采集模式连续采集
    status = GXSetEnum(m_hDevice,GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
    VERIFY_STATUS_RET(status);
    //设置触发模式为开
    status = GXSetEnum(m_hDevice,GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_ON);
    VERIFY_STATUS_RET(status);
    //调用SetPixelFormat8bit函数将图像数据格式设置为8Bit
    status = SetPixelFormat8bit();
    VERIFY_STATUS_RET(status);
    //选择触发源为软触发
    //    status = GXSetEnum(m_device_handle,GX_ENUM_TRIGGER_MODE, GX_COMMAND_TRIGGER_SOFTWARE);
    //    GX_VERIFY(status);

    //设置自动白平衡
    //    status=GXSetEnum(m_device_handle,GX_ENUM_BALANCE_WHITE_AUTO,GX_BALANCE_WHITE_AUTO_OFF);
    //    GX_VERIFY(status);

    bool flag = false;
    GXIsImplemented(m_hDevice, GX_ENUM_BALANCE_WHITE_AUTO, &flag);
    if (flag == true)
    {
        //设置白平衡
        status = GXSetEnum(m_hDevice, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_OFF);
        GX_VERIFY(status);
    }
    //设置曝光
    //int exposure = OpenConfig::get<int>("conf/camera.json",_cam_index+".exposure");
    setExposure(11000);
    //status = GXSetEnum(m_hDevice, GX_ENUM_EXPOSURE_AUTO, GX_EXPOSURE_AUTO_CONTINUOUS);
    return status;
}


void CameraHelper::PrepareForShowImg()
{
    std::lock_guard<std::recursive_mutex> lck(_mtx);
    bool ret = false;
    m_frame_data.pImgBuf = new char[m_payload_size];
    m_frame_data.nHeight = m_image_height;
    m_frame_data.nWidth = m_image_width;
    m_frame_data.nPixelFormat = m_pixel_color;

    if(m_frame_data.pImgBuf == NULL)
        return;
    ret = PrepareForShowMonoImg();

    if(!ret){
        //释放为保存图像分配Buffer,释放为图像显示准备资源
        UnPrepareForShowImg();
    }
}

bool  CameraHelper::PrepareForShowMonoImg()
{
    //创建QImage对象，用于显示图像
    std::lock_guard<std::recursive_mutex> lck(_mtx);
    m_show_image = new QImage(m_image_width, m_image_height, QImage::Format_RGB888);
    if(m_show_image == nullptr)
    {
        return false;
    }

    m_result_image = m_show_image->bits();
    return true;
}


void CameraHelper::UnPrepareForShowImg()
{
    std::lock_guard<std::recursive_mutex> lck(_mtx);
    //释放显示空间
    if (m_ShowImage != NULL)
    {
        delete m_ShowImage;
        m_ShowImage = NULL;
    }
}



cv::Mat CameraHelper::GetStream()
{
    std::lock_guard<recursive_mutex> lck(_mtx);
    try
    {
        if(device_isOpenFlag && m_bIsSnaping && m_hDevice != nullptr)
        {
            GX_STATUS status  = GX_STATUS_ERROR;
            //发送软触发命令
            status = GXSendCommand(m_hDevice,GX_COMMAND_TRIGGER_SOFTWARE);
            GX_VERIFY(status);
            //获取图像
            status = GXGetImage(m_hDevice, &m_frame_data, m_time_out);
            GX_VERIFY(status);
            // 获取到的图像是否为完整的图像
            if ((&m_frame_data)->nStatus != 0)
            {
                return _mat;;
                cout<<"图片采集失败"<<endl;
            }

            DxRaw8toRGB24((char*)(&m_frame_data)->pImgBuf, m_result_image, m_image_width, m_image_height,RAW2RGB_NEIGHBOUR,
                          DX_PIXEL_COLOR_FILTER(NONE),false);
            _mat.release();
            cv::Mat mat(m_image_height,m_image_width,CV_8UC3);
            memcpy(mat.data,m_result_image,m_image_width*m_image_height*3);
            {
                std::lock_guard<std::recursive_mutex> lck(_mtxMat);
                cv::cvtColor(mat,_mat,CV_RGB2BGR);
            }
            return _mat;
        }else{
            //BOOST_THROW_EXCEPTION(WZError() << err_camera("连接相机失败")) ;
        }
    }
    catch(...)
    {
        connectDevice(deviceIndex);
        disconnectDevice();
    }
    return _mat;

}


void CameraHelper::connectDevice(int32_t deviceIndex)
{
    GX_STATUS status = GX_STATUS_SUCCESS;
    OpenDevice(deviceIndex);
    deviceIndex = deviceIndex;
    m_bOpenFlag = true;
    status = InitDevice();
    GX_VERIFY(status);
    status = GetDeviceInitParam();
    GX_VERIFY(status);
    //为图像显示准备资源
    PrepareForShowImg();

    //发送开采命令
    status = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_START);
    GX_VERIFY(status);
    m_bIsSnaping = true;

}

void CameraHelper::disconnectDevice()
{
    std::lock_guard<std::recursive_mutex> lck(_mtx);

    GX_STATUS status = GX_STATUS_SUCCESS;

    if(m_bIsSnaping)
    {
        status = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP);
        GX_VERIFY(status);
        m_bIsSnaping = false;
    }

    CloseDevice();

    m_bOpenFlag = false;
    m_hDevice = NULL;
}

void CameraHelper::CloseDevice()
{
    std::lock_guard<std::recursive_mutex> lck(_mtx);

    GX_STATUS status = GX_STATUS_SUCCESS;
    // 释放资源
    UnPrepareForShowImg();
    // 关闭设备
    status = GXCloseDevice(m_hDevice);
    GX_VERIFY(status);
}

void CameraHelper::setRoiSize(QSize size)
{
    std::lock_guard<std::recursive_mutex> lck(_mtx) ;
    m_image_width = size.width();
    m_image_height = size.height();
    cv::Size dsize = cv::Size(m_image_width,m_image_height);
    _mat = cv::Mat(dsize,CV_8U);
}

void CameraHelper::setRoi(float offsetX, float offsetY, int width, int height)
{
    std::lock_guard<std::recursive_mutex> lck(_mtx) ;
    if(m_hDevice == nullptr)
        return;
    GX_STATUS status = GX_STATUS_SUCCESS;
    GX_INT_RANGE stIntRange;

    status = GXGetIntRange(m_hDevice, GX_INT_WIDTH, &stIntRange);

    status = GXSetInt(m_hDevice, GX_INT_WIDTH, width);
    GX_VERIFY(status);
    status = GXSetInt(m_hDevice, GX_INT_HEIGHT, height);
    GX_VERIFY(status);
    status = GXSetInt(m_hDevice, GX_INT_OFFSET_X, offsetX);
    GX_VERIFY(status);
    status = GXSetInt(m_hDevice, GX_INT_OFFSET_Y, offsetY);
    GX_VERIFY(status);

}

void CameraHelper::setGain(float gain)
{
    std::lock_guard<std::recursive_mutex> lck(_mtx) ;
    if(m_hDevice == nullptr)
        return;
    GX_STATUS status     = GX_STATUS_ERROR;
    status = GXSetEnum(m_hDevice, GX_ENUM_GAIN_SELECTOR, GX_GAIN_SELECTOR_ALL);
    GX_VERIFY(status);

    status = GXSetFloat(m_hDevice, GX_FLOAT_GAIN, gain);
    GX_VERIFY(status);
}

