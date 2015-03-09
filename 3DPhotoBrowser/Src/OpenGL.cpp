/**
 * @file OpenGL.cpp
 * @brief OpenGL graphics implementation file
 */

#include "OpenGL.h"
#include <GL/GL.h>
#include <GL/GLU.h>

// Debugging
#ifdef DEBUG
	#define CHECK_ERRORS { if(glGetError() != GL_NO_ERROR) { assert(false); } }
#else
	#define CHECK_ERRORS
#endif // DEBUG

//-----------------------------------------------------------------------------------------------------------------------------
// OpenGL

void OpenGL::Init()
{
	// Black background
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// Use depth testing (needed for Unproject)
	glEnable(GL_DEPTH_TEST);

	// Use 2d texturing
	glEnable(GL_TEXTURE_2D);

	// Use backface culling
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	// Use vertex arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	CHECK_ERRORS;
}

//-----------------------------------------------------------------------------------------------------------------------------

void OpenGL::Shutdown()
{
}

//-----------------------------------------------------------------------------------------------------------------------------

bool OpenGL::GetLastError(string& out_Error)
{
	// Get the readable error message
	GLenum l_Error = glGetError();
	if(l_Error != GL_NO_ERROR)
	{
		const char* l_CStr = (const char*)gluErrorString(l_Error);
		if(l_CStr)
		{
			out_Error += l_CStr;
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------------------------------------------------------

void OpenGL::SetupViewport(int in_SizeX, int in_SizeY)
{
	glViewport(0, 0, in_SizeX, in_SizeY);
	CHECK_ERRORS;
}

//-----------------------------------------------------------------------------------------------------------------------------

void OpenGL::SetupProjectionMatrix(float in_FOV, float in_AspectRatio, float in_ClipNear, float in_ClipFar)
{
	glPushAttrib(GL_TRANSFORM_BIT);		// Save the current matrix mode

		glMatrixMode(GL_PROJECTION);	// Switch to the projection matrix

		// Load in the new perspective matrix
		glLoadIdentity();
		gluPerspective(in_FOV, in_AspectRatio, in_ClipNear, in_ClipFar);

	glPopAttrib();						// Restore the matrix mode
	CHECK_ERRORS;
}

//-----------------------------------------------------------------------------------------------------------------------------

void OpenGL::SetupCamera(float in_EyeX, float in_EyeY, float in_EyeZ, float in_LookAtX, float in_LookAtY, float in_LookAtZ, float in_UpX, float in_UpY, float in_UpZ)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(in_EyeX, in_EyeY, in_EyeZ, in_LookAtX, in_LookAtY, in_LookAtZ, in_UpX, in_UpY, in_UpZ);
	CHECK_ERRORS;
}

//-----------------------------------------------------------------------------------------------------------------------------

void OpenGL::Unproject(float in_ScreenX, float in_ScreenY, float in_ScreenZ, float& out_WorldX, float& out_WorldY, float& out_WorldZ)
{
	GLint l_ViewportMatrix[4];
	GLdouble l_ModelviewMatrix[16];
	GLdouble l_ProjectionMatrix[16];
	GLdouble l_WorldX, l_WorldY, l_WorldZ;

	glGetDoublev(GL_MODELVIEW_MATRIX, l_ModelviewMatrix );		// Get the modelview matrix
	glGetDoublev(GL_PROJECTION_MATRIX, l_ProjectionMatrix );	// Get the projection matrix
	glGetIntegerv(GL_VIEWPORT, l_ViewportMatrix );				// Get the viewport matrix

	gluUnProject(in_ScreenX, in_ScreenY, in_ScreenZ,						// Screen coordinates
				 l_ModelviewMatrix, l_ProjectionMatrix, l_ViewportMatrix,	// Matricies
				 &l_WorldX, &l_WorldY, &l_WorldZ);							// Output world coordinates

	// Return the world coorindates
	out_WorldX = (float)l_WorldX;
	out_WorldY = (float)l_WorldY;
	out_WorldZ = (float)l_WorldZ;

	CHECK_ERRORS;
}

//-----------------------------------------------------------------------------------------------------------------------------

TextureHandle OpenGL::CreateTexture(int in_Width, int in_Height, TextureFormat in_Format, const unsigned char* in_Pixels)
{
	TextureHandle l_TextureHandle;

	// Generate space for the texture
	glGenTextures(1, &l_TextureHandle);

	// Bind the new texture and use linear filtering
	glBindTexture(GL_TEXTURE_2D, l_TextureHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Clamp textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Fill texture memory
	int l_BytesPerPixel = 0;
	int l_GLFormat = 0;
	switch(in_Format)
	{
	case TextureFormat_RGB:

		l_BytesPerPixel = 3;
		l_GLFormat = GL_RGB;
		break;

	case TextureFormat_RGBA:

		l_BytesPerPixel = 4;
		l_GLFormat = GL_RGBA;
		break;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, l_BytesPerPixel, in_Width, in_Height, 0, l_GLFormat, GL_UNSIGNED_BYTE, in_Pixels);

	CHECK_ERRORS;
	return l_TextureHandle;
}

//-----------------------------------------------------------------------------------------------------------------------------

void OpenGL::BindTexture(TextureHandle in_Handle)
{
	glBindTexture(GL_TEXTURE_2D, in_Handle);
	CHECK_ERRORS;
}

//-----------------------------------------------------------------------------------------------------------------------------

void OpenGL::FreeTexture(TextureHandle in_Handle)
{
	glDeleteTextures(1, &in_Handle);
	CHECK_ERRORS;
}

//-----------------------------------------------------------------------------------------------------------------------------

void OpenGL::DrawQuadOutline2D(float in_ScreenX, float in_ScreenY, float in_ColorR, float in_ColorG, float in_ColorB, float in_Width, float in_Height)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);	// We are going to change a bunch of attributes, so save them all
	
		glMatrixMode(GL_PROJECTION);	// Switch to the projection matrix
		glPushMatrix();					// Save the projection matrix

		// Get the current viewport matrix
		GLint l_Viewport[4];					
		glGetIntegerv(GL_VIEWPORT, l_Viewport);

		// Load in a 2d orthographic projection matrix matching the viewport size
		glLoadIdentity();
		gluOrtho2D(0, l_Viewport[2], 0, l_Viewport[3]);

		glMatrixMode(GL_MODELVIEW);		// Switch to the modelview matrix

			glPushMatrix();						// Save the modelview matrix
			glLoadIdentity();					// Clear the modelview matrix
			glPolygonMode(GL_FRONT, GL_LINE);	// Draw outline only
			glDisable(GL_TEXTURE_2D);			// Do not texture the outline

			// Draw the quad
			// We need to adjust the in_ScreenX, in_ScreenY to be at the center of the quad
			DrawQuad(in_ScreenX + in_Width * 0.5f, in_ScreenY + in_Height * 0.5f, 0.0f,
					 in_ColorR, in_ColorG, in_ColorB,
					 in_Width, in_Height);

			glPopMatrix();						// Restore the modelview matrix

		glMatrixMode(GL_PROJECTION);	// Switch to the projection matrix
		glPopMatrix();					// Restore the projection matrix

	glPopAttrib();						// Restore previous OpenGL attribute states
}

//-----------------------------------------------------------------------------------------------------------------------------

void OpenGL::DrawQuad(float in_CenterX, float in_CenterY, float in_CenterZ, float in_ColorR, float in_ColorG, float in_ColorB, float in_Width, float in_Height)
{
	float l_HalfWidth = in_Width * 0.5f;
	float l_HalfHeight = in_Height * 0.5f;
	float l_Right = l_HalfWidth + in_CenterX;
	float l_Left = -l_HalfWidth + in_CenterX;
	float l_Top = l_HalfHeight + in_CenterY;
	float l_Bottom = -l_HalfHeight + in_CenterY;


	glColor3f(in_ColorR, in_ColorG, in_ColorB);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 1); glVertex3f(l_Left,  l_Top,    in_CenterZ); // Top left
		glTexCoord2f(0, 0); glVertex3f(l_Left,  l_Bottom, in_CenterZ); // Bottom left
		glTexCoord2f(1, 0); glVertex3f(l_Right, l_Bottom, in_CenterZ); // Bottom right
		glTexCoord2f(1, 1); glVertex3f(l_Right, l_Top,    in_CenterZ); // Top right
	glEnd();	
}

