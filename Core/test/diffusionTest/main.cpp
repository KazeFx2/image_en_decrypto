//
// Created by Fx Kaze on 25-1-4.
//
#include <private/ImageEncrypto.h>

#include "private/Util.h"
#include "Bitmap.h"
#include "../vars.h"
#include "private/Random.h"

typedef struct {
    u32 r;
    u32 c;
} Item;

int main(int argc, const char *argv[]) {
    // const u32 H = 50;
    chdir(homePath);
    auto img = imread("inputs/test.png", cv::IMREAD_UNCHANGED);
    if (img.data == nullptr) {
        std::cout << "Read image failed." << std::endl;
        return 1;
    }
    // resize(img, img, cv::Size(H, H));
    auto s = img.size();
    auto en = img.clone();
    Keys k = RANDOM_KEYS;
    ParamControl conf = DEFAULT_CONFIG;
    conf.nThread = 1;
    cv::Size size = img.size();
    threadParams params = {
        nullptr,
        nullptr,
        &size,
        0,
        (u32) 3 * s.width * s.height,
        k,
        &conf,
    };
    u32 rowStart, rowEnd, colStart, colEnd;
    u8 *byteSeq = new u8[params.iterations * params.config->byteReserve];
    u8 *diffusionSeedArray = new u8[3 * params.config->diffusionConfusionIterations];
    PreAssist(rowStart, rowEnd, colStart, colEnd, params, byteSeq, diffusionSeedArray);

    u32 seqIdx = 0;
    Diffusion(en, img,
              rowStart, rowEnd, colStart, colEnd,
              diffusionSeedArray, byteSeq, seqIdx, 3);
    imwrite("outputs/test_en_di.jpeg", en);
    auto de = en.clone();
    InvertDiffusion(de, en,
                    rowStart, rowEnd, colStart, colEnd,
                    diffusionSeedArray, byteSeq, seqIdx, 3);
    imwrite("outputs/test_de_di.jpeg", de);
    // FILE *fd = fopen("outputs/test_dump_mat_ori.txt", "w+");
    // DumpMat(fd, "None", img);
    // fclose(fd);
    // fd = fopen("outputs/test_dump_mat_dec.txt", "w+");
    // DumpMat(fd, "None", de);
    // fclose(fd);
    return 0;
}
