#pragma once

#include <string>
#include <math.h>

//
// Suite of generalise functions, should be useful for generic and game programs.
//

//-------------------------------------------------------------------------------
// Mathematical Functions
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// Manipulations
//-------------------------------------------------------------------------------

/*
	Given a filename that might have a path and an extension, we need to be able to pull
	the path bit and the file extension bit out
*/
void StripPathAndExtension(std::string& fileName, std::string* pPath = nullptr, std::string* pExt = nullptr);

//wstring to string conversion
std::string WStringToString(const std::wstring& s);
//string to wstring conversion
std::wstring StringtoWString(const std::string s);

