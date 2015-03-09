/**
 * @file Util.cpp
 * @brief Utility routine implementations
 */

//-----------------------------------------------------------------------------------------------------------------------------
// HashString

unsigned HashString(const char* in_String)
{
	// djb2 hashing algorithm
	unsigned long l_Hash = 5381;
	while(int c = *in_String++) l_Hash = ((l_Hash << 5) + l_Hash) + c;
	return l_Hash;
}
