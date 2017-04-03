#ifndef WIDGET_H
#define WIDGET_H

#include "VideoDec.h"

#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

signals:
    void signalFileName(QString name);

public slots:
    void slotShowVideo(QImage img);

private slots:
    void on_openFile_pushButton_clicked();

private:
    Ui::Widget *ui;
    QThread *m_videoDecThread;
    VideoDec *m_videoDec;
    QString m_fileName;
    QImage mImage;
};

#endif // WIDGET_H
