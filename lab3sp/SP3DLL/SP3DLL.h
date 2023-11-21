#pragma once

#ifdef SP3DLL_EXPORTS
#define SP3DLL_API __declspec(dllexport)
#else
#define SP3DLL_API __declspec(dllimport)
#endif

extern "C" SP3DLL_API void FindAndReplaceString(const char* targetString, const char* replacementString);