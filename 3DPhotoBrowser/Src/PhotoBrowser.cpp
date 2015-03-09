/**
 * @file PhotoBrowser.cpp
 * @brief PhotoBrowser class implementation file
 */

#include "PhotoBrowser.h"
#include "CalendarLayout.h"
#include "CompactLayout.h"
#include "ImageTile.h"
#include "OpenGL.h"
#include "IL/il.h"

// Platform specific window
#ifdef WIN32
	#include "MSWindow.h"
#else
	#error You platform window include goes here
#endif // WIN32

//-----------------------------------------------------------------------------------------------------------------------------
// PhotoBrowser

PhotoBrowser::PhotoBrowser()
: mMainContext(NULL)
, mWorkerContext(NULL)
, mCurrentLayoutChangedThisFrame(false)
, mWindowReceivedFocusThisFrame(false)
, mAllowClickZoomThisFrame(false)
, mDone(false)
, mAverageFrameTime(0)
, mWindow(NULL)
, mCamera(NULL)
, mCurrentLayoutIndex(-1)
, mLeftClick(false)
, mRightClick(false)
, mLeftClickRelease(false)
, mRightClickRelease(false)
, mLeftClickPosX(0), mLeftClickPosY(0)
, mRightClickPosX(0), mRightClickPosY(0)
, mLeftDragX(0), mLeftDragY(0)
, mMousePosX(0), mMousePosY(0)
, mMouseWheelAccumulator(0)
//NEW/MATTHEW
,outlined_img(NULL)
{}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::AcquireOpenGLWorkerContext()
{
	mWindow->AcquireGLContext(mWorkerContext);
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::ReleaseOpenGLWorkerContext()
{
	mWindow->ReleaseGLContext(mWorkerContext);
}

//-----------------------------------------------------------------------------------------------------------------------------

bool PhotoBrowser::Startup()
{
	// Configure the photo browser layouts
	RegisterLayout<CalendarLayout>();
	RegisterLayout<CompactLayout>();

	// Initialize DevIL
	ilInit();

	// Instantiate the platform specific window
#ifdef WIN32
	mWindow = new MSWindow();
#else
	#error You platform window instantiation goes here
#endif // WIN32

	// The way sprintf is used here is perfectly safe
#ifdef WIN32
	#pragma warning (push)
	#pragma warning (disable: 4996)
#endif // WIN32

	// Create the window title
	char l_Buff[64];
	sprintf(l_Buff, "Photo Browser v%.1f.%04d", PHOTOBROWSER_VERSION, PHOTOBROWSER_REVISION);
	mWindowTitle = l_Buff;

#ifdef WIN32
	#pragma warning (pop)
#endif // WIN32

	// Initialize the window
	int l_WindowSizeX = 640;
	int l_WindowSizeY = 480;
	if(!mWindow->Init(mWindowTitle, l_WindowSizeX, l_WindowSizeY))
	{
		logf("Failed to initialize application window");
		return false;
	}

	// Show the user preferences dialog by default
	mWindow->ShowUserPreferencesDialog(true);

	// Register for window events
	mWindow->AddWindowListener(this);
	mWindow->AddMouseListener(this);
	mWindow->AddKeyboardListener(this);

	// Create the OpenGL contexts
	mMainContext = mWindow->CreateGLContext();

#if USE_THREADED_TEXTURE_LOADING
	mWorkerContext = mWindow->CreateGLContext();

	// Share the two contexts
	if(!mWindow->ShareGLContexts(mMainContext, mWorkerContext))
	{
		logf("Failed to initialize shared OpenGL contexts");
		return false;
	}
#endif // USE_THREADED_TEXTURE_LOADING

	// Acquire the main context in the main thread
	if(!mWindow->AcquireGLContext(mMainContext))
	{
		logf("Failed to acquire main OpenGL context");
		return false;
	}

	// Should vsync be on or off?
	mWindow->EnableVerticalSync(UserPreferences::Instance()->EnableVerticalSync());

	// Configure the graphics renderer for OpenGL
	Graphics::ConfigureRenderer<OpenGL>();

	// Initialize the photo browser's camera
	mCamera = new Camera();
	mCamera->ResizeViewport(l_WindowSizeX, l_WindowSizeY);

	// Load images into the image context
	ImageContext::Instance()->CreateContext();

	UserPreferences* l_Prefs = UserPreferences::Instance();

	// Listen for user preference change events
	l_Prefs->AddUserPreferenceListener(this);

	// Set the current layout
	SelectLayout(l_Prefs->CurrentLayout());
	mCurrentLayoutChangedThisFrame = false; // This gets set by SelectLayout, but we don't want it to apply on init

	// Check if we should restore the camera position to whatever was saved by the user preferences
	if(l_Prefs->SaveCameraPosition())
	{
		mCamera->SetPosition( l_Prefs->SavedCameraX(),
							  l_Prefs->SavedCameraY(),
							  l_Prefs->SavedCameraZ() );
	}

	// If not using threaded loading, load all these thumbnails at startup
#if !USE_THREADED_TEXTURE_LOADING
	unsigned l_ImageCount = ImageContext::Instance()->GetImageCount();
	for(unsigned i = 0; i < l_ImageCount; i++)
	{
		ImageTile* l_Tile = ImageContext::Instance()->GetImage(i);
		l_Tile->ActivateThumbnail(ThumbnailSize_64x64);
	}
#endif // USE_THREADED_TEXTURE_LOADING

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------

bool PhotoBrowser::Shutdown()
{
	// Stop the texture loader thread
	TextureLoader::Instance()->Shutdown();

	// Save the last camera position in the UserPreferences
	UserPreferences* l_Prefs = UserPreferences::Instance();
	float l_PosX, l_PosY, l_PosZ;
	mCamera->GetPosition(l_PosX, l_PosY, l_PosZ);
	l_Prefs->SavedCameraX(l_PosX);
	l_Prefs->SavedCameraY(l_PosY);
	l_Prefs->SavedCameraZ(l_PosZ);

	// Free images from the image context
	ImageContext::Instance()->DestroyContext();

	// Free the camera
	delete mCamera;
	mCamera = NULL;

	// Free the graphics singleton
	Graphics::Destroy();

	// Free the main GL context
	// @TODO - this slows down shutdown a lot
//	mWindow->ReleaseGLContext(mMainContext);

	// Destroy the window
	mWindow->Destroy();
	delete mWindow;
	mWindow = NULL;

	// Shutdown DevIL
	ilShutDown();

	// Free registered layout memory
	for(unsigned i = 0; i < mRegisteredLayouts.size(); i++)
	{
		Layout* l_Layout = mRegisteredLayouts[i].LayoutRef;
		delete l_Layout;
	}
	mRegisteredLayouts.clear();

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::SelectLayout(unsigned in_Index)
{
	// Save the camera position of the previous layout before we switch
	if(mCurrentLayoutIndex >= 0)
	{
		LayoutData& l_Data = mRegisteredLayouts[mCurrentLayoutIndex];
		l_Data.CameraSaved = true;
		mCamera->GetPosition(l_Data.CameraX, l_Data.CameraY, l_Data.CameraZ);
	}

	// Switch to the new layout
	mCurrentLayoutChangedThisFrame = mCurrentLayoutIndex != in_Index;
	mCurrentLayoutIndex = in_Index;
	LayoutData& l_Data = mRegisteredLayouts[in_Index];
	l_Data.LayoutRef->DoLayout(ImageContext::Instance(), mCamera, !l_Data.CameraSaved);

	// Restore the camera position if one was saved
	if(l_Data.CameraSaved)
	{
		mCamera->MoveTo(l_Data.CameraX, l_Data.CameraY, l_Data.CameraZ, UserPreferences::Instance()->ImageMoveTime());
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::MainLoop()
{
	UserPreferences* l_Prefs = UserPreferences::Instance();

	// Frame time variable
	double l_LastFrameTime = Timer::Instance()->GetSeconds();

	// While we should still be running
	while(!Done())
	{
		double l_FrameTimeTarget = 1.0f / l_Prefs->FramerateLimit();

		// Process any queued window messages
		mWindow->ProcessMessages();

		// Compute the amount of time that passed since last frame
		double l_NewFrameTime = Timer::Instance()->GetSeconds();
		double l_DeltaTime = l_NewFrameTime - l_LastFrameTime;

		// Clamp our framerate to some target
		while(l_DeltaTime < l_FrameTimeTarget)
		{
			Sleep(0); // Yield timeslice

			// Recompute frame time
			l_NewFrameTime = Timer::Instance()->GetSeconds();
			l_DeltaTime = l_NewFrameTime - l_LastFrameTime;
		}

		// Store the frametime
		l_LastFrameTime = l_NewFrameTime;

		// Process the photo browser frame
		Tick((float)l_DeltaTime);

		// Update the average frametime
		mAverageFrameTime = (mAverageFrameTime * 0.75f) + (float)(l_DeltaTime * 0.25f);

		// Should we show the framerate?
		if(l_Prefs->ShowFramerate())
		{
			stringstream l_Title;
			
			l_Title << "AvgFPS=" << setprecision(2) << (1.0f/mAverageFrameTime) << " (" << mAverageFrameTime << " ms)"
					<< " FPS=" << setprecision(4) << (1.0f/l_DeltaTime) << " (" << l_DeltaTime << " ms)";
			mWindow->SetTitle(l_Title.str());
		}

#ifdef DEBUG
		logf("FrameTime=%.2fms FrameRate=%d", (float)l_DeltaTime, (int)(1.0/l_DeltaTime));
#endif // DEBUG
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::Tick(float in_DeltaTime)
{
	// Variables used for following the nearest image when image follow mode is used
	bool l_FindClosest = false;
	float l_MouseWorldX = 0;
	float l_MouseWorldY = 0;
	float l_ClosestImageX = 0;
	float l_ClosestImageY = 0;
	float l_ClosestImageDistance = 0;
	ImageTile* l_ClosestImage = NULL;

	bool is_outlined;

	// Update the camera
	mCamera->Tick(in_DeltaTime);

	// Update controls
	UpdateControls(in_DeltaTime);

	// Should we follow the closest image this frame?
	if(mCurrentLayoutChangedThisFrame && UserPreferences::Instance()->LayoutImageFollowMode())
	{
		l_FindClosest = true; // Indiciate we need to do some extra work in the image tile loop below

		// Get the mouse world coordinates
		mCamera->GetImagePlaneWorldPosition(mMousePosX, mMousePosY, l_MouseWorldX, l_MouseWorldY);

		// Prime the nearest image variables
		l_ClosestImage = ImageContext::Instance()->GetImage(0);
		l_ClosestImage->GetPosition(l_ClosestImageX, l_ClosestImageY);
		float l_DistX = l_MouseWorldX - l_ClosestImageX;
		float l_DistY = l_MouseWorldY - l_ClosestImageY;
		l_ClosestImageDistance = l_DistX*l_DistX + l_DistY*l_DistY;
	}

	// Clear the frame buffer
	Graphics* l_Graphics = Graphics::Instance();
	l_Graphics->ClearBuffers();

	// Apply the camera transform
	mCamera->Apply();

	// Get the world space that is currently visible by the camera
	float l_MinWorldX, l_MinWorldY, l_MaxWorldX, l_MaxWorldY;
	mCamera->GetVisibleWorldBounds(l_MinWorldX, l_MinWorldY, l_MaxWorldX, l_MaxWorldY);

	// Determine which thumbnail size we should be using at the current camera distance
	float l_CameraDist = mCamera->GetPositionZ();
	ThumbnailSize l_ThumbnailSize = ThumbnailSize_None;

	// @TODO - this algorithm needs to be written, arbitrary constant used for now
	if(l_CameraDist < 30.0f)
	{
		l_ThumbnailSize = ThumbnailSize_64x64;
	}

	// Image tile processing
	float l_HalfImageSize = UserPreferences::Instance()->ImageSize() * 0.5f;
	unsigned l_ImageCount = ImageContext::Instance()->GetImageCount();
	for(unsigned i = 0; i < l_ImageCount; i++)
	{
		float l_X, l_Y;
		ImageTile* l_Tile = ImageContext::Instance()->GetImage(i);

		// If we are looking for the closest image, see if this image is closest
		if(l_FindClosest)
		{
			// We need the position BEFORE the image is ticked
			l_Tile->GetPosition(l_X, l_Y);

			// Find the distance of this image from the current mouse position
			float l_DistX = l_X - l_MouseWorldX;
			float l_DistY = l_Y - l_MouseWorldY;
			float l_Dist = (l_DistX * l_DistX + l_DistY * l_DistY);
			if(l_Dist < l_ClosestImageDistance)
			{
				// This is the new closest image
				l_ClosestImageX = l_X;
				l_ClosestImageY = l_Y;
				l_ClosestImageDistance = l_Dist;
				l_ClosestImage = l_Tile;
			}
		}

		// Update the image
		l_Tile->Tick(in_DeltaTime);

		// Check if this image is visible
		l_Tile->GetPosition(l_X, l_Y);
		if( l_X + l_HalfImageSize < l_MinWorldX || l_X - l_HalfImageSize > l_MaxWorldX ||
			l_Y + l_HalfImageSize < l_MinWorldY || l_Y - l_HalfImageSize > l_MaxWorldY )
		{
			// If not, don't bother drawing it
			continue;
		}

		// Set the thumbnail size to use, then draw the tile
		l_Tile->ActivateThumbnail(l_ThumbnailSize);
		// l_Tile->Draw();

		//NEW/MATTHEW
		//Outline image if mouse is over it
	    is_outlined =  l_Tile->Outline(mMousePosX, mMousePosY);
		
		if (is_outlined)	
		{
			//l_Tile->ActivateThumbnail(ThumbnailSize_1024x1024);
			outlined_img = l_Tile;
			//l_Tile->GetPosition(outline_imgX, outline_imgY);
			//l_Tile->GetSize(outline_imgWidth, outline_imgHeight);
		}
		else
			outlined_img = NULL;
		
		l_Tile->Draw();
	}

	// If there is a closest image, then we need to move towards it
	if(l_ClosestImage)
	{
		float l_ImageX, l_ImageY, l_ImageZ;
		l_ClosestImage->GetMoveToGoalPosition(l_ImageX, l_ImageY, l_ImageZ);

		// We want the image to fall under the mouse again, so add the camera offset from the images
		// position to the new world position
		float l_OffsetX = ((l_MinWorldX + l_MaxWorldX) * 0.5f) - l_ClosestImageX;
		float l_OffsetY = ((l_MinWorldY + l_MaxWorldY) * 0.5f) - l_ClosestImageY;
		mCamera->MoveTo(l_ImageX + l_OffsetX, l_ImageY + l_OffsetY, l_CameraDist, UserPreferences::Instance()->ImageMoveTime());
	}

	// Draw the right click selection box
	if(mRightClick)
	{
		DrawRightClickSelectionBox();
	}

	// Swap the window back buffers
	mWindow->SwapBuffers();

	// Reset frame flags
	mWindowReceivedFocusThisFrame = false;	// Reset the focus flag
	mCurrentLayoutChangedThisFrame = false; // Reset the changed layout flag
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::OnResize(int in_SizeX, int in_SizeY)
{
	mCamera->ResizeViewport(in_SizeX, in_SizeY);
	mRegisteredLayouts[mCurrentLayoutIndex].LayoutRef->DoLayout(ImageContext::Instance(), mCamera, false);
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::OnFocus(bool in_Focus)
{
	mWindowReceivedFocusThisFrame |= in_Focus;

	// Whenever we lose keyboard focus, debounce all keys
	if(!in_Focus)
	{
		DebounceKeys();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::OnClose()
{
	Done(true);
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::OnWheelRoll(int in_Roll)
{
	mMouseWheelAccumulator += in_Roll;
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::OnMove(int in_PosX, int in_PosY)
{
	// Track the mouse position
	mMousePosX = (float)in_PosX;
	mMousePosY = (float)in_PosY;
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::OnClick(MouseButton in_Button, int in_PosX, int in_PosY)
{
	// If we just received focus, click zoom is not allowed until the mouse release is processed
	if(mWindowReceivedFocusThisFrame)
	{
		mAllowClickZoomThisFrame = false;
	}

	if(in_Button == MouseButton_Left)
	{
		mLeftClick = true;

		// Track where the click was made
		mLeftDragX = mLeftClickPosX = (float)in_PosX;
		mLeftDragY = mLeftClickPosY = (float)in_PosY;
	}
	else if(in_Button == MouseButton_Right)
	{
		mRightClick = true;

		// Track where the click was made
		mRightClickPosX = (float)in_PosX;
		mRightClickPosY = (float)in_PosY;
	}

	// Track mouse position
	mMousePosX = (float)in_PosX;
	mMousePosY = (float)in_PosY;
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::OnRelease(MouseButton in_Button, int in_PosX, int in_PosY)
{
	if(in_Button == MouseButton_Left)
	{
		mLeftClickRelease = mLeftClick;
		mLeftClick = false;
	}
	else if(in_Button == MouseButton_Right)
	{
		mRightClickRelease = mRightClick;
		mRightClick = false;
	}

	// Track the mouse position
	mMousePosX = (float)in_PosX;
	mMousePosY = (float)in_PosY;
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::OnLeave()
{
	mLeftClickRelease = mLeftClick; // If was clicked, release
	mLeftClick = false;
	mRightClickRelease = mRightClick; // If was clicked, release
	mRightClick = false;
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::OnKeyDown(unsigned in_Key)
{
	// Set this key as pressed in the key mapping
	mKeys[in_Key] = true;
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::OnKeyUp(unsigned in_Key)
{
	// Set this key as release in the key mapping
	mKeys[in_Key] = false;
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::OnUserPreferenceUpdate()
{
	UserPreferences* l_Prefs = UserPreferences::Instance();

	// Set an arbitrary minimum for the framerate limit value
	if(l_Prefs->FramerateLimit() <= 0)
	{
		l_Prefs->FramerateLimit(5);
	}

	// Is framerate display off?
	if(!l_Prefs->ShowFramerate())
	{
		// Restore the normal application window title
		mWindow->SetTitle(mWindowTitle);
	}

	// Update vsync setting
	mWindow->EnableVerticalSync(l_Prefs->EnableVerticalSync());

	// Whenever a user preference changes while the browser is running, apply the layout again
	if(!Done())
	{
		// Check if its a new layout
		unsigned l_LayoutIndex = l_Prefs->CurrentLayout();
		if(l_LayoutIndex != mCurrentLayoutIndex)
		{
			SelectLayout(l_LayoutIndex);
		}
		// If not, don't do a select, this messes with the camera. Just redo the layout
		else
		{
			mRegisteredLayouts[l_LayoutIndex].LayoutRef->DoLayout(ImageContext::Instance(), mCamera, false);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

float PhotoBrowser::CalculateDistanceCorrectedZoomValue(float in_ZoomAount)
{
	float l_X, l_Y, l_Z;
	mCamera->GetPosition(l_X, l_Y, l_Z);

	// Scroll zoom profile = f(z)
	//	f(z) = { 
	//		(z/factor)^2				, z < factor
	//		ln(z) - (ln(factor) - 1)	, z > factor
	//	}
	float l_Factor = UserPreferences::Instance()->CameraZoomWheelFactor();
	float l_FofZ = l_Z < l_Factor ? 
		((l_Z * l_Z)/(l_Factor * l_Factor)) : 
		(log(l_Z) - (log(l_Factor) - 1.0f));

	return in_ZoomAount * l_FofZ;
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::DrawRightClickSelectionBox()
{
	// Draw an outlined quad
	Graphics::Instance()->DrawQuadOutline2D
	(
		min(mRightClickPosX, mMousePosX), min(mRightClickPosY, mMousePosY),		// Bottom left screen coord
		1.0f, 1.0f, 1.0f,														// Color
		fabs(mRightClickPosX - mMousePosX), fabs(mRightClickPosY - mMousePosY)	// Width / height
	);
}



//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::UpdateControls(float in_DeltaTime)
{
	// Check for left click
	if(mLeftClick)
	{
		// Update the delta for this frame
		mLeftDeltaX = mLeftDragX - mMousePosX;
		mLeftDeltaY = mLeftDragY - mMousePosY;

		// Move the camera
		if(mLeftDeltaX != 0 || mLeftDeltaY != 0)
		{
			mCamera->MoveScreenDelta(mLeftDeltaX, mLeftDeltaY);
		}

		// Reset mouse click drag to negate the delta for this frame
		mLeftDragX = mMousePosX;
		mLeftDragY = mMousePosY;
	}

	// Check for left click release
	if(mLeftClickRelease)
	{
		mLeftClickRelease = false;

		// When we release the left button, perform a swipe
		mCamera->Swipe(mLeftDeltaX, mLeftDeltaY);

		// If the left click was released and the mouse never moved,
		// this is equivalent to a zoom in/out
		// NOTE: Don't LMB zoom when the window received focus this frame
		if( mMousePosX == mLeftClickPosX && 
			mMousePosY == mLeftClickPosY &&
			mAllowClickZoomThisFrame )
		{
			float l_ZoomAmount = UserPreferences::Instance()->CameraZoomMagnification();
			float l_ZoomTime = UserPreferences::Instance()->CameraZoomTime();

			//NEW/MATTHEW
			// if image is outlined, zoom into it and update thumbnail to 1K x 1K
			if (outlined_img)
			{
				float x, y, width, height;

				outlined_img->GetPosition(x,y);
				outlined_img->GetSize(width, height);

				//outlined_img->ActivateThumbnail(ThumbnailSize_1024x1024);
				mCamera->ZoomExtents(x - width * 0.5f, y - height * 0.5f, x + width * 0.5f, y + height * 0.5f);	
				outlined_img->Draw();
			}
			else
			{
				// If shift is held, zoom out, otherwise zoom in
				mCamera->ZoomScreenPoint(mMousePosX, mMousePosY, 
				mKeys[VirtualKey_Shift] ? -l_ZoomAmount : l_ZoomAmount, l_ZoomTime);
			}
		}

		mAllowClickZoomThisFrame = true;
	}

	// Check for right click release
	if(mRightClickRelease)
	{
		// When released, if a drag box was created, zoom the box extents
		mRightClickRelease = false;
		if(mMousePosX != mRightClickPosX && mMousePosY != mRightClickPosY)
		{
			mCamera->ZoomScreenExtents( min(mMousePosX, mRightClickPosX), min(mMousePosY, mRightClickPosY),
										max(mMousePosX, mRightClickPosX), max(mMousePosY, mRightClickPosY) );
		}
	}

	// Mouse wheel
	// Fine zoom
	if(mMouseWheelAccumulator != 0)
	{
		float l_Sensitivity = UserPreferences::Instance()->CameraZoomSensitivity();
		float l_ZoomAmount = CalculateDistanceCorrectedZoomValue(mMouseWheelAccumulator * l_Sensitivity);
		mCamera->ZoomScreenPoint(mMousePosX, mMousePosY, l_ZoomAmount, UserPreferences::Instance()->CameraZoomWheelTime());
		mMouseWheelAccumulator = 0;
	}

	// PageUp/PageDown keys
	// Zoom in/out - equivalent to LMB/SHIFT+LMB click
	if(mKeys[VirtualKey_PageUp])
	{
		mKeys[VirtualKey_PageUp] = false;

		// Zoom in at the center of the screen
		float l_ZoomAmount = UserPreferences::Instance()->CameraZoomMagnification();
		float l_ZoomTime = UserPreferences::Instance()->CameraZoomTime();
		mCamera->ZoomScreenPoint(mMousePosX, mMousePosY,
			CalculateDistanceCorrectedZoomValue(l_ZoomAmount), l_ZoomTime);
	}
	else if(mKeys[VirtualKey_PageDown])
	{
		mKeys[VirtualKey_PageDown] = false;

		// Zoom out at the center of the screen
		float l_ZoomAmount = UserPreferences::Instance()->CameraZoomMagnification();
		float l_ZoomTime = UserPreferences::Instance()->CameraZoomTime();
		mCamera->ZoomScreenPoint(mMousePosX, mMousePosY,
			-CalculateDistanceCorrectedZoomValue(l_ZoomAmount), l_ZoomTime);
	}

	// Home key
	// Return to / save home position
	if(mKeys[VirtualKey_Home])
	{
		mKeys[VirtualKey_Home] = false;

		// Save new home position
		if(mKeys[VirtualKey_Shift])
		{
			float l_X, l_Y, l_Z;
			mCamera->GetPosition(l_X, l_Y, l_Z);
			mCamera->SetHomePosition(l_X, l_Y, l_Z);
		}
		// Go to home position
		else
		{
			mCamera->MoveToHome();
		}
	}

	// Backspace key
	// Undo camera movement
	if(mKeys['\b'])
	{
		mKeys['\b'] = false;
		mCamera->UndoLastMove();
	}

	// F1 key
	// Toggle the UPF
	if(mKeys[VirtualKey_F1])
	{
		mKeys[VirtualKey_F1] = false;
		mWindow->ShowUserPreferencesDialog(!mWindow->IsUserPreferencesDialogVisible());
	}

	// V key
	// Toggle layout
	if(mKeys['V'])
	{
		mKeys['V'] = false;
		UserPreferences::Instance()->CurrentLayout((mCurrentLayoutIndex+1) % mRegisteredLayouts.size());
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void PhotoBrowser::DebounceKeys()
{
	// For each key in the key mapping, set its pressed state to false
	for(map<unsigned, bool>::iterator It = mKeys.begin(); It != mKeys.end(); It++)
	{
		It->second = false;
	}
}
