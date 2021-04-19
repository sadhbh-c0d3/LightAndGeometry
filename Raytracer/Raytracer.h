#ifndef INCLUDED_RAYTRACER_H
#define INCLUDED_RAYTRACER_H

#define MEAN_AND_LEAN
#include <Windows.h>
#include <gl/GL.h>

#include "Linear.h"
#include "App.h"
#include "SceneGraph.h"
#include "Light.h"
#include "Camera.h"
#include "Thread.h"


class Raytracer : public App
{
public:
    Raytracer();

    ~Raytracer();

    void beforeWindowShown();

    void windowSizeChanged(int width, int height);
    
    void mouseMotion(const MotionEvent &evt);

    void buttonDown(const ButtonEvent &evt);

    void buttonUp(const ButtonEvent &evt);

    void keyDown(int keyCode);

    void keyUp(int keyCode);

    void willQuit();

    void processTick();

private:
    SceneGraph3d scene;
    Camera3d camera;
    TargetBuffer<PixelRGBA32> targetBuffer;

    GLuint texture;
    int windowWidth;
    int windowHeight;

    typedef std::shared_ptr<Thread> ThreadPtr;
    typedef std::list<ThreadPtr> ThreadListType;
    ThreadListType threads;

    bool needRedraw;

    void startRaytrace();
    bool allThreadsFinished();
    void waitAllThreads();
    void interruptAllThreads();

    void prepareTargetBuffer(int width, int height);
};


#endif
