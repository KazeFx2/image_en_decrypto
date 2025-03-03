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

class ImageCrypto
{
public:
    ImageCrypto(ThreadPool& threadPool): threadPool(&threadPool), size(new ORIGINAL_SIZE), config(DEFAULT_CONFIG),
                                         keys(RANDOM_KEYS), tmp_size(cv::Size(0, 0)), tmp_config({}), tmp_keys({}),
                                         update_params(NONE),
                                         tmp_cuda(config.cuda)
    {
        preProcess();
    };

    ImageCrypto(ThreadPool& threadPool, const ParamControl& config,
                const Keys& keys): threadPool(&threadPool), size(new ORIGINAL_SIZE), config(config),
                                   keys(keys), tmp_size(cv::Size(0, 0)), tmp_config({}), tmp_keys({}), update_params(NONE),
                                   tmp_cuda(config.cuda)
    {
        preProcess();
    };

    ImageCrypto(ThreadPool& threadPool, const cv::Size& size): threadPool(&threadPool),
                                                               size(new cv::Size(size)), config(DEFAULT_CONFIG),
                                                               keys(RANDOM_KEYS), tmp_size(cv::Size(0, 0)), tmp_config({}),
                                                               tmp_keys({}), update_params(NONE),
                                                               tmp_cuda(config.cuda)
    {
        preProcess();
    };

    ImageCrypto(ThreadPool& threadPool, const cv::Size& size, const ParamControl& config): threadPool(&threadPool),
        size(new cv::Size(size)), config(config),
        keys(RANDOM_KEYS), tmp_size(cv::Size(0, 0)), tmp_config({}), tmp_keys({}), update_params(NONE),
        tmp_cuda(config.cuda)
    {
        preProcess();
    };

    ImageCrypto(ThreadPool& threadPool, const cv::Size& size, const ParamControl& config,
                const Keys& keys): threadPool(&threadPool),
                                   size(new cv::Size(size)),
                                   config(config),
                                   keys(keys), tmp_size(cv::Size(0, 0)), tmp_config({}), tmp_keys({}), update_params(NONE),
                                   tmp_cuda(config.cuda)
    {
        preProcess();
    };

    ImageCrypto(const ImageCrypto& src)
    {
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

    ImageCrypto(ImageCrypto&& src)
    {
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

    ImageCrypto& operator=(const ImageCrypto& src)
    {
        threadPool = src.threadPool;
        if (rets) DestroyReturn(rets, config);
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

    ImageCrypto& operator=(ImageCrypto&& src)
    {
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

    ParamControl getConfig() const
    {
        return config;
    }

    Keys getKeys() const
    {
        return keys;
    }

    ThreadPool& getThreadPool() const
    {
        return *threadPool;
    }

    void setThreadPool(ThreadPool& threadPool)
    {
        this->threadPool = &threadPool;
    }

    cv::Size getSize() const
    {
        return *size;
    }

    u8 getChannel() const
    {
        return config.nChannel;
    }

    cv::Mat encrypt(const cv::Mat& image, cv::Size imageSize = ORIGINAL_SIZE)
    {
        if (threadPool->getMaxThreads() < config.nThread) threadPool->setMax(config.nThread);
        else threadPool->reduceTo(config.nThread);
        u8 nChannel = image.channels();
        if (imageSize == ORIGINAL_SIZE)
        {
            imageSize = image.size();
        }
        if (imageSize != *size || nChannel != config.nChannel)
        {
            auto tmpC = config;
            tmpC.nChannel = nChannel;
            setAll(imageSize, tmpC, keys);
        }
        if (!rets)preProcess();
        cv::Mat imageMat = image.clone();
        config.cuda = tmp_cuda;
        __setAll();
        EncryptoImage(imageMat, *size, keys, rets, config, *threadPool);
        config.cuda = tmp_cuda;
        __setAll();
        return imageMat;
    }

    cv::Mat decrypt(const cv::Mat& image, cv::Size imageSize = ORIGINAL_SIZE)
    {
        if (threadPool->getMaxThreads() < config.nThread) threadPool->setMax(config.nThread);
        else threadPool->reduceTo(config.nThread);
        u8 nChannel = image.channels();
        if (imageSize == ORIGINAL_SIZE)
        {
            imageSize = image.size();
        }
        if (imageSize != *size || nChannel != config.nChannel)
        {
            auto tmpC = config;
            tmpC.nChannel = nChannel;
            setAll(imageSize, tmpC, keys);
        }
        if (!rets)preProcess();
        cv::Mat imageMat = image.clone();
        config.cuda = tmp_cuda;
        __setAll();
        DecryptoImage(imageMat, *size, keys, rets, config, *threadPool);
        config.cuda = tmp_cuda;
        __setAll();
        return imageMat;
    }

    static bool cuda_is_available()
    {
#ifdef __USE_CUDA
        return CudaAvailable();
#else
        return false;
#endif
    }

    static cv::Mat loadImage(const std::string& url)
    {
        return cv::imread(url);
    }

    bool cpu()
    {
        tmp_cuda = false;
        return true;
    }

    bool cuda()
    {
        if (!cuda_is_available()) return false;
        tmp_cuda = true;
        return true;
    }


    void setKeys(const Keys& keys)
    {
        if (StructureEqual(&this->keys, &keys, sizeof(Keys))) return;
        tmp_keys = keys;
        update_params |= KEY;
    }

    void setKeys(const ParamControl& config)
    {
        if (StructureEqual(&this->config, &config, sizeof(ParamControl))) return;
        tmp_config = config;
        update_params |= CONFIG;
    }

    void setKeys(const ParamControl& config, const Keys& keys)
    {
        setKeys(config);
        setKeys(keys);
    }

    void setSize(const cv::Size size)
    {
        if (this->size->width == size.width && this->size->height == size.height) return;
        tmp_size = size;
        update_params |= SIZE;
    }

    void setChannel(const u8 nChannel)
    {
        if (config.nChannel == nChannel) return;
        tmp_config = config;
        tmp_config.nChannel = nChannel;
        update_params |= CONFIG;
    }

    void setAll(const cv::Size& size, const ParamControl& config, const Keys& keys)
    {
        setSize(size);
        setKeys(config, keys);
    }

    ~ImageCrypto()
    {
        delete size;
        if (rets)
            DestroyReturn(rets, config);
    }

private:
    typedef enum
    {
        NONE = 0x0,
        CONFIG = 0x1,
        KEY = 0x2,
        SIZE = 0x4
    } UpdateType;

    void preProcess()
    {
        if (config.cuda)
        {
            config.cuda = cuda_is_available();
        }
        rets = GenerateThreadKeys(*size, keys, config, *threadPool);
    }

    void __setAll()
    {
        if (update_params == NONE) return;
        if (rets != nullptr)
        {
            DestroyReturn(rets, this->config);
        }
        if (update_params & CONFIG) this->config = tmp_config;
        if (update_params & KEY) this->keys = tmp_keys;
        if (update_params & SIZE) *this->size = tmp_size;
        update_params = NONE;
        preProcess();
    }

    ThreadPool* threadPool;
    cv::Size* size;
    ParamControl config;
    Keys keys;
    threadReturn** rets;
    cv::Size tmp_size;
    ParamControl tmp_config;
    Keys tmp_keys;
    u8 update_params;
    bool tmp_cuda;
};

#endif //IMAGECRYPTO_H
