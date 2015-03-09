/**
 * @file CalendarLayout.h
 * @brief CalendarLayout class header file
 */

#ifndef CALENDARLAYOUT_H_
#define CALENDARLAYOUT_H_

#include "Layout.h"

/**
 * CalendarLayout
 * Layout implementation for a calendar like layout
 */
class CalendarLayout : public Layout
{
public:

	/**
	 * Layout interface
	 */
	virtual void DoLayout(ImageContext* in_ImageContext, Camera* in_Camera, bool in_CenterCamera);
	virtual const char* GetName() const { return "Calendar"; }

protected:

	/**
	 * GetMonth
	 * Returns the month index (1 for january 12 for december) for the specified day
	 */
	int GetMonth(int in_Day);
};

#endif // CALENDARLAYOUT_H_
