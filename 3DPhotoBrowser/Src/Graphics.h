/**
 * @file Graphics.h
 * @brief Graphics interface header file
 */

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include "Global.h"

/**
 * TextureHandle
 * Handle to a graphics texture resource
 */
typedef unsigned TextureHandle;

/**
 * TextureFormat
 * Supported texture formats
 */
enum TextureFormat
{
	TextureFormat_RGB,
	TextureFormat_RGBA
};

/**
 * Graphics
 * The 3d graphics interface. Derived classes implement this interface then Configure
 * this singleton for use.
 */
class Graphics
{
public:

	/**
	 * Instance
	 * Singleton access
	 */
	static Graphics* Instance()
	{ 
		return sInstance;
	}

	/**
	 * Destroy
	 * Singleton teardown
	 */
	static void Destroy()
	{
		if(sInstance)
		{
			sInstance->Shutdown();
			delete sInstance;
		}
	}

	/**
	 * ConfigureRenderer
	 * Used to configure the singleton instance
	 */
	template <class T> static void ConfigureRenderer()
	{
		Destroy();
		sInstance = new T();
		sInstance->Init();
	}

	/**
	 * Virtual destructor
	 */
	virtual ~Graphics() {}

	/**
	 * Init
	 * Initialize the graphics system
	 */
	virtual void Init() = 0;

	/**
	 * Shutdown
	 * Shutdown the graphics system
	 */
	virtual void Shutdown() = 0;

	/**
	 * GetLastError
	 * Get the last error that occurred within the graphics system
	 */
	virtual bool GetLastError(string& out_Error) = 0;

	/**
	 * SetupViewport
	 * Setup the rendering viewport
	 */
	virtual void SetupViewport(int in_SizeX, int in_SizeY) = 0;

	/**
	 * SetupProjectionMatrix
	 * Setup the rendering projection matrix
	 */
	virtual void SetupProjectionMatrix(float in_FOV, float in_AspectRatio, float in_ClipNear, float in_ClipFar) = 0;

	/**
	 * SetupCamera
	 * Setup the camera position, looking position, and up direction
	 */
	virtual void SetupCamera(float in_EyeX, float in_EyeY, float in_EyeZ,
							 float in_LookAtX, float in_LookAtY, float in_LookAtZ,
							 float in_UpX, float in_UpY, float in_UpZ) = 0;

	/**
	 * Unproject
	 * Get the world coordinates for a set of screen coordinates
	 */
	virtual void Unproject(float in_ScreenX, float in_ScreenY, float in_ScreenZ, float& out_WorldX, float& out_WorldY, float& out_WorldZ) = 0;

	/**
	 * CreateTexture
	 * Create a handle to a texture resource based on the passed in data pixels
	 */
	virtual TextureHandle CreateTexture(int in_Width, int in_Height, TextureFormat in_Format, const unsigned char* in_Pixels) = 0;

	/**
	 * BindTexture
	 * Bind a texture for rendering
	 */
	virtual void BindTexture(TextureHandle in_Handle) = 0;

	/**
	 * FreeTexture
	 * Free a previously created texture handle
	 */
	virtual void FreeTexture(TextureHandle in_Handle) = 0;

	/**
	 * DrawQuadOutline2D
	 * Draw a 2D wireframe quad at the specified screen coordinates
	 */
	virtual void DrawQuadOutline2D(float in_ScreenX, float in_ScreenY,
								   float in_ColorR, float in_ColorG, float in_ColorB, 
								   float in_Width, float in_Height) = 0;


	 /**
	 * DrawQuadOutline
	 * Draw a wireframe quad at the plane co-ordinates
	 */
	virtual void DrawQuadOutline(float in_CenterX, float in_CenterY, float in_CenterZ,
						  float in_ColorR, float in_ColorG, float in_ColorB, 
						  float in_Width, float in_Height) = 0;


	/**
	 * DrawQuad
	 * Draw a single quad. Quads drawn in this way will be in the xy plane
	 */
	virtual void DrawQuad(float in_CenterX, float in_CenterY, float in_CenterZ,
						  float in_ColorR, float in_ColorG, float in_ColorB, 
						  float in_Width, float in_Height) = 0;

	/**
	 * DrawQuads
	 * Draw a set of quads.
	 * For each vertex there should be 3 entries in the in_Verticies array and 2 entries in the in_TexCoords array
	 * There must be exactly 4 verticies specified per quad
	 * e.g. If in_NumQuads == 4, then in_Verticies should contain 4 * 4 * 3 elements and in_TexCoods should contain 4 * 4 * 2 elements
	 */
	virtual void DrawQuads(float* in_Verticies, float* in_TexCoords, unsigned short* in_Indicies, unsigned in_NumQuads) = 0;

	/**
	 * ClearBuffers
	 * Clear the rendering back buffers
	 */
	virtual void ClearBuffers() = 0;

	/**
	 * Flush
	 * When this method returns, all graphics calls must be complete
	 */
	virtual void Flush() = 0;

protected:

	/**
	 * Singleton implementation
	 */
	Graphics() {}
	Graphics(const Graphics&);
	const Graphics& operator=(const Graphics&);

	static Graphics* sInstance;
};

#endif // GRAPHICS_H_
