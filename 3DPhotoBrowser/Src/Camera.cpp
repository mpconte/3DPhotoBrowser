/**
 * @file Camera.cpp
 * @brief Camera implementation file
 */

#include "Camera.h"

//-----------------------------------------------------------------------------------------------------------------------------
// Camera

Camera::Camera()
: mViewportSizeX(640)
, mViewportSizeY(480)
, mFovy(90.0f)
, mAspectRatio((float)mViewportSizeX/(float)mViewportSizeY)
, mNearPlaneDist(0.1f)
, mFarPlaneDist(1000.0f)
, mHomePosX(0), mHomePosY(0), mHomePosZ(20)
, mPosX(0), mPosY(0), mPosZ(20)
, mLookX(0), mLookY(0), mLookZ(0)
, mUpX(0), mUpY(1), mUpZ(0)
, mSlewX(0), mSlewY(0)
, mSlewGoalX(0), mSlewGoalY(0)
, mVelocityX(0), mVelocityY(0), mVelocityZ(0)
, mMinBoundaryX(-1), mMaxBoundaryX(1)
, mMinBoundaryY(-1), mMaxBoundaryY(1)
, mMinBoundaryZ(mNearPlaneDist), mMaxBoundaryZ(500)
, mMoveTime(0)
, mMoveToX(0), mMoveToY(0), mMoveToZ(0)
, mMinFramesForSavePosition(5)
, mFrameCount(0)
{
	// Precalculate this value so we don't have to keep computing the tangent
	mTanHalfFovy = (float)tan(mFovy * 0.5f * DEG_TO_RAD);
}

//-----------------------------------------------------------------------------------------------------------------------------

