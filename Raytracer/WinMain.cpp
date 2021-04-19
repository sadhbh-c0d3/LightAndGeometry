#define MEAN_AND_LEAN
#include <windows.h>
#include <tchar.h>
#include <exception>
#include <sstream>
#include "Console.h"
#include "App.h"
#include "Raytracer.h"

class WinApp
{
public:
    WinApp(App &app, HINSTANCE hInstance, LPCSTR appName, int width, int height):
        mApp(app),
        mHInstance(hInstance),
        mWndClassName(appName),
        mWndName(appName),
        mWidth(width),
        mHeight(height)
    {
    }

    ~WinApp()
    {
    }

    void run()
    {
        registerWindowClass();
        createWindow();
        createOpenGLContext();
        messageLoop();
    }

private:
    App      &mApp;
    HINSTANCE mHInstance;
    LPCSTR    mWndClassName;
    LPCSTR    mWndName;
    HWND      mHWnd;
    HDC       mHDC;
    HGLRC     mHGLRC;
    int       mWidth;
    int       mHeight;
    int       mLastX;
    int       mLastY;

    void registerWindowClass()
    {
        WNDCLASSEX ex;
        ex.cbSize = sizeof(WNDCLASSEX);
        ex.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
        ex.lpfnWndProc = WinProc;
        ex.cbClsExtra = 0;
        ex.cbWndExtra = sizeof(LONG_PTR);// space for this ptr
        ex.hInstance = mHInstance;
        ex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        ex.hCursor = LoadCursor(NULL, IDC_ARROW);
        ex.hbrBackground = NULL;
        ex.lpszMenuName = NULL;
        ex.lpszClassName = mWndClassName;
        ex.hIconSm = NULL;

        if (!RegisterClassEx(&ex)) {
            throw std::exception("Cannot register window class!");
        }
    }

    void createWindow()
    {
        // center position of the window
        int posx = (GetSystemMetrics(SM_CXSCREEN) / 2) - (mWidth / 2);
        int posy = (GetSystemMetrics(SM_CYSCREEN) / 2) - (mHeight / 2);

        // set up the window for a windowed application
        long wndStyle = WS_OVERLAPPEDWINDOW;

        // create the window
        mHWnd = CreateWindowEx(NULL,
                mWndClassName,
                mWndName,
                wndStyle|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
                posx, posy,
                mWidth, mHeight,
                NULL,
                NULL,
                mHInstance,
                NULL);

        // at this point WM_CREATE message is sent/received
        // the WM_CREATE branch inside WinProc function will execute here

        if (0 == mHWnd) {
            throw std::exception("Cannot create window!");
        }

        SetWindowLongPtr(mHWnd, 0, (LONG_PTR)this);

        if (NULL == (mHDC = GetDC(mHWnd)))
        {
            throw std::exception("Window has no device context!");
        }
    }

    void createOpenGLContext()
    {
        setPixelFormat(24);
        if (NULL == (mHGLRC = wglCreateContext(mHDC)))
        {
            throw std::exception("Cannot create OpenGL context!");
        }
    }
   
    void setPixelFormat(int bitsPerPel)
    {
        PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
            PFD_TYPE_RGBA,
            bitsPerPel,
            0,0,0,0,0,0,0,0,0,0,0,0,0, // useles parameters
            16,
            0,0,PFD_MAIN_PLANE,0,0,0,0
        };

        int indexPixelFormat = ChoosePixelFormat(mHDC, &pfd);

        // Choose the closest pixel format available
        if (0 == indexPixelFormat)
        {
            throw std::exception("Cannot find suitable pixel format!");
        }

        // Set the pixel format for the provided window DC
        if (!SetPixelFormat(mHDC, indexPixelFormat, &pfd))
        {
            throw std::exception("Cannot set pixel format!");
        }
    }

    void messageLoop()
    {
        MSG msg;
        bool quit = false;

        mApp.beforeWindowShown();

        ShowWindow(mHWnd, SW_NORMAL);

        while(!quit)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (WM_QUIT == msg.message)
                {
                    quit = true;
                    willQuit();
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else {
                processTick();
            }
        }
    }

    void processTick()
    {
        BOOL ok = wglMakeCurrent(mHDC, mHGLRC);

        if (TRUE == ok)
        {
            mApp.processTick();

            SwapBuffers(mHDC);

            Sleep(100);
        }
    }

    void willQuit()
    {
        BOOL ok = wglMakeCurrent(mHDC, mHGLRC);
        mApp.willQuit();
        wglMakeCurrent(mHDC, NULL);
    }


    void windowSizeChanged(int width, int height)
    {
        BOOL ok = wglMakeCurrent(mHDC, mHGLRC);

        mWidth  = width;
        mHeight = height;
        mApp.windowSizeChanged(width, height);
    }

    void mouseMotion(int x, int y)
    {
        MotionEvent evt;
        evt.position = Point(x,y);
        evt.movement = Point(x - mLastX, y - mLastY);
        mLastX = x;
        mLastY = y;
        mApp.mouseMotion(evt);
    }

    void setModifiers(Modifiers &m, int modifiers)
    {
        m.controlKey = MK_CONTROL == (MK_CONTROL & modifiers);
        m.shiftKey = MK_SHIFT == (MK_SHIFT & modifiers);
        m.altKey = MK_ALT == (MK_ALT & modifiers);
    }

    void buttonDown(int buttonNo, int modifiers)
    {
        ButtonEvent evt;
        evt.button = buttonNo;
        evt.position = Point(mLastX, mLastY);
        setModifiers(evt.modifiers, modifiers);
        mApp.buttonDown(evt);
    }

    void buttonUp(int buttonNo, int modifiers)
    {
        ButtonEvent evt;
        evt.button = buttonNo;
        evt.position = Point(mLastX, mLastY);
        setModifiers(evt.modifiers, modifiers);
        mApp.buttonUp(evt);
    }

    void keyDown(int keyCode)
    {
        mApp.keyDown(keyCode);
    }

    void keyUp(int keyCode)
    {
        mApp.keyUp(keyCode);
    }

    static WinApp &ThisFromWindow(HWND hWnd)
    {
        LONG_PTR longPtr = GetWindowLongPtr(hWnd, 0);
        WinApp *app = (WinApp *)longPtr;
        return *app;
    }

    static LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        int buttonNo = 0;

        switch(msg)
        {
        case WM_CREATE:
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_SIZE:
            ThisFromWindow(hWnd).windowSizeChanged(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_MOUSEMOVE:
            ThisFromWindow(hWnd).mouseMotion(LOWORD(lParam),HIWORD(lParam));
            break;

        case WM_MBUTTONDOWN: ++buttonNo;
        case WM_RBUTTONDOWN: ++buttonNo;
        case WM_LBUTTONDOWN:
            ThisFromWindow(hWnd).buttonDown(buttonNo, wParam);
            break;

        case WM_MBUTTONUP: ++buttonNo;
        case WM_RBUTTONUP: ++buttonNo;
        case WM_LBUTTONUP:
            ThisFromWindow(hWnd).buttonDown(buttonNo, wParam);
            break;

        case WM_KEYDOWN:
            ThisFromWindow(hWnd).keyDown(wParam);
            break;

        case WM_KEYUP:
            ThisFromWindow(hWnd).keyUp(wParam);
            break;
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
};

int WINAPI WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nShowCmd)
{
    Raytracer app;
    WinApp winApp(app, hInstance, _T("Raytracer"), 1280, 720);

    try {
        winApp.run();

        Console::writeln("Bye.");
    }
    catch(std::exception exc)
    {
        Console::writeln(exc.what());
    }

    return 0;
}
