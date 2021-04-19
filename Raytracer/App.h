#ifndef INCLUDED_APP_H
#define INCLUDED_APP_H

#include "Linear.h"

typedef GAL::P2i Point;

struct Modifiers
{
    bool controlKey;
    bool shiftKey;
    bool altKey;
};

struct MotionEvent
{
    Point position;
    Point movement;
};

struct ButtonEvent
{
    Point       position;
    int         button;
    Modifiers   modifiers;
};

class App
{
public:
    virtual ~App() {}

    virtual void beforeWindowShown() = 0;
    
    virtual void willQuit() = 0;

    virtual void windowSizeChanged(int width, int height) = 0;

    virtual void mouseMotion(const MotionEvent &evt) = 0;

    virtual void buttonDown(const ButtonEvent &ev) = 0;

    virtual void buttonUp(const ButtonEvent &ev) = 0;

    virtual void keyDown(int keyCode) = 0;

    virtual void keyUp(int keyCode) = 0;

    virtual void processTick() = 0;
};


#endif
