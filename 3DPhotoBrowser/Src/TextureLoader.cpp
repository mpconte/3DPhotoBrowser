/**
 * @file TextureLoader.cpp
 * @brief TextureLoader implementation file
 */

#include "TextureLoader.h"
#include "PhotoBrowser.h"
#include "IL/il.h"

//-----------------------------------------------------------------------------------------------------------------------------
// TextureLoader

TextureLoader::TextureLoader()
: mThreadStarted(false)
, mThreadDone(false)
, mStopThread(false)
{
#if USE_THREADED_TEXTURE_LOADING
	StartThread();
#endif // USE_THREADED_TEXTURE_LOADING
}

//-----------------------------------------------------------------------------------------------------------------------------

TextureLoader::~TextureLoader()
{
}

//-----------------------------------------------------------------------------------------------------------------------------

void TextureLoader::Shutdown()
{
#if USE_THREADED_TEXTURE_LOADING
	StopThread();
#endif // USE_THREADED_TEXTURE_LOADING
	mRequestQueue.empty();
}

//-----------------------------------------------------------------------------------------------------------------------------

void TextureLoader::StartThread()
{
	Start(true);
	mThreadStarted = true;

}

//-----------------------------------------------------------------------------------------------------------------------------

void TextureLoader::StopThread()
{
	// Terminate the load thread routine
	mStopThread = true;
	Resume(); // Resume the thread so it can exit
	while(mThreadStarted && !mThreadDone)
	{
		Sleep(0); // Yield timeslice
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

TextureHandle TextureLoader::LoadTexture(const char* in_Filename, unsigned in_TextureOffset, unsigned in_TextureSize)
{
	TextureHandle l_TextureHandle = NULL;

	// Try to open the thumbnail file
	ifstream l_File;
	l_File.open(in_Filename, ios_base::in | ios_base::binary);
	if(l_File.fail())
	{
		logf("Failed to open thumbnail file '%s'", in_Filename);
		return l_TextureHandle;
	}

	// Create a temporary buffer to store the thumbnail image data
	unsigned l_DataSize = in_TextureSize;
	char* l_Data = new char[l_DataSize];

	// Seek to where the thumbnail data begins, and read the data into our temporary buffer
	l_File.seekg(in_TextureOffset);
	l_File.read(l_Data, l_DataSize);
	l_File.close();

	// Get DevIL to handle the dirty image loading details
	unsigned l_ImageHandle;
	ilGenImages(1, &l_ImageHandle);
	ilBindImage(l_ImageHandle);
	if(ilLoadL(IL_TYPE_UNKNOWN, l_Data, l_DataSize))
	{
		int l_Width = ilGetInteger(IL_IMAGE_WIDTH);
		int l_Height = ilGetInteger(IL_IMAGE_HEIGHT);
		int l_Format = ilGetInteger(IL_IMAGE_FORMAT);

		switch(l_Format)
		{
		case IL_RGB: l_Format = TextureFormat_RGB; break;
		case IL_RGBA: l_Format = TextureFormat_RGBA; break;
		default: logf("Warning: unrecognized texture format");
		}

		// Create the graphics texture resource
		l_TextureHandle = Graphics::Instance()->CreateTexture(
			l_Width, l_Height, TextureFormat(l_Format), ilGetData());

		// Wait until the texture is fully loaded into graphics memory
		Graphics::Instance()->Flush();
	}
	else
	{
		ILenum l_Error =  ilGetError();
		logf("Failed to load texture: ilError=%d", l_Error);
	}

	// Free DevIL resources
	ilDeleteImages(1, &l_ImageHandle);

	// Release the temporary buffer
	delete [] l_Data;

	return l_TextureHandle;
}

//-----------------------------------------------------------------------------------------------------------------------------

void TextureLoader::LoadTexture(const char* in_Filename, unsigned in_TextureOffset, unsigned in_TextureSize, TextureLoaderListener* in_Listener, void* in_UserData)
{
	// If we are using a loading thread, queue some work for it to do
	if(mThreadStarted)
	{
		mQueueLock.Lock();

		// Queue the request data
		RequestData l_Data;
		l_Data.UserData = in_UserData;
		l_Data.Filename = in_Filename;
		l_Data.Listener = in_Listener;
		l_Data.TextureSize = in_TextureSize;
		l_Data.TextureOffset = in_TextureOffset;
		mRequestQueue.push(l_Data);

		mQueueLock.Unlock();

		// Resume the worker thread
		Resume();
	}
	// If not, do a synchronous load right away instead
	else
	{
		// Load the texture then let the listener know
		in_Listener->OnLoadComplete(
			LoadTexture(in_Filename, in_TextureOffset, in_TextureSize),
			in_UserData);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void TextureLoader::Run()
{
	PhotoBrowser::Instance()->AcquireOpenGLWorkerContext();

	RequestData l_RequestData;

	// While we should keep working
	while(!mStopThread)
	{
		RequestData* l_Request = NULL;

		// Check if there is anything in the queue
		mQueueLock.Lock();
		if(mRequestQueue.size() > 0)
		{
			l_RequestData = mRequestQueue.front();
			l_Request = &l_RequestData;
			mRequestQueue.pop();
		}
		mQueueLock.Unlock();

		// If there is a request, process it
		if(l_Request)
		{
			// Load the texture
			TextureHandle l_Handle = LoadTexture(l_Request->Filename.c_str(), l_Request->TextureOffset, l_Request->TextureSize);

			// Notify the requestor that the texture is ready
			l_Request->Listener->OnLoadComplete(l_Handle, l_Request->UserData);

			Sleep(1); // Yield some time before processing next load
		}
		// Nothing to do, suspend ourself. Main thread will wake us up when it adds something to the queue
		else
		{
			Suspend();
		}
	}

	PhotoBrowser::Instance()->ReleaseOpenGLWorkerContext();

	// We are now done
	mThreadDone = true;
}
