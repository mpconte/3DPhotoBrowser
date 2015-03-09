/**
 * @file Debug.cpp
 * @brief Debug implementation file
 */

#include "Debug.h"

//-----------------------------------------------------------------------------------------------------------------------------
// Debug

void Debug::Logf(const char* in_Fmt, ...)
{
	static const int l_BuffSize = 100;
	static char l_Buff[l_BuffSize+2];

	// Get the variadic arguments
	va_list l_Args;
	va_start(l_Args, in_Fmt);

	// The way the function is used here is perfectly safe
#ifdef WIN32
	#pragma warning (push)
	#pragma warning (disable: 4996)
#endif // WIN32
	int l_LineLength = vsnprintf(l_Buff, l_BuffSize, in_Fmt, l_Args);
#ifdef WIN32
	#pragma warning (pop)
#endif // WIN32

	// Terminate the line
	l_Buff[l_LineLength+0] = '\n';
	l_Buff[l_LineLength+1] = '\0';

	OutputDebugStringA(l_Buff);	// Output to the windows debug console
	cout << l_Buff;				// Output to the standard output stream
}

