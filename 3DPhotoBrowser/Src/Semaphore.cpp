/**
 * @file Semaphore.cpp
 * @brief Semaphore implementation file
 */

#include "Semaphore.h"

//-----------------------------------------------------------------------------------------------------------------------------
// Semaphore

Semaphore::Semaphore()
{
#ifdef WIN32
	InitializeCriticalSection(&mLockPrimitive);
#else
	#error Your platform goes here
#endif // WIN32
}

//-----------------------------------------------------------------------------------------------------------------------------

Semaphore::~Semaphore()
{
#ifdef WIN32
	DeleteCriticalSection(&mLockPrimitive);
#else
	#error Your platform goes here
#endif // WIN32
}

//-----------------------------------------------------------------------------------------------------------------------------

void Semaphore::Lock()
{
#ifdef WIN32
	EnterCriticalSection(&mLockPrimitive);
#else
	#error Your platform goes here
#endif // WIN32
}

//-----------------------------------------------------------------------------------------------------------------------------

void Semaphore::Unlock()
{
#ifdef WIN32
	LeaveCriticalSection(&mLockPrimitive);
#else
	#error Your platform goes here
#endif // WIN32
}

