#if !defined(WIN32_MAUSTRAP)
#include <cmath>
#include <cstdint>

#include "Pi32.h"
#include "bool32.h"

#include "maustrap.h"
#include "maustrap.cpp"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <xinput.h>
#include <dsound.h>

struct Win32_Offscreen_Buffer
{
	BITMAPINFO info;
	void *memory;
	int   width;
	int   height;
	int   pitch;
};

struct Win32_Window_Dimensions
{
	int width;
	int height;
};

struct Win32_Sound_Output
{
	int 	 samplesPerSecond;
	uint32_t runningSampleIndex;
	int 	 bytesPerSample;
	int 	 secondaryBufferSize;
	float	 tsine;
	int 	 latencySampleCount;
};

#define WIN32_MAUSTRAP
#endif // WIN32_MAUSTRAP
