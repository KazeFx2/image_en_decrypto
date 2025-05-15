
//
// Created by Fx Kaze on 24-12-30.
//

#ifndef IMAGECRYPTO_H
#define IMAGECRYPTO_H

#include "ThreadPool.h"
#include "private/includes.h"
#include "private/ImageEncrypto.h"
#include "private/ImageDecrypto.h"
#include "private/Random.h"
#include "private/Util.h"

#ifdef __USE_CUDA
#include "private/Cuda.cuh"
#endif

class ImageCrypto {
public:
    ImageCrypto(ThreadPool &threadPool): threadPool(&threadPool), size(new ORIGINAL_SIZE), config(DEFAULT_CONFIG),
                                         keys(RANDOM_KEYS), tmp_size(cv::Size(0, 0)), tmp_config({}), tmp_keys({}),
                                         update_params(NONE),
                                         tmp_cuda(config.cuda) {
        rets = nullptr;
        // preProcess();
    };

    ImageCrypto(ThreadPool &threadPool, const ParamControl &config,
                const Keys &keys): threadPool(&threadPool), size(new ORIGINAL_SIZE), config(config),
                                   keys(keys), tmp_size(cv::Size(0, 0)), tmp_config({}), tmp_keys({}),
                                   update_params(NONE),
                                   tmp_cuda(config.cuda) {
        rets = nullptr;
        // preProcess();
    };

    ImageCrypto(ThreadPool &threadPool, const cv::Size &size): threadPool(&threadPool),
                                                               size(new cv::Size(size)), config(DEFAULT_CONFIG),
                                                               keys(RANDOM_KEYS), tmp_size(cv::Size(0, 0)),
                                                               tmp_config({}),
                                                               tmp_keys({}), update_params(NONE),
                                                               tmp_cuda(config.cuda) {
        rets = nullptr;
        // preProcess();
    };

    ImageCrypto(ThreadPool &threadPool, const cv::Size &size, const ParamControl &config): threadPool(&threadPool),
        size(new cv::Size(size)), config(config),
        keys(RANDOM_KEYS), tmp_size(cv::Size(0, 0)), tmp_config({}), tmp_keys({}), update_params(NONE),
        tmp_cuda(config.cuda) {
        rets = nullptr;
        // preProcess();
    };

    ImageCrypto(ThreadPool &threadPool, const cv::Size &size, const ParamControl &config,
                const Keys &keys): threadPool(&threadPool),
                                   size(new cv::Size(size)),
                                   config(config),
                                   keys(keys), tmp_size(cv::Size(0, 0)), tmp_config({}), tmp_keys({}),
                                   update_params(NONE),
                                   tmp_cuda(config.cuda) {
        rets = nullptr;
        // preProcess();
    };

    ImageCrypto(const ImageCrypto &src) {
        threadPool = src.threadPool;
        size = new cv::Size(*src.size);
        config = src.config;
        keys = src.keys;
        rets = CopyReturn(src.rets, config, *size);
        tmp_size = src.tmp_size;
        tmp_config = src.tmp_config;
        tmp_keys = src.tmp_keys;
        update_params = src.update_params;
        tmp_cuda = src.tmp_cuda;
    }

    ImageCrypto(ImageCrypto &&src) {
        threadPool = src.threadPool;
        size = src.size;
        src.size = nullptr;
        config = src.config;
        keys = src.keys;
        rets = src.rets;
        src.rets = nullptr;
        tmp_size = src.tmp_size;
        tmp_config = src.tmp_config;
        tmp_keys = src.tmp_keys;
        update_params = src.update_params;
        tmp_cuda = src.tmp_cuda;
    }

    ImageCrypto &operator=(const ImageCrypto &src) {
        threadPool = src.threadPool;
        if (rets) {
            DestroyReturn(rets, config);
            rets = nullptr;
        }
        if (!size) size = new cv::Size;
        *size = *src.size;
        config = src.config;
        keys = src.keys;
        rets = CopyReturn(src.rets, config, *size);
        tmp_size = src.tmp_size;
        tmp_config = src.tmp_config;
        tmp_keys = src.tmp_keys;
        update_params = src.update_params;
        tmp_cuda = src.tmp_cuda;
        return *this;
    }

    ImageCrypto &operator=(ImageCrypto &&src) {
        threadPool = src.threadPool;
        size = src.size;
        src.size = nullptr;
        config = src.config;
        keys = src.keys;
        rets = src.rets;
        src.rets = nullptr;
        tmp_size = src.tmp_size;
        tmp_config = src.tmp_config;
        tmp_keys = src.tmp_keys;
        update_params = src.update_params;
        tmp_cuda = src.tmp_cuda;
        return *this;
    }

    ParamControl getConfig() const {
        if (update_params & CONFIG)
            return tmp_config;
        return config;
    }

