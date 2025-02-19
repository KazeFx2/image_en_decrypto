//
// Created by Fx Kaze on 25-1-11.
//

#include <iostream>
#include <ostream>

#include "Bitmap.h"

int main() {
        FastBitmap bitmap(128, true);
        bitmap[1999] = true;
        bitmap[1998] = false;
        std::cout << bitmap.findNextFalse(1999) << std::endl;
        return 0;
}
