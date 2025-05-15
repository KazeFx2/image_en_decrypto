//
// Created by kazefx on 25-2-27.
//

#ifndef AUDIOPROVIDER_H
#define AUDIOPROVIDER_H

#include <QAudioFormat>
#include <QAudioSink>
#include <QTimer>

#include "ImageCrypto.h"
#include <QQuickImageProvider>
#include <QIODevice>
#include "Bitmap.h"
#include <queue>
#include <opencv2/videoio.hpp>
#include "Semaphore.h"

#include "Mutex.h"
#include "singleton.h"

#define MAX_CACHE 128

typedef struct {
    Mutex mtx;

    audio_control_t ctrl;

    QAudioFormat fmt;
    QAudioSink *sink;
    QIODevice *io;

    QByteArray internalBuffer;
    QTimer* timer;

    u64 totalBytesWriten;

    u32 bytes_per_pts;

    double current_msec;
    double total_msec;
    u64 totao_pts;
    bool pause;
    bool fin;

    class Audio *parent;
    void (Audio::*freeAudio)(int);
    int id;
} AudioControl;

class PCMIODevice : public QIODevice {
    Q_OBJECT

private:
    QByteArray buffer;

public:
    PCMIODevice(QObject *parent = nullptr);

    void appendData(const QByteArray &data);

    void clear();

    qint64 readData(char *data, qint64 maxlen) override;

    qint64 writeData(const char *, qint64) override;

    qint64 bytesAvailable() const override;
};

class Audio : public QObject {
    Q_OBJECT
    QML_SINGLETON

private:
    void __cvtAudio(const QUrl &in_file, const QUrl &out_file, const QString &key_id, int width, int height,
                    bool encrypt, bool cuda);

    void __freeAudio(int id);

public:
    SINGLETON(Audio)

    enum DecodeType {
        Raw = 0,
        Encrypt,
        Decrypt
    };

    Q_ENUM(DecodeType)

    Audio();

    Q_INVOKABLE void cvtAudio(const QUrl &in_file, const QUrl &out_file, const QString &key_id, int width, int height,
                              bool encrypt,
                              bool cuda);

    Q_INVOKABLE void force_stop_cvt();

    Q_INVOKABLE int loadAudio(const QUrl &file, const QString &key_id, DecodeType type, int width, int height,
                              bool cuda);

    Q_INVOKABLE void delAudio(int id);

    Q_INVOKABLE double total_msec(int id);

    Q_INVOKABLE double get_msec(int id);

    Q_INVOKABLE int get_decode_type(int id);

    Q_INVOKABLE bool get_cuda(int id);

    Q_INVOKABLE void set_cuda(int id, bool cuda);

    Q_INVOKABLE bool get_pause(int id);

    Q_INVOKABLE void set_type(int id, DecodeType type);

    Q_INVOKABLE void pause(int id);

    Q_INVOKABLE void resume(int id);

    Q_INVOKABLE void goto_msec(int id, double time);

    Q_INVOKABLE void set_param(int id, const QString &key_id, int width, int height);

signals:
    void audioCvtSignal(int i, int len);

    void audioUpdated(int i, double msec);

    void audioPaused(int i);

    void audioResumed(int i);

    void audioDecodeTypeChanged(int i);

    void audioCudaChanged(int i);

private:
    static u64 get_putIdx(bool isRet, u64 oldIdx);
    static AudioControl *idx_ava(int id);

    static FastBitmap bitmap;
    static std::vector<AudioControl *> acbs;
};

#endif //AUDIOPROVIDER_H
