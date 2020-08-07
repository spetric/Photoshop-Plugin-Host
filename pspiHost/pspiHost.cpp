#include "stdafx.h"
#include "pspiHost.h"
#include "TPspiCore.h"
TPspiCore piCore;
static int imgSclIndex, maskSclIndex;
static char version[4];
//-----------------------------------------------------------------
// get version
//-----------------------------------------------------------------
const char* __stdcall pspiGetVersion(void)
{  
	memcpy(version, "0.6\0", 4);
	return version;
}
//-----------------------------------------------------------------
// set path
//-----------------------------------------------------------------
int __stdcall pspiSetPath(const wchar_t *filterFolder)
{
	return piCore.SetPath(filterFolder);
}
//---------------------------------------------------------------------------
// set filter roi
//---------------------------------------------------------------------------
int __stdcall pspiSetRoi(int top, int left, int bottom, int right)
{
	piCore.HostRecord.roiRect.top = top;
	piCore.HostRecord.roiRect.left = left;
	piCore.HostRecord.roiRect.bottom = bottom;
	piCore.HostRecord.roiRect.right = right;
	piCore.HostRecord.roiRect.Normalize();
	return 0;
}
//-----------------------------------------------------------------
// set working image - pass contiguous buffer
//-----------------------------------------------------------------
int __stdcall pspiSetImage(TImgType type, int width, int height, void *imageBuff, int imageStride, void *alphaBuff, int alphaStride)
{
	pspiSetRoi();	// clear roi
	return piCore.SetImage(type, width, height, imageBuff, imageStride, alphaBuff, alphaStride);
}
//-----------------------------------------------------------------
// set working mask - pass contiguous buffer (grayscale image)
//-----------------------------------------------------------------
int __stdcall pspiSetMask(int width, int height, void *maskBuff, int maskStride, bool useMaskByPi)
{
	return piCore.SetMask(width, height, maskBuff, maskStride, useMaskByPi);
}
//-----------------------------------------------------------------
// start working image creation - for passing scanlines
//-----------------------------------------------------------------
int __stdcall pspiStartImageSL(TImgType type, int width, int height, bool externalAplha)
{
	pspiSetRoi();	// clear roi
	imgSclIndex = 0;
	return piCore.StartImageSL(type, width, height, externalAplha);
}
//-----------------------------------------------------------------
// add image scanline (use it in loop)
//-----------------------------------------------------------------
int __stdcall pspiAddImageSL(void *imageScanLine, void *alphaScanLine)
{
	return piCore.AddImageSL(imgSclIndex++, imageScanLine, alphaScanLine);
}
//-----------------------------------------------------------------
// finish image creation using scanlines
//-----------------------------------------------------------------
int __stdcall pspiFinishImageSL(int imageStride, int alphaStride)
{
	return piCore.FinishImageSL(imageStride, alphaStride);
}
//-----------------------------------------------------------------
// start mask creation - for passing scanlines
//-----------------------------------------------------------------
int __stdcall pspiStartMaskSL(int width, int height, bool useMaskByPi)
{
	maskSclIndex = 0;
	return piCore.StartMaskSL(width, height, useMaskByPi);
}
//-----------------------------------------------------------------
// add mask scanline (use it in loop)
//-----------------------------------------------------------------
int __stdcall pspiAddMaskSL(void *maskScanLine)
{
	return piCore.AddMaskSL(maskSclIndex++, maskScanLine);
}
//-----------------------------------------------------------------
// finish mask creation using scanlies
//-----------------------------------------------------------------
int __stdcall pspiFinishMaskSL(int maskStride)
{
	return piCore.FinishMaskSL(maskStride);
}
//-----------------------------------------------------------------
// set progress procedure call back
//-----------------------------------------------------------------
int __stdcall pspiReleaseAllImages(void)
{
	pspiSetRoi();	// clear roi
	piCore.ReleaseAllImages();
	return 0;
}
//-----------------------------------------------------------------
// set progress procedure call back
//-----------------------------------------------------------------
int __stdcall pspiSetProgressCallBack(PROGRESSCALLBACK progressProc)
{
	piCore.ProgressCallBack = progressProc;
	return 0;
}
//-----------------------------------------------------------------
// set color picker procedure call back
//-----------------------------------------------------------------
int __stdcall pspiSetColorPickerCallBack(COLORPICKERCALLBACK colorPickerProc)
{
	piCore.ColorPickerCallback = colorPickerProc;
	return 0;
}
//-----------------------------------------------------------------
// Load plugin
//-----------------------------------------------------------------
int __stdcall pspiPlugInLoad(const wchar_t *filter)
{
	return piCore.PlugInLoad(filter);
}
//-----------------------------------------------------------------
// Show plug-in about
//-----------------------------------------------------------------
int __stdcall pspiPlugInAbout(HWND hWnd)
{
	return piCore.PlugInAbout(hWnd);
}
//-----------------------------------------------------------------
// Execute loaded plug-in
//-----------------------------------------------------------------
int __stdcall pspiPlugInExecute(HWND hWnd)
{
	return piCore.PlugInExecute(hWnd);
}
//---------------------------------------------------------------------------
// Enumerate Filters from directory using call back function
//---------------------------------------------------------------------------
int __stdcall pspiPlugInEnumerate(ENUMCALLBACK enumFunc, bool recurseSubFolders)
{
	if (enumFunc == NULL)
		return PSPIW_ERR_FILTER_BAD_PROC;
	return piCore.PlugInEnumerate(enumFunc, recurseSubFolders);
}



