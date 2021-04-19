#ifndef INCLUDED_THREAD_H
#define INCLUDED_THREAD_H

#define MEAN_AND_LEAN
#include <Windows.h>


class Thread
{
	DWORD id;
	HANDLE handle;
    bool finished;

	static DWORD WINAPI threadFunc(LPVOID lpThreadParameter)
	{
		return ((Thread*)lpThreadParameter)->run0();
	}

    int run0()
    {
        int rv = run();
        finished = true;
        return rv;
    }

public:
	Thread(): finished(false)
	{
		handle = CreateThread(0,0, &Thread::threadFunc, (LPVOID)this, 0, &id);
	}

	virtual ~Thread()
	{
		CloseHandle(handle);
	}

	virtual int run() = 0;

    virtual void interrupt() = 0;

	void join()
	{
		WaitForSingleObject(handle, INFINITE);
	}

    bool isFinished()
    {
        return finished;
    }
};


#endif