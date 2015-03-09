/**
 * @file Timer.h
 * @brief Simple performance timer singleton
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "Global.h"

/**
 * Timer
 * Class used to retrieve the application time
 */
class Timer
{
public:
	
	/**
	 * Instance
	 * Get the timer instnace
	 */
	static const Timer* Instance() { static Timer l_Instance; return &l_Instance; }

	/**
	 * GetSeconds
	 * Returns the current application time in seconds
	 */
	double GetSeconds() const;

private:

	/**
	 * Singleton implementation
	 */
	Timer();
	Timer(const Timer&);
	const Timer& operator=(const Timer&);

#ifdef WIN32
	__int64
#else
	#error Your platform 64 bit integer type goes here
#endif // WIN32
	mTimerFrequency;
};

#endif // TIMER_H_
