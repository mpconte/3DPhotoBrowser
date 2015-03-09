/**
 * @file CompactLayout.cpp
 * @brief CompactLayout implementation file
 */

#include "Global.h"
#include "Camera.h"
#include "ImageContext.h"
#include "CompactLayout.h"

//-----------------------------------------------------------------------------------------------------------------------------
// CompactLayout

void CompactLayout::DoLayout(ImageContext* in_ImageContext, Camera* in_Camera, bool in_CenterCamera)
{
	UserPreferences* l_Prefs = UserPreferences::Instance();
	unsigned l_ImageCount = in_ImageContext->GetImageCount();
	int l_MinYear = in_ImageContext->GetYearMinimum();

	// NOTE: This code assumes that the index file is sorted in ascending order according to date.
	// Therefore, it is expected that each successive image in the image context is older than the previous.

	float l_MaxImageX = 0;			// The maximum image x position
	float l_MaxImageY = 0;			// The maximum image y position
	float l_YearStartY = 0;			// The starting y offset for the current year
	float l_DayStartX = 0;			// The starting x offset for the current day
	unsigned l_MaxSubrows = 0;		// The max number of subrows for this year
	unsigned l_DayImageCount = 0;	// The current number of images for the current day

	// Prime the l_PrevDay variable
	ImageTile* l_Tile = in_ImageContext->GetImage(0);
	unsigned l_PrevTime, l_PrevDay, l_PrevYear;
	l_Tile->GetTimeStamp(l_PrevTime, l_PrevDay, l_PrevYear);

	// Update the size and position of each image tile
	for(unsigned i = 0; i < l_ImageCount; i++)
	{
		ImageTile* l_Tile = in_ImageContext->GetImage(i);

		// Get the date information for the image
		unsigned l_Time, l_Day, l_Year;
		l_Tile->GetTimeStamp(l_Time, l_Day, l_Year);

		// Did the day change?
		if(l_Day != l_PrevDay)
		{
			// Move the x position to the end of this day
			l_DayStartX += l_Prefs->CompactColPitch() * ceil((float)l_DayImageCount / l_Prefs->CompactRowCount());

			// Add the padding
			l_DayStartX += l_Prefs->CompactDayPadding();

			// Reset for next day
			l_PrevDay = l_Day;		// Update the previous day
			l_DayImageCount = 0;	// Reset the image count for the new day
		}

		// Did the year change?
		if(l_Year != l_PrevYear)
		{
			// Move the position to the end of this year row
			l_YearStartY -= (l_MaxSubrows+1) * l_Prefs->CompactRowPitch();

			// Add the year padding
			l_YearStartY -= l_Prefs->CompactYearPadding();

			// Reset for next year
			l_PrevYear = l_Year;	// Update the previous year
			l_DayStartX = 0;		// Reset the starting x position
			l_DayImageCount = 0;	// Reset the image count for the next year
			l_MaxSubrows = 0;		// Reset the max number of sub rows for the next year
		}

		// Precalculate
		unsigned l_RelativeYear = l_Year - l_MinYear;
		unsigned l_ImageSubColumn = (l_DayImageCount / l_Prefs->CompactRowCount());
		unsigned l_ImageSubRow = l_DayImageCount % l_Prefs->CompactRowCount();

		// Compute the image position
		float l_X = l_DayStartX +									// The day columnstarting x offset
					l_ImageSubColumn * l_Prefs->CompactColPitch();	// The x offset into the day column

		float l_Y = l_YearStartY -									// The year starting point
					l_ImageSubRow * l_Prefs->CompactRowPitch();		// The offset into the year row 

		// Update maximums
		l_MaxSubrows = max(l_MaxSubrows, l_ImageSubRow);
		l_MaxImageX = max(l_MaxImageX, l_X);
		l_MaxImageY = max(l_MaxImageY, -l_Y);

		// Update the size and position of this tile
		l_Tile->MoveTo(l_X, l_Y, 0.0f);
		l_Tile->SetSize(l_Prefs->ImageSize(), l_Prefs->ImageSize());

		// Increment the number of images for this day
		l_DayImageCount++;
	}

	// Center all photos about the origin
	float l_HalfSpanX = (l_MaxImageX + l_Prefs->ImageSize()) * 0.5f;
	float l_HalfSpanY = (l_MaxImageY + l_Prefs->ImageSize()) * 0.5f;
	float l_HalfImageSize = l_Prefs->ImageSize() * 0.5f;
	for(unsigned i = 0; i < l_ImageCount; i++)
	{
		ImageTile* l_Tile = in_ImageContext->GetImage(i);

		float l_PosX, l_PosY, l_PosZ;
		l_Tile->GetMoveToGoalPosition(l_PosX, l_PosY, l_PosZ);
		l_Tile->MoveTo(l_PosX - l_HalfSpanX + l_HalfImageSize, l_PosY + l_HalfSpanY - l_HalfImageSize, 0.0f);
	}	

	// Update the camera boundaries
	UpdateCameraBoundaries(l_HalfSpanX * 2, l_HalfSpanY * 2, in_Camera, in_CenterCamera);
}
