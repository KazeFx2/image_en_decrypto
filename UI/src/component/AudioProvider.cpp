//
// Created by kazefx on 25-2-27.
//

// #include <qfuture.h>
#include "component/AudioProvider.h"

#include <QThread>
#include <QAudioFormat>
#include <QAudioSink>
// #include <QtConcurrent/qtconcurrentrun.h>

#include "GlobalVariables.h"
#include "Semaphore.h"
#include "component/Crypto.h"
#include "component/ImageProvider.h"
#include "component/KeyKeeper.h"

std::vector<AudioControl *> Audio::acbs;

AudioControl *Audio::idx_ava(int id) {
    if (id >= acbs.size()) return nullptr;
    return acbs[id];
}

static void
send_smp(const u8 *data, u32 size, void *ctx) {
    auto *acb = static_cast<AudioControl *>(ctx);
    if (!acb) return;

    acb->internalBuffer.append(reinterpret_cast<const char *>(data), size);
}

static void
set_msec(double msec, u32 pts, void *ctx) {
    auto *acb = static_cast<AudioControl *>(ctx);
    if (!acb) return;
    acb->mtx.lock();
    acb->internalBuffer.clear();
    acb->totalBytesWriten = pts * acb->bytes_per_pts;
    acb->current_msec = msec;
    acb->mtx.unlock();
}

static void
cleanup(void *ctx) {
    auto *acb = static_cast<AudioControl *>(ctx);
    if (!acb) return;
    if (acb->parent && acb->freeAudio) {
        (acb->parent->*(acb->freeAudio))(acb->id);
    }
}

static void
type_change(void *ctx) {
    auto *acb = static_cast<AudioControl *>(ctx);
    if (!acb) return;
    if (acb->parent) {
        emit (acb->parent)->audioDecodeTypeChanged(acb->id);
    }
}

PCMIODevice::PCMIODevice(QObject *parent)
    : QIODevice(parent) {
}

void PCMIODevice::appendData(const QByteArray &data) {
    buffer.append(data);
}

void PCMIODevice::clear() {
    buffer.clear();
}

qint64 PCMIODevice::readData(char *data, qint64 maxlen) {
    qint64 len = qMin(maxlen, (qint64) buffer.size());
    if (len == 0) {
        memset(data, 0, maxlen);
        return maxlen;
    }
    memcpy(data, buffer.constData(), len);
    buffer.remove(0, len);
    return len;
}

qint64 PCMIODevice::writeData(const char *, qint64) {
    return 0;
}

qint64 PCMIODevice::bytesAvailable() const {
    return buffer.size();
}

Audio::Audio(): QObject() {
}

static void (Audio::*func)(int, int) = nullptr;

static Audio *ptr = nullptr;

static bool term = false;

static void audio_cvt_signal(u64 i, u64 len) {
    if (func && ptr) {
        emit (ptr->*func)(i, len);
    }
}

void Audio::__cvtAudio(const QUrl &in_file, const QUrl &out_file, const QString &key_id,
                       const int width, const int height, const bool encrypt, const bool cuda) {
    qDebug() << key_id;
    auto crypto(keyMap[key_id].imageCrypto);
    if (cuda) crypto.cuda();
    else crypto.cpu();
    func = &Audio::audioCvtSignal;
    ptr = this;
    auto output =
#ifdef _WIN32
                    GBKtoUTF8(
#endif
                        FileUnique(
#ifdef _WIN32
                            UTF8toGBK(out_file.toLocalFile().toStdString())
#else
                      out_file.toLocalFile().toStdString()
#endif
                            .c_str(), "wav")

#ifdef _WIN32
                    )
#endif
            ;
    term = false;
    if (encrypt) {
        crypto.audioEncrypt(
            in_file.toLocalFile().toStdString().c_str(),
            output.c_str(),
            width,
            height,
            &term,
            audio_cvt_signal
        );
    } else {
        crypto.audioDecrypt(
            in_file.toLocalFile().toStdString().c_str(),
            output.c_str(),
            width,
            height,
            &term,
            audio_cvt_signal
        );
    }
}

