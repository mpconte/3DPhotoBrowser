/**
 * @file Window.h
 * @brief Window interface header file
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include "Global.h"

/**
 * WindowListener
 * Implemented by objects to recieve window events
 */
class WindowListener
{
public:

	virtual void OnResize(int in_SizeX, int in_SizeY) = 0;
	virtual void OnFocus(bool in_Focus) = 0;
	virtual void OnClose() = 0;
};

/**
 * WindowListener
 * Implemented by objects to recieve mouse events
 */
class MouseListener
{
public:

	enum MouseButton
	{
		MouseButton_Left,
		MouseButton_Middle,
		MouseButton_Right
	};

	virtual void OnWheelRoll(int in_Roll) = 0;
	virtual void OnMove(int in_PosX, int in_PosY) = 0;
	virtual void OnClick(MouseButton in_Button, int in_PosX, int in_PosY) = 0;
	virtual void OnRelease(MouseButton in_Button, int in_PosX, int in_PosY) = 0;
	virtual void OnLeave() = 0;
};

/**
 * WindowListener
 * Implemented by objects to recieve keyboard events
 */
class KeyboardListener
{
public:

	/**
	 * VirtualKey
	 * Virtual key press translations
	 */
	enum VirtualKey
	{
		VirtualKey_FIRST = 256,  // Start after ascii characters
		VirtualKey_Home,
		VirtualKey_Shift,
		VirtualKey_PageUp,
		VirtualKey_PageDown,
		VirtualKey_F1,
	};

	virtual void OnKeyDown(unsigned in_Key) = 0;
	virtual void OnKeyUp(unsigned in_Key) = 0;
};

/**
 * GLContext
 * A handle to an OpenGL window context
 */
typedef unsigned GLContext;

/**
 * Window
 * Window interface
 */
class Window
{
public:

	/**
	 * Virtual destructor
	 */
	virtual ~Window() {}

	/**
	 * Init
	 * Initialize and show this window with the specified title and dimensions
	 */
	virtual bool Init(const string& in_Title, int in_SizeX, int in_SizeY) = 0;

	/**
	 * SetTitle
	 * Set the window change
	 */
	virtual void SetTitle(const string& in_Title) = 0;

	/**
	 * Destroy
	 * Cleanup the window
	 */
	virtual	bool Destroy() = 0;

	/**
	 * ProcessMessage
	 * Process any queued messages for this window
	 */
	virtual void ProcessMessages() = 0;

	/**
	 * SwapBuffers
	 * Swap window back buffers
	 */
	virtual void SwapBuffers() = 0;

	/**
	 * EnableVerticalSync
	 * Enable/disable window vertical sync
	 */
	virtual void EnableVerticalSync(bool in_Enable) = 0;

	/**
	 * TranslateKey
	 * This method must modify the input key to conform with the KeyboardListener::VirtualKey enum
	 * If the input key is within the Ascii character range, no modification should be made
	 */
	virtual void TranslateKey(unsigned& in_Key) = 0;

	/**
	 * ShowUserPreferencesDialog
	 * Set whether the user preferences dialog should be visible or not
	 */
	virtual void ShowUserPreferencesDialog(bool in_Show) = 0;

	/**
	 * IsUserPreferencesDialogVisible
	 * Is the user preferences dialog currently visible
	 */
	virtual bool IsUserPreferencesDialogVisible() = 0;

	/**
	 * CreateGLContext
	 * Create an OpenGL context for this window
	 */
	virtual GLContext CreateGLContext() = 0;

	/**
	 * ShareGLContexts
	 * Have the two specified OpenGL contexts share resources; necessary for asynchronous loading
	 * Note: Contexts can only be shared if they have not yet been previously acquired
	 */
	virtual bool ShareGLContexts(GLContext in_Context1, GLContext in_Context2) = 0;

	/**
	 * AcquireGLContext
	 * Acquire the specified context in the current thread
	 * Note: only one thread may acquire any one context at a time
	 */
	virtual bool AcquireGLContext(GLContext in_Context) = 0;

	/**
	 * ReleaseGLContext
	 * Release the currently acquired context in the current thread
	 */
	virtual bool ReleaseGLContext(GLContext in_Context) = 0;

