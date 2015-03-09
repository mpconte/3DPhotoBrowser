/**
 * @file CalendarLayout.cpp
 * @brief CalendarLayout implementation file
 */

#include "Global.h"
#include "Camera.h"
#include "ImageContext.h"
#include "CalendarLayout.h"

//-----------------------------------------------------------------------------------------------------------------------------
// CalendarLayout

void CalendarLayout::DoLayout(ImageContext* in_ImageContext, Camera* in_Camera, bool in_CenterCamera)
{
	UserPreferences* l_Prefs = UserPreferences::Instance();
	unsigned l_ImageCount = in_ImageContext->GetImageCount();

	// Get range boundary variables from the images context
	int l_MinDay = in_ImageContext->GetDayMinimum();
	int l_MaxDay = in_ImageContext->GetDayMaximum();
	int l_MinTime = in_ImageContext->GetTimeMinimum();
	int l_MaxTime = in_ImageContext->GetTimeMaximum();
	int l_MinYear = in_ImageContext->GetYearMinimum();
	int l_MaxYear = in_ImageContext->GetYearMaximum();

	// Precompute the half total span in xy
	float l_HalfSpanX = ( l_Prefs->CalendarColPitch() * (l_MaxDay - l_MinDay + 1) + l_Prefs->MonthPadding() * 11 ) * 0.5f;
	float l_HalfSpanY = ( l_Prefs->CalendarRowPitch() * (l_MaxYear - l_MinYear + 1) + l_Prefs->YearPadding() * (l_MaxYear - l_MinYear) ) * 0.5f;

	// Update the size and position of each image tile
	for(unsigned i = 0; i < l_ImageCount; i++)
	{
		ImageTile* l_Tile = in_ImageContext->GetImage(i);

		// Get the date information for the image
		unsigned l_Day;
		unsigned l_Time;
		unsigned l_Year;
		l_Tile->GetTimeStamp(l_Time, l_Day, l_Year);

		// Precalculate some values
		float l_TimeOfDayPercent = (float)(l_Time - l_MinTime) / (l_MaxTime - l_MinTime);
		float l_RelativeYear = (float)(l_MaxYear - l_Year);

		// Place the photo according to its date
		float l_X = (l_Day - l_MinDay) * l_Prefs->CalendarColPitch() +		// The day column
					(GetMonth(l_Day)-1) * l_Prefs->MonthPadding();	// The month padding

		float l_Y = ( l_RelativeYear +								// The year (earlier years start higher)
			          l_TimeOfDayPercent ) * l_Prefs->CalendarRowPitch() +	// The time of day within the year row
					  l_RelativeYear * l_Prefs->YearPadding();		// The year padding

		// To prevent z-fighting of overlapping images, distribute images throughout z coordinate
		// The z-position is determined by the time of day
		float l_Z = l_Prefs->ImageSize() * 0.02f * (l_TimeOfDayPercent - 0.5f);
		
		// Have the image plane centered at 0,0
		l_X = l_X + l_Prefs->ImageSize() * 0.5f - l_HalfSpanX;
		l_Y = l_Y - l_HalfSpanY;

		// Set the image position and size
		l_Tile->MoveTo(l_X, l_Y, l_Z);
		l_Tile->SetSize(l_Prefs->ImageSize(), l_Prefs->ImageSize());
	}

	// Update the camera boundaries
	UpdateCameraBoundaries(l_HalfSpanX * 2, l_HalfSpanY * 2, in_Camera, in_CenterCamera);
}

//-----------------------------------------------------------------------------------------------------------------------------

int CalendarLayout::GetMonth(int in_Day)
{
	static int l_DaysPerMonth[12] =
	{
		31,	// Jan
		29,	// Feb - assume always 29 days in feb
		31, // Mar
		30, // Apr
		31, // May
		30, // Jun
		31, // Jul
		31, // Aug
		30, // Sep
		31, // Oct
		30, // Nov
		31  // Dec
	};

	// Subtract days per month from the input day until we go negative
	// As soon as we do, that index+1 was the month
	int i = 0;
	for(i = 0; i < 12 && in_Day > 0; i++)
	{
		in_Day -= l_DaysPerMonth[i];
	}

	// Index is incremented before breaking the for loop, so i >= 1 && i <= 12
	return i;
}
