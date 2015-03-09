/**
 * @file ImageContext.h
 * @brief ImageContext class header file
 */

#ifndef IMAGECONTEXT_H_
#define IMAGECONTEXT_H_

#include "Global.h"
#include "ImageTile.h"

/**
 * Forwards
 */
class ImageTile;

/**
 * ImageContext
 * The image context is used to load and manage ImageTile objects.
 * Loading the ImageTiles into the image context does NOT load textures,
 * it only loads the image meta data and constructs the associated ImageTile object.
 */
class ImageContext
{
public:

	/**
	 * Singleton access
	 */
	static ImageContext* Instance() { static ImageContext l_Instance; return &l_Instance; }

	/**
	 * CreateContext
	 * Load and create an ImageTile for each image listed in the data/photo_index.dat file
	 */
	bool CreateContext();

	/**
	 * DestroyContext
	 * Free all ImageTiles
	 */
	bool DestroyContext();

	/**
	 * GetImageCount
	 * Returns the number of currently loaded ImageTiles
	 */
	unsigned GetImageCount() { return mImageTileCount; }

	/**
	 * GetImage
	 * Return the ImageTile at the specified index
	 */
	ImageTile* GetImage(unsigned in_Index) { assert(in_Index < GetImageCount()); return &mImageTiles[in_Index]; }

	/**
	 * GetYearMinimum
	 * Get the minimum year for the year range among all images in this context
	 */
	int GetYearMinimum() { return mMinYear; }

	/**
	 * GetYearMaximum
	 * Get the maximum year for the year range among all images in this context
	 */
	int GetYearMaximum() { return mMaxYear; }

	/**
	 * GetDayMinimum
	 * Get the minimum day for the day range among all images in this context
	 */
	int GetDayMinimum() { return 1; }

	/**
	 * GetDayMaximum
	 * Get the maximum day for the day range among all images in this context
	 */
	int GetDayMaximum() { return 366; }

	/**
	 * GetTimeMinimum
	 * Get the minimum time for the time range among all images in this context
	 */
	int GetTimeMinimum() { return 0; }

	/**
	 * GetTimeMaximum
	 * Get the maximum time for the time range among all images in this context
	 */
	int GetTimeMaximum() { return 24 * 60 * 60 * 1000; } // milliseconds

private:

	int mMinYear;
	int mMaxYear;

	ImageTile* mImageTiles;		// List of image tiles
	unsigned mImageTileCount;	// Number of image tiles

	/**
	 * Singleton implementation
	 */
	ImageContext();
	ImageContext(const ImageContext&);
	const ImageContext& operator=(const ImageContext&);
};

#endif // IMAGECONTEXT_H_
