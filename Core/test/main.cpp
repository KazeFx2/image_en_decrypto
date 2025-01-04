//
// Created by Fx Kaze on 24-12-30.
//

#include <private/Util.h>

#include "ImageCrypto.h"
#include "private/Random.h"
#include "private/ImageEncrypto.h"
#include "private/ImageDecrypto.h"
#include "unistd.h"
#include "vars.h"
#include "ThreadPool.h"
#include "Mutex.h"
#include <cmath>

Mutex mu;

void *threadFunc(void *arg) {
    mu.lock();
    // mu.lock();
    u64 ret = 0;
    u64 a = 1e10 * rand();
    while (a--) {
        ret += a;
    }
    std::cout << ret << std::endl;
    mu.unlock();
    return (void *) ret;
}

int main(int argc, const char **argv) {
    // cv::VideoCapture capture(argv[1]);
    // if (!capture.isOpened()) {
    //     printf("failed to open the video file\n");
    //     exit(1);
    // }
    //
    // ///
    // auto ct = 48;
    // cv::Mat img;
    // while (ct) {
    //     capture >> img;
    //     ct--;
    // }
    // capture >> img;
    //
    // // const u32 H = std::min(img.size().height, img.size().width);
    // resize(img, img, cv::Size(512, 512));
    // auto img_copy = img.clone();
    // Confusion(img_copy, img, 0, img_copy.size().height, 0, img_copy.size().width, img_copy.size(), 0x1234);
    // imshow("confusion", img_copy);
    // InvertConfusion(img, img_copy, 0, img.size().height, 0, img.size().width, img.size(), 0x1234);
    // imshow("invert_confusion", img);
    // cv::waitKey(0);
    // return 0;
    ///
    chdir(homePath);

    int times = 100;
    double tmp;
    while (times--) {
        if ((tmp = GenDoubleFloatFrom1To0Random()) > 1 || tmp < 0) {
            std::cout << tmp << std::endl;
        }
    }
    std::cout << getcwd(nullptr, 0) << std::endl;
    auto img = imread("inputs/1.jpeg", cv::IMREAD_UNCHANGED);
    if (img.data == nullptr) {
        std::cout << "Read image failed." << std::endl;
        return 1;
    }
    imshow("original", img);
    cv::waitKey(0);
    ThreadPool sp(10);
    Keys k = RANDOM_KEYS;
    auto encrypted = img.clone();
    EncryptoImage(encrypted, {512, 512}, k, DEFAULT_CONFIG, sp);
    imshow("encrypted", encrypted);
    auto decrypted = encrypted.clone();
    DecryptoImage(decrypted, {512, 512}, k, DEFAULT_CONFIG, sp);
    imshow("decrypted", decrypted);
    // auto img_copy = img.clone();
    // Confusion(img_copy, img, 0, img_copy.size().height, 0, img_copy.size().width, img_copy.size(), 0x1234);
    // imshow("confusion", img_copy);
    // InvertConfusion(img, img_copy, 0, img.size().height, 0, img.size().width, img.size(), 0x1234);
    // imshow("inverted", img);
    //
    cv::waitKey(0);
    sp.reduceTo(0);
    sp.waitReduce();
    return 0;
}
