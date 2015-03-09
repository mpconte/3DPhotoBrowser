/**
 * @file Debug.h
 * @brief Debug header file
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include "Global.h"

/**
 * Debug
 * Class used for debugging
 */
class Debug
{
public:

	/**
	 * Instance
	 * Singleton access
	 */
	static Debug* Instance() { static Debug l_Instance; return &l_Instance; }

	/**
	 * Logf
	 * Formatted output logging
	 */
	void Logf(const char* in_Fmt, ...);

private:

	/**
	 * Singleton implementation
	 */
	Debug() {}
	Debug(const Debug&);
	const Debug& operator=(Debug&);
};

/**
 * Logf
 * Convenience macro
 */
#ifdef DEBUG
	#define logf(...) Debug::Instance()->Logf(__VA_ARGS__);
#else
	#define logf(...)
#endif // DEBUG

#endif // DEBUG_H_
