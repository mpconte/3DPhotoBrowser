/**
 * @file UserPreferences.h
 * @brief UserPreferences class header file
 */

#ifndef USERPREFERENCES_H_
#define USERPREFERENCES_H_

/**
 * Forwards
 */
class UserPreferences;
class PreferenceData;

#define MIN_IMAGE_SIZE 0.1f
#define MAX_IMAGE_SIZE 25.0f

/**
 * PREFERENCE_LIST
 * Listing of all user preferences
 * -------------Display in UI---Type----Name--------------------Default--Description------
 */
#define PREFERENCE_LIST	\
	REGISTER_PREFERENCE(true,	bool,			EnableVerticalSync,			true,		"Enable VSync")				\
	REGISTER_PREFERENCE(true,	bool,			ShowFramerate,				false,		"Show Framrate")			\
	REGISTER_PREFERENCE(true,	int,			FramerateLimit,				60,			"Max Framerate")			\
	REGISTER_PREFERENCE(true,	LayoutIndex,	CurrentLayout,				0,			"Layout Type")				\
	REGISTER_PREFERENCE(true,	bool,			LayoutImageFollowMode,		false,		"Layout Image Follow Mode")	\
	REGISTER_PREFERENCE(true,	float,			CalendarRowPitch,			15.0f,		"Calendar - Row Pitch")		\
	REGISTER_PREFERENCE(true,	float,			CalendarColPitch,			1.0f,		"Calendar - Column Pitch")	\
	REGISTER_PREFERENCE(true,	float,			MonthPadding,				1.0f,		"Calendar - Month Padding")	\
	REGISTER_PREFERENCE(true,	float,			YearPadding,				1.0f,		"Calendar - Year Padding")	\
	REGISTER_PREFERENCE(true,	float,			CompactRowPitch,			1.0f,		"Compact - Row Pitch")		\
	REGISTER_PREFERENCE(true,	float,			CompactColPitch,			1.0f,		"Compact - Column Pitch")	\
	REGISTER_PREFERENCE(true,	float,			CompactDayPadding,			1.0f,		"Compact - Day Padding")	\
	REGISTER_PREFERENCE(true,	float,			CompactYearPadding,			1.0f,		"Compact - Year Padding")	\
	REGISTER_PREFERENCE(true,	int,			CompactRowCount,			1,			"Compact - Row count")		\
	REGISTER_PREFERENCE(true,	FloatSlider,	ImageSize,					1.0f,		"Image Tile Size")			\
	REGISTER_PREFERENCE(true,	float,			ImageMoveTime,				1.0f,		"Image Tile Move Time")		\
	REGISTER_PREFERENCE(true,	float,			CameraSlewScalar,			0.1f,		"Camera Slew Scalar")		\
	REGISTER_PREFERENCE(true,	bool,			CameraSlewEnabled,			false,		"Camera Slew Enabled")		\
	REGISTER_PREFERENCE(true,	float,			CameraVelocityDecay,		200.0f,		"Camera Pan Decceleration")	\
	REGISTER_PREFERENCE(true,	float,			CameraZoomSensitivity,		1.0f,		"Wheel Zoom Sensitivity")	\
	REGISTER_PREFERENCE(true,	float,			CameraZoomWheelFactor,		2.718f,		"Wheel Zoom Factor")		\
	REGISTER_PREFERENCE(true,	float,			CameraZoomWheelTime,		0.25f,		"Wheel Zoom Time")			\
	REGISTER_PREFERENCE(true,	float,			CameraZoomMagnification,	100.0f,		"Click Zoom Magnification")	\
	REGISTER_PREFERENCE(true,	float,			CameraZoomTime,				0.5f,		"Click Zoom Time")			\
	REGISTER_PREFERENCE(true,	bool,			SaveCameraPosition,			false,		"Save Current View")		\
	REGISTER_PREFERENCE(false,	float,			SavedCameraX,				0.0f,		"Saved Camera PosX")		\
	REGISTER_PREFERENCE(false,	float,			SavedCameraY,				0.0f,		"Saved Camera PosY")		\
	REGISTER_PREFERENCE(false,	float,			SavedCameraZ,				0.0f,		"Saved Camera PosZ")		\

/**
 * LayoutIndex
 * Typedef used for the unique layout enumeration combo box
 */
typedef int LayoutIndex;

/**
 * FloatSlider
 * Typedef to support different floating point UI item
 */
typedef float FloatSlider;

/**
 * PreferenceType
 * Valid preferences data types
 */
enum PreferenceType
{
	PreferenceType_int,
	PreferenceType_bool,
	PreferenceType_float,
	PreferenceType_LayoutIndex,
	PreferenceType_FloatSlider,
};

// Declare an enum of each preference for id purposes
#define REGISTER_PREFERENCE(displayUI, type, name, val, desc) name,

	enum PreferenceId
	{
		PREFERENCE_LIST
	};

#undef REGISTER_PREFERENCE

/**
 * PreferenceData
 * Metadata structure for a user preference
 */
class PreferenceData
{
public:

