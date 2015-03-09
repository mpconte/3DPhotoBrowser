/**
 * @file Camera.h
 * @brief Camera class header file
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include "Global.h"

/**
 * Camera
 * Class used to manage the OpenGL camera
 */
class Camera
{
	/**
	 * SavedPosition
	 * Supporting structure used for the undo move stack
	 */
	struct SavedPosition
	{
		float PosX;
		float PosY;
		float PosZ;
	};

public:

	/**
	 * Constructors
	 */
	Camera();
	
	/**
	 * Destructor
	 */
	~Camera();

	/**
	 * GetPosition
	 * Get the camera's current position
	 */
	void GetPosition(float& out_PosX, float& out_PosY, float& out_PosZ) const 
	{
		out_PosX = mPosX;
		out_PosY = mPosY;
		out_PosZ = mPosZ;
	}

	/**
	 * GetPositionZ
	 * Get the camera's current z position
	 */
	float GetPositionZ() { return mPosZ; }

	/**
	 * SetPosition
	 * Change the camera's position
	 */
	void SetPosition(float in_PosX, float in_PosY, float in_PosZ)
	{
		mPosX = in_PosX;
		mPosY = in_PosY;
		mPosZ = in_PosZ;
	}

	/**
	 * SetPosition
	 * Change the camera's home position
	 */
	void SetHomePosition(float in_PosX, float in_PosY, float in_PosZ)
	{
		mHomePosX = in_PosX;
		mHomePosY = in_PosY;
		mHomePosZ = in_PosZ;
	}

	/**
	 * GetLookPosition
	 * Get the camera's current position
	 */
	void GetLookPosition(float& out_LookX, float& out_LookY, float& out_LookZ) const 
	{
		out_LookX = mLookX;
		out_LookY = mLookY;
		out_LookZ = mLookZ;
	}

	/**
	 * SetLookPosition
	 * Change the camera's look position
	 */
	void SetLookPosition(float in_LookX, float in_LookY, float in_LookZ)
	{
		mLookX = in_LookX;
		mLookY = in_LookY;
		mLookZ = in_LookZ;
	}

	/**
	 * GetUpDirection
	 * Get the camera's current up direction
	 */
	void GetUpDirection(float& out_UpX, float& out_UpY, float& out_UpZ) const 
	{
		out_UpX = mUpX;
		out_UpY = mUpY;
		out_UpZ = mUpZ;
	}

	/**
	 * SetUpDirection
	 * Change the camera's up direction
	 */
	void SetUpDirection(float in_UpX, float in_UpY, float in_UpZ)
	{
		mUpX = in_UpX;
		mUpY = in_UpY;
		mUpZ = in_UpZ;
	}

	/**
	 * SetBoundaryX
	 * Change the x-direction boundaries for the camera
	 */
	void SetBoundaryX(float in_Min, float in_Max)
	{ 
		mMinBoundaryX = in_Min;
		mMaxBoundaryX = in_Max;
	}

	/**
	 * SetBoundaryY
	 * Change the x-direction boundaries for the camera
	 */
	void SetBoundaryY(float in_Min, float in_Max)
	{ 
		mMinBoundaryY = in_Min;
		mMaxBoundaryY = in_Max;
	}

	/**
	 * SetBoundaryZ
	 * Change the z-direction boundaries for the camera
	 */
	void SetBoundaryZ(float in_Min, float in_Max)
	{ 
		mMinBoundaryZ = in_Min;
		mMaxBoundaryZ = in_Max;
	}

	/**
	 * GetMoveToPosition
	 * Get the position the camera is moving towards
	 */
	void GetMoveToPosition(float& out_PosX, float& out_PosY, float& out_PosZ) const 
	{
		out_PosX = mMoveToX;
		out_PosY = mMoveToY;
		out_PosZ = mMoveToZ;
	}

	/**
	 * GetVisibleWorldBounds
	 * Get the visible world (in the xy plane) from the camera's current position
	 */
	void GetVisibleWorldBounds(float& out_MinWorldX, float& out_MinWorldY, float& out_MaxWorldX, float& out_MaxWorldY);

	/**
	 * GetImagePlaneWorldPosition
	 * Return the xy world coordinates for a set of screen coordinates in the image plane
	 */
	void GetImagePlaneWorldPosition(float in_ScreenX, float in_ScreenY, float& in_WorldX, float& in_WorldY);

	/**
	 * Swipe
	 * Slides the camera beyond its current positions based on the magnitude of the input delta
	 */
	void Swipe(float in_DeltaScreenX, float in_DeltaSreenY);

	/**
	 * MoveTo
	 * Accelerate the camera to the specified position in the allotted time
	 */
	void MoveTo(float in_PosX, float in_PosY, float in_PosZ, float in_Time);

