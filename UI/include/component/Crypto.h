//
// Created by kazefx on 25-2-23.
//

#ifndef CRYPTO_H
#define CRYPTO_H

#include "component/KeyKeeper.h"
#include "component/ImageProvider.h"

QImage cvMat2QImage(const cv::Mat& mat);

class Crypto : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Crypto)
    QML_SINGLETON

    explicit Crypto(QObject* parent = nullptr): QObject(parent)
    {
        // connect(this, &Crypto::cryptoStart, this, &Crypto::__doCrypto);
        termCrypto = false;
        termSave = false;
    }

public:
    SINGLETON(Crypto)

    Q_INVOKABLE QString encrypt(const QUrl& url, const QString& key_id, bool cuda);

    Q_INVOKABLE QString decrypt(const QUrl& url, const QString& key_id, bool cuda);

    Q_INVOKABLE void removeImage(const QString& url);

    Q_INVOKABLE void saveImage(const QUrl& path, const QString& name, const QString& imageUrl);

    Q_INVOKABLE bool cudaAvailable();

    Q_INVOKABLE void doCrypto(const QVariantList &list, const QString &key_id, bool encrypt, bool cuda);

    Q_INVOKABLE void doSave(const QVariantList &list, const QUrl &path);

    Q_INVOKABLE void stopCrypto();

    Q_INVOKABLE void stopSave();

    // Q_INVOKABLE void cryptoVideo(const QUrl &file, const QUrl &out_path, const QString &name, const QString &key_id, bool encrypt, bool cuda);

    // Q_INVOKABLE void cryptoVideoPlay(const QUrl &file, const QUrl &out_path, const QString &name, const QString &key_id, bool encrypt, bool cuda);

    QString encrypt(const QUrl& url, C_FullKey& key, bool cuda);

    QString decrypt(const QUrl& url, C_FullKey& key, bool cuda);

    // public slots:
    void __doCrypto(const QVariantList &list, const QString &key_id, bool encrypt, bool cuda);
    void __doSave(const QVariantList &list, const QUrl &path);
    // void __doCryptoVideo(const QUrl &file, const QUrl &out_path, const QString &name, const QString &key_id, bool encrypt, bool cuda);
    signals:
    void cryptoFinished(int idx, QString resultUrl);
    // void cryptoStart(const QVariantList &list, const QString &key_id, bool encrypt, bool cuda);
    void saveImageFinished(int idx);
    // void cryptoVideoFinished(int idx);
private:
    bool termCrypto;
    bool termSave;
};

#endif //CRYPTO_H
