//
// Created by Fx Kaze on 25-1-3.
//

#ifndef BITMAP_H
#define BITMAP_H

#include <vector>
#include "types.h"

#define BITMAP_NOT_FOUND (~static_cast<count_t>(0x0))

class FastBitmap {
public:
    class element {
    public:
        element(uptr &data, u8 offset);

        element(const element &other) = delete;

        element(element &&other);

        element &operator=(const element &other) = delete;

        element &operator=(element &&other);

        ~element();

        operator bool() const;

        bool operator=(bool val) const;

    private:
        uptr *data;
        u8 offset;
    };

    using u_size_t = u64;

    FastBitmap(u_size_t initBits = 128, bool defaultV = false);

    FastBitmap(const FastBitmap &other);

    FastBitmap(FastBitmap &&other);

    FastBitmap &operator=(const FastBitmap &other);

    FastBitmap &operator=(FastBitmap &&other);

    ~FastBitmap();

    element operator[](u_size_t index);

    count_t findNext(count_t start = 0, count_t end = ~static_cast<count_t>(0x0)) const;

    count_t findNextTrue(count_t start = 0, count_t end = ~static_cast<count_t>(0x0)) const;

    count_t findNextFalse(count_t start = 0, count_t end = ~static_cast<count_t>(0x0)) const;

private:
    bool defaultValue;
    u_size_t max;
    std::vector<uptr> bitmap;

    count_t _findNext(bool value = true, count_t start = 0, count_t end = ~static_cast<count_t>(0x0)) const;
};

#endif //BITMAP_H
