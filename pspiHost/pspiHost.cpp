#include "stdafx.h"
#include "pspiHost.h"
#include "TPspiCore.h"
TPspiCore piCore;
static int imgSclIndex, maskSclIndex;
static char version[4];
//-----------------------------------------------------------------
// get version
//-----------------------------------------------------------------
const char* pspiGetVersion(void)
{  
	memcpy(version, "0.9\0", 4);
	return version;
}
//-----------------------------------------------------------------
// set path
//-----------------------------------------------------------------
int pspiSetPath(const wchar_t *filterFolder)
{
	return piCore.SetPath(filterFolder);
}
//---------------------------------------------------------------------------
// set filter roi
//---------------------------------------------------------------------------
int pspiSetRoi(int top, int left, int bottom, int right)
{
	piCore.HostRecord.roiRect.top = top;
	piCore.HostRecord.roiRect.left = left;
	piCore.HostRecord.roiRect.bottom = bottom;
	piCore.HostRecord.roiRect.right = right;
	piCore.HostRecord.roiRect.Normalize();
	return 0;
}
//-----------------------------------------------------------------
// set imege orientatio - As is or invert scanlines (this applies to mask also)
//-----------------------------------------------------------------
int pspiSetImageOrientation(TImgOrientation orientation)
{
	piCore.HostRecord.imgOrientation = orientation;
	return 0;
}
//-----------------------------------------------------------------
// set working image - pass contiguous buffer
//-----------------------------------------------------------------
int pspiSetImage(TImgType type, int width, int height, void *imageBuff, int imageStride, void *alphaBuff, int alphaStride)
{
	pspiSetRoi();	// clear roi
	return piCore.SetImage(type, width, height, imageBuff, imageStride, alphaBuff, alphaStride);
}
//-----------------------------------------------------------------
// set working mask - pass contiguous buffer (grayscale image)
//-----------------------------------------------------------------
int pspiSetMask(int width, int height, void *maskBuff, int maskStride, bool useMaskByPi)
{
	return piCore.SetMask(width, height, maskBuff, maskStride, useMaskByPi);
}
//-----------------------------------------------------------------
// start working image creation - for passing scanlines
//-----------------------------------------------------------------
int pspiStartImageSL(TImgType type, int width, int height, bool externalAplha)
{
	pspiSetRoi();	// clear roi
	imgSclIndex = 0;
	return piCore.StartImageSL(type, width, height, externalAplha);
}
//-----------------------------------------------------------------
// add image scanline (use it in loop)
//-----------------------------------------------------------------
int pspiAddImageSL(void *imageScanLine, void *alphaScanLine)
{
	return piCore.AddImageSL(imgSclIndex++, imageScanLine, alphaScanLine);
}
//-----------------------------------------------------------------
// finish image creation using scanlines
//-----------------------------------------------------------------
int pspiFinishImageSL(int imageStride, int alphaStride)
{
	return piCore.FinishImageSL(imageStride, alphaStride);
}
//-----------------------------------------------------------------
// start mask creation - for passing scanlines
//-----------------------------------------------------------------
int pspiStartMaskSL(int width, int height, bool useMaskByPi)
{
	maskSclIndex = 0;
	return piCore.StartMaskSL(width, height, useMaskByPi);
}
//-----------------------------------------------------------------
// add mask scanline (use it in loop)
//-----------------------------------------------------------------
int pspiAddMaskSL(void *maskScanLine)
{
	return piCore.AddMaskSL(maskSclIndex++, maskScanLine);
}
//-----------------------------------------------------------------
// finish mask creation using scanlies
//-----------------------------------------------------------------
int pspiFinishMaskSL(int maskStride)
{
	return piCore.FinishMaskSL(maskStride);
}
//-----------------------------------------------------------------
// set progress procedure call back
//-----------------------------------------------------------------
int pspiReleaseAllImages(void)
{
	pspiSetRoi();	// clear roi
	piCore.ReleaseAllImages();
	return 0;
}
//-----------------------------------------------------------------
// set progress procedure call back
//-----------------------------------------------------------------
int pspiSetProgressCallBack(PROGRESSCALLBACK progressProc)
{
	piCore.ProgressCallBack = progressProc;
	return 0;
}
//-----------------------------------------------------------------
// set color picker procedure call back
//-----------------------------------------------------------------
int pspiSetColorPickerCallBack(COLORPICKERCALLBACK colorPickerProc)
{
	piCore.ColorPickerCallback = colorPickerProc;
	return 0;
}
//-----------------------------------------------------------------
// Load plugin
//-----------------------------------------------------------------
int pspiPlugInLoad(const wchar_t *filter)
{
	return piCore.PlugInLoad(filter);
}
//-----------------------------------------------------------------
// Show plug-in about
//-----------------------------------------------------------------
int pspiPlugInAbout(HWND hWnd)
{
	return piCore.PlugInAbout(hWnd);
}
//-----------------------------------------------------------------
// Execute loaded plug-in
//-----------------------------------------------------------------
int pspiPlugInExecute(HWND hWnd)
{
	return piCore.PlugInExecute(hWnd);
}
//---------------------------------------------------------------------------
// Enumerate Filters from directory using call back function
//---------------------------------------------------------------------------
int pspiPlugInEnumerate(ENUMCALLBACK enumFunc, bool recurseSubFolders)
{
	if (enumFunc == NULL)
		return PSPI_ERR_FILTER_BAD_PROC;
	return piCore.PlugInEnumerate(enumFunc, recurseSubFolders);
}



