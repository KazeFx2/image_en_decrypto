//
// Created by Fx Kaze on 25-1-16.
//

#include "include/Painter/SketchPad.h"

bool SketchPadWindow::initialized = false;

Mutex SketchPadWindow::mutex;

void SketchPadWindow::_init() {
    mutex.lock();
    if (!initialized) {
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            
        }
    }
    mutex.unlock();
}

