/**
 * @file UserPreferences.cpp
 * @brief UserPreferences implementation file
 */

#include "Global.h"
#include "UserPreferences.h"

#define PREFERENCE_FILENAME "data/UserPreferences.dat"

//-----------------------------------------------------------------------------------------------------------------------------
// UserPreferences

void UserPreferences::LoadPreferences()
{
	ifstream l_File;

	// Try to open the preferences file
	l_File.open(PREFERENCE_FILENAME);
	if(l_File.fail())
	{
		return;
	}

	// Get the file size
	l_File.seekg(0, ios::end);
	unsigned l_FileSize = l_File.tellg();
	l_File.seekg(0, ios::beg);

	// Get the hash code from the file
	unsigned l_Hash;
	l_File >> l_Hash;

	// Only load the properties if the hash code matches
	if(l_Hash == GetHash())
	{
#define REGISTER_PREFERENCE(displayUI, type, name, val, desc) l_File.ignore(l_FileSize, '='); l_File >> m##name;
		PREFERENCE_LIST
#undef REGISTER_PREFERENCE
	}

	l_File.close();
}

//-----------------------------------------------------------------------------------------------------------------------------

void UserPreferences::SavePreferences()
{
	ofstream l_File;

	// Try to open the preferences file
	l_File.open(PREFERENCE_FILENAME);
	if(l_File.fail())
	{
		return;
	}

	// Write the hash code first
	l_File << GetHash() << endl;

	// Write each property
#define REGISTER_PREFERENCE(displayUI, type, name, val, desc) l_File << #name << "=" << m##name << endl;
	PREFERENCE_LIST
#undef REGISTER_PREFERENCE

	l_File.close();
}