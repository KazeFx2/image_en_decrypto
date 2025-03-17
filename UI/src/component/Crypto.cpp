//
// Created by kazefx on 25-2-23.
//

#include "component/Crypto.h"
#include <QCryptographicHash>

#include "GlobalVariables.h"
// #include <qfuture.h>
// #include <QtConcurrent/qtconcurrentrun.h>

QImage cvMat2QImage(const cv::Mat &mat) {
    QImage image;
    switch (mat.type()) {
        case CV_8UC1:
            // QImage构造：数据，宽度，高度，每行多少字节，存储结构
            image = QImage(static_cast<const u8 *>(mat.data), mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
            break;
        case CV_8UC3:
            image = QImage(static_cast<const u8 *>(mat.data), mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            image = image.rgbSwapped(); // BRG转为RGB
        // Qt5.14增加了Format_BGR888
        // image = QImage((const unsigned char*)mat.data, mat.cols, mat.rows, mat.cols * 3, QImage::Format_BGR888);
            break;
        case CV_8UC4:
            image = QImage(static_cast<const u8 *>(mat.data), mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
            break;
        case CV_16UC4:
            image = QImage(static_cast<const u8 *>(mat.data), mat.cols, mat.rows, mat.step, QImage::Format_RGBA64);
            image = image.rgbSwapped(); // BRG转为RGB
            break;
    }
    return image;
}

QString Crypto::encrypt(const QUrl &url, const QString &key_id, bool cuda) {
    auto key = keyMap[key_id];
    if (cuda && key.imageCrypto.cuda_is_available()) key.imageCrypto.cuda();
    else key.imageCrypto.cpu();
    auto mat = key.imageCrypto.loadImage(url.toLocalFile().toStdString());
    // if (threadPool.getMaxThreads() < key.key.control.nThread) threadPool.setMax(key.key.control.nThread);
    // else threadPool.reduceTo(key.key.control.nThread);
    auto ret = key.imageCrypto.encrypt(mat);
    return memImage->loadImage(cvMat2QImage(ret));
}

QString Crypto::encrypt(const QUrl &url, C_FullKey &key, bool cuda) {
    if (cuda && key.imageCrypto.cuda_is_available()) key.imageCrypto.cuda();
    else key.imageCrypto.cpu();
    auto mat = key.imageCrypto.loadImage(url.toLocalFile().toStdString());
    // if (threadPool.getMaxThreads() < key.key.control.nThread) threadPool.setMax(key.key.control.nThread);
    // else threadPool.reduceTo(key.key.control.nThread);
    auto ret = key.imageCrypto.encrypt(mat);
    return memImage->loadImage(cvMat2QImage(ret));
}

QString Crypto::decrypt(const QUrl &url, const QString &key_id, bool cuda) {
    auto key = keyMap[key_id];
    if (cuda && key.imageCrypto.cuda_is_available()) key.imageCrypto.cuda();
    else key.imageCrypto.cpu();
    auto mat = key.imageCrypto.loadImage(url.toLocalFile().toStdString());
    // if (threadPool.getMaxThreads() < key.key.control.nThread) threadPool.setMax(key.key.control.nThread);
    // else threadPool.reduceTo(key.key.control.nThread);
    auto ret = key.imageCrypto.decrypt(mat);
    return memImage->loadImage(cvMat2QImage(ret));
}

QString Crypto::decrypt(const QUrl &url, C_FullKey &key, bool cuda) {
    if (cuda && key.imageCrypto.cuda_is_available()) key.imageCrypto.cuda();
    else key.imageCrypto.cpu();
    auto mat = key.imageCrypto.loadImage(url.toLocalFile().toStdString());
    // if (threadPool.getMaxThreads() < key.key.control.nThread) threadPool.setMax(key.key.control.nThread);
    // else threadPool.reduceTo(key.key.control.nThread);
    auto ret = key.imageCrypto.decrypt(mat);
    return memImage->loadImage(cvMat2QImage(ret));
}

void Crypto::removeImage(const QString &url) {
    MemoryImage::removeImage(url);
}

void Crypto::saveImage(const QUrl &path, const QString &name, const QString &imageUrl) {
    MemoryImage::saveImage(path, name, imageUrl);
}

bool Crypto::cudaAvailable() {
    return ImageCrypto::cuda_is_available();
}

void Crypto::__doCrypto(const QVariantList &list, const QString &key_id, bool is_encrypt, bool cuda) {
    auto key = keyMap[key_id];
    for (auto i = 0; i < list.length(); i++) {
        if (termCrypto) {
            termCrypto = false;
            emit cryptoFinished(-1, "");
            return;
        }
        auto map = list[i].toMap();
        QUrl source = map.value("source").toUrl();
        if (is_encrypt)
            emit cryptoFinished(i, encrypt(source, key, cuda));
        else
            emit cryptoFinished(i, decrypt(source, key, cuda));
    }
}

void Crypto::__doSave(const QVariantList &list, const QUrl &path) {
    for (auto i = 0; i < list.length(); i++) {
        if (termSave) {
            termSave = false;
            emit saveImageFinished(-1);
            return;
        }
        auto map = list[i].toMap();
        QString source = map.value("source").toString();
        QString name = map.value("name").toString();
        saveImage(path, name, source);
        emit saveImageFinished(i);
    }
}

void Crypto::doCrypto(const QVariantList &list, const QString &key_id, bool is_encrypt, bool cuda) {
    termCrypto = false;
    g_threadPool.addThreadEX(
        &Crypto::__doCrypto, this, list, key_id, is_encrypt, cuda
    ).setNoWait().start();
}

void Crypto::doSave(const QVariantList &list, const QUrl &path) {
    termSave = false;
    g_threadPool.addThreadEX(
        &Crypto::__doSave, this, list, path
    ).setNoWait().start();
}

void Crypto::stopCrypto() {
    termCrypto = true;
}

void Crypto::stopSave() {
    termSave = true;
}
