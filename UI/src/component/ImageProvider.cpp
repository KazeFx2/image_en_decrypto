//
// Created by kazefx on 25-2-23.
//

#include "component/ImageProvider.h"

#include <private/Util.h>

#include "Semaphore.h"
std::vector<QImage> MemoryImage::images;

MemoryImage* memImage = nullptr;

MemoryImage::MemoryImage()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QString MemoryImage::loadImage(const QImage& image)
{
    auto idx = get_putIdx(false, 0);
    if (idx == images.size())
        images.push_back(image);
    else
    {
        images[idx] = image;
        emit imageUpdated(QString::number(idx));
    }
    return "image://MemoryImage/" + QString::number(idx);
}

QImage MemoryImage::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    auto& i = images[id.split("/")[0].toInt()];
    size->setWidth(i.width());
    size->setHeight(i.height());
    return i;
}

void MemoryImage::removeImage(const QString& url)
{
    auto array = url.split('/');
    if (array.size() < 4) return;
    bool ok;
    auto idx = array[3].toInt(&ok);
    if (!ok) return;
    get_putIdx(true, idx);
}


void MemoryImage::saveImage(const QUrl& path, const QString& name, const QString& imageUrl)
{
    auto array = imageUrl.split('/');
    if (array.size() < 4) return;
    bool ok;
    auto idx = array[3].toInt(&ok);
    if (!ok) return;
    auto image = images[idx];
    auto file = FileUniqueForceSuffix((path.toLocalFile() + "/" + name).toStdString().c_str(), "png");
    image.save(QString::fromStdString(file), "png");
}

u64 MemoryImage::get_putIdx(const bool isRet, const u64 oldIdx)
{
    static u64 idx = 0;
    static Semaphore semaphore(1);
    static FastBitmap bitmap;
    semaphore.wait();
    if (isRet == true)
    {
        bitmap[oldIdx] = false;
        semaphore.post();
        return oldIdx;
    }
    auto id = bitmap.findNextFalse(0, idx);
    if (id != BITMAP_NOT_FOUND)
    {
        bitmap[id] = true;
        semaphore.post();
        return id;
    }
    idx++;
    const u64 ret = idx - 1;
    bitmap[ret] = true;
    semaphore.post();
    return ret;
}