	/**
	 * MoveScreenDelta
	 * Translate the delta screen coorindates to delta world coordinates and move the camera
	 */
	void MoveScreenDelta(float in_DeltaScreenX, float in_DeltaSreenY);

	/**
	 * MoveToHome
	 * Accelerate to the camera's home position. If in_Snap == true, the camera will immediately change
	 * positions without accelerating
	 */
	void MoveToHome();

	/**
	 * ZoomScreenPoint
	 * Zoom the camera towards the specified screen position
	 */
	void ZoomScreenPoint(float in_ScreenX, float in_ScreenY, float in_ZoomAmount, float in_ZoomTime);

	/**
	 * ZoomScreenExtents
	 * Zoom the extents of the specified rectangular screen region
	 */
	void ZoomScreenExtents(float in_MinX, float in_MinY, float in_MaxX, float in_MaxY);

	/**
	 * ZoomExtents
	 * Zoom the extents of the specified world position plane rectangle in the XY plane
	 */
	void ZoomExtents(float in_MinWorldX, float in_MinWorldY, float in_MaxWorldX, float in_MaxWorldY);

	/**
	 * UndoLastMove
	 * Return the camera to its last saved position
	 */
	void UndoLastMove();

	/**
	 * Apply
	 * Apply the camera transform to the current model-view matrix
	 */
	void Apply();

	/**
	 * Tick
	 * Called each frame with the elapsed frametime to perform any relevant updates
	 */
	void Tick(float in_DeltaTime);

	/**
	 * ResizeViewport
	 * Used to update the camera's viewport settings whenever the window dimensions change
	 */
	void ResizeViewport(int in_SizeX, int in_SizeY);

	/**
	 * Get*
	 * Get the various camera parameters
	 */
	int GetViewportSizeX() const { return mViewportSizeX; }
	int GetViewportSizeY() const { return mViewportSizeY; }
	float GetFovy() const { return mFovy; }
	float GetAspectRatio() const { return mAspectRatio; }
	float GetNearPlaneDist() const { return mNearPlaneDist; }
	float GetFarPlaneDist() const { return mFarPlaneDist; }

	/**
	 * Set*
	 * Set the various camera parameters
	 */
	void SetFovy(float in_Fovy) { mFovy = in_Fovy; }
	void SetAspectRatio(float in_AspectRatio) { mAspectRatio = in_AspectRatio; }
	void SetNearPlaneDist(float in_NearPlaneDist) { mNearPlaneDist = in_NearPlaneDist; }
	void SetFarPlaneDist(float in_FarPlaneDist) { mFarPlaneDist = in_FarPlaneDist; }

private:

	/**
	 * Helpers
	 */
	void SavePosition();
	void UpdateVelocity(float in_DeltaTime);
	void UpdateVelocityComponent(float& in_VelComponent, float in_PosComponent, float in_Decceleration, float in_PosMin, float in_PosMax, float in_DeltaTime);

	float GetPixelWorldConversionRatio()
	{
		return (mTanHalfFovy * mPosZ) / mViewportSizeX;
	}

	float GetPanDecceleration() 
	{ 
		return UserPreferences::Instance()->CameraVelocityDecay() * GetPixelWorldConversionRatio();
	}

	float GetZoomDecceleration() 
	{ 
		return UserPreferences::Instance()->CameraVelocityDecay() * 0.1f;
	}

private:

	// Viewport parameters
	int mViewportSizeX;
	int mViewportSizeY;

	// Perspective camera parameters
	float mTanHalfFovy;	// A commonly calculated value
	float mFovy;
	float mAspectRatio;
	float mNearPlaneDist;
	float mFarPlaneDist;

	// Positioning/Direction
	float mHomePosX, mHomePosY, mHomePosZ;
	float mPosX, mPosY, mPosZ;
	float mLookX, mLookY, mLookZ;
	float mUpX, mUpY, mUpZ;

	// Camera slew
	float mSlewX, mSlewY;
	float mSlewGoalX, mSlewGoalY;

	// Movement
	float mVelocityX, mVelocityY, mVelocityZ;	// Camera velocity
	float mMinBoundaryX, mMaxBoundaryX;			// Camera x boundaries
	float mMinBoundaryY, mMaxBoundaryY;			// Camera y boundaries
	float mMinBoundaryZ, mMaxBoundaryZ;			// Camera z boundaries

	// Animated movement
	float mMoveTime;							// Time remaining to perform the MoveTo
	float mMoveToX, mMoveToY, mMoveToZ;			// The MoveTo destination

	// Undo movement
	const unsigned mMinFramesForSavePosition;	// The minimum number of frames that must pass between AddVelocity calls to save camera position
	unsigned mFrameCount;						// The current number of frames that have passed since the last AddVelocity call
	stack<SavedPosition> mUndoStack;			// Saved move stack
};

#endif // CAMERA_H_