	/**
	 * AddWindowListener
	 * Register a listener to receive window events
	 */
	void AddWindowListener(WindowListener* in_Listener)
	{
		mWindowListeners.push_back(in_Listener);
	}

	/**
	 * AddMouseListener
	 * Register a listener to receive Mouse events
	 */
	void AddMouseListener(MouseListener* in_Listener)
	{
		mMouseListeners.push_back(in_Listener);
	}

	/**
	 * AddKeyboardListener
	 * Register a listener to receive Keyboard events
	 */
	void AddKeyboardListener(KeyboardListener* in_Listener)
	{
		mKeyboardListeners.push_back(in_Listener);
	}

protected:

	/**
	 * FireResizeEvent
	 * Let window listeners know the window is resizing
	 */
	void FireResizeEvent(int in_SizeX, int in_SizeY)
	{
		for(unsigned i = 0; i < mWindowListeners.size(); i++)
			mWindowListeners[i]->OnResize(in_SizeX, in_SizeY);
	}

	/**
	 * FireFocusEvent
	 * Let window listeners know the window keyboard focus is changing
	 */
	void FireFocusEvent(bool in_Focus)
	{
		for(unsigned i = 0; i < mWindowListeners.size(); i++)
			mWindowListeners[i]->OnFocus(in_Focus);
	}

	/**
	 * FireCloseEvent
	 * Let window listeners know the window is closing
	 */
	void FireCloseEvent()
	{
		for(unsigned i = 0; i < mWindowListeners.size(); i++)
			mWindowListeners[i]->OnClose();
	}

	/**
	 * FireMouseMoveEvent
	 * Let mouse listeners know the mouse is moving
	 */
	void FireMouseMoveEvent(int in_PosX, int in_PosY)
	{
		for(unsigned i = 0; i < mMouseListeners.size(); i++)
			mMouseListeners[i]->OnMove(in_PosX, in_PosY);
	}

	/**
	 * FireMouseWheelEvent
	 * Let mouse listeners know the mouse wheel is being used
	 */
	void FireMouseWheelEvent(int in_Roll)
	{
		for(unsigned i = 0; i < mMouseListeners.size(); i++)
			mMouseListeners[i]->OnWheelRoll(in_Roll);
	}

	/**
	 * FireMouseClickEvent
	 * Let mouse listeners know a mouse button is being clicked
	 */
	void FireMouseClickEvent(MouseListener::MouseButton in_Button, int in_PosX, int in_PosY)
	{
		for(unsigned i = 0; i < mMouseListeners.size(); i++)
			mMouseListeners[i]->OnClick(in_Button, in_PosX, in_PosY);
	}

	/**
	 * FireMouseReleaseEvent
	 * Let mouse listeners know a mouse button is being released
	 */
	void FireMouseReleaseEvent(MouseListener::MouseButton in_Button, int in_PosX, int in_PosY)
	{
		for(unsigned i = 0; i < mMouseListeners.size(); i++)
			mMouseListeners[i]->OnRelease(in_Button, in_PosX, in_PosY);
	}

	/**
	 * FireMouseLeaveEvent
	 * Let mouse listeners know the mouse is leaving the window client area
	 */
	void FireMouseLeaveEvent()
	{
		for(unsigned i = 0; i < mMouseListeners.size(); i++)
			mMouseListeners[i]->OnLeave();
	}

	/**
	 * FireKeyDownEvent
	 * Let keyboard listeners know a key is being pressed
	 */
	void FireKeyDownEvent(unsigned in_Key)
	{
		TranslateKey(in_Key);
		for(unsigned i = 0; i < mKeyboardListeners.size(); i++)
			mKeyboardListeners[i]->OnKeyDown(in_Key);
	}

	/**
	 * FireKeyUpEvent
	 * Let keyboard listeners know a key is being released
	 */
	void FireKeyUpEvent(unsigned in_Key)
	{
		TranslateKey(in_Key);
		for(unsigned i = 0; i < mKeyboardListeners.size(); i++)
			mKeyboardListeners[i]->OnKeyUp(in_Key);
	}

private:

	vector<WindowListener*> mWindowListeners;
	vector<MouseListener*> mMouseListeners;
	vector<KeyboardListener*> mKeyboardListeners;
};

#endif // WINDOW_H_
