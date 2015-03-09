/**
 * @file OpenGL.h
 * @brief OpenGL graphics class file
 */

#ifndef OPENGL_H_
#define OPENGL_H_

#include "Graphics.h"

/**
 * OpenGL
 * The OpenGL Graphics interface implementation
 */
class OpenGL : public Graphics
{
	/**
	 * Graphics interface
	 */
	virtual void Init();
	virtual void Shutdown();

	virtual bool GetLastError(string& out_Error);

	virtual void SetupViewport(int in_SizeX, int in_SizeY);
	virtual void SetupProjectionMatrix(float in_FOV, float in_AspectRatio, float in_ClipNear, float in_ClipFar);
	virtual void SetupCamera(float in_EyeX, float in_EyeY, float in_EyeZ,
							 float in_LookAtX, float in_LookAtY, float in_LookAtZ,
							 float in_UpX, float in_UpY, float in_UpZ);

	virtual void Unproject(float in_ScreenX, float in_ScreenY, float in_ScreenZ, float& out_WorldX, float& out_WorldY, float& out_WorldZ);

	virtual TextureHandle CreateTexture(int in_Width, int in_Height, TextureFormat in_Format, const unsigned char* in_Pixels);
	virtual void BindTexture(TextureHandle in_Handle);
	virtual void FreeTexture(TextureHandle in_Handle);

	virtual void DrawQuadOutline2D(float in_ScreenX, float in_ScreenY,
								   float in_ColorR, float in_ColorG, float in_ColorB,
								   float in_Width, float in_Height);

	virtual void DrawQuad(float in_CenterX, float in_CenterY, float in_CenterZ,
						  float in_ColorR, float in_ColorG, float in_ColorB, 
						  float in_Width, float in_Height);

	//NEW/MATTHEW
	virtual void DrawQuadOutline(float in_CenterX, float in_CenterY, float in_CenterZ,
						  float in_ColorR, float in_ColorG, float in_ColorB, 
						  float in_Width, float in_Height);
	//
	virtual void DrawQuads(float* in_Verticies, float* in_TexCoords, unsigned short* in_Indicies, unsigned in_NumQuads);

	virtual void ClearBuffers();
	virtual void Flush();
};

#endif // OPENGL_H_

