/**
 * @file ImageTile.h
 * @brief ImageTile class header file
 */

#ifndef IMAGETILE_H_
#define IMAGETILE_H_

#include "Global.h"
#include "TextureLoader.h"

/**
 * Forwards
 */
class ImageContext;

/**
 * ThumbnailSize
 * Supported thumbnail sizes
 */
enum ThumbnailSize
{
	ThumbnailSize_None = -1,
	ThumbnailSize_32x32,
	ThumbnailSize_64x64,
	ThumbnailSize_128x128,
	ThumbnailSize_256x256,
	ThumbnailSize_512x512,
	ThumbnailSize_1024x1024,
	ThumbnailSize_MAX,
};

/**
 * ImageTile
 * Exclusively constructed by the ImageContext singleton. ImageTiles represent the visible quads in the PhotoBrowser
 */
class ImageTile : public TextureLoaderListener
{
	/**
	 * ThumbnailInfo
	 * Supporting structure used to store information required to load a thumbnails on the fly
	 */
	struct ThumbnailInfo 
	{
		/**
		 * Constructor
		 * Default initialization
		 */
		ThumbnailInfo()
			: Offset(0), Size(0), TexHandle(0), LoadPending(false) {}

		string Filename;			// Of the form data/thumbnail32/container00000.dat
		unsigned Offset;			// Offset into the file (in bytes) where the thumbnail begins
		unsigned Size;				// Size of thumbnail data (in bytes) within Filename, beginning at Offset
	
		TextureHandle TexHandle;	// The graphics texture handle
		bool LoadPending;			// Does this thumbnail already have a load pending?
	};

public:

	/**
	 * GetAspectRatio
	 * Returns the native aspect ratio of the image
	 */
	float GetAspectRatio() const { return mAspectRatio; }

	/**
	 * GetAverageColor
	 * Returns the average RGB color of the image
	 */
	void GetAverageColor(float& out_Red, float& out_Green, float& out_Blue) const
	{
		out_Red = mAverageRed;
		out_Green = mAverageGreen;
		out_Blue = mAverageBlue;
	}

	/**
	 * GetTimeStamp
	 * Returns the date this image was created
	 */
	void GetTimeStamp(unsigned& out_TimeOfDay, unsigned& out_DayOfYear, unsigned& out_Year) const
	{
		out_TimeOfDay = mTimeOfDay;
		out_DayOfYear = mDayOfYear;
		out_Year = mYear;
	}

	/**
	 * GetPosition
	 * Get the position of this image tile
	 */
	void GetPosition(float& out_PosX, float& out_PosY)
	{
		out_PosX = mPosX;
		out_PosY = mPosY;
	}


	/** NEW/MATTHEW
	* GetSize
	* Get the height/width of the image tile
	**/
	void GetSize(float& out_SizeX, float& out_SizeY)
	{
		out_SizeX = mSizeX;
		out_SizeY = mSizeY;
	}

	/**
	 * GetMoveToGoalPosition
	 * Get the goal position of this image tile
	 */
	void GetMoveToGoalPosition(float& out_PosX, float& out_PosY, float& out_PosZ)
	{
		out_PosX = mMoveToGoalX;
		out_PosY = mMoveToGoalY;
		out_PosZ = mMoveToGoalZ;
	}

	/**
	 * SetSize
	 * Set the size of this image tile
	 */
	void SetSize(float in_SizeX, float in_SizeY)
	{
		mSizeX = in_SizeX;
		mSizeY = in_SizeY;
	}

	/**
	 * SetPosition
	 * Set the position of this image tile
	 */
	void SetPosition(float in_PosX, float in_PosY, float in_PosZ)
	{
		mPosX = in_PosX;
		mPosY = in_PosY;
		mPosZ = in_PosZ;
	}

	/**
	 * MoveTo
	 */
	void MoveTo(float in_PosX, float in_PosY, float in_PosZ)
	{
		mMoveToStartX = mPosX;
		mMoveToStartY = mPosY;
		mMoveToStartZ = mPosZ;
		mMoveToGoalX = in_PosX;
		mMoveToGoalY = in_PosY;
		mMoveToGoalZ = in_PosZ;
		mMoveTotalTime = mMoveTime = UserPreferences::Instance()->ImageMoveTime();
	}

	/**
	 * ActivateThumbnail
	 * Load and activate a specified thumbnail texture
	 * If the specified thumbnail has no info, this function has no effect
	 */
	void ActivateThumbnail(ThumbnailSize in_ThumbnailSize);

	/**
	 * Draw
	 * Draw this ImageTile
	 */
	void Draw();


	/** NEW/MATTHEW
	* Outline
	* Outline Image with green wireframe quad
	*/
	bool Outline(float mouse_x, float mouse_y);

	/**
	 * Tick
	 * Frame update
	 */
	void Tick(float in_DeltaTime);

	/**
	 * TextureLoaderListener interface
	 */
	void OnLoadComplete(TextureHandle in_Handle, void* in_UserData);

private:

	/**
	 * AddThumbnailInfo
	 * Add thumbnail info to this ImageTile. This method is used exclusively by the ImageContext singleton
	 * during context initialization
	 */
	void AddThumbnailInfo(ThumbnailSize in_ThumbSize, const string& in_Filename, unsigned in_Offset, unsigned in_Size);

private:

	float mMoveToStartX, mMoveToStartY, mMoveToStartZ;	// Image move to position
	float mMoveToGoalX, mMoveToGoalY, mMoveToGoalZ;		// Image move to position
	float mMoveTotalTime;								// Time to move from the start to goal position
	float mMoveTime;									// Time remaining to move from the start to goal position

	float mPosX, mPosY, mPosZ;						// Image draw position
	float mSizeX, mSizeY;							// Image draw position
	ThumbnailInfo* mActiveThumbnail;				// The thumbnail actively being used for rendering

	// Static image data
	float mAspectRatio;								// Native image aspect ratio
	unsigned mTimeOfDay;							// Time of day: Units: 1/1000 seconds
	short mDayOfYear;								// Day of year: 1-366 (includes leap year day)
	short mYear;									// Year
	float mAverageRed, mAverageGreen, mAverageBlue;	// Average image color (0.0f - 1.0f)

	// Thumbnail info for this ImageTile. If a pointer in this array in NULL, it indicates
	// that the associated Thumbnail size doesn't exist for this image
	ThumbnailInfo mThumbnailInfo[ThumbnailSize_MAX];

	/**
	 * Only the ImageContext singleton may construct objects of this type
	 */
	friend class ImageContext;

	ImageTile();
	~ImageTile();
	ImageTile(const ImageTile&);
	const ImageTile& operator=(const ImageTile&);
};

#endif // IMAGETILE_H_
