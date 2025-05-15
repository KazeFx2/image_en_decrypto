//
// Created by kazefx on 25-2-27.
//

#ifndef VIDEOPROVIDER_H
#define VIDEOPROVIDER_H

#include <ImageCrypto.h>
#include <QQuickImageProvider>
#include "Bitmap.h"
#include <queue>
#include <opencv2/videoio.hpp>
#include "Semaphore.h"

#include "Mutex.h"

#define MAX_CACHE 128

typedef struct {
    Mutex mtx;
    std::queue<QImage> cache;
    QImage current;
    int frame_count;
    int current_frame;
    double current_msec;
    QString file;
    cv::VideoCapture cap;
    double fps;
    double total_msec;
    volatile bool paused;
    ImageCrypto *crypto;
    volatile int type;
    volatile bool cuda;
    volatile bool update_pos;
    volatile bool terminate;
    volatile bool reader_wait;
    volatile bool writer_wait;
    volatile bool play_over;
    int idx;
    int real_width;
    int real_height;
    int width;
    int height;
    Semaphore decoder_start_sem;
    Semaphore reader_start_sem;
    Semaphore decoder_fin_sem;
    Semaphore reader_fin_sem;
    double new_pos;
} VideoControl;

class Video : public QQuickImageProvider {
    Q_OBJECT

public:
    Video();

    // QQuickImageProvider interface
private:
    void goto_msec_locked(int idx, double msec);

    bool url_available(const QString &url, int &idx);

    void __cvtVideo(const QUrl &in_file, const QUrl &out_file, const QString &key_id, bool encrypt, bool cuda);

    void __cvtVideoWH(const QUrl &in_file, const QUrl &out_file, const QString &key_id, bool encrypt, bool cuda,
                      int width, int height);

public:
    enum DecodeType {
        Raw,
        Encrypt,
        Decrypt
    };

    Q_ENUM(DecodeType)

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    void decoder(VideoControl *vcb);

    void reader(VideoControl *vcb);

    Q_INVOKABLE QString loadVideo(const QUrl &file, const QString &key_id, DecodeType type, bool cuda,
                                  int recommend_width, int recommend_height);

    Q_INVOKABLE void delVideo(const QString &url);

    Q_INVOKABLE void pause(const QString &url);

    Q_INVOKABLE void resume(const QString &url);

    Q_INVOKABLE double total_msec(const QString &url);

    Q_INVOKABLE void goto_msec(const QString &url, double msec);

    Q_INVOKABLE double get_msec(const QString &url);

    Q_INVOKABLE void set_type(const QString &url, DecodeType type);

    Q_INVOKABLE int get_decode_type(const QString &url);

    Q_INVOKABLE void set_wh(const QString &url, int width, int height);

    Q_INVOKABLE bool get_cuda(const QString &url);

    Q_INVOKABLE void set_cuda(const QString &url, bool cuda);

    Q_INVOKABLE void set_param(const QString &url, const QString &key_id);

    Q_INVOKABLE bool get_pause(const QString &url);

    Q_INVOKABLE void cvtVideo(const QUrl &in_file, const QUrl &out_file, const QString &key_id, bool encrypt,
                              bool cuda);

    Q_INVOKABLE void cvtVideoWH(const QUrl &in_file, const QUrl &out_file, const QString &key_id, bool encrypt,
                                bool cuda, int width, int height);

    Q_INVOKABLE QVariantMap getVideoWH(const QUrl &in_file);

    Q_INVOKABLE void force_stop_cvt();

signals:
    void videoUpdated(QString id, double msec);

    void videoPaused(QString id);

    void videoResumed(QString id);

    void videoDecodeTypeChanged(QString id);

    void videoCudaChanged(QString id);

    void videoLoading(QString id);

    void videoLoaded(QString id);

    void videoCvtSignal(int i, int len);

private:
    static u64 get_putIdx(bool isRet, u64 oldIdx);

    static FastBitmap bitmap;
    static std::vector<VideoControl *> vcbs;
    bool term;
};

extern Video *VideoProvider;


#endif //VIDEOPROVIDER_H
