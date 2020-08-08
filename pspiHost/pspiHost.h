#pragma once
#ifdef PSPIHOST_EXPORTS
#define PSPI_API __declspec(dllexport)
#else
#define PSPI_API __declspec(dllimport)
#endif
#include <windows.h>
#include "pspiGlobals.h"
// get psipHost version
extern "C" PSPI_API const char* __stdcall pspiGetVersion(void);
// set path
extern "C" PSPI_API int __stdcall pspiSetPath(const wchar_t *filterFolder);
// set roi
extern "C" PSPI_API int __stdcall pspiSetRoi(int top = 0, int left = 0, int bottom = 0, int right = 0);
// set image orientation
extern "C" PSPI_API int __stdcall pspiSetImageOrientation(TImgOrientation orientation);
// set image using contiguous memory buffer pointer
// note: source image is shared - do not destroy source image in your host program before plug-in is executed
extern "C" PSPI_API int __stdcall pspiSetImage(TImgType type, int width, int height, void *imageBuff, int imageStride, void *alphaBuff = 0, int alphaStride = 0);
// set mask using contiguous memory buffer pointer
// note: source mask is shared - do not destroy source maske in your host program before plug-in is executed
extern "C" PSPI_API int __stdcall pspiSetMask(int width, int height, void *maskBuff, int maskStride, bool useMaskByPi = true);
// block for adding image scanlines (possibly non-contiguous image)
// note: source image is shared - do not destroy source image in your host program before plug-in is executed
extern "C" PSPI_API int __stdcall pspiStartImageSL(TImgType type, int width, int height, bool externalAlpha = false);
extern "C" PSPI_API int __stdcall pspiAddImageSL(void *imageScanLine, void *alphaScanLine = 0);
extern "C" PSPI_API int __stdcall pspiFinishImageSL(int imageStride = 0, int alphaStride = 0);
// block dor addding mask scanlines (possibly non-contiguous mask)
// note: source mask is shared - do not destroy source maske in your host program before plug-in is executed
extern "C" PSPI_API int __stdcall pspiStartMaskSL(int width, int height, bool useMaskByPi = true);
extern "C" PSPI_API int __stdcall pspiAddMaskSL(void *maskScanLine);
extern "C" PSPI_API int __stdcall pspiFinishMaskSL(int maskStride = 0);
// release all images - all image buffers including mask are released when application is closed, but sometimes it's necessary to free memory (big images) 
extern "C" PSPI_API int __stdcall pspiReleaseAllImages(void);	
// set call backs of interset
extern "C" PSPI_API int __stdcall pspiSetProgressCallBack(PROGRESSCALLBACK progressProc);
extern "C" PSPI_API int __stdcall pspiSetColorPickerCallBack(COLORPICKERCALLBACK colorPickerProc);
// plug-in related 
extern "C" PSPI_API int __stdcall pspiPlugInLoad(const wchar_t *filter);
extern "C" PSPI_API int __stdcall pspiPlugInAbout(HWND hWnd = 0);
extern "C" PSPI_API int __stdcall pspiPlugInExecute(HWND hWnd = 0);
extern "C" PSPI_API int __stdcall pspiPlugInEnumerate(ENUMCALLBACK enumFunc, bool recurseSubFolders = true);

