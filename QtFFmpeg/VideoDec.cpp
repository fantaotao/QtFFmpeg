#include "VideoDec.h"

#include <QImage>
#include <QDebug>
#include <QThread>

VideoDec::VideoDec(QObject *parent)
    : QObject(parent)
{

}

VideoDec::~VideoDec()
{
    if(pFormatCtx != NULL)
    {
        avformat_close_input(&pFormatCtx);
    }

    if(pCodecCtx != NULL)
    {
        avcodec_close(pCodecCtx);
    }

    if(pCodec != NULL)
    {
        av_free(pCodec);
    }

    if(pFrameRGB != NULL)
    {
        av_free(pFrameRGB);
    }

    if(pFrame != NULL)
    {
        av_free(pFrame);
    }
}

/**
 * @brief
 *    初始化ffmpeg信息
 */
void VideoDec::slotInit()
{
    //注册库中所有可用的文件格式和解码器
    avcodec_register_all();
    av_register_all();

    //申请一个AVFormatContext结构的内存,并进行简单初始化
    pFormatCtx = avformat_alloc_context();

    //打开输入流并读取标题
    if(avformat_open_input(&pFormatCtx, fileName.toLocal8Bit().constData(), NULL, NULL) != 0)
    {
        qDebug() << "OpenFail";
    }

    //读取媒体文件的数据包以获取流信息
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        qDebug() << "FindFail";
    }

    videoindex = -1;
    //循环查找视频中包含的流信息，直到找到视频类型的流
    //便将其记录下来 保存到videoStream变量中
    //这里现在只处理视频流  音频流先不管
    for(int i = 0; pFormatCtx->nb_streams; ++i)
    {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    }

    //如果videoindex为-1 说明没有找到视频流
    if(videoindex == -1)
    {
        qDebug() << "Don't find video stream";
    }

    //查找解码器
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec == NULL)
    {
        qDebug() << "codec not find";
    }

    //打开解码器
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        qDebug() << "Can't open codec";
    }
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    play();
}

/**
 * @brief
 *    开始读取视频文件中的视频流
 */
void VideoDec::play()
{
    int got_picture;
    int size = pCodecCtx->width*pCodecCtx->height;

    //分配一个packet
    packet=(AVPacket *)malloc(sizeof(AVPacket));

    //分配packet的数据
    av_new_packet(packet, size);
    uint8_t *out_buffer;
    out_buffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_BGRA, pCodecCtx->width, pCodecCtx->height)];
    avpicture_fill((AVPicture *)pFrameRGB, out_buffer, AV_PIX_FMT_BGRA, pCodecCtx->width, pCodecCtx->height);
    avpicture_fill((AVPicture *)pFrame, out_buffer, AV_PIX_FMT_BGRA, pCodecCtx->width, pCodecCtx->height);
    //下一帧 返回值小于0，视频读取完毕
    while(av_read_frame(pFormatCtx,packet) >= 0)
    {
        if(packet->stream_index == videoindex)
        {
            int rec = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if(rec > 0)
            {
                //将解码后的YUV数据转换成RGB32
                SwsContext *convertCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGRA, SWS_BICUBIC, NULL, NULL, NULL);
                sws_scale(convertCtx, (const uint8_t* const*)pFrame->data,  pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                //把这个RGB数据 用QImage加载
                QImage img((uchar *)pFrameRGB->data[0], pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB32);
                //把图像复制一份 传递给界面显示
                QImage image = img.copy();
                emit SendImage(image);
            }
        }
        av_free_packet(packet);
        QThread::msleep(5);
    }
    av_free(out_buffer);
}

/**
 * @brief
 *    设置读取的文件名称，并初始化ffmpeg
 * @param name
 *    文件名
 */
void VideoDec::slotSetFileName(QString name)
{
    fileName = name;
    slotInit();
}
