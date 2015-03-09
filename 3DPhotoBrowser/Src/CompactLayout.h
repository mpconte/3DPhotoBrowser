/**
 * @file CompactLayout.h
 * @brief CompactLayout class header file
 */

#ifndef COMPACTLAYOUT_H_
#define COMPACTLAYOUT_H_

#include "Layout.h"

/**
 * CompactLayout
 * Layout implementation for a calendar like layout
 */
class CompactLayout : public Layout
{
public:

	/**
	 * Layout interface
	 */
	virtual void DoLayout(ImageContext* in_ImageContext, Camera* in_Camera, bool in_CenterCamera);
	virtual const char* GetName() const { return "Compact"; }
};

#endif // COMPACTLAYOUT_H_
