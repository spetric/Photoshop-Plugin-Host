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
	int alphaChans;
	// image
	void *imageBuff;
	int imageStride;
	LPBYTE *imageScan;
	// alpha
	void *alphaBuff;
	int alphaStride;
	LPBYTE *alphaScan;
	SpspiImage()
	{
		width = 0;
		height = 0;
		channels = 0;
		alphaChans = 0;
		imageBuff = 0;
		imageStride = 0;
		imageScan = 0;
		alphaBuff = 0;
		alphaStride = 0;
		alphaScan = 0;
	}
};
struct SpspiMask
{
	int width;
	int height;
	// image (mask)
	void *maskBuff;
	int maskStride;
	LPBYTE *maskScan;
	bool useByPi;
	SpspiMask()
	{
		width = 0;
		height = 0;
		maskBuff = 0;
		maskStride = 0;
		maskScan = 0;
		useByPi = false; // for now
	}
};
struct SpspiHostRecord
{
	SpspiImage *srcImage;
	SpspiMask  *srcMask;
	SpspiImage *dstImage;
	SpspiMask  *dstMask;
	HINSTANCE dllHandle;
	std::wstring filterPathName;	// filter name (full path)  
	std::string piCategory;
	std::string piName;
	std::string piEntrypointName;
	bool filterLoaded;
	uint32 foregroundColor;
	uint32 backgroundColor;
	HWND hWnd;
    PROGRESSCALLBACK progressProc;
	bool hasBoundingRectangle;
	Rect roiRect;
	std::wstring initialEnvPath;
	std::wstring workingEnvPath;
	std::wstring piPath;
	SpspiHostRecord()
	{
		srcImage = 0;
		srcMask = 0;
		dstImage = 0;
		dstMask = 0;
		dllHandle = 0;
		filterPathName = L"";
		piCategory = "";
		piName = "";
		piEntrypointName = "";
		filterLoaded = false;
		foregroundColor = (int)0x00ffffff;
		backgroundColor = 0;
		hWnd = 0;
		progressProc = 0;
		hasBoundingRectangle = false;
		roiRect.top = 0;
		roiRect.bottom = 0;
		roiRect.left = 0;
		roiRect.right = 0;
		initialEnvPath = L"";
		workingEnvPath = L"";
		piPath = L"";
	}
};
struct SpspiAdvanceState
{
	bool copyTgt2Out;
	bool inBuffOK;
	bool outBuffOK;
	int inSize;
	int outSize;
	Rect lastOutRect;
	int lastOutRowBytes;
	int lastOutLoPlane;
	int lastOutHiPlane;
	void Init()
	{
		copyTgt2Out= true;
		inBuffOK = false;
		outBuffOK = false;
		inSize = 0;
		outSize = 0;
	    lastOutRowBytes = 0;
	    lastOutLoPlane = 0;
	    lastOutHiPlane = 0;
		lastOutRect = Rect();
		lastOutRect = Rect();
	}
	SpspiAdvanceState()
	{
		Init();
	}
};