//
// Created by Fx Kaze on 24-12-30.
//

#include "unistd.h"
#include "private/vars.h"
#include "ThreadPool.h"
#include "ImageCrypto.h"
#include <sys/time.h>

class NoPool : ThreadPool {
private:
    std::vector<pthread_t> threads;
    std::vector<bool> ava;

public:
    NoPool(int n): ThreadPool(n) {
    }

    task_descriptor_t addThread(funcHandler func, void *param, bool wait = true,
                                u32 *taskUnique = nullptr) override {
        for (int i = 0; i < ava.size(); ++i) {
            if (ava[i] == false) {
                pthread_t &t = threads[i];
                ava[i] = true;
                pthread_create(&t, nullptr, func, param);
                return i;
            }
        }
        ava.push_back(true);
        threads.push_back(pthread_t());
        pthread_t &t = threads.back();
        pthread_create(&t, nullptr, func, param);
        return threads.size() - 1;
    }

    bool isDescriptorAvailable(task_descriptor_t descriptor) override { return descriptor < threads.size(); }

    void *waitThread(task_descriptor_t index) override {
        if (index >= threads.size()) { return nullptr; }
        const pthread_t &t = threads[index];
        void *ret = nullptr;
        pthread_join(t, &ret);
        return ret;
    }

    bool destroyThread(u_count_t index, bool onlyIdly = false) override {
        return true;
    }

    void reduceTo(s_count_t tar, bool force = false) override {
    }

    void setMax(u_count_t max) override {
    }

    void waitReduce() override {
    }

    void waitFinish() override {
    }

    u_count_t getNumThreads() const { return 0; }

    u_count_t getIdlyThreads() const { return 0; }

    u_count_t getMaxThreads() const { return 0; }
};

double GetCPUSecond() {
    timeval tp;
    gettimeofday(&tp, nullptr);
    return (static_cast<double>(tp.tv_sec) + static_cast<double>(tp.tv_usec) * 1.e-6);
}

double time_cost(cv::Mat &img, ImageCrypto &crypto, int n_mean) {
    double cost = 0, start = 0, end = 0;
    int ct = 0;
    while (n_mean--) {
        start = GetCPUSecond();
        crypto.encrypt(img);
        end = GetCPUSecond();
        cost += end - start;
        ct++;
    }
    return cost / ct;
}

double time_cost_n_thread(cv::Mat &img, ThreadPool &pool, Keys &k, ParamControl &config, int n_thread, int n_mean) {
    config.nThread = n_thread;
    ImageCrypto crypto(pool, ORIGINAL_SIZE, config, k);
    return time_cost(img, crypto, n_mean);
}

int main(int argc, const char **argv) {
    chdir(homePath);

    NoPool nPool(1);
    ThreadPool tPool(32);

    Keys keys = {
        0x134,
        {0.789, 0.114},
        {0.962, 0.415}
    };

    ParamControl config = DEFAULT_CONFIG;

    std::cout << getcwd(nullptr, 0) << std::endl;
    auto img = imread("inputs/LenaRGB.png", cv::IMREAD_UNCHANGED);
    if (img.data == nullptr) {
        std::cout << "Read image failed." << std::endl;
        return 1;
    }

    for (int i = 17; i <= 32; i++) {
        double t = time_cost_n_thread(img, (ThreadPool &)nPool, keys, config, i, 1000);
        printf("time for encryption [no pool,   %d threads]: %.3f (ms)\n",i, t * 1000);
        t = time_cost_n_thread(img, tPool, keys, config, i, 100);
        printf("time for encryption [with pool, %d threads]: %.3f (ms)\n",i, t * 1000);
    }

    return 0;
}