    Keys getKeys() const {
        if (update_params & KEY)
            return tmp_keys;
        return keys;
    }

    ThreadPool &getThreadPool() const {
        return *threadPool;
    }

    void setThreadPool(ThreadPool &threadPool) {
        this->threadPool = &threadPool;
    }

    cv::Size getSize() const {
        if (update_params & SIZE)
            return tmp_size;
        return *size;
    }

    u8 getChannel() const {
        if (update_params & CONFIG)
            return tmp_config.nChannel;
        return config.nChannel;
    }

    cv::Mat encrypt(const cv::Mat &image, cv::Size imageSize = ORIGINAL_SIZE) {
        u8 nChannel = image.channels();
        if (imageSize == ORIGINAL_SIZE) {
            imageSize = image.size();
        }
        if (imageSize != *size || nChannel != config.nChannel) {
            auto tmpC = config;
            tmpC.nChannel = nChannel;
            setAll(imageSize, tmpC, keys);
        }
        cv::Mat imageMat = image.clone();
        config.cuda = tmp_cuda;
        __setAll();
        if (!rets)preProcess();
        if (threadPool->getMaxThreads() < config.nThread) threadPool->setMax(config.nThread);
        else threadPool->reduceTo(static_cast<int>(config.nThread));
        EncryptoImage(imageMat, *size, keys, rets, config, *threadPool);
        config.cuda = tmp_cuda;
        __setAll();
        return imageMat;
    }

    cv::Mat decrypt(const cv::Mat &image, cv::Size imageSize = ORIGINAL_SIZE) {
        u8 nChannel = image.channels();
        if (imageSize == ORIGINAL_SIZE) {
            imageSize = image.size();
        }
        if (imageSize != *size || nChannel != config.nChannel) {
            auto tmpC = config;
            tmpC.nChannel = nChannel;
            setAll(imageSize, tmpC, keys);
        }
        cv::Mat imageMat = image.clone();
        config.cuda = tmp_cuda;
        __setAll();
        if (!rets)preProcess();
        if (threadPool->getMaxThreads() < config.nThread) threadPool->setMax(config.nThread);
        else threadPool->reduceTo(static_cast<int>(config.nThread));
        DecryptoImage(imageMat, *size, keys, rets, config, *threadPool);
        config.cuda = tmp_cuda;
        __setAll();
        return imageMat;
    }

    void *encrypt(const void *src, const u32 width, const u32 height, const u32 nC) {
        const u8 nChannel = nC;
        if (width != size->width || height != size->height || nChannel != config.nChannel) {
            auto tmpC = config;
            tmpC.nChannel = nChannel;
            setAll(cv::Size(static_cast<int>(width), static_cast<int>(height)), tmpC, keys);
        }
        void *ret = new u8[width * height * nChannel];
        memcpy(ret, src, width * height * nChannel);
        __setAll();
        config.cuda = tmp_cuda;
        if (!rets)preProcess();
        if (threadPool->getMaxThreads() < config.nThread) threadPool->setMax(config.nThread);
        else threadPool->reduceTo(static_cast<int>(config.nThread));
        EncryptoImage(ret, width, height, keys, rets, config, *threadPool);
        config.cuda = tmp_cuda;
        __setAll();
        return ret;
    }

    void *decrypt(const void *src, const u32 width, const u32 height, const u32 nC) {
        const u8 nChannel = nC;
        if (width != size->width || height != size->height || nChannel != config.nChannel) {
            auto tmpC = config;
            tmpC.nChannel = nChannel;
            setAll(cv::Size(static_cast<int>(width), static_cast<int>(height)), tmpC, keys);
        }
        void *ret = new u8[width * height * nChannel];
        memcpy(ret, src, width * height * nChannel);
        __setAll();
        config.cuda = tmp_cuda;
        if (!rets)preProcess();
        if (threadPool->getMaxThreads() < config.nThread) threadPool->setMax(config.nThread);
        else threadPool->reduceTo(static_cast<int>(config.nThread));
        DecryptoImage(ret, width, height, keys, rets, config, *threadPool);
        config.cuda = tmp_cuda;
        __setAll();
        return ret;
    }

    void audioEncrypt(const char *input, const char *output, u32 width, u32 height, const bool *term,
                      void (*signal)(u64, u64) = nullptr) {
        void *data = nullptr, *o_data = nullptr;
        u8 nChannel = 0;
        u32 sample_size = 0;
        audio_open(input, &nChannel, nullptr, &data);
        audio_open_out(data, output, &sample_size, &o_data);
        if (nChannel == 0 || sample_size == 0) {
            if (signal)
                signal(0, 0);
        }
        if (width * sample_size != getSize().width || height != getSize().height || nChannel != getConfig().nChannel) {
            auto tmpC = getConfig();
            tmpC.nChannel = nChannel;
            setAll(cv::Size(static_cast<int>(width * sample_size), static_cast<int>(height)), tmpC, getKeys());
        }
        __setAll();
        config.cuda = tmp_cuda;
        if (!rets)preProcess();
        if (threadPool->getMaxThreads() < config.nThread) threadPool->setMax(config.nThread);
        else threadPool->reduceTo(static_cast<int>(config.nThread));
        if (audio_crypto(data, o_data, keys, rets, config, *threadPool, width, height, true, term, signal) == 0) {
            if (signal)
                signal(1, 1);
        } else {
            if (signal)
                signal(0, 0);
        }
        config.cuda = tmp_cuda;
        __setAll();
    }

