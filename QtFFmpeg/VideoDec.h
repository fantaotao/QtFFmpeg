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

signals:
    void SendImage(QImage img);

public slots:
    void slotInit();
    void slotSetFileName(QString name);

private slots:
    void play();

private:
    int videoindex;
    AVFormatContext *m_FormatCtx;
    AVCodecContext *m_CodecCtx;
    AVCodec *m_Codec;
    AVPacket *m_packet;
    AVFrame *m_frame;
    AVFrame *m_frameRGB;
    QString m_fileName;

};

#endif // VIDEODEC_H
