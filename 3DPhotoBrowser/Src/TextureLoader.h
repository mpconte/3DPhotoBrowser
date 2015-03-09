/**
 * @file TextureLoader.h
 * @brief TextureLoader class header file
 */

#ifndef TEXTURELOADER_H_
#define TEXTURELOADER_H_

#include "Global.h"
#include "Thread.h"
#include "Semaphore.h"

/**
 * TextureLoaderListener
 * Interface for objects to receive texture loader events
 */
class TextureLoaderListener
{
public:

	/**
	 * OnLoadComplete
	 * When a texture load completes, this method is called on the listener that requested the load
	 */
	virtual void OnLoadComplete(TextureHandle in_Handle, void* in_UserData) = 0;
};

/**
 * TextureLoader
 * Singleton used to manage asynchronous texture load requests
 */
class TextureLoader : public Thread
{
	/**
	 * RequestData
	 * Supporting structure used to queue load requests
	 */
	struct RequestData
	{
		TextureLoaderListener* Listener;
		void* UserData;
		string Filename;
		unsigned TextureOffset;
		unsigned TextureSize;
	};

public:

	/**
	 * Instance
	 * Singleton access
	 */
	static TextureLoader* Instance() { static TextureLoader l_Instance; return &l_Instance; }

	/**
	 * Shutdown
	 * Perform cleanup
	 */
	void Shutdown();

	/**
	 * StartThread
	 * Start the worker thread
	 */
	void StartThread();

	/**
	 * StopThread
	 * Stop the worker thread
	 */
	void StopThread();

	/**
	 * LoadTexture
	 * Synchronous texture load
	 */
	TextureHandle LoadTexture(const char* in_Filename, unsigned in_TextureOffset, unsigned in_TextureSize);

	/**
	 * LoadTexture
	 * Request an asynchronous texture load
	 */
	void LoadTexture(const char* in_Filename, unsigned in_TextureOffset, unsigned in_TextureSize, TextureLoaderListener* in_Listener, void* in_UserData);

	/**
	 * Thread interface
	 */
	virtual void Run();

private:

	bool mThreadStarted;
	bool mThreadDone;
	bool mStopThread;

	Semaphore mQueueLock;
	queue<RequestData> mRequestQueue;

	/**
	 * Singleton implementation
	 */
	TextureLoader();
	~TextureLoader();
	TextureLoader(const TextureLoader&);
	const TextureLoader& operator=(const TextureLoader&);
};

#endif // TEXTURELOADER_H_