void Audio::__freeAudio(int id) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return;
    delete ac;
    acbs[id] = nullptr;
    get_putIdx(true, id);
}

void Audio::force_stop_cvt() {
    term = true;
}


void Audio::cvtAudio(const QUrl &in_file, const QUrl &out_file, const QString &key_id,
                     const int width, const int height, const bool encrypt, const bool cuda) {
    g_threadPool.addThreadEX(&Audio::__cvtAudio, this, in_file, out_file, key_id, width, height, encrypt,
                             cuda).setNoWait().start();
}

int Audio::loadAudio(const QUrl &file, const QString &key_id, DecodeType type, int width, int height, bool cuda) {
    const auto id = get_putIdx(false, 0);
    if (id >= acbs.size()) {
        acbs.push_back(new AudioControl);
    } else acbs[id] = new AudioControl;
    auto *acb = acbs[id];
    void *data;
    u8 channels;
    u32 sample_size;
    int sample_rate;
    bool is_float;
    double total_msec;
    u64 total_pts;
    audio_open(file.toLocalFile().toStdString().c_str(), &channels,
               &sample_size, &data, &sample_rate, &is_float, &total_msec, &total_pts
    );
    if (channels == 0 || sample_size == 0 || sample_rate == 0) {
        return -1;
    }
    auto &crypto = keyMap[key_id].imageCrypto;
    acb->ctrl.keys = crypto.getKeys();
    acb->ctrl.config = crypto.getConfig();
    acb->ctrl.config.nChannel = channels;
    acb->ctrl.pool = &crypto.getThreadPool();
    acb->ctrl.width = width;
    acb->ctrl.height = height;
    acb->ctrl.type = type;
    if (cuda && crypto.cuda_is_available())
        acb->ctrl.config.cuda = true;
    else
        acb->ctrl.config.cuda = false;
    acb->total_msec = total_msec;
    acb->bytes_per_pts = channels * 2;
    acb->totao_pts = total_pts;

    acb->ctrl.term = false;
    acb->ctrl.fin = false;
    acb->fin = false;
    acb->ctrl.update_type = false;
    acb->ctrl.update_keys = false;
    acb->ctrl.update_size = false;
    acb->ctrl.update_msec = false;
    acb->pause = true;
    acb->totalBytesWriten = 0;
    acb->current_msec = 0;
    acb->timer = acb->timer = new QTimer();
    QObject::connect(acb->timer, &QTimer::timeout, [=]()-> void {
        acb->mtx.lock();
        if (!acb->pause && acb->sink->bytesFree() > 0 && acb->internalBuffer.size() > 0) {
            int bytesToWrite = MIN(acb->sink->bytesFree(), acb->internalBuffer.size());
            int written = acb->io->write(acb->internalBuffer.constData(), bytesToWrite);
            if (written > 0)
                acb->internalBuffer.remove(0, written);
            acb->totalBytesWriten += written;
            acb->current_msec = static_cast<double>(acb->totalBytesWriten - (
                                                        acb->sink->bufferSize() - acb->sink->bytesFree())) * 1000 / (
                                    channels * 2 * sample_rate);
            emit audioUpdated(id, acb->current_msec);
        }
        if (!acb->pause && acb->ctrl.fin && (acb->sink->bytesFree() == acb->sink->bufferSize() || acb->sink->state() ==
                                             QAudio::IdleState)) {
            acb->fin = true;
            acb->internalBuffer.clear();
            acb->pause = true;
            acb->sink->suspend();
            emit audioPaused(id);
        }
        acb->mtx.unlock();
    });

    acb->ctrl.ctx = acb;
    acb->parent = this;
    acb->freeAudio = &Audio::__freeAudio;
    acb->id = id;

    acb->fmt.setSampleRate(sample_rate);
    acb->fmt.setChannelCount(channels);
    acb->fmt.setSampleFormat(QAudioFormat::Int16);
    acb->sink = new QAudioSink(acb->fmt);
    acb->io = acb->sink->start();
    acb->ctrl.send_samples = send_smp;
    acb->ctrl.set_msec = set_msec;
    acb->ctrl.cleanup = cleanup;
    acb->ctrl.type_change = type_change;
    acb->sink->suspend();
    g_threadPool.addThreadEX(
        audio_crypto_realtime,
        data, &(acb->ctrl)
    ).setNoWait().start();

    acb->timer->start(20);
    return id;
}

