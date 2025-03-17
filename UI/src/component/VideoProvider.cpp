//
// Created by kazefx on 25-2-27.
//

#include "component/VideoProvider.h"

// #include <qfuture.h>
#include <QThread>
// #include <QtConcurrent/qtconcurrentrun.h>
#ifndef _WIN32
#include <sys/time.h>
#else
void gettimeofday(struct timeval *tv, void *tz) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    const uint64_t EPOCH_OFFSET = 116444736000000000ULL;
    uli.QuadPart -= EPOCH_OFFSET;
    uli.QuadPart /= 10;
    tv->tv_sec = uli.QuadPart / 1000000ULL;
    tv->tv_usec = uli.QuadPart % 1000000ULL;
}
#endif

#include "GlobalVariables.h"
#include "Semaphore.h"
#include "component/Crypto.h"
#include "component/ImageProvider.h"
#include "component/KeyKeeper.h"

Video *VideoProvider = nullptr;

std::vector<VideoControl *> Video::vcbs;

void calcWH(int &w_out, int &h_out, int w_real, int h_real, int w_recommend, int h_recommend) {
    if (w_real * h_recommend > w_recommend * h_real) {
        w_out = w_recommend;
        h_out = h_real * w_recommend / w_real;
    } else {
        h_out = h_recommend;
        w_out = w_real * h_recommend / h_real;
    }
}

inline double GetCPUSecond() {
    timeval tp;
    gettimeofday(&tp, nullptr);
    return (static_cast<double>(tp.tv_sec) + static_cast<double>(tp.tv_usec) * 1.e-6);
}

Video::Video()
    : QQuickImageProvider(QQuickImageProvider::Image) {
    term = false;
}

void Video::decoder(VideoControl *vcb) {
    cv::Mat frame, out;
    QImage img;
    while (true) {
        vcb->cap >> frame;
        if (frame.empty()) {
            vcb->mtx.lock();
            vcb->play_over = true;
            goto DECODE_WAIT;
        }
        switch (vcb->type) {
            case Raw:
                out = frame;
                break;
            case Decrypt:
                out = vcb->crypto->decrypt(frame);
                break;
            case Encrypt:
                out = vcb->crypto->encrypt(frame);
                break;
        }
        img = cvMat2QImage(out);
        vcb->mtx.lock();
        if (vcb->terminate) {
            vcb->mtx.unlock();
            goto DECODE_TERM;
        }
        vcb->cache.push(img);
        if (vcb->cache.size() == 1 && vcb->reader_wait) {
            emit videoLoaded(QString::number(vcb->idx));
            if (!vcb->paused) {
                vcb->reader_wait = false;
                vcb->reader_start_sem.post();
            }
        }
        if (vcb->cache.size() >= MAX_CACHE) {
        DECODE_WAIT:
            vcb->writer_wait = true;
            vcb->mtx.unlock();
            vcb->decoder_start_sem.wait();
        } else vcb->mtx.unlock();
        if (vcb->terminate) {
        DECODE_TERM:
            vcb->decoder_fin_sem.post();
            vcb->cap.release();
            // printf("exit decoder %d\n", vcb->idx);
            return;
        }
        vcb->mtx.lock();
        if (vcb->update_pos) {
            std::queue<QImage> empty;
            std::swap(vcb->cache, empty);
            vcb->cap.set(cv::CAP_PROP_POS_MSEC, vcb->new_pos);
            vcb->current_frame = vcb->cap.get(cv::CAP_PROP_POS_FRAMES);
            vcb->update_pos = false;
            vcb->play_over = false;
        }
        vcb->mtx.unlock();
    }
}

