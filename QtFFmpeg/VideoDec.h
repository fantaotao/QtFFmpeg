#ifndef VIDEODEC_H
#define VIDEODEC_H

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/mem.h"
#include "libavutil/fifo.h"
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
}

#include <QObject>
#include <QImage>

class VideoDec : public QObject
{
    Q_OBJECT
public:
    explicit VideoDec(QObject *parent = 0);
    ~VideoDec();
    int videoindex;
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVPacket *packet;
    AVFrame *pFrame;
    AVFrame *pFrameRGB;
    QString fileName;

signals:
    void SendImage(QImage img);

public slots:
    void slotInit();
    void play();
    void slotSetFileName(QString name);

};

#endif // VIDEODEC_H
