#ifndef QCVDISPLAY_H
#define QCVDISPLAY_H


#include <opencv2/opencv.hpp>

#include <QObject>
#include <QWidget>
#include <QGestureEvent>
#include <QPainter>
#include <QEvent>
#include <QDebug>



class QCvDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit QCvDisplay(QWidget *parent = nullptr);
    void display(cv::Mat cvMatImage);
    void setPicture(QImage & image);


protected:
    void paintEvent(QPaintEvent*);
    // 放大/缩小
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;


    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    // 平移
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;


signals:
    void sig_picPressed(QPoint);
    void resizeWidget(QSize);

public slots:
    void zoomIn();  // 放大
    void zoomOut();  // 缩小
    void zoom(float scale); // 缩放 - scaleFactor：缩放的比例因子
    void translate(QPointF delta);  // 平移


private:
    bool gestureEvent(QGestureEvent *event);
    void panTriggered(QPanGesture*);
    void pinchTriggered(QPinchGesture*);


    QImage currentImage;

    qreal horizontalOffset;
    qreal verticalOffset;

    qreal scaleFactor;
    qreal currentStepScaleFactor;
    Qt::MouseButton m_translateButton;  // 平移按钮
    bool m_bMouseTranslate;
    qreal m_zoomDelta;  // 缩放的增量
    QPoint m_lastMousePos;  // 鼠标最后按下的位置

};

#endif // QCVDISPLAY_H
