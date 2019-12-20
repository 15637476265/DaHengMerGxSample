//#include "DataRecvThread.h"

//#include <QFile>

//void ShowErrorString(GX_STATUS error_status)
//{
//    char*     error_info = NULL;
//    size_t    size        = 0;
//    GX_STATUS status     = GX_STATUS_ERROR;

//    status = GXGetLastError(&error_status, NULL, &size);
//    error_info = new char[size];
//    if (NULL == error_info)
//    {
//        return;
//    }

//    status = GXGetLastError (&error_status, error_info, &size);
//    if (status != GX_STATUS_SUCCESS)
//    {
//        cout<< "Error , GXGetLastError接口调用失败 ! "<<endl;;
//    }
//    else
//    {
//        cout <<"Error ,纠正相机 : "<<error_info<<endl;;
//    }

//    if (NULL != error_info)
//    {
//        delete[] error_info;
//        error_info = NULL;
//    }
//}

//DataRecvThread::DataRecvThread(GX_DEV_HANDLE &hDevice, QObject *parent) :
//    QThread(parent)
//{
//    int64_t nValue = 0;
//    bool bColorFliter = false;
//    GX_STATUS emStatus = GX_STATUS_SUCCESS;
//    //初始化成员变量
//    for (int i = 0; i < MAX_NUM; i++)
//    {
//        m_pImage[i] = NULL;
//    }
//    m_pResultImage = NULL;
//    m_IndexPos = 0;
//    m_hDevice = hDevice;
//    memset(&m_FrameData, 0, sizeof(GX_FRAME_DATA));
//    m_bGetImg = false;
//    m_RecvImgCount = 0;

//    m_SaveImageFlag = false;
//    m_nPixelColorFilter = GX_COLOR_FILTER_NONE;
//    //判断相机是否支持bayer格式
//    emStatus = GXIsImplemented(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &bColorFliter);
//    GX_VERIFY(emStatus);
//    if (bColorFliter)
//    {
//        emStatus = GXGetEnum(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &m_nPixelColorFilter);
//        GX_VERIFY(emStatus);
//    }

//    emStatus = GXGetEnum(m_hDevice, GX_ENUM_PIXEL_FORMAT, &m_nPixelFormat);
//    GX_VERIFY(emStatus);
//    m_FrameData.nPixelFormat = m_nPixelFormat;

//    //获取宽度(需在停止采集状态下设置)
//    emStatus = GXGetInt(m_hDevice, GX_INT_WIDTH, &nValue);
//    GX_VERIFY(emStatus)
//    m_nImageWidth = (int)nValue;
//    m_FrameData.nWidth = m_nImageWidth;

//    //获取高度(需在停止采集状态下设置)
//    emStatus = GXGetInt(m_hDevice, GX_INT_HEIGHT, &nValue);
//    GX_VERIFY(emStatus);
//    m_nImageHeight = (int)nValue;
//    m_FrameData.nHeight = m_nImageHeight;

//    //获取图象数据大小
//    emStatus = GXGetInt(m_hDevice, GX_INT_PAYLOAD_SIZE, &nValue);
//    GX_VERIFY(emStatus)
//    m_nPayLoadSize = (int)nValue;

//    for (int i = 0; i < MAX_NUM; i++)
//    {
//        m_RecvBuffer[i].frameData.pImgBuf = new char[m_nPayLoadSize];
//        m_RecvBuffer[i].frameData.nHeight = m_nImageHeight;
//        m_RecvBuffer[i].frameData.nWidth = m_nImageWidth;
//        m_RecvBuffer[i].frameData.nPixelFormat = m_nPixelFormat;
//        m_RecvBuffer[i].nPixelColorFilter = m_nPixelColorFilter;
//        m_RecvBuffer[i].bStatus = true;
//        m_RecvBuffer[i].nBufferId = i;
//    }

//    //建立信号和槽
//    QObject::connect(this, SIGNAL(_sigShow(RECV_BUFFER *)), parent, SLOT(slotRecvImg(RECV_BUFFER *)));
//}

//DataRecvThread::~DataRecvThread()
//{
//    for (int i = 0; i < MAX_NUM; i++)
//    {
//        if (m_RecvBuffer[i].frameData.pImgBuf != NULL)
//        {
//            delete []m_RecvBuffer[i].frameData.pImgBuf;
//            m_RecvBuffer[i].frameData.pImgBuf = NULL;
//        }
//        m_RecvBuffer[i].frameData.nHeight = 0;
//        m_RecvBuffer[i].frameData.nWidth = 0;
//        m_RecvBuffer[i].frameData.nPixelFormat = 0;
//        m_RecvBuffer[i].nPixelColorFilter = 0;
//        m_RecvBuffer[i].bStatus = false;
//        m_RecvBuffer[i].nBufferId = 0;
//    }
//}

////---------------------------------------------------------------------------------
///**
//\brief   接收线程
//接收线程负责接收图像，此处使用2个buffer，接收线程接收到图像数据放到第一个buffer中，将buffer
//状态标志位设置为false，然后判断下一个buffer状态是否为true，如果不为true，说明下一个buffer
//还在被主线程使用，接收线程接收下一帧图像接续放到第一个buffer中，直到下一个buffer状态为true，
//当下一个buffer状态为true时，说明主线程已经使用完该buffer，发信号通知主线程，将当前的buffer
//交给主线程处理，接收线程将接收到的图像放到下一个buffer中。这样能够保证接收图像的帧率
//\return  无
//*/
////----------------------------------------------------------------------------------
//void DataRecvThread::run()
//{
//    GX_STATUS status = GX_STATUS_SUCCESS;
//    m_IndexPos       = 0;
//    int nextPos      = m_IndexPos + 1;

