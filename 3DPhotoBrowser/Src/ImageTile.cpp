/**
 * @file ImageTile.cpp
 * @brief ImageTile implementation file
 */

#include "ImageTile.h"
#include "TextureLoader.h"

//-----------------------------------------------------------------------------------------------------------------------------
// ImageTile

ImageTile::ImageTile()
: mMoveToStartX(0), mMoveToStartY(0), mMoveToStartZ(0)
, mMoveToGoalX(0), mMoveToGoalY(0), mMoveToGoalZ(0)
, mMoveTime(0), mMoveTotalTime(0)
, mPosX(0), mPosY(0), mPosZ(0)
, mSizeX(1), mSizeY(1)
, mAspectRatio(1)
, mTimeOfDay(0)
, mDayOfYear(0)
, mYear(0)
, mAverageRed(1), mAverageGreen(1), mAverageBlue(1)
, mActiveThumbnail(0)
{
	memset(mThumbnailInfo, 0, sizeof(mThumbnailInfo));
}

//-----------------------------------------------------------------------------------------------------------------------------

ImageTile::~ImageTile()
{
}

//-----------------------------------------------------------------------------------------------------------------------------

void ImageTile::Draw()
{
	// Assume we are going to use the average image color for rendering
	float l_ColorRed = mAverageRed;
	float l_ColorGreen = mAverageGreen;
	float l_ColorBlue = mAverageBlue;
	
	// If there is an activated thumbnail with a loaded texture, use it
	if(mActiveThumbnail && mActiveThumbnail->TexHandle)
	{
		// Bind the texture
		Graphics::Instance()->BindTexture(mActiveThumbnail->TexHandle);

		// Render white so we don't tint the texture
		l_ColorRed = l_ColorGreen = l_ColorBlue = 1.0f;
	}
	else
	{
		// No texture
		Graphics::Instance()->BindTexture(NULL);
	}

	// Draw the quad
	Graphics::Instance()->DrawQuad
	(
		mPosX, mPosY, mPosZ,					// Position in xy-plane
		l_ColorRed, l_ColorGreen, l_ColorBlue,	// Color
		mSizeX, mSizeY							// Width/Height
	);
}


void ImageTile::Tick(float in_DeltaTime)
{
	if(mMoveTime > 0)
	{
		mMoveTime = max(mMoveTime - in_DeltaTime, 0);
		float l_InterpPct = 1.0f - mMoveTime / mMoveTotalTime;

		// Interpolate to the goal position
		mPosX = mMoveToStartX + (mMoveToGoalX - mMoveToStartX) * l_InterpPct;
		mPosY = mMoveToStartY + (mMoveToGoalY - mMoveToStartY) * l_InterpPct;
		mPosZ = mMoveToStartZ + (mMoveToGoalZ - mMoveToStartZ) * l_InterpPct;
	}
}


//NEW/MATTHEW
// Outline image in green if mouse cursor is over it
bool ImageTile::Outline(float mouse_x, float mouse_y)
{
	float l_FarX, l_FarY, l_FarZ;
	float l_NearX, l_NearY, l_NearZ;
	Graphics::Instance()->Unproject(mouse_x, mouse_y, 1, l_FarX, l_FarY, l_FarZ);
	Graphics::Instance()->Unproject(mouse_x, mouse_y, 0, l_NearX, l_NearY, l_NearZ);

	// Line equation
	// p = p1 + r * (p2 - p1)

	// We want to know xy value at z=0 (Solve for parameter r when z = 0)
	//	0 = l_FarZ + r * (l_NearZ - l_FarZ)
	//	r = -l_FarZ / (l_NearZ - l_FarZ)
	float l_R = -l_FarZ / (l_NearZ - l_FarZ);

	// Now using r, solve line equation for xy components
	float out_WorldX = l_FarX + l_R * (l_NearX - l_FarX);
	float out_WorldY = l_FarY + l_R * (l_NearY - l_FarY);


	if (out_WorldX >= mPosX - (mSizeX * 0.5) && out_WorldX <= mPosX + (mSizeX * 0.5)
	&&  out_WorldY >= mPosY - (mSizeY * 0.5) && out_WorldY <= mPosY + (mSizeY * 0.5))
	{
		Graphics::Instance()->DrawQuadOutline(mPosX, mPosY, mPosZ, 
												0.0, 1.0, 0.0, 
												mSizeX, mSizeY);
		return true;
	}
	else
		return false;
}

//-----------------------------------------------------------------------------------------------------------------------------

void ImageTile::OnLoadComplete(TextureHandle in_Handle, void* in_UserData)
{
	ThumbnailInfo* l_Info = (ThumbnailInfo*)in_UserData;

	// Make this loaded thumbnail active
	l_Info->TexHandle = in_Handle;
	mActiveThumbnail = l_Info;
}

//-----------------------------------------------------------------------------------------------------------------------------

void ImageTile::ActivateThumbnail(ThumbnailSize in_ThumbnailSize)
{
	// Get the thumbnail being requested
	ThumbnailInfo* l_Info = in_ThumbnailSize > ThumbnailSize_None ? &mThumbnailInfo[in_ThumbnailSize] : NULL;

	// If we are already using this thumbnail, there is nothing to do
	if(mActiveThumbnail == l_Info)
	{
		return;
	}

	// If we don't have the texture loaded, request an async load for it
	if(l_Info && !l_Info->TexHandle && !l_Info->LoadPending)
	{
#if USE_THREADED_TEXTURE_LOADING
		l_Info->LoadPending = true;
		TextureLoader::Instance()->LoadTexture(l_Info->Filename.c_str(), l_Info->Offset, l_Info->Size, this, (void*)l_Info);
#else
		l_Info->TexHandle = TextureLoader::Instance()->LoadTexture(l_Info->Filename.c_str(), l_Info->Offset, l_Info->Size);
		mActiveThumbnail = l_Info;
#endif // USE_THREADED_TEXTURE_LOADING
	}
	// Otherwise we already have the texture, so just switch the thumbnail
	else
	{
		mActiveThumbnail = l_Info;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void ImageTile::AddThumbnailInfo(ThumbnailSize in_ThumbSize, const string& in_Filename, unsigned in_Offset, unsigned in_Size)
{
	// Add the new info
	ThumbnailInfo* l_Info = &mThumbnailInfo[in_ThumbSize];
	l_Info->Filename = in_Filename;
	l_Info->Offset = in_Offset;
	l_Info->Size = in_Size;
}
