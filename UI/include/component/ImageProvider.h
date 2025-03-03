//
// Created by kazefx on 25-2-23.
//

#ifndef IMAGE_PROVIDER_H
#define IMAGE_PROVIDER_H

#include <QQuickImageProvider>
#include "Bitmap.h"

class MemoryImage : public QQuickImageProvider
{
    Q_OBJECT
public:
    MemoryImage();

    // QQuickImageProvider interface
public:
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;
    QString loadImage(const QImage& image);
    static void removeImage(const QString& url);
    static void saveImage(const QUrl& path, const QString &name, const QString& imageUrl);

    signals:
        void imageUpdated(QString id);
private:
    static u64 get_putIdx(bool isRet, u64 oldIdx);
    static std::vector<QImage> images;
    static FastBitmap bitmap;
};

extern MemoryImage *memImage;

#endif //IMAGE_PROVIDER_H
