//
// Created by Fx Kaze on 25-1-3.
//

#ifndef BITMAP_H
#define BITMAP_H

#include <vector>
#include "private/types.h"

class FastBitmap {
public:
    class element {
    public:
        element(uptr &data, const u8 offset) : data(&data), offset(offset) {
        };

        element(const element &other) = delete;

        ~element() {
        };

        operator bool() const {
            return static_cast<bool>(*data & static_cast<uptr>(0x1) << offset);
        }

        bool operator=(const bool val) const {
            if (val) {
                *data |= static_cast<uptr>(0x1) << offset;
            } else {
                *data &= ~(static_cast<uptr>(0x1) << offset);
            }
            return val;
        }

    private:
        uptr *data;
        u8 offset;
    };

    using u_size_t = u64;

    FastBitmap(u_size_t initBits = 128);

    ~FastBitmap();

    element operator[](u_size_t index);

private:
    u_size_t max;
    std::vector<uptr> bitmap;
};

#endif //BITMAP_H
