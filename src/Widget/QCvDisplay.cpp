#include "QCvDisplay.h"




QImage mat2image(cv::Mat cvImg){
    QImage qImg;
    if (cvImg.channels() == 3)                             //三通道彩色图像
    {

        cv::cvtColor(cvImg, cvImg, CV_BGR2RGB);
        qImg = QImage((const unsigned char*)(cvImg.data),	cvImg.cols, cvImg.rows,	cvImg.cols*cvImg.channels(),QImage::Format_RGB888);
    }
    else if (cvImg.channels() == 1)                    //单通道（灰度图）
    {
        qImg = QImage((const unsigned char*)(cvImg.data),cvImg.cols, cvImg.rows,cvImg.cols*cvImg.channels(),QImage::Format_Indexed8);

        QVector<QRgb> colorTable;
        for (int k = 0; k<256; ++k)
        {
            colorTable.push_back(qRgb(k, k, k));
        }
        qImg.setColorTable(colorTable);//把qImg的颜色按像素点的颜色给设置
    }
    else
    {
        qImg = QImage((const unsigned char*)(cvImg.data),cvImg.cols, cvImg.rows,cvImg.cols*cvImg.channels(),QImage::Format_RGB888);
    }

    return qImg;
}





QCvDisplay::QCvDisplay(QWidget *parent)
    : QWidget(parent),
        horizontalOffset(0),
        verticalOffset(0),
        scaleFactor(1),
        currentStepScaleFactor(1),
        m_translateButton(Qt::LeftButton),
        m_bMouseTranslate(false),
        m_zoomDelta(0.2)
{
    this->setFocusPolicy(Qt::ClickFocus);

    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);

}

void QCvDisplay::display(cv::Mat cvMatImage)
{
    currentImage = mat2image(cvMatImage);
    update();
}



void QCvDisplay::setPicture(QImage &image)
{
    currentImage = image.convertToFormat(QImage::Format_RGB888);
    update();
}


bool QCvDisplay::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
        return gestureEvent(static_cast<QGestureEvent*>(event));
    return QWidget::event(event);
}

void QCvDisplay::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QImage image = currentImage;


    if(!image.isNull()){
        image = image.scaled(this->width()*currentStepScaleFactor * scaleFactor,
                             this->height()*currentStepScaleFactor * scaleFactor,
                             Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
    }

    const qreal iw = image.width();
    const qreal ih = image.height();
    const qreal wh = height();
    const qreal ww = width();

    painter.translate(ww/2, wh/2);
    painter.translate(horizontalOffset, verticalOffset);
    //painter.scale(currentStepScaleFactor * scaleFactor, currentStepScaleFactor * scaleFactor);
    painter.translate(-iw/2, -ih/2);
    painter.drawImage(0,0,image);

}

void QCvDisplay::mouseDoubleClickEvent(QMouseEvent *)
{
    scaleFactor = 1;
    currentStepScaleFactor = 1;
    verticalOffset = 0;
    horizontalOffset = 0;
    update();
}

bool QCvDisplay::gestureEvent(QGestureEvent *event)
{
    if (QGesture *pan = event->gesture(Qt::PanGesture))
        panTriggered(static_cast<QPanGesture *>(pan));
    if (QGesture *pinch = event->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
    return true;
}

void QCvDisplay::panTriggered(QPanGesture *gesture)
{
#ifndef QT_NO_CURSOR
    switch (gesture->state()) {
        case Qt::GestureStarted:
        case Qt::GestureUpdated:
            setCursor(Qt::SizeAllCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
    }
#endif
    QPointF delta = gesture->delta();
    horizontalOffset += delta.x();
    verticalOffset += delta.y();
    update();
}

void QCvDisplay::pinchTriggered(QPinchGesture *gesture)
{
    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        currentStepScaleFactor = gesture->totalScaleFactor();
    }
    if (gesture->state() == Qt::GestureFinished) {
        scaleFactor *= currentStepScaleFactor;
        currentStepScaleFactor = 1;
    }
    update();
}

void QCvDisplay::resizeEvent(QResizeEvent*e)
{
    update();
    QWidget::resizeEvent(e);
    emit resizeWidget(QSize(this->width(),this->height()));
}

// 上/下/左/右键向各个方向移动、加/减键进行缩放
void QCvDisplay::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        translate(QPointF(0, -5));  // 上移
        break;
    case Qt::Key_Down:
        translate(QPointF(0, 5));  // 下移
        break;
    case Qt::Key_Left:
        translate(QPointF(-5, 0));  // 左移
        break;
    case Qt::Key_Right:
        translate(QPointF(5, 0));  // 右移
        break;
    case Qt::Key_Plus:  // 放大
        zoomIn();
        break;
    case Qt::Key_Minus:  // 缩小
        zoomOut();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
    QWidget::keyPressEvent(event);
}

// 平移
void QCvDisplay::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bMouseTranslate){
        QPointF mouseDelta = event->pos() - m_lastMousePos;
        translate(mouseDelta);
    }

    m_lastMousePos = event->pos();

    QWidget::mouseMoveEvent(event);
}

void QCvDisplay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == m_translateButton) {
        m_bMouseTranslate = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::OpenHandCursor);
    }

    QWidget::mousePressEvent(event);
    emit sig_picPressed(m_lastMousePos);
}

void QCvDisplay::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == m_translateButton)
    {
        m_bMouseTranslate = false;
        setCursor(Qt::ArrowCursor);
    }

    QWidget::mouseReleaseEvent(event);
}

// 放大/缩小
void QCvDisplay::wheelEvent(QWheelEvent *event)
{
//    QPoint numPixels = event->pixelDelta();
    QPoint scrallAmount = event->angleDelta();
    if(scrallAmount.y() > 0){
        zoomIn();
    }
    else if(scrallAmount.y() < 0){
        zoomOut();
    }
    QWidget::wheelEvent(event);
}

// 放大
void QCvDisplay::zoomIn()
{
    zoom(1 + m_zoomDelta);
}

// 缩小
void QCvDisplay::zoomOut()
{
    zoom(1 - m_zoomDelta);
}

// 缩放 - scaleFactor：缩放的比例因子
void QCvDisplay::zoom(float scale)
{
    scaleFactor *= scale;
    update();
}

// 平移
void QCvDisplay::translate(QPointF delta)
{
    horizontalOffset += delta.x();
    verticalOffset += delta.y();
    update();
}
