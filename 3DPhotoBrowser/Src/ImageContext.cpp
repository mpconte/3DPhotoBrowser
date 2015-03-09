/**
 * @file ImageContext.cpp
 * @brief ImageContext class implementation file
 */

#include "ImageContext.h"
#include "ImageTile.h"

//-----------------------------------------------------------------------------------------------------------------------------
// ImageContext

ImageContext::ImageContext()
: mMinYear( 0x7FFFFFFF ) // Max positive 32 bit signed int value
, mMaxYear( 0x80000000 ) // Max negative 32 bit signed int value
, mImageTiles(NULL)
, mImageTileCount(0)
{}

//-----------------------------------------------------------------------------------------------------------------------------

bool ImageContext::CreateContext()
{
	// Open the photo index file
	ifstream l_File;
	l_File.open("data/photo_index.dat", ios_base::in | ios_base::binary);

	// Make sure the file opened correctly
	if(l_File.fail())
	{
		logf("Failed to open photo index file");
		return false;
	}

	// Read the number of images
	l_File.read((char*)&mImageTileCount, 4);
	mImageTiles = new ImageTile[mImageTileCount];

	// @TODO - this is an unsafe solution
	// This structure is a mirror of what is used by the photo indexing tool when writing data to the index file.
	// NOTE: Should the photo index tool change the way it writes to the index file this structure MUST be updated
	const int MAX_THUMBNAILS = 6;
	struct ImageData
	{
		int				Width;
		int				Height;
		unsigned		TimeOfDay;
		short			DayOfYear;
		short			Year;
		unsigned char	AverageRed;
		unsigned char	AverageGreen;
		unsigned char	AverageBlue;
		int				FolderIndex;
		char			Filename[256];

		struct
		{
			unsigned ThumbFileOffset;
			unsigned ThumbContainerIndex; // Lower 3 bits are the thumbnail level, remaining bits are the file number
			unsigned ThumbImageSize;

		} Thumbnails[MAX_THUMBNAILS];
	};

	// The way sprintf is used here is perfectly safe
#ifdef WIN32
	#pragma warning (push)
	#pragma warning (disable: 4996)
#endif // WIN32

	// For each image in the index file
	char l_Buff[64]; // Temporary buffer
	for(unsigned i = 0; i < mImageTileCount; i++)
	{
		// Read the data for each image
		ImageData l_Data;
		l_File.read((char*)&l_Data, sizeof(ImageData));

		// Update the min/max year
		if(l_Data.Year < mMinYear) mMinYear = l_Data.Year;
		if(l_Data.Year > mMaxYear) mMaxYear = l_Data.Year;

		// Copy only the data we care about to our ImageTile class
		ImageTile* l_Tile = &mImageTiles[i];
		l_Tile->mAspectRatio = (float)l_Data.Width / l_Data.Height;
		l_Tile->mTimeOfDay = l_Data.TimeOfDay;
		l_Tile->mDayOfYear = l_Data.DayOfYear;
		l_Tile->mYear = l_Data.Year;
		l_Tile->mAverageRed = l_Data.AverageRed / 256.0f;		// Convert byte to 0.0 - 1.0 range
		l_Tile->mAverageGreen = l_Data.AverageGreen / 256.0f;	// Convert byte to 0.0 - 1.0 range
		l_Tile->mAverageBlue = l_Data.AverageBlue / 256.0f;		// Convert byte to 0.0 - 1.0 range

		// For each thumbnail, add a thumbnail record to the ImageTile
		for(unsigned i = 0; i < MAX_THUMBNAILS; i++)
		{
			// If the thumbnail image size is zero, then there is no thumbnail info at this index
			if(l_Data.Thumbnails[i].ThumbImageSize <= 0)
			{
				continue;
			}

			// The ThumbContainerIndex is bit packed to store two values:
			//	- The lower 3 bits are the thumbnail size index
			//	- The remaining bits are the container file index
			unsigned l_ThumbnailSizeIndex = l_Data.Thumbnails[i].ThumbContainerIndex & 0x07;
			unsigned l_ThumbnailContainerIndex = l_Data.Thumbnails[i].ThumbContainerIndex >> 3;

			// The ThumbnailSizeIndex has the following meaning
			// Beginning at 1024x1024, each time the index increases, the power of two is decreased
			//	i.e. 0 -> 1024x1024, 1 -> 512x512 ... 5 -> 32x32
			// Remap this value to our enum
			ThumbnailSize l_ThumbnailSize = ThumbnailSize_MAX;
			switch(l_ThumbnailSizeIndex)
			{
			case 0:	l_ThumbnailSize = ThumbnailSize_1024x1024; break;
			case 1: l_ThumbnailSize = ThumbnailSize_512x512; break;
			case 2: l_ThumbnailSize = ThumbnailSize_256x256; break;
			case 3: l_ThumbnailSize = ThumbnailSize_128x128; break;
			case 4: l_ThumbnailSize = ThumbnailSize_64x64; break;
			case 5: l_ThumbnailSize = ThumbnailSize_32x32; break;
			default: assert(false && "Unsupported thumbnail size index");
			}

			// The thumbnails are stored in subdirectories of the data folder
			// They are of the form: data/thumbnail32/container00000.dat
			// We precalculate the folder name here to save the ImageTile from doing this work later
			sprintf(l_Buff, "data/thumbnails%d/container%05d.dat", 32 << l_ThumbnailSize, l_ThumbnailContainerIndex);

			// Add this information to the ImageTile.
			// It can later use this information to load the thumbnail on the fly
			l_Tile->AddThumbnailInfo(l_ThumbnailSize, l_Buff,
				l_Data.Thumbnails[i].ThumbFileOffset, l_Data.Thumbnails[i].ThumbImageSize);
		}
	}

#ifdef WIN32
	#pragma warning (pop)
#endif // WIN32

	// Close the file
	l_File.close();

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------

bool ImageContext::DestroyContext()
{
	// Delete dynamically allocated ImageTiles
	delete [] mImageTiles;

	return true;
}
