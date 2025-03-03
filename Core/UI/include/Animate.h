//
// Created by Fx Kaze on 25-1-16.
//

#ifndef ANI_ANIMATE_H
#define ANI_ANIMATE_H

#include "Widget.h"

/// for an animation, we have those attributes
/// how to play (from where to where, from size to size), how long
///
/// for the animation sys, when/who/how to tell tat the animation ends
/// when playing it?, who invoke `play` ? the owner widget? YES
/// widget `display`s itself and also check if there is a animation, animation need to tell
/// the owner if it ends, if not, the fresh will continue

/// how the animate work?
class Animate {
public:
    typedef enum {
        ANI_DIRECTION_FORWARD = 0x0,
        ANI_DIRECTION_BACKWARD = 0x1,
    } AniDirectionType;

private:
    // member variables
    Widget *owner;
    AniFrameCounter animeFrameCounter;
    AniDirectionType direction;

    // member functions
    /// @param procedure (0.0f, 1.0f] the procedure of animation
    __OVERWRITE
    virtual void play(AniProcedure procedure);

    /// @return bool - returns `true` if this frame is the
    /// last one, otherwise `false` will be returned
    bool _play() {
        play(animeFrameCounter.next());
        return animeFrameCounter.isFinished();
    }
};

#endif //ANI_ANIMATE_H