//NEW/MATTHEW
void OpenGL::DrawQuadOutline(float in_CenterX, float in_CenterY, float in_CenterZ, float in_ColorR, float in_ColorG, float in_ColorB, float in_Width, float in_Height)
{
	float l_HalfWidth = in_Width * 0.5f;
	float l_HalfHeight = in_Height * 0.5f;
	float l_Right = l_HalfWidth + in_CenterX;
	float l_Left = -l_HalfWidth + in_CenterX;
	float l_Top = l_HalfHeight + in_CenterY;
	float l_Bottom = -l_HalfHeight + in_CenterY;

	glPushAttrib(GL_ALL_ATTRIB_BITS);   // Save Attributes

	glPolygonMode(GL_FRONT, GL_LINE);	// Draw outline only
	glDisable(GL_TEXTURE_2D);			// Do not texture the outline

	glColor3f(in_ColorR, in_ColorG, in_ColorB);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 1); glVertex3f(l_Left,  l_Top,    in_CenterZ); // Top left
		glTexCoord2f(0, 0); glVertex3f(l_Left,  l_Bottom, in_CenterZ); // Bottom left
		glTexCoord2f(1, 0); glVertex3f(l_Right, l_Bottom, in_CenterZ); // Bottom right
		glTexCoord2f(1, 1); glVertex3f(l_Right, l_Top,    in_CenterZ); // Top right
	glEnd();

	glPopAttrib();						// Restore previous OpenGL Attributes
}
//-----------------------------------------------------------------------------------------------------------------------------

void OpenGL::DrawQuads(float* in_Verticies, float* in_TexCoords, unsigned short* in_Indicies, unsigned in_NumQuads)
{
	glVertexPointer(4, GL_FLOAT, 0, in_Verticies);
	glTexCoordPointer(2, GL_FLOAT, 0, in_TexCoords);
	glDrawElements(GL_QUADS, in_NumQuads, GL_UNSIGNED_SHORT, in_Indicies);
}

//-----------------------------------------------------------------------------------------------------------------------------

void OpenGL::ClearBuffers()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CHECK_ERRORS;
}

//-----------------------------------------------------------------------------------------------------------------------------

void OpenGL::Flush()
{
	glFlush();
	CHECK_ERRORS;
}
