#ifndef SLICEPROCESSOR_H
#define SLICEPROCESSOR_H

#include <QThread>
#include <QStringList>
#include <QImage>


class SliceProcessor : public QThread
{
    Q_OBJECT
public:
    explicit SliceProcessor(QObject *parent = 0);

    virtual ~SliceProcessor() {}

    void run() Q_DECL_OVERRIDE;

    QStringList images;
    float blending;

    bool preview;
    QImage preview_image;
    bool reverse;
    float angle;
    float scale_x, scale_y;

signals:
    void resultReady(const QString &s);
    void info(const QString &s);
    void error(const QString &s);
    void progress(int i);

    void preview_ready();

public slots:

private:
    float blend(float pos);
    QList<QImage> thumbs;

};

#endif // SLICEPROCESSOR_H
