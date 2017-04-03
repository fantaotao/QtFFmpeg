#include "widget.h"
#include "ui_widget.h"

#include <QThread>
#include <QTimer>
#include <QFileDialog>
#include <QDebug>
#include <QPainter>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_videoDecThread = new QThread(this);
    m_videoDec = new VideoDec();
    m_videoDec->moveToThread(m_videoDecThread);
    m_videoDecThread->start();
    connect(this, &Widget::signalFileName, m_videoDec, &VideoDec::slotSetFileName);
    connect(m_videoDec, &VideoDec::SendImage,this, &Widget::slotShowVideo);
}

Widget::~Widget()
{
    m_videoDecThread->quit();
    m_videoDecThread->wait();
    delete m_videoDec;
    delete ui;
}

/**
 * @brief
 *    显示视频流
 * @param img
 *    解析视频流后的图片数据
 */
void Widget::slotShowVideo(QImage img)
{
    if(img.isNull())
    {
        return;
    }
    mImage = img;
    ui->videoshow_label->setPixmap(QPixmap::fromImage(img));
}

/**
 * @brief
 *    响应界面按钮操作
 */
void Widget::on_openFile_pushButton_clicked()
{
    m_fileName = QFileDialog::getOpenFileName(this, "Open Video", "", "video file(*.mp4;*.avi;*.rmvb)");
    if(m_fileName.isEmpty())
    {
        return;
    }
    emit signalFileName(m_fileName);
}

