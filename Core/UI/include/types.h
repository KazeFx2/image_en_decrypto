//
// Created by Fx Kaze on 25-1-16.
//

#ifndef ANI_TYPES_H
#define ANI_TYPES_H

#include "private/includes.h"

typedef f32 AniFPS;
typedef f32 AniProcedure;

typedef u64 AniTime;
#define ANI_TIME_MS(dur) (dur)
#define ANI_TIME_SEC(dur) (ANI_TIME_MS(dur) * 1000)
#define ANI_TIME_MINUTES(dur) (ANI_TIME_SEC(dur) * 60)

#define ANI_FLOAT_EPSILON 1e-5f
#define ANI_FLOAT_EQ(a, b) (abs((a) - (b)) < ANI_FLOAT_EPSILON)

class AniFrameCounter {
public:
    AniFrameCounter(AniFPS &parentFPS, AniTime animateDuration);

    AniProcedure next();

    bool isFinished() const;

private:
    AniFPS *fps;
    AniFPS prevFps;
    u32 frameCounter;
    u32 frameTotal;
    AniTime animateDuration;

    void update();

    void calculateFrames();
};

#endif //ANI_TYPES_H
