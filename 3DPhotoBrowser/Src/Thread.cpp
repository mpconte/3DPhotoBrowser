/**
 * @file Thread.cpp
 * @brief Thread implementation file
 */

#include "Thread.h"

// Windows specific implementation details
#ifdef WIN32
static DWORD WINAPI ThreadProc(LPVOID in_Param)
{
	Thread* l_Thread = (Thread*)in_Param;
	l_Thread->Run();
	return 0;
}
#endif // WIN32

//-----------------------------------------------------------------------------------------------------------------------------
// Thread

void Thread::Start(bool in_StartSuspended)
{
#ifdef WIN32
	DWORD l_ThreadId;
	mThreadHandle = CreateThread( NULL, NULL, ThreadProc, (void*)this, in_StartSuspended ? CREATE_SUSPENDED : NULL, &l_ThreadId);
#else
	#error Your platform thread creation goes here
#endif // WIN32
}

//-----------------------------------------------------------------------------------------------------------------------------

void Thread::Stop()
{
#ifdef WIN32
	TerminateThread(mThreadHandle, 0);
#else
	#error Your platform thread stop goes here
#endif // WIN32
}

//-----------------------------------------------------------------------------------------------------------------------------

void Thread::Suspend()
{
#ifdef WIN32
	SuspendThread(mThreadHandle);
#else
	#error Your platform thread suspend goes here
#endif // WIN32
}

//-----------------------------------------------------------------------------------------------------------------------------

void Thread::Resume()
{
#ifdef WIN32
	ResumeThread(mThreadHandle);
#else
	#error Your platform thread resume goes here
#endif // WIN32
}
