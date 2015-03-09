/**
 * @file Semaphore.h
 * @brief Semaphore class header file
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include "Global.h"

/**
 * Semaphore
 * Simple synchronization primitive
 */
class Semaphore
{
public:

	Semaphore();
	~Semaphore();
	
	/**
	 * Lock
	 * Lock this semaphore
	 */
	void Lock();

	/**
	 * Unlock
	 * Unlock this semaphore
	 */
	void Unlock();

private:

#ifdef WIN32
	CRITICAL_SECTION
#else
	#error Your platform lock primitive goes here
#endif // WIN32
	mLockPrimitive;
};

#endif // SEMAPHORE_H_
