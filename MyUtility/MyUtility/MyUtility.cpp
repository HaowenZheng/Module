// MyUtility.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "MyUtility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

MYUTILITY_FUNC_DEFINE(std::vector<std::string>) Split(LPCTSTR str, TCHAR spChr, LPSTR mEOF)
{
	LPCTSTR tempStr = str;
	LPCTSTR tempSp = str;
	std::vector<std::string> retString;
	while (tempSp != '\0' && *tempSp != '\0')
	{
		if (spChr == *tempSp)
		{
			TCHAR coyTemp[1024] = { 0 };
			int len = (tempSp - tempStr) * sizeof(TCHAR);
			std::memcpy(coyTemp, tempStr, len);
			retString.push_back(coyTemp);
			tempStr = tempSp + 1;
		}

		if (mEOF != nullptr && memcmp(tempSp, mEOF, 2) == 0)
		{
			TCHAR coyTemp[1024] = { 0 };
			int len = (tempSp - tempStr) * sizeof(TCHAR);
			std::memcpy(coyTemp, tempStr, len);
			retString.push_back(coyTemp);
			tempStr = "\0";
			break;
		}
		tempSp++;
	}

	if (strlen(tempStr) != 0)
		retString.push_back(tempStr);
	return retString;
}

MYUTILITY_FUNC_DEFINE(std::vector<std::string>) Split_Length(LPCTSTR str, TCHAR spChr, int mSize)
{
	LPCTSTR tempStr = str;
	LPCTSTR tempSp = str;
	std::vector<std::string> retString;
	std::vector<std::string> retStrNoEmptr;
	for (int i = 0; i < mSize; i++)
	{
		if (spChr == *tempSp || memcmp("\x00\x00", tempSp, 2) == 0)
		{
			TCHAR coyTemp[255] = { 0 };
			std::memcpy(coyTemp, tempStr, tempSp - tempStr);
			retString.push_back(coyTemp);
			tempStr = tempSp + 1;
		}
		tempSp++;
	}

	ForEachContain(retString, [&retStrNoEmptr](std::string str)->void {if (!str.empty())retStrNoEmptr.push_back(str); });

	return retStrNoEmptr;
}

MYUTILITY_FUNC_DEFINE(std::vector<std::string>) Split_MultiChar(LPCTSTR str, std::vector<TCHAR> spChrs, int mSize)
{
	LPCTSTR tempStr = str;
	LPCTSTR tempSp = str;
	std::vector<std::string> retString;
	std::vector<std::string> retStrNoEmptr;
	for (int i = 0; i < mSize; i++)
	{
		if (FindContain(spChrs, [tempSp](TCHAR spChr)->bool {return spChr == *tempSp; }) != spChrs.end() || *tempSp == '\0')
		{
			TCHAR coyTemp[255] = { 0 };
			std::memcpy(coyTemp, tempStr, tempSp - tempStr);
			retString.push_back(coyTemp);
			tempStr = tempSp + 1;
		}
		tempSp++;
	}

	ForEachContain(retString, [&retStrNoEmptr](std::string str)->void {if (!str.empty())retStrNoEmptr.push_back(str); });

	return retStrNoEmptr;
}


std::unordered_map<std::string, HINSTANCE> ModuleMap;
std::mutex moduleMtx;
MYUTILITY_FUNC_DEFINE(HINSTANCE) LoadModule(LPCTSTR dllName)
{
	std::unique_lock<std::mutex> lck(moduleMtx);
	HINSTANCE result = 0;
	if (ModuleMap.find(dllName) != ModuleMap.end())
		result = ModuleMap[dllName];
	else if (PathFileExists(dllName))
	{
		result = LoadLibraryEx(dllName, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
		if(0 != result)
			ModuleMap.insert(std::unordered_map<std::string, HINSTANCE>::value_type(dllName, result));
	}

	return result;
}

MYUTILITY_FUNC_DEFINE(void) DeleteModule(HINSTANCE dllInstance)
{
	std::unique_lock<std::mutex> lck(moduleMtx);
	auto find_iterator = FindContain(ModuleMap, [dllInstance](decltype(ModuleMap)::value_type & item) {return item.second == dllInstance; });
	if (find_iterator != ModuleMap.end())
	{
		ModuleMap.erase(find_iterator);
		FreeLibrary(dllInstance);
	}
}