	PreferenceData(UserPreferences* in_PrefRef, PreferenceId in_Id, void* in_DataRef, PreferenceType in_DataType, const char* in_DataDesc)
		: mPreferences(in_PrefRef), mId(in_Id), mDataRef(in_DataRef), mDataDesc(in_DataDesc), mDataType(in_DataType) {}

	/**
	 * Get
	 * Get the data reference as some data type. Care should be taken to only use the correct template
	 * for the data reference type.
	 */
	template <class T>
	const T& Get() { return *(T*)mDataRef; }
	
	/**
	 * Set
	 * Set the data reference equal to some data. When data is set it triggers a user preference update event
	 * Care should be taken to only use the correct template for the data reference type.
	 */
	template <class T>
	void Set(const T& in_Data)
	{
		if(*(T*)mDataRef != in_Data)
		{
			*(T*)mDataRef = in_Data;
			mPreferences->FireUserPreferenceUpdateEvent();
		}
	}

	/**
	 * GetId
	 * Get the id of this preference
	 */
	PreferenceId GetId() { return mId; }

	/**
	 * GetDescription
	 * Get the description for this preference
	 */
	const char* GetDescription() { return mDataDesc; }

	/**
	 * GetType
	 * Get the data type for this preference
	 */
	PreferenceType GetType() { return mDataType; }

private:

	UserPreferences* mPreferences;
	PreferenceId mId;
	void* mDataRef;
	const char* mDataDesc;
	PreferenceType mDataType;
};

/**
 * UserPreferenceListener
 * Listen for changes to user preferences
 */
class UserPreferenceListener
{
public:

	virtual void OnUserPreferenceUpdate() = 0;
};

/**
 * UserPreferences
 * Set of user preferences for the application. This class stores metadata regarding
 * the preferences as well to accomodate the use by a GUI system. This class is
 * implemented as a singleton so that the user preferences can be accessed from any
 * context
 */
class UserPreferences
{
public:

	/**
	 * Singleton access
	 */
	static UserPreferences* Instance() { static UserPreferences l_Instance; return &l_Instance; }

	/**
	 * AddUserPreferenceListener
	 * Add a listener for user preference changes
	 */
	void AddUserPreferenceListener(UserPreferenceListener* in_Listener)
	{
		mListeners.push_back(in_Listener);
	}

	/**
	 * GetPreferenceData
	 * Get a reference to the preference metadata
	 */
	vector<PreferenceData>& GetPreferenceData() { return mPreferenceData; }

	// Declare the preferences as class members and provide accessors and mutators appropriately
#define REGISTER_PREFERENCE(displayUI, type, name, val, desc)	\
	private:													\
		type m##name;											\
	public:														\
		const type& name() const { return m##name; }			\
		void name(const type& in)								\
		{														\
			if(m##name != in)									\
			{													\
				m##name = in;									\
				FireUserPreferenceUpdateEvent();				\
			}													\
		}

	PREFERENCE_LIST

#undef REGISTER_PREFERENCE

protected:

	/**
	 * PreferenceData must be able to call FireUserPreferenceUpdateEvent
	 */
	friend class PreferenceData;

	/**
	 * GetHash
	 * Returns the hash code for this set of user preferences
	 */
	unsigned GetHash()
	{
#define REGISTER_PREFERENCE(displayUI, type, name, val, desc) #type#name
		static unsigned l_Hash = HashString(PREFERENCE_LIST);
#undef REGISTER_PREFERENCE

		return l_Hash;
	}

	/**
	 * LoadPreferences
	 * Loads preferences from disk
	 */
	void LoadPreferences();

	/**
	 * SavePreferences
	 * Saves preferences to disk
	 */
	void SavePreferences();

	/**
	 * FireUserPreferenceUpdateEvent
	 * Notify listeners that one of the user preferences has changed
	 */
	void FireUserPreferenceUpdateEvent()
	{
		for(unsigned i = 0; i < mListeners.size(); i++)
			mListeners[i]->OnUserPreferenceUpdate();
	}

	/**
	 * Constructor
	 * Initialize all preferences and track their associated metadata
	 */
	UserPreferences()
	{
		// Initialize the preference default values
#define REGISTER_PREFERENCE(displayUI, type, name, val, desc) m##name = val;
		PREFERENCE_LIST
#undef REGISTER_PREFERENCE

		// Add the metadata to the PreferenceList
#define REGISTER_PREFERENCE(displayUI, type, name, val, desc) if(displayUI) mPreferenceData.push_back(PreferenceData(this, ::name, (void*)&m##name, PreferenceType_##type, desc));
		PREFERENCE_LIST
#undef REGISTER_PREFERENCE

		// Load preferences from disk
		LoadPreferences();
	}

	/**
	 * Destructor
	 * Save preferences to disk
	 */
	~UserPreferences()
	{
		SavePreferences();
	}

	/**
	 * Singleton implementation
	 */
	UserPreferences(const UserPreferences&);
	const UserPreferences& operator=(const UserPreferences&);

private:

	// List list of preference metadata
	vector<PreferenceData> mPreferenceData;

	// List of user preference listeners
	vector<UserPreferenceListener*> mListeners;
};

#endif // USERPREFERENCES_H_