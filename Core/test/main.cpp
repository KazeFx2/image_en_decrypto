//
// Created by Fx Kaze on 24-12-30.
//

#include "unistd.h"
#include "private/vars.h"
#include "ThreadPool.h"
#include "ImageCrypto.h"
#include <sys/time.h>

double GetCPUSecond() {
    timeval tp;
    gettimeofday(&tp, nullptr);
    return (static_cast<double>(tp.tv_sec) + static_cast<double>(tp.tv_usec) * 1.e-6);
}

int main(int argc, const char **argv) {
    chdir(homePath);

    ThreadPool pool(16);
    const u32 W = 499;
    cv::Size size{W, W};

    Keys keys = {
        0x134,
        {0.789, 0.114},
        {0.962, 0.415}
    };

    ImageCrypto crypter(pool, size, DEFAULT_CONFIG, keys);

    std::cout << getcwd(nullptr, 0) << std::endl;
    auto img = imread("inputs/test.png", cv::IMREAD_UNCHANGED);
    if (img.data == nullptr) {
        std::cout << "Read image failed." << std::endl;
        return 1;
    }

    auto encrypted = crypter.encrypt(img);
    auto decrypted = crypter.decrypt(encrypted);
    imshow("original", img);
    imshow("encrypted", encrypted);
    imshow("decrypted", decrypted);
    cv::waitKey(0);
    img = imread("inputs/3.png", cv::IMREAD_UNCHANGED);
    if (img.data == nullptr) {
        std::cout << "Read image failed." << std::endl;
        return 1;
    }

    encrypted = crypter.encrypt(img);
    decrypted = crypter.decrypt(encrypted);
    imshow("original", img);
    imshow("encrypted", encrypted);
    imshow("decrypted", decrypted);
    cv::waitKey(0);
    // return 0;

    cv::VideoCapture cap(argv[1]);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, W);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, W);
    double fps = cap.get(cv::CAP_PROP_FPS);
    while (true) {
        double start = GetCPUSecond();
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            break;
        }
        resize(frame, frame, cv::Size(960, 540));
        auto encry = crypter.encrypt(frame);
        auto decry = crypter.decrypt(encry);
        imshow("original", frame);
        imshow("encrypted", encry);
        imshow("decrypted", decry);
        double end = GetCPUSecond();

        printf("time for encrypting and decrypting the frame: %.3f (ms)\n",
               (end - start) * 1000);

        auto delay = static_cast<int>(1000.0 / fps - (end - start) * 1000);
        // cv::waitKey(0);
        if (delay > 1)
            cv::waitKey(delay);
        else
            cv::waitKey(1);
    }
    return 0;
}