void Video::reader(VideoControl *vcb) {
    bool init = false;
    double time_rec = 0.0, time_rec_next = 0.0, delay = 0.0;
    while (true) {
        vcb->mtx.lock();
        if (vcb->terminate) {
        READER_TERM:
            vcb->reader_fin_sem.post();
            vcb->mtx.unlock();
            // printf("exit reader %d\n", vcb->idx);
            return;
        }
        if (vcb->cache.empty() || vcb->paused) {
        READER_WAIT:
            vcb->reader_wait = true;
            if ((vcb->play_over && vcb->cache.empty()) || vcb->current_frame >= vcb->frame_count)
                emit videoPaused(QString::number(vcb->idx));
            else if (!vcb->paused) emit videoLoading(QString::number(vcb->idx));
            vcb->mtx.unlock();
            vcb->reader_start_sem.wait();
            vcb->mtx.lock();
            if (vcb->terminate) goto READER_TERM;
            if (vcb->cache.empty()) goto READER_WAIT;
        }
        if (!init) {
            time_rec = GetCPUSecond();
            init = true;
        }
        // vcb->current_msec = vcb->cap.get(cv::CAP_PROP_POS_MSEC);
        // vcb->mtx.lock();
        vcb->current_msec = vcb->current_frame * 1000.0 / vcb->fps;
        vcb->current = vcb->cache.front().scaled(vcb->width, vcb->height);
        vcb->cache.pop();
        vcb->current_frame++;
        if (vcb->cache.size() + 1 >= MAX_CACHE && vcb->writer_wait) {
            vcb->writer_wait = false;
            vcb->decoder_start_sem.post();
        }
        vcb->mtx.unlock();
        emit videoUpdated(QString::number(vcb->idx), vcb->current_msec);
        time_rec_next = GetCPUSecond();
        delay = static_cast<int>(1000.0 / vcb->fps - (time_rec_next - time_rec) * 1000);
        time_rec = time_rec_next;
        QThread::msleep(delay * 2 >= 0 ? delay * 2 : 0);
        if ((vcb->play_over && vcb->cache.empty()) || vcb->current_frame >= vcb->frame_count) {
            emit videoUpdated(QString::number(vcb->idx), vcb->current_frame * 1000.0 / vcb->fps);
            vcb->paused = true;
            emit videoPaused(QString::number(vcb->idx));
        }
    }
}

QString Video::loadVideo(const QUrl &file, const QString &key_id, DecodeType type, bool cuda, int recommend_width,
                         int recommend_height) {
    const auto id = get_putIdx(false, 0);
    if (id >= vcbs.size()) {
        vcbs.push_back(new VideoControl);
    } else vcbs[id] = new VideoControl;
    auto *vcb = vcbs[id];
    if (!vcb->cap.open(file.toLocalFile().toStdString())) {
        delete vcb;
        return QString("");
    }
    vcb->real_width = vcb->cap.get(cv::CAP_PROP_FRAME_WIDTH);
    vcb->real_height = vcb->cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    calcWH(vcb->width, vcb->height, vcb->real_width, vcb->real_height, recommend_width, recommend_height);
    cv::Mat tmp;
    vcb->cap.read(tmp);
    vcb->current = cvMat2QImage(tmp).scaled(vcb->width, vcb->height);
    vcb->cap.set(cv::CAP_PROP_POS_FRAMES, 0);
    vcb->file = file.toLocalFile();
    vcb->fps = vcb->cap.get(cv::CAP_PROP_FPS);
    vcb->frame_count = vcb->cap.get(cv::CAP_PROP_FRAME_COUNT);
    vcb->current_frame = 0;
    vcb->current_msec = 0.0;
    vcb->total_msec = 1000.0 * vcb->frame_count / vcb->fps;
    vcb->paused = true;
    vcb->update_pos = false;
    vcb->terminate = false;
    vcb->reader_wait = false;
    vcb->writer_wait = false;
    vcb->play_over = false;
    vcb->new_pos = 0.0;
    vcb->cuda = ImageCrypto::cuda_is_available() ? cuda : false;
    vcb->type = type;
    vcb->crypto = new ImageCrypto(keyMap[key_id].imageCrypto);
    if (cuda) vcb->crypto->cuda();
    else vcb->crypto->cpu();
    vcb->idx = id;
    g_threadPool.addThreadEX(
        &Video::decoder,
        this, vcb
    ).setNoWait().start();
    g_threadPool.addThreadEX(
        &Video::reader,
        this, vcb
    ).setNoWait().start();
    return "image://Video/" + QString::number(vcb->idx);
}

QImage Video::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
    auto idx = id.split('/')[0].toInt();
    if (idx >= vcbs.size()) return QImage();
    return vcbs[idx]->current;
}

void Video::delVideo(const QString &url) {
    int idx;
    if (!url_available(url, idx)) return;
    vcbs[idx]->mtx.lock();
    if (vcbs[idx]->terminate) {
        vcbs[idx]->mtx.unlock();
        return;
    }
    vcbs[idx]->terminate = true;
    if (vcbs[idx]->writer_wait) vcbs[idx]->decoder_start_sem.post();
    if (vcbs[idx]->reader_wait) vcbs[idx]->reader_start_sem.post();
    vcbs[idx]->mtx.unlock();
    vcbs[idx]->decoder_fin_sem.wait();
    vcbs[idx]->reader_fin_sem.wait();
    delete vcbs[idx]->crypto;
    delete vcbs[idx];
    get_putIdx(true, idx);
}


void Video::pause(const QString &url) {
    int idx;
    if (!url_available(url, idx)) return;
    vcbs[idx]->mtx.lock();
    vcbs[idx]->paused = true;
    emit videoPaused(QString::number(idx));
    vcbs[idx]->mtx.unlock();
}

