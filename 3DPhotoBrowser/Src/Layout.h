/**
 * @file Layout.h
 * @brief Layout class header file
 */

#ifndef LAYOUT_H_
#define LAYOUT_H_

/**
 * Forwards
 */
class ImageContext;
class Camera;

/**
 * Layout
 * Interface used for organizing ImageTiles within an ImageContext
 */
class Layout
{
public:

	virtual ~Layout() {}
	virtual void DoLayout(ImageContext* in_ImageContext, Camera* in_Camera, bool in_CenterCamera) = 0;
	virtual const char* GetName() const = 0;

protected:

	/**
	 * Helper method for derived classes
	 */
	void UpdateCameraBoundaries(float in_SpanX, float in_SpanY, Camera* in_Camera, bool in_CenterCamera);
};

#endif // LAYOUT_H_
