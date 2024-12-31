//
// Created by Fx Kaze on 24-12-30.
//

#include "private/ImageEncrypto.h"

using namespace cv;

typedef struct {
    __IN Mat &img;
    __OUT Mat &dst;
    __IN Size &size;
    __IN size_t nThreads;
    __IN size_t threadId;
    __IN Keys &keys;
}encryptoParams;

void EncryptoImage(__IN_OUT Mat &Image,__IN const std::string &QuantumBitsFile,__IN const ImageSize &Size) {

}


void EncryptoImage(__IN_OUT Mat &Image, __IN const Keys &Keys, __IN const ImageSize &Size) {
    // resize image if needed
    if (Size.width != 0 && Size.height != 0) {
        resize(Image, Image, ::Size(Size.width, Size.height));
    }
}

void *encryptoAssistant(__IN void *param) {
    encryptoParams *params = reinterpret_cast<encryptoParams *>(param);

}
