#ifndef GLOBAL_H_
#define GLOBAL_H_

// Defines
#define PHOTOBROWSER_VERSION 1.0
#define PHOTOBROWSER_REVISION 169

#define NULL		0
#define DEG_TO_RAD	0.017453292519943295769
#define RAD_TO_DEG	57.295779513082320876798

#define USE_THREADED_TEXTURE_LOADING 0

// Platform Dependent Header Files
#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <tchar.h>
	#include <direct.h>
	#include <commctrl.h>
	#include "../resource.h"
#else
	// Other platform
#endif // WIN32

// C++ Standard Header Files
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <stack>
#include <map>
using namespace std;

// Application Header Files
#include "Util.h"
#include "Debug.h"
#include "Timer.h"
#include "Graphics.h"
#include "UserPreferences.h"

// Macros
#define PROFILE_BLOCK(executionTime, code) \
{ \
	double l_Time = Timer::Instance()->GetSeconds(); \
	code \
	extern double executionTime; \
	executionTime = Timer::Instance()->GetSeconds() - l_Time; \
}

#endif // GLOBAL_H_