Camera::~Camera()
{
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::GetVisibleWorldBounds(float& out_MinWorldX, float& out_MinWorldY, float& out_MaxWorldX, float& out_MaxWorldY)
{
	float l_HalfHeight = mTanHalfFovy * mPosZ;
	float l_HalfWidth = l_HalfHeight * mAspectRatio;
	
	out_MinWorldX = mPosX - l_HalfWidth;
	out_MinWorldY = mPosY - l_HalfHeight;
	out_MaxWorldX = mPosX + l_HalfWidth;
	out_MaxWorldY = mPosY + l_HalfHeight;
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::GetImagePlaneWorldPosition(float in_ScreenX, float in_ScreenY, float& out_WorldX, float& out_WorldY)
{
	// Get the world coordinates for the ray that is cast through these screen coordinates
	float l_FarX, l_FarY, l_FarZ;
	float l_NearX, l_NearY, l_NearZ;
	Graphics::Instance()->Unproject(in_ScreenX, in_ScreenY, 1, l_FarX, l_FarY, l_FarZ);
	Graphics::Instance()->Unproject(in_ScreenX, in_ScreenY, 0, l_NearX, l_NearY, l_NearZ);

	// Line equation
	// p = p1 + r * (p2 - p1)

	// We want to know xy value at z=0 (Solve for parameter r when z = 0)
	//	0 = l_FarZ + r * (l_NearZ - l_FarZ)
	//	r = -l_FarZ / (l_NearZ - l_FarZ)
	float l_R = -l_FarZ / (l_NearZ - l_FarZ);

	// Now using r, solve line equation for xy components
	out_WorldX = l_FarX + l_R * (l_NearX - l_FarX);
	out_WorldY = l_FarY + l_R * (l_NearY - l_FarY);
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::Swipe(float in_DeltaScreenX, float in_DeltaScreenY)
{
	float l_VisibleHeight = mTanHalfFovy * mPosZ * 2.0f;
	float l_VisibleWidth = l_VisibleHeight * mAspectRatio;

	mVelocityX += l_VisibleWidth * (in_DeltaScreenX / mViewportSizeX) * 15.0f;
	mVelocityY += l_VisibleHeight * (in_DeltaScreenY / mViewportSizeY) * 15.0f;

	// If velocity is being changed, check if we should save the camera position
	if(in_DeltaScreenX || in_DeltaScreenY)
	{
		// At least mMinFramesForSavePosition frames must have passed since the last AddVelocity call
		if(mFrameCount > mMinFramesForSavePosition)
		{
			SavePosition();
		}
		mFrameCount = 0;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::MoveTo(float in_PosX, float in_PosY, float in_PosZ, float in_Time)
{
	// Set the movement parameters. The camera is affected by these parameters in UpdateVelocity
	mMoveTime = in_Time;
	mMoveToX = in_PosX;
	mMoveToY = in_PosY;
	mMoveToZ = in_PosZ;

	// Save the camera position
	SavePosition();
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::MoveScreenDelta(float in_DeltaScreenX, float in_DeltaScreenY)
{
	float l_VisibleHeight = mTanHalfFovy * mPosZ * 2.0f;
	float l_VisibleWidth = l_VisibleHeight * mAspectRatio;

	// Stop velocity motion
	mVelocityX = mVelocityY = mVelocityZ = 0;

	// Convert the screen coorindate deltas into world coordinate deltas and update our position appropriately
	mLookX = mPosX += l_VisibleWidth * (in_DeltaScreenX / mViewportSizeX);
	mLookY = mPosY += l_VisibleHeight * (in_DeltaScreenY / mViewportSizeY);

	// If position is being changed, check if we should save the camera position
	if(in_DeltaScreenX || in_DeltaScreenY)
	{
		// At least mMinFramesForSavePosition frames must have passed since the last AddVelocity call
		if(mFrameCount > mMinFramesForSavePosition)
		{
			SavePosition();
		}
		mFrameCount = 0;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::MoveToHome()
{
	// Rapidly move to the home position 
	MoveTo(mHomePosX, mHomePosY, mHomePosZ, 1.0f);
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::ZoomScreenPoint(float in_ScreenX, float in_ScreenY, float in_ZoomAmount, float in_ZoomTime)
{
	// Determine the ray that is being cast into the world by this screen point
	float l_FarX, l_FarY, l_FarZ;
	float l_NearX, l_NearY, l_NearZ;
	Graphics::Instance()->Unproject(in_ScreenX, in_ScreenY, 1, l_FarX, l_FarY, l_FarZ);
	Graphics::Instance()->Unproject(in_ScreenX, in_ScreenY, 0, l_NearX, l_NearY, l_NearZ);

	// Move along this ray
	float l_DirX = l_FarX - l_NearX;
	float l_DirY = l_FarY - l_NearY;
	float l_DirZ = l_FarZ - l_NearZ;
	float l_Magnitude = in_ZoomAmount / fabs(l_DirZ);

	// Calculate the minimum zoom distance, it can't be less than the near plane distance or the image will get clipped
	float l_MinZoomDistance = max(UserPreferences::Instance()->ImageSize() * 0.5f / mTanHalfFovy, mNearPlaneDist);

	// If the zoom would cause us to get closer than the minimum zoom distance
	// we need to adjust the zoom magnitude
	if((mPosZ + l_DirZ * l_Magnitude) < l_MinZoomDistance)
	{
		// Solve:
		//	l_MinZoomDistance >= mPosZ + l_DirZ * l_Magnitude;
		// -l_DirZ * l_Magnitude >= mPosZ - l_MinZoomDistance;
		l_Magnitude = -(mPosZ - l_MinZoomDistance) / l_DirZ;
	}

	// Move towards the screen point along the casted ray
	MoveTo(	mPosX + l_DirX * l_Magnitude,
			mPosY + l_DirY * l_Magnitude,
			mPosZ + l_DirZ * l_Magnitude, in_ZoomTime );
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::ZoomScreenExtents(float in_MinX, float in_MinY, float in_MaxX, float in_MaxY)
{
	// Get the xy world coordinates that correspond to these screen coordinates
	float l_MinWorldX, l_MinWorldY, l_MaxWorldX, l_MaxWorldY;
	GetImagePlaneWorldPosition(in_MinX, in_MinY, l_MinWorldX, l_MinWorldY);
	GetImagePlaneWorldPosition(in_MaxX, in_MaxY, l_MaxWorldX, l_MaxWorldY);

	ZoomExtents(l_MinWorldX, l_MinWorldY, l_MaxWorldX, l_MaxWorldY);
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::ZoomExtents(float in_MinWorldX, float in_MinWorldY, float in_MaxWorldX, float in_MaxWorldY)
{
	// We want to zoom to the center of this rectangle
	float l_WorldX = (in_MinWorldX + in_MaxWorldX) * 0.5f;
	float l_WorldY = (in_MinWorldY + in_MaxWorldY) * 0.5f;
	float l_WorldZ = 0; // Calculated below

	// We want to choose the z distance such that the entire rectangle is visible in the viewport

	// We need to determine which dimension (x or y) is larger with respect to the camera's viewport
	float l_ExtentWidth = (in_MaxWorldX - in_MinWorldX);
	float l_ExtentHeight = (in_MaxWorldY - in_MinWorldY);
	float l_ExtentAspectRatio = l_ExtentWidth / l_ExtentHeight;
	
	// If the extent aspect ratio is bigger than our aspect ratio, it means the box is wider than us
	// Otherwise its taller than us. We use the half width of the aspect ratio corrected height, 
	// along with the camera fov to determine the z distance of the camera required to contain the box in the viewport
	if(l_ExtentAspectRatio > mAspectRatio)
	{
		// Convert the width to the equivalent height as the current aspect ratio
		l_WorldZ = (float)(l_ExtentWidth / mAspectRatio * 0.5f / mTanHalfFovy);
	}
	else
	{
		l_WorldZ = (float)(l_ExtentHeight * 0.5f / mTanHalfFovy);
	}

	// Move to the calculated world position
	MoveTo(l_WorldX, l_WorldY, l_WorldZ, 1.0f);
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::UndoLastMove()
{
	// If there are entries in the undo stack
	if(mUndoStack.size() > 0)
	{
		// Get the last saved position
		SavedPosition l_Undo = mUndoStack.top();
		mUndoStack.pop();

		// Move to it
		MoveTo(l_Undo.PosX, l_Undo.PosY, l_Undo.PosZ, 1.0f);

		// Moving generates another undo entry, so pop this one off as well
		mUndoStack.pop();
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::Apply()
{
	Graphics::Instance()->SetupProjectionMatrix(mFovy, mAspectRatio, max(mPosZ - 1, 0.001f), mPosZ + 1);
	Graphics::Instance()->SetupCamera(mPosX, mPosY, mPosZ,
									  mLookX, mLookY, mLookZ,
									  mUpX, mUpY, mUpZ);
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::Tick(float in_DeltaTime)
{
	// Update velocity
	UpdateVelocity(in_DeltaTime);

	// Update position based on current velocity
	mPosX += mVelocityX * in_DeltaTime;
	mPosY += mVelocityY * in_DeltaTime;
	mPosZ += mVelocityZ * in_DeltaTime;

	// Update look slew based on current velocity
	// The look position advances ahead of the camera position at higher velocities
	if(UserPreferences::Instance()->CameraSlewEnabled())
	{
		// Set the current slew goal
		mSlewGoalX = mVelocityX * UserPreferences::Instance()->CameraSlewScalar();
		mSlewGoalY = mVelocityY * UserPreferences::Instance()->CameraSlewScalar();

		// Converge to goal
		mSlewX += (mSlewGoalX - mSlewX) * 0.75f;
		mSlewY += (mSlewGoalY - mSlewY) * 0.75f;

		// Offset the look position from the current camera position in the xy plane
		// based on the slew amount
		mLookX = mPosX + mSlewX;
		mLookY = mPosY + mSlewY;
	}
	else
	{
		mLookX = mPosX;
		mLookY = mPosY;
	}

	// Increment the frame counter, while preventing overflow
	mFrameCount++;
	mFrameCount &= 0x1FFFFFFF;
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::ResizeViewport(int in_SizeX, int in_SizeY)
{
	// Viewport dimensions must be greater than 0
	mViewportSizeX = in_SizeX > 0 ? in_SizeX : 1;
	mViewportSizeY = in_SizeY > 0 ? in_SizeY : 1;

	// Update the aspect ratio
	mAspectRatio = (float)mViewportSizeX / mViewportSizeY;

	Graphics::Instance()->SetupViewport(in_SizeX, in_SizeY);
	Graphics::Instance()->SetupProjectionMatrix(mFovy, mAspectRatio, max(mPosZ - 1, 0.001f), mPosZ + 1);
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::SavePosition()
{
	// Add an entry to the undo stack
	SavedPosition l_Save;
	l_Save.PosX = mPosX;
	l_Save.PosY = mPosY;
	l_Save.PosZ = mPosZ;
	mUndoStack.push(l_Save);
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::UpdateVelocity(float in_DeltaTime)
{
	// Check if we have any animated movement to account for
	if(mMoveTime > 0)
	{
		// 1D acceleration formulae
		// v = 0

		// Formula 1:
		// v = u + a * t
		// a = (v - u) / t
		// a = -u / t

		// Formula 2:
		// v^2 = u^2 + 2 * a * s
		// 0 = u^2 + 2 * a * s
		// Substitute 1:
		// 0 = u^2 + 2 * (-u/t) * s
		// u^2 = -2 * (-u/t) * s

		// Result: u = -(2 * s) / t

		// Update the velocity to achieve the movement in the required time
		mVelocityX = -2 * (mPosX - mMoveToX) / mMoveTime;
		mVelocityY = -2 * (mPosY - mMoveToY) / mMoveTime;
		mVelocityZ = -2 * (mPosZ - mMoveToZ) / mMoveTime;

		// Decement the move time, once we have run out of movement time
		// Ensure we are at our destination
		mMoveTime = max(0, mMoveTime - in_DeltaTime);
		if(mMoveTime <= in_DeltaTime)
		{
			mMoveTime = 0;
			mPosX = mMoveToX;
			mPosY = mMoveToY;
			mPosZ = mMoveToZ;
			mVelocityX = mVelocityY = mVelocityZ = 0;
		}
	}
	// Update each velocity component normally
	else
	{
		UpdateVelocityComponent(mVelocityX, mPosX, GetPanDecceleration(), mMinBoundaryX, mMaxBoundaryX, in_DeltaTime);
		UpdateVelocityComponent(mVelocityY, mPosY, GetPanDecceleration(), mMinBoundaryY, mMaxBoundaryY, in_DeltaTime);
		UpdateVelocityComponent(mVelocityZ, mPosZ, GetZoomDecceleration(), mMinBoundaryZ, mMaxBoundaryZ, in_DeltaTime);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void Camera::UpdateVelocityComponent(float& in_VelComponent, float in_PosComponent, float in_Decceleration, float in_PosMin, float in_PosMax, float in_DeltaTime)
{
	// 1D acceleration formula
	// v^2 = u^2 + 2 * a * s
	// s = (v^2 - u^2) / ( 2 * a )
	// a = (v^2 - u^2) / ( 2 * s )	

	// Decay the the velocity component in 1D by the decceleration
	if(in_VelComponent > 0)
	{
		// Compute the camera's resting position at the current velocity and decceleration
		float l_RestingPosition = in_PosComponent + (in_VelComponent * in_VelComponent - 0) / ( 2 * in_Decceleration);

		// If our current resting position is past the lateral boundary,
		// we need to adjust our decceleration appropriately
		if(l_RestingPosition > in_PosMax)
		{
			in_Decceleration = (in_VelComponent * in_VelComponent - 0) / ( 2 * (in_PosMax - in_PosComponent) );

			// If the decceleration sign flips (i.e. we are trying to accelerate)
			// We must have fallen outside the camera boundary, in this case, do not allow motion in this direction
			if(in_Decceleration < 0)
			{
				in_VelComponent = 0;
				in_Decceleration = 0;
			}
		}

		// Prevent sign flipping in the velocity
		in_VelComponent = max(0, in_VelComponent - in_Decceleration * in_DeltaTime);
	}
	else if(in_VelComponent < 0)
	{
		// Compute the camera's resting position at the current velocity and decceleration
		float l_RestingPosition = in_PosComponent - (in_VelComponent * in_VelComponent - 0) / ( 2 * in_Decceleration);

		// If our current resting position is past the lateral boundary,
		// we need to adjust our decceleration appropriately
		if(l_RestingPosition < in_PosMin)
		{
			in_Decceleration = (in_VelComponent * in_VelComponent - 0) / ( 2 * (in_PosComponent - in_PosMin) );

			// If the decceleration sign flips (i.e. we are trying to accelerate)
			// We must have fallen outside the camera boundary, in this case, do not allow motion in this direction
			if(in_Decceleration < 0)
			{
				in_VelComponent = 0;
				in_Decceleration = 0;
			}
		}

		// Prevent sign flipping in the velocity
		in_VelComponent = min(0, in_VelComponent + in_Decceleration * in_DeltaTime);
	}
}
