/**
 * @file Util.h
 * @brief Utility routines
 */

#ifndef UTIL_H_
#define UTIL_H_

/**
 * Clamp
 * Clamp a value between a range
 */
template <class T>
T Clamp(T& in_Clamped, const T& in_Min, const T& in_Max)
{
	if(in_Clamped < in_Min)
	{
		return in_Clamped = in_Min;
	}
	if(in_Clamped > in_Max)
	{
		return in_Clamped = in_Max;
	}
	return in_Clamped;
}

/**
 * HashString
 * Get the hash code for a string value
 */
unsigned HashString(const char* in_String);

#endif // UTIL_H_
