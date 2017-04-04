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
    if(m_FormatCtx != NULL)
    {
        avformat_close_input(&m_FormatCtx);
    }

    if(m_CodecCtx != NULL)
    {
        avcodec_close(m_CodecCtx);
    }

    if(m_Codec != NULL)
    {
        av_free(m_Codec);
    }

    if(m_frameRGB != NULL)
    {
        av_free(m_frameRGB);
    }

    if(m_frame != NULL)
    {
        av_free(m_frame);
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
    m_FormatCtx = avformat_alloc_context();

    //打开输入流并读取标题
    if(avformat_open_input(&m_FormatCtx, m_fileName.toLocal8Bit().constData(), NULL, NULL) != 0)
    {
        qDebug() << "OpenFail";
    }

    //读取媒体文件的数据包以获取流信息
    if(avformat_find_stream_info(m_FormatCtx, NULL) < 0)
    {
        qDebug() << "FindFail";
    }

    videoindex = -1;
    //循环查找视频中包含的流信息，直到找到视频类型的流
    //便将其记录下来 保存到videoStream变量中
    //这里现在只处理视频流  音频流先不管
    for(int i = 0; m_FormatCtx->nb_streams; ++i)
    {
        if(m_FormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
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
    m_CodecCtx = m_FormatCtx->streams[videoindex]->codec;
    m_Codec = avcodec_find_decoder(m_CodecCtx->codec_id);
    if(m_Codec == NULL)
    {
        qDebug() << "codec not find";
    }

    //打开解码器
    if(avcodec_open2(m_CodecCtx, m_Codec, NULL) < 0)
    {
        qDebug() << "Can't open codec";
    }
    m_frame = av_frame_alloc();
    m_frameRGB = av_frame_alloc();
    play();
}

/**
 * @brief
 *    开始读取视频文件中的视频流
 */
void VideoDec::play()
{
    int got_picture;
    int size = m_CodecCtx->width*m_CodecCtx->height;

    //分配一个packet
    m_packet=(AVPacket *)malloc(sizeof(AVPacket));

    //分配packet的数据
    av_new_packet(m_packet, size);
    uint8_t *out_buffer;
    out_buffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_BGRA, m_CodecCtx->width, m_CodecCtx->height)];
    avpicture_fill((AVPicture *)m_frameRGB, out_buffer, AV_PIX_FMT_BGRA, m_CodecCtx->width, m_CodecCtx->height);
    avpicture_fill((AVPicture *)m_frame, out_buffer, AV_PIX_FMT_BGRA, m_CodecCtx->width, m_CodecCtx->height);
    //下一帧 返回值小于0，视频读取完毕
    while(av_read_frame(m_FormatCtx,m_packet) >= 0)
    {
        if(m_packet->stream_index == videoindex)
        {
            int rec = avcodec_decode_video2(m_CodecCtx, m_frame, &got_picture, m_packet);
            if(rec > 0)
            {
                //将解码后的YUV数据转换成RGB32
                SwsContext *convertCtx = sws_getContext(m_CodecCtx->width, m_CodecCtx->height, m_CodecCtx->pix_fmt, m_CodecCtx->width, m_CodecCtx->height, AV_PIX_FMT_BGRA, SWS_BICUBIC, NULL, NULL, NULL);
                sws_scale(convertCtx, (const uint8_t* const*)m_frame->data,  m_frame->linesize, 0, m_CodecCtx->height, m_frameRGB->data, m_frameRGB->linesize);

                //把这个RGB数据 用QImage加载
                QImage img((uchar *)m_frameRGB->data[0], m_CodecCtx->width, m_CodecCtx->height, QImage::Format_RGB32);
                //把图像复制一份 传递给界面显示
                QImage image = img.copy();
                emit SendImage(image);
            }
        }
        av_free_packet(m_packet);
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
    m_fileName = name;
    slotInit();
}
