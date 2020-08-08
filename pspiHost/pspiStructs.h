#pragma once
#include <string>
#include "PITypes.h"
// typedefs
typedef void (* PLUGINPROC)(const int16, void *, intptr_t*, int16*);
// structures
struct SpspiHandle
{
	Ptr hPointer;
	int hSize;
};
struct SpspiImage
{
	int width;
	int height;
	int channels;
	int exaChannels;		// external alpha channels
	// image
	void *imageBuff;
	int imageStride;
	LPBYTE *imageScan;
	// alpha
	void *exaBuff;
	int exaStride;
	LPBYTE *exaScan;
	SpspiImage()
	{
		width = 0;
		height = 0;
		channels = 0;
		exaChannels = 0;
		imageBuff = 0;
		imageStride = 0;
		imageScan = 0;
		exaBuff = 0;
		exaStride = 0;
		exaScan = 0;
	}
};
/*
struct SpspiMask
{
	int width;
	int height;
	// image (mask)
	void *maskBuff;
	int maskStride;
	LPBYTE *maskScan;
	SpspiMask()
	{
		width = 0;
		height = 0;
		maskBuff = 0;
		maskStride = 0;
		maskScan = 0;
	}
};
*/
struct SpspiRect
{
	int top;
	int left;
	int bottom;
	int right;
	void Init()
	{
		top = left = right = bottom = 0;
	}
	void Normalize()
	{
		if (top < 0)
			top = 0;
		if (left < 0)
			left = 0;
		if (bottom < 0)
			bottom = 0;
		if (right < 0)
			right = 0;
		if (top > bottom)	// swap
		{
			top = top ^ bottom;
			bottom = bottom ^ top;
			top = top ^ bottom;
		}
		if (left > right)	// swap
		{
			left = left ^ right;
			right = right ^ left;
			left = left ^ right;
		}
	}
	bool IsEmpty()
	{
		return ((bottom - top) == 0) || ((right - left) == 0);
	}
	SpspiRect()
	{
		Init();
	}
};
struct SpspiHostRecord
{
	SpspiImage *srcImage;
	SpspiImage *dstImage;
	SpspiImage  *mask;
	TImgType imgType;
	TImgOrientation imgOrientation;
	bool useMaskByPi;
	HINSTANCE dllHandle;
	std::wstring filterPathName;	// filter name (full path)  
	std::string piCategory;
	std::string piName;
	std::string piEntrypointName;
	bool filterLoaded;
	uint32 foregroundColor;
	uint32 backgroundColor;
	HWND hWnd;
	SpspiRect roiRect;
	std::wstring initialEnvPath;
	std::wstring workingEnvPath;
	std::wstring piPath;
	int colorChannels;
	int alphaChannels;
	void Init()
	{
		srcImage = 0;
		dstImage = 0;
		mask = 0;
		imgType = PSPI_IMG_TYPE_BGR;
		imgOrientation = PSPI_IMG_ORIENTATION_ASIS;
		useMaskByPi = false;
		dllHandle = 0;
		filterPathName = L"";
		piCategory = "";
		piName = "";
		piEntrypointName = "";
		filterLoaded = false;
		foregroundColor = (int)0x00ffffff;
		backgroundColor = 0;
		hWnd = 0;
		roiRect.top = 0;
		roiRect.bottom = 0;
		roiRect.left = 0;
		roiRect.right = 0;
		initialEnvPath = L"";
		workingEnvPath = L"";
		piPath = L"";
		colorChannels = 0;
		alphaChannels = 0;
	}
	SpspiHostRecord()
	{
		Init();
	}
};
struct SpspiAdvanceState
{
	bool inBuffOK;
	bool outBuffOK;
	bool maskBuffOK;
	int inSize;
	int outSize;
	int maskSize;
	Rect lastInRect;
	Rect lastOutRect;
	Rect lastMaskRect;
	int lastOutRowBytes;
	int lastOutLoPlane;
	int lastOutHiPlane;
	void Init()
	{
		inBuffOK = false;
		outBuffOK = false;
		maskBuffOK = false;
		inSize = 0;
		outSize = 0;
		maskSize = 0;
		lastInRect = Rect();
		lastOutRect = Rect();
		lastMaskRect = Rect();
		lastOutRowBytes = 0;
	    lastOutLoPlane = 0;
	    lastOutHiPlane = 0;
	}
	SpspiAdvanceState()
	{
		Init();
	}
};