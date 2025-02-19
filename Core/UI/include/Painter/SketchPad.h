//
// Created by Fx Kaze on 25-1-16.
//

#ifndef ANI_SKETCHPAD_H
#define ANI_SKETCHPAD_H

#include "../includes.h"
#include "Mutex.h"
#include "SDL2/SDL.h"

class SketchPad {
public:
  typedef enum {
    ANI_SKETCHPAD_TYPE_WINDOW = 0x0,
  }SketchPadType;
    void flush();
private:

};

class SketchPadWindow : public SketchPad {
public:
    SketchPadWindow();
private:
  void _init();

  static bool initialized;
  static Mutex mutex;
};

#endif //ANI_SKETCHPAD_H