void Video::resume(const QString &url) {
    int idx;
    if (!url_available(url, idx)) return;
    vcbs[idx]->mtx.lock();
    if (vcbs[idx]->play_over || vcbs[idx]->current_frame >= vcbs[idx]->frame_count)
        goto_msec_locked(idx, 0.0);
    vcbs[idx]->paused = false;
    emit videoResumed(QString::number(idx));
    if (vcbs[idx]->reader_wait && !vcbs[idx]->cache.empty()) {
        emit videoLoaded(QString::number(idx));
        vcbs[idx]->reader_start_sem.post();
        vcbs[idx]->reader_wait = false;
    }
    vcbs[idx]->mtx.unlock();
}

double Video::total_msec(const QString &url) {
    int idx;
    if (!url_available(url, idx)) return 0.0;
    return vcbs[idx]->total_msec;
}

void Video::goto_msec_locked(int idx, double msec) {
    vcbs[idx]->new_pos = msec;
    vcbs[idx]->update_pos = true;
    if (vcbs[idx]->writer_wait) {
        vcbs[idx]->decoder_start_sem.post();
        vcbs[idx]->writer_wait = false;
    }
}

bool Video::url_available(const QString &url, int &idx) {
    auto array = url.split('/');
    if (array.size() < 4) return false;
    bool ok;
    idx = array[3].toInt(&ok);
    if (!ok) return false;
    if (idx >= vcbs.size()) return false;
    return true;
}

void Video::goto_msec(const QString &url, double msec) {
    int idx;
    if (!url_available(url, idx)) return;
    vcbs[idx]->mtx.lock();
    goto_msec_locked(idx, msec);
    vcbs[idx]->mtx.unlock();
}

double Video::get_msec(const QString &url) {
    int idx;
    if (!url_available(url, idx)) return 0.0;
    return vcbs[idx]->current_msec;
}

void Video::set_type(const QString &url, DecodeType type) {
    int idx;
    if (!url_available(url, idx)) return;
    if (vcbs[idx]->type == type) return;
    vcbs[idx]->mtx.lock();
    vcbs[idx]->type = type;
    std::queue<QImage> empty;
    std::swap(vcbs[idx]->cache, empty);
    if (vcbs[idx]->writer_wait) {
        vcbs[idx]->decoder_start_sem.post();
        vcbs[idx]->writer_wait = false;
    }
    vcbs[idx]->mtx.unlock();
    emit videoDecodeTypeChanged(QString::number(idx));
}

int Video::get_decode_type(const QString &url) {
    int idx;
    if (!url_available(url, idx)) return Raw;
    return vcbs[idx]->type;
}

void Video::set_wh(const QString &url, int width, int height) {
    int idx;
    if (!url_available(url, idx)) return;
    calcWH(vcbs[idx]->width, vcbs[idx]->height, vcbs[idx]->real_width, vcbs[idx]->real_height, width, height);
}

bool Video::get_cuda(const QString &url) {
    int idx;
    if (!url_available(url, idx)) return false;
    return vcbs[idx]->cuda;
}

void Video::set_cuda(const QString &url, bool cuda) {
    int idx;
    if (!url_available(url, idx)) return;
    if (vcbs[idx]->cuda == cuda) return;
    if (!ImageCrypto::cuda_is_available()) return;
    if (cuda) vcbs[idx]->crypto->cuda();
    else vcbs[idx]->crypto->cpu();
    vcbs[idx]->cuda = cuda;
    emit videoCudaChanged(QString::number(idx));
}

void Video::set_param(const QString &url, const QString &key_id) {
    int idx;
    if (!url_available(url, idx)) return;
    auto key = keyMap[key_id];
    vcbs[idx]->crypto->setKeys(key.key.control, key.key.keys);
}

bool Video::get_pause(const QString &url) {
    int idx;
    if (!url_available(url, idx)) return true;
    return vcbs[idx]->paused;
}

void Video::__cvtVideo(const QUrl &in_file, const QUrl &out_file, const QString &key_id, bool encrypt,
                       bool cuda) {
    cv::VideoCapture cap;
    if (!cap.open(in_file.toLocalFile().toStdString())) {
        emit videoCvtSignal(0, 0);
        return;
    }
    auto fps = cap.get(cv::CAP_PROP_FPS);
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    width &= 0xfffffffe;
    if (width == 0) width = 2;
    height &= 0xfffffffe;
    if (height == 0) height = 2;
    cv::VideoWriter wrt;
    if (!wrt.open(FileUniqueForceSuffix(
#ifdef _WIN32
                      UTF8toGBK(out_file.toLocalFile().toStdString())
#else
                      out_file.toLocalFile().toStdString()
#endif
                      .c_str(), "avi"),
                  cv::VideoWriter::fourcc('F', 'F', 'V', '1'), fps,
                  cv::Size(width, height),
                  true)) {
        emit videoCvtSignal(0, 0);
        return;
    }
    auto crypto(keyMap[key_id].imageCrypto);
    if (cuda) crypto.cuda();
    else crypto.cpu();
    auto total_frames = cap.get(cv::CAP_PROP_FRAME_COUNT);
    cv::Mat frame;
    cv::Mat out;
    while (true) {
        cap >> frame;
        if (frame.empty() || term)
            break;
        if (encrypt)
            out = crypto.encrypt(frame);
        else
            out = crypto.decrypt(frame);
        wrt << out;
        emit videoCvtSignal(static_cast<int>(cap.get(cv::CAP_PROP_POS_FRAMES)), total_frames);
    }
    if (term) {
        emit videoCvtSignal(0, 0);
        term = false;
    }
    cap.release();
    wrt.release();
}

void Video::__cvtVideoWH(const QUrl &in_file, const QUrl &out_file, const QString &key_id, bool encrypt,
                         bool cuda, int width, int height) {
    width &= 0xfffffffe;
    if (width == 0) width = 2;
    height &= 0xfffffffe;
    if (height == 0) height = 2;
    cv::VideoCapture cap;
    if (!cap.open(in_file.toLocalFile().toStdString())) {
        emit videoCvtSignal(0, 0);
        return;
    }
    auto fps = cap.get(cv::CAP_PROP_FPS);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cv::VideoWriter wrt;
    if (!wrt.open(FileUniqueForceSuffix(
#ifdef _WIN32
                      UTF8toGBK(out_file.toLocalFile().toStdString())
#else
                      out_file.toLocalFile().toStdString()
#endif
                      .c_str(), "avi"),
                  cv::VideoWriter::fourcc('F', 'F', 'V', '1'), fps,
                  cv::Size(width, height),
                  true)) {
        emit videoCvtSignal(0, 0);
        return;
    }
    auto crypto(keyMap[key_id].imageCrypto);
    if (cuda) crypto.cuda();
    else crypto.cpu();
    auto total_frames = cap.get(cv::CAP_PROP_FRAME_COUNT);
    cv::Mat frame;
    cv::Mat out;
    int count = 0;
    while (true) {
        cap >> frame;
        if (frame.empty() || term)
            break;
        cv::resize(frame, frame, cv::Size(width, height));
        if (encrypt) {
            out = crypto.encrypt(frame);
        } else {
            out = crypto.decrypt(frame);
        }
        wrt << out;
        count = static_cast<int>(cap.get(cv::CAP_PROP_POS_FRAMES));
        emit videoCvtSignal(count, total_frames);
    }
    if (term) {
        emit videoCvtSignal(0, 0);
        term = false;
    } else if (count != total_frames) {
        emit videoCvtSignal(total_frames, total_frames);
    }
    cap.release();
    wrt.release();
}


void Video::cvtVideo(const QUrl &in_file, const QUrl &out_file, const QString &key_id, bool encrypt, bool cuda) {
    term = false;
    g_threadPool.addThreadEX(&Video::__cvtVideo, this, in_file, out_file, key_id, encrypt,
                             cuda).setNoWait().start();
}

void Video::cvtVideoWH(const QUrl &in_file, const QUrl &out_file, const QString &key_id, bool encrypt, bool cuda,
                       int width, int height) {
    term = false;
    g_threadPool.addThreadEX(&Video::__cvtVideoWH, this, in_file, out_file, key_id, encrypt,
                             cuda, width, height).setNoWait().start();
}

QVariantMap Video::getVideoWH(const QUrl &in_file) {
    cv::VideoCapture cap;
    QVariantMap ret;
    if (!cap.open(in_file.toLocalFile().toStdString())) {
        ret["width"] = 0;
        ret["height"] = 0;
        return ret;
    }
    ret["width"] = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    ret["height"] = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    return ret;
}

void Video::force_stop_cvt() {
    term = true;
}

u64 Video::get_putIdx(const bool isRet, const u64 oldIdx) {
    static u64 idx = 0;
    static Semaphore semaphore(1);
    static FastBitmap bitmap;
    semaphore.wait();
    if (isRet == true) {
        bitmap[oldIdx] = false;
        semaphore.post();
        return oldIdx;
    }
    auto id = bitmap.findNextFalse(0, idx);
    if (id != BITMAP_NOT_FOUND) {
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
