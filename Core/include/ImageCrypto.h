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

class ImageCrypto {
public:
    ImageCrypto(ThreadPool *threadPool): threadPool(threadPool), size(new ORIGINAL_SIZE), config(DEFAULT_CONFIG),
                                         keys(RANDOM_KEYS) {
        preProcess();
    };

    ImageCrypto(ThreadPool *threadPool, const cv::Size &size): threadPool(threadPool),
                                                               size(new cv::Size(size)), config(DEFAULT_CONFIG),
                                                               keys(RANDOM_KEYS) {
        preProcess();
    };

    ImageCrypto(ThreadPool *threadPool, const cv::Size &size, const ParamControl &config): threadPool(threadPool),
        size(new cv::Size(size)), config(config),
        keys(RANDOM_KEYS) {
        preProcess();
    };

    ImageCrypto(ThreadPool *threadPool, const cv::Size size, const ParamControl &config,
                const Keys &keys): threadPool(threadPool),
                                   size(new cv::Size(size)),
                                   config(config),
                                   keys(keys) {
        preProcess();
    };

    ParamControl getConfig() const {
        return config;
    }

    Keys getKeys() const {
        return keys;
    }

    void setKeys(const Keys &keys) {
        if (rets != nullptr) {
            DestroyReturn(rets, config);
        }
        this->keys = keys;
        preProcess();
    }

    void setKeys(const ParamControl &config) {
        if (rets != nullptr) {
            DestroyReturn(rets, config);
        }
        this->config = config;
        preProcess();
    }

    ThreadPool *getThreadPool() const {
        return threadPool;
    }

    void setThreadPool(ThreadPool *threadPool) {
        this->threadPool = threadPool;
    }

    cv::Size getSize() const {
        return *size;
    }

    void setSize(const cv::Size size) {
        if (rets != nullptr) {
            DestroyReturn(rets, config);
        }
        *this->size = size;
        preProcess();
    }

    cv::Mat encrypt(const cv::Mat &image, cv::Size imageSize = ORIGINAL_SIZE) {
        if (imageSize == ORIGINAL_SIZE) {
            imageSize = image.size();
        }
        if (imageSize != *size) {
            setSize(imageSize);
        }
        cv::Mat imageMat = image.clone();
        EncryptoImage(imageMat, *size, keys, rets, config, *threadPool);
        return imageMat;
    }

    cv::Mat decrypt(const cv::Mat &image, cv::Size imageSize = ORIGINAL_SIZE) {
        if (imageSize == ORIGINAL_SIZE) {
            imageSize = image.size();
        }
        if (imageSize != *size) {
            setSize(imageSize);
        }
        cv::Mat imageMat = image.clone();
        DecryptoImage(imageMat, *size, keys, rets, config, *threadPool);
        return imageMat;
    }

    ~ImageCrypto() {
        delete size;
    }

private:
    void preProcess() {
        rets = GenerateThreadKeys(*size, keys, config, *threadPool);
    }

    ThreadPool *threadPool;
    cv::Size *size;
    ParamControl config;
    Keys keys;
    threadReturn **rets;
};

#endif //IMAGECRYPTO_H