    void audioDecrypt(const char *input, const char *output, u32 width, u32 height, const bool *term,
                      void (*signal)(u64, u64) = nullptr) {
        void *data = nullptr, *o_data = nullptr;
        u8 nChannel = 0;
        u32 sample_size = 0;
        audio_open(input, &nChannel, &sample_size, &data);
        audio_open_out(data, output, nullptr, &o_data);
        if (nChannel == 0 || sample_size == 0) {
            if (signal)
                signal(0, 0);
        }
        if (width * sample_size != getSize().width || height != getSize().height || nChannel != getConfig().nChannel) {
            auto tmpC = getConfig();
            tmpC.nChannel = nChannel;
            setAll(cv::Size(static_cast<int>(width * sample_size), static_cast<int>(height)), tmpC, getKeys());
        }
        __setAll();
        config.cuda = tmp_cuda;
        if (!rets)preProcess();
        if (threadPool->getMaxThreads() < config.nThread) threadPool->setMax(config.nThread);
        else threadPool->reduceTo(static_cast<int>(config.nThread));
        if (audio_crypto(data, o_data, keys, rets, config, *threadPool, width, height, false, term, signal) == 0) {
            if (signal)
                signal(1, 1);
        } else {
            if (signal)
                signal(0, 0);
        }
        config.cuda = tmp_cuda;
        __setAll();
    }

    static bool cuda_is_available() {
#ifdef __USE_CUDA
        return CudaAvailable();
#else
        return false;
#endif
    }

    static cv::Mat loadImage(const std::string &url) {
#ifdef _WIN32
        return cv::imread(UTF8toGBK(url));
#else
        return cv::imread(url);
#endif
    }

    bool cpu() {
        tmp_cuda = false;
        return true;
    }

    bool cuda() {
        if (!cuda_is_available()) return false;
        tmp_cuda = true;
        return true;
    }


    void setKeys(const Keys &keys) {
        if (StructureEqual(&this->keys, &keys, sizeof(Keys))) return;
        tmp_keys = keys;
        update_params |= KEY;
    }

    void setKeys(const ParamControl &config) {
        if (StructureEqual(&this->config, &config, sizeof(ParamControl))) return;
        tmp_config = config;
        update_params |= CONFIG;
    }

    void setKeys(const ParamControl &config, const Keys &keys) {
        setKeys(config);
        setKeys(keys);
    }

    void setSize(const cv::Size size) {
        if (this->size->width == size.width && this->size->height == size.height) return;
        tmp_size = size;
        update_params |= SIZE;
    }

    void setChannel(const u8 nChannel) {
        if (config.nChannel == nChannel) return;
        tmp_config = config;
        tmp_config.nChannel = nChannel;
        update_params |= CONFIG;
    }

    void setAll(const cv::Size &size, const ParamControl &config, const Keys &keys) {
        setSize(size);
        setKeys(config, keys);
    }

    ~ImageCrypto() {
        delete size;
        if (rets)
            DestroyReturn(rets, config);
        rets = nullptr;
    }

private:
    typedef enum {
        NONE = 0x0,
        CONFIG = 0x1,
        KEY = 0x2,
        SIZE = 0x4
    } UpdateType;

    void preProcess() {
        if (config.cuda) {
            config.cuda = cuda_is_available();
        }
        if (rets) {
            DestroyReturn(rets, config);
            rets = nullptr;
        }
        rets = GenerateThreadKeys(*size, keys, config, *threadPool);
    }

    void __setAll() {
        if (update_params == NONE) return;
        if (rets) {
            DestroyReturn(rets, this->config);
            rets = nullptr;
        }
        if (update_params & CONFIG) this->config = tmp_config;
        if (update_params & KEY) this->keys = tmp_keys;
        if (update_params & SIZE) *this->size = tmp_size;
        update_params = NONE;
        preProcess();
    }

    ThreadPool *threadPool;
    cv::Size *size;
    ParamControl config;
    Keys keys;
    threadReturn **rets;
    cv::Size tmp_size;
    ParamControl tmp_config;
    Keys tmp_keys;
    u8 update_params;
    bool tmp_cuda;
};

#endif //IMAGECRYPTO_H
