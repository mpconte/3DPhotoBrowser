/**
 * @file Thread.h
 * @brief Thread class header file
 */

#ifndef THREAD_H_
#define THREAD_H_

#include "Global.h"

/**
 * Thread
 * Interface for executing threads
 */
class Thread
{
public:

	virtual ~Thread() {}

	/**
	 * Start
	 * Start executing this thread
	 */
	void Start(bool in_StartSuspended);

	/**
	 * Stop
	 * Stop executing this thread
	 */
	void Stop();

	/**
	 * Suspend
	 * Suspend execution
	 */
	void Suspend();

	/**
	 * Resume
	 * Resume a previously suspended thread
	 */
	void Resume();

	/**
	 * Run
	 * The thread routine to execute
	 */
	virtual void Run() = 0;

protected:

	// Win32 implementation details
#ifdef WIN32
	HANDLE mThreadHandle;
#endif // WIN32
};

#endif // THREAD_H_
