/**
 * @file Main.cpp
 * @brief Main application entry point
 */

#include "Global.h"
#include "PhotoBrowser.h"

// The main application entry point
// NOTE: Win32 uses a different entry point signature when using a SUBSYSTEM:WINDOWS configuration
#ifdef WIN32
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
#else // All other platforms
int main(int argc, const char* argv[])
#endif // WIN32
{
	// Window specific initialization
#ifdef WIN32

	// Enable memory leak tracking and reporting on shutdown
	#ifdef DEBUG
		_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetReportMode ( _CRT_ERROR, _CRTDBG_MODE_DEBUG);
	#endif // DEBUG

	// If the debugger is running us from the project directory rather than the binaries directory
	// change the current working directory to binaries
	_chdir("Binaries");

#endif // WIN32

	// Initialize the photo browser instance
	PhotoBrowser* l_PhotoBrowser = PhotoBrowser::Instance();
	if(!l_PhotoBrowser->Startup())
	{
#ifdef WIN32
		MessageBox(NULL, "Application startup failure", "Fatal Error", MB_OK | MB_ICONERROR);
#endif
		return -1;
	}

	// Main application loop
	l_PhotoBrowser->MainLoop();

	// Run the shutdown routine
	l_PhotoBrowser->Shutdown();

	return 0;
}
