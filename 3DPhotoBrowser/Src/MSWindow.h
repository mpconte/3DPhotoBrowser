/**
 * @file MSWindow.h
 * @brief Microsoft window class header file
 */

#ifndef MSWINDOW_H_
#define MSWINDOW_H_

#include "Window.h"
#include "UserPreferences.h"

/**
 * MSWindow
 * The Microsoft window implementation. This is a Win32 dependent class
 */
class MSWindow 
: public Window
, public UserPreferenceListener
{
public:

	/**
	 * Constructors
	 */
	MSWindow();

	/**
	 * Window interface
	 */
	virtual bool Init(const string& in_Title, int in_SizeX, int in_SizeY);
	virtual void SetTitle(const string& in_Title);
	virtual	bool Destroy();
	virtual void ProcessMessages();
	virtual void SwapBuffers();
	virtual void EnableVerticalSync(bool in_Enable);
	virtual void TranslateKey(unsigned& in_Key);
	virtual void ShowUserPreferencesDialog(bool in_Show);
	virtual bool IsUserPreferencesDialogVisible();
	virtual GLContext CreateGLContext();
	virtual bool ShareGLContexts(GLContext in_Context1, GLContext in_Context2);
	virtual bool AcquireGLContext(GLContext in_Context);
	virtual bool ReleaseGLContext(GLContext in_Context);

	/**
	 * UserPreferenceListener interface
	 */
	virtual void OnUserPreferenceUpdate();

protected:

	/**
	 * Helpers
	 */
	bool CreateDeviceContext();
	bool CreateUsePreferenceDialog();

	static bool RegisterWindowClass();
	static LRESULT CALLBACK WindowMessageRedirector(HWND in_Hwnd, UINT in_Message, WPARAM in_wParam, LPARAM in_lParam);
	static BOOL CALLBACK DialogProc(HWND in_Hwnd, UINT in_Message, WPARAM in_wParam, LPARAM in_lParam);

private:

	static const int MAX_SLIDER_VALUE = 1000;

	static map<HWND, MSWindow*> sWindowMapping;			// Allows the WNDPROC callback to retrieve MSWindow instance based on HWND parameter
	static map<HWND, PreferenceData*> sControlMapping;	// Allows the dialog callback to retrieve PreferenceData based on control HWND parmeters
	
	static bool sWindowClassRegistered;			// Used to statically initialize the window class only once
	static const char* sWindowClassName;		// The window class name

	HWND mWindowHandle;		// The main application window handle
	HWND mDialogHandle;		// The user preferences dialog handle
	HDC mDeviceContext;		// The window device context
};

#endif // MSWINDOW_H_
