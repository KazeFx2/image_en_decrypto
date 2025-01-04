//
// Created by Fx Kaze on 24-12-30.
//

#include "unistd.h"
#include "vars.h"
#include "ThreadPool.h"
#include "ImageCrypto.h"


int main(int argc, const char **argv) {
    chdir(homePath);

    ThreadPool pool(64);
    ImageCrypto crypter(&pool);

    std::cout << getcwd(nullptr, 0) << std::endl;
    auto img = imread("inputs/1.jpeg", cv::IMREAD_UNCHANGED);
    if (img.data == nullptr) {
        std::cout << "Read image failed." << std::endl;
        return 1;
    }

    const u32 W = 512;
    ImageSize size{W, W};

    auto encrypted = crypter.encrypt(img, size);
    auto decrypted = crypter.decrypt(encrypted, size);
    imshow("original", img);
    imshow("encrypted", encrypted);
    imshow("decrypted", decrypted);
    cv::waitKey(0);
    return 0;
}
