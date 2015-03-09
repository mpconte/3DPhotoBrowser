/**
 * @file Timer.cpp
 * @brief Win32 Timer implementation
 */

#include "Timer.h"

//-----------------------------------------------------------------------------------------------------------------------------
// Timer

Timer::Timer()
{
	// Get the system clock frequency
#ifdef WIN32
	QueryPerformanceFrequency((LARGE_INTEGER*)&mTimerFrequency);
#else
	#error Your platform goes here
#endif // WIN32
}

//-----------------------------------------------------------------------------------------------------------------------------

double Timer::GetSeconds() const 
{
	__int64 l_Counter;

#ifdef WIN32
	QueryPerformanceCounter((LARGE_INTEGER*)&l_Counter);
#else
	#error Your platform goes here
#endif // WIN32

	return (double)l_Counter / (double)mTimerFrequency;
}
