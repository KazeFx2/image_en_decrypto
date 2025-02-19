//
// Created by Fx Kaze on 25-1-16.
//

#include "include/types.h"

// AniFrameCounter

AniFrameCounter::AniFrameCounter(AniFPS &parentFPS, const AniTime animateDuration): frameCounter(0),
    animateDuration(animateDuration) {
    fps = &parentFPS;
    prevFps = *fps;
    calculateFrames();
}

AniProcedure AniFrameCounter::next() {
    update();
    return static_cast<AniProcedure>(frameCounter) / frameTotal;
}

bool AniFrameCounter::isFinished() const {
    return frameCounter >= frameTotal;
}

void AniFrameCounter::update() {
    if (!ANI_FLOAT_EQ(*fps, prevFps)) {
        calculateFrames();
        prevFps = *fps;
    }
    frameCounter++;
}

void AniFrameCounter::calculateFrames() {
    frameCounter = frameCounter / prevFps * *fps;
    frameTotal = animateDuration * *fps / ANI_TIME_SEC(1);
}


// EndAniFrameCounter
