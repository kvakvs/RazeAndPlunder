#pragma once

#include <windows.h>	// for Windows APIs

class Timer
{
private:
	LARGE_INTEGER lFreq, lStart;
	LONGLONG elapsedTime;

public:
	Timer()
		: elapsedTime(0)
	{
		QueryPerformanceFrequency(&lFreq);
	}

	inline void start()
	{
		QueryPerformanceCounter(&lStart);
		elapsedTime = 0;
	}

	inline void stop()
	{
		LARGE_INTEGER lEnd;
		QueryPerformanceCounter(&lEnd);
		elapsedTime += lEnd.QuadPart - lStart.QuadPart;
	}

	// Return duration in seconds
	inline double getElapsedTime()
	{
		return (double(elapsedTime) / lFreq.QuadPart);
	}

	// Return duration in seconds
	inline double stopAndGetTime()
	{
		stop();
		return getElapsedTime();
	}
};