void Audio::delAudio(int id) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return;
    if (ac->ctrl.term)
        return;
    ac->ctrl.mtx.lock();
    if (ac->ctrl.term) {
        ac->ctrl.mtx.unlock();
        return;
    }
    if (ac->timer) {
        ac->timer->stop();
        delete ac->timer;
        ac->timer = nullptr;
    }
    if (ac->sink) {
        delete ac->sink;
        ac->sink = nullptr;
        ac->io = nullptr;
    }
    ac->ctrl.term = true;
    ac->ctrl.sem.post();
    ac->ctrl.mtx.unlock();
}

double Audio::total_msec(int id) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return 0;
    return ac->total_msec;
}

double Audio::get_msec(int id) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return 0;
    return ac->current_msec;
}

int Audio::get_vol(int id) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return 0;
    return ac->sink->volume() * 100;
}

void Audio::set_vol(int id, int vol) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return;
    ac->sink->setVolume(static_cast<qreal>(vol) / 100);
}

int Audio::get_decode_type(int id) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return Raw;
    return ac->ctrl.type;
}

bool Audio::get_cuda(int id) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return false;
    if (ac->ctrl.update_cuda) return ac->ctrl.new_cuda;
    return ac->ctrl.config.cuda;
}

void Audio::set_cuda(int id, bool cuda) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return;
    ac->ctrl.mtx.lock();
    ac->ctrl.new_cuda = cuda;
    ac->ctrl.update_cuda = true;
    emit audioCudaChanged(id);
    ac->ctrl.mtx.unlock();
}

bool Audio::get_pause(int id) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return true;
    return ac->pause;
}

void Audio::set_type(int id, DecodeType type) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return;
    ac->ctrl.mtx.lock();
    ac->ctrl.new_type = type;
    ac->ctrl.update_type = true;
    ac->ctrl.new_msec = ac->current_msec;
    ac->ctrl.update_msec = true;
    ac->ctrl.sem.post();
    ac->ctrl.mtx.unlock();
}

void Audio::pause(int id) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return;
    ac->pause = true;
    ac->sink->suspend();
    emit audioPaused(id);
}

void Audio::resume(int id) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return;
    ac->ctrl.mtx.lock();
    if (ac->ctrl.fin && ac->fin) {
        ac->ctrl.sem.post();
        ac->ctrl.fin = false;
        ac->fin = false;
    }
    ac->ctrl.mtx.unlock();
    ac->pause = false;
    ac->sink->resume();
    emit audioResumed(id);
}

void Audio::goto_msec(int id, double time) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return;
    ac->ctrl.mtx.lock();
    ac->ctrl.new_msec = time;
    ac->ctrl.update_msec = true;
    ac->ctrl.sem.post();
    ac->ctrl.mtx.unlock();
}

void Audio::set_param(int id, const QString &key_id, int width, int height) {
    AudioControl *ac = idx_ava(id);
    if (!ac) return;
    auto &crypto = keyMap[key_id].imageCrypto;
    ac->ctrl.mtx.lock();
    ac->ctrl.new_msec = ac->current_msec;
    ac->ctrl.new_keys = crypto.getKeys();
    ac->ctrl.new_config = crypto.getConfig();
    ac->ctrl.new_config.nChannel = ac->bytes_per_pts / 2;
    ac->ctrl.new_width = width;
    ac->ctrl.new_height = height;
    ac->ctrl.update_msec = true;
    ac->ctrl.update_keys = true;
    ac->ctrl.update_size = true;
    ac->ctrl.sem.post();
    ac->ctrl.mtx.unlock();
}

u64 Audio::get_putIdx(const bool isRet, const u64 oldIdx) {
    static u64 idx = 0;
    static Semaphore semaphore(1);
    static FastBitmap bitmap;
    return getPutIdx(idx, semaphore, bitmap, isRet, oldIdx);
}