//    //初始化为true
//    for (int i = 0; i < MAX_NUM; i++)
//    {
//        m_RecvBuffer[i].bStatus = true;
//    }

//    while(1)
//    {
//        //判断是否停采，停采退出
//        if (m_bGetImg == false)
//        {
//            //qDebug() << "quit thread !";
//            break;
//        }

//        if(m_RecvBuffer[m_IndexPos].frameData.pImgBuf == NULL)
//        {

//            QThread::usleep(100000);
//            continue;
//        }

//        //获取图像
//        status = GXGetImage(m_hDevice, &m_RecvBuffer[m_IndexPos].frameData, GET_IMAGE_TIME_OUT);
//        if(status == 0)
//        {
//            //判断采集帧是否为残帧，只有正常帧采做处理
//            if(m_RecvBuffer[m_IndexPos].frameData.nStatus != 0)
//            {
//                //qDebug() << "m_IndexPos = " << m_RecvBuffer[m_IndexPos].frameData.nStatus;
//                continue;
//            }
//        }
//        else
//        {
//            QThread::msleep(10);
//            continue;
//        }

//        //统计成率加1
//        m_RecvImgCount++;

//        //buffer 接收到完整图像，空标志位为false
//        m_RecvBuffer[m_IndexPos].bStatus = false;

//        //判断下一个buffer是否为空
//        if (m_RecvBuffer[nextPos].bStatus == false)
//        {
//            //qDebug() << "empty";
//            continue;
//        }


//        emit _sigShow(&m_RecvBuffer[m_IndexPos]);

//        m_IndexPos = nextPos;

//        //获取下一个缓冲区位置
//        nextPos = (m_IndexPos + 1) % MAX_NUM;
//    }
//    QThread::usleep(100000);
//}

////---------------------------------------------------------------------------------
///**
//\brief   设置开停采标志位
//\param   flag   开停采标志
//\return  无
//*/
////----------------------------------------------------------------------------------
//void DataRecvThread::SetImgFlg(bool flag)
//{
//    m_bGetImg = flag;
//}

////-------------------------------------------------
///**
//\brief 保存图像
//\return void
//*/
////-------------------------------------------------
//void DataRecvThread::SaveImageFile()
//{
//    QImage *pImage = NULL;
//    QString fileName;

//    static int nRawFileIndex = 1;
//    if (m_SaveImageFlag == true)
//    {
//        //m_Mutex.lock();
//        pImage = m_pImage[m_IndexPos];
//        if (m_nPixelColorFilter == GX_COLOR_FILTER_NONE)
//        {
//            fileName.sprintf("RAW%d.pgm", nRawFileIndex++);
//            pImage->save(fileName, "PGM");
//        }
//        else
//        {

//            fileName.sprintf("RGB%d.ppm", nRawFileIndex++);

//            pImage->save(fileName, "PPM");
//        }
//        ///m_Mutex.unlock();
//    }
//}

////----------------------------------------------------------------------------------
///**
//\brief  弹出错误信息
//\param  emErrorStatus  错误码
//\return 无返回值
//*/
////----------------------------------------------------------------------------------
//void DataRecvThread::ShowErrorString(GX_STATUS emErrorStatus)
//{
//    char      chErrorInof[512] = {0};
//    size_t    nSize     = 512;
//    GX_STATUS emStatus = GX_STATUS_ERROR;

//    emStatus = GXGetLastError (&emErrorStatus, chErrorInof, &nSize);
//    if (emStatus != GX_STATUS_SUCCESS)
//    {
//        QMessageBox::about(NULL, "About", "GXGetLastError接口调用失败！");
//    }
//    else
//    {
//        qDebug() << "error";
//        QMessageBox::about(NULL, "About", tr("%1").arg(QString(QLatin1String(chErrorInof))));
//    }

//}

//void DataRecvThread::setSaveImageFlag(bool flag)
//{
//    m_SaveImageFlag = flag;
//}

////----------------------------------------------------------------------------------
///**
//\brief  获取图像的宽
//\param  int  图像宽
//\return 无返回值
//*/
////----------------------------------------------------------------------------------
//int DataRecvThread::getImageWidth()
//{
//    return m_nImageWidth;
//}

////----------------------------------------------------------------------------------
///**
//\brief  获取图像高
//\param  int  图像高
//\return 无返回值
//*/
////----------------------------------------------------------------------------------
//int DataRecvThread::getImageHeight()
//{
//    return m_nImageHeight;
//}

////----------------------------------------------------------------------------------
///**
//\brief  设置buffer 状态
//\param  int  图像高
//\return 无返回值
//*/
////----------------------------------------------------------------------------------
//void DataRecvThread::setRecvBuffStatus(int id, bool status)
//{
//    m_RecvBuffer[id].bStatus = status;
//}

////----------------------------------------------------------------------------------
///**
//\brief  获取PixelColorFilter
//\param  int  图像高
//\return 无返回值
//*/
////----------------------------------------------------------------------------------
//int64_t DataRecvThread::getPixelColorFilter()
//{
//    return m_nPixelColorFilter;
//}

////----------------------------------------------------------------------------------
///**
//\brief  设置获取接收帧数
//\param  int  图像高
//\return 无返回值
//*/
////----------------------------------------------------------------------------------
//int DataRecvThread::getRecvImgCount()
//{
//    int count = m_RecvImgCount;
//    m_RecvImgCount = 0;

//    return count;
//}
