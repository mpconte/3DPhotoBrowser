/**
 * @file PhotoBrowser.h
 * @brief PhotoBrowser class header file
 */

#ifndef PHOTOBROWSER_H_
#define PHOTOBROWSER_H_

#include "Global.h"
#include "Window.h"
#include "Camera.h"
#include "Layout.h"
//NEW/MATTHEW
#include "ImageTile.h"
#include "ImageContext.h"
#include "UserPreferences.h"

/**
 * PhotoBrowser
 * The main application class
 */
class PhotoBrowser
	: public WindowListener
	, public MouseListener
	, public KeyboardListener
	, public UserPreferenceListener
{
	/**
	 * LayoutData
	 * Supporting structure
	 */
	struct LayoutData
	{
		Layout* LayoutRef;
		bool CameraSaved;
		float CameraX;
		float CameraY;
		float CameraZ;
	};

public:

	/**
	 * Instance
	 * Get the singleton instance
	 */
	static PhotoBrowser* Instance() { static PhotoBrowser l_Instance; return &l_Instance; }

	/**
	 * AcquireOpenGLWorkerContext
	 * Acquire the opengl worker context in the current thread
	 */
	void AcquireOpenGLWorkerContext();

	/**
	 * ReleaseOpenGLWorkerContext
	 * Release the opengl worker context for the current thread
	 */
	void ReleaseOpenGLWorkerContext();

	/**
	 * Startup
	 * Must be called once before the window message loop begins
	 */
	bool Startup();

	/**
	 * Shutdown
	 * Must be called once after the window message loop terminates
	 */
	bool Shutdown();

	/**
	 * RegisterLayout
	 * Register a new layout that can be used by the photo browser.
	 */
	template <class T>
	void RegisterLayout()
	{
		LayoutData l_Data;
		l_Data.CameraSaved = false;
		l_Data.LayoutRef = new T();
		mRegisteredLayouts.push_back(l_Data);
	}

	/**
	 * GetRegisteredLayoutCount
	 * Get the number of registered layout types
	 */
	unsigned GetRegisteredLayoutCount() { return mRegisteredLayouts.size(); }

	/**
	 * GetRegisteredLayout
	 * Get a registered layout by its index. in_Index must be less than GetRegisteredLayoutCount()
	 */
	const Layout* GetRegisteredLayout(unsigned in_Index) { return mRegisteredLayouts[in_Index].LayoutRef; }

	/**
	 * SelectLayout
	 * Select one of the registered layouts to use
	 */
	void SelectLayout(unsigned in_Index);

	/**
	 * Done
	 * Return whether the PhotoBrowser is done executing and should now terminate
	 */
	bool Done() const { return mDone; }

	/**
	 * Done
	 * Set whether the PhotoBrowser is done executing
	 */
	void Done(bool in_Done) { mDone = in_Done; }

	/**
	 * MainLoop
	 * The main application loop
	 */
	void MainLoop();

	/**
	 * Tick
	 * Called each frame with the elapsed frametime to perform any relevant updates
	 */
	void Tick(float in_DeltaTime);

	/**
	 * WindowListener interface
	 */
	virtual void OnResize(int in_SizeX, int in_SizeY);
	virtual void OnFocus(bool in_Focus);
	virtual void OnClose();

	/**
	 * MouseListener interface
	 */
	virtual void OnWheelRoll(int in_Roll);
	virtual void OnMove(int in_PosX, int in_PosY);
	virtual void OnClick(MouseButton in_Button, int in_PosX, int in_PosY);
	virtual void OnRelease(MouseButton in_Button, int in_PosX, int in_PosY);
	virtual void OnLeave();

	/**
	 * KeyboardListener interface
	 */
	void OnKeyDown(unsigned in_Key);
	void OnKeyUp(unsigned in_Key);

	/**
	 * UserPreferenceListener interface
	 */
	void OnUserPreferenceUpdate();

protected:

	/**
	 * Helpers
	 */
	float CalculateDistanceCorrectedZoomValue(float in_ZoomAount);
	void DrawRightClickSelectionBox();	
	void UpdateControls(float in_DeltaTime);
	void DebounceKeys();

	/**
	 * Singleton implementation
	 */
	PhotoBrowser();
	PhotoBrowser(const PhotoBrowser&);
	PhotoBrowser& operator=(const PhotoBrowser&);

private:

	GLContext mMainContext;					// The main thread OpenGL context
	GLContext mWorkerContext;				// The texture load thread OpenGL context

	bool mCurrentLayoutChangedThisFrame;	// The current layout changed this frame
	bool mWindowReceivedFocusThisFrame;		// Keep track of window focus events
	bool mAllowClickZoomThisFrame;			// A bit of a hack to get the desired functionality

	string mWindowTitle;					// The application window title

	float mAverageFrameTime;				// The average frame time delta

	Window* mWindow;						// The application window
	Camera* mCamera;						// The application camera
	bool mDone;								// Is the app done yet?

	int mCurrentLayoutIndex;				// The current application layout
	vector<LayoutData> mRegisteredLayouts;	// The registered layouts

	// Mouse controls
	bool mLeftClick;							// Is the left mouse button clicked?
	bool mRightClick;							// Is the right mouse button clicked?
	bool mLeftClickRelease;						// Was the left mouse button released this frame?
	bool mRightClickRelease;					// Was the right mouse button released this frame?
	float mLeftClickPosX, mLeftClickPosY;		// The position at which the left mouse button was clicked
	float mRightClickPosX, mRightClickPosY;		// The position at which the left mouse button was clicked
	float mLeftDragX, mLeftDragY;				// The drag distance while the left button is clicked
	float mLeftDeltaX, mLeftDeltaY;				// The left delta drag for the last frame
	float mMousePosX, mMousePosY;				// The current mouse position
	float mMouseWheelAccumulator;				// The accumulated amount of mouse wheel roll
	
	//NEW/MATTHEW
	ImageTile* outlined_img;					// Current image outlined by mouse (if any)	

	// Keyboard controls
	map<unsigned, bool> mKeys;
};

#endif // PHOTOBROWSER_H_
