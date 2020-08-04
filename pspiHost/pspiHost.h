#pragma once
#ifdef PSPIHOST_EXPORTS
#define PSPI_API __declspec(dllexport)
#else
#define PSPI_API __declspec(dllimport)
#endif
#include <windows.h>
#include "pspiGlobals.h"
extern "C" PSPI_API int __stdcall pspiSetPath(wchar_t *filterFolder);
// block for setting setting image/mask  using contiguous memory buffer pointer
// NOTE: source image is shared - do not destroy source image in your host program before plug-in is executed
extern "C" PSPI_API int __stdcall pspiSetImage(TImgType type, int width, int height, void *imageBuff, int imageStride, void *alphaBuff = 0, int alphaStride = 0);
extern "C" PSPI_API int __stdcall pspiSetMask(int width, int height, void *maskBuff, int maskStride, bool useMaskByPi);
// block for adding image scanlines (possibly non-contiguous image)
// NOTE: source image is shared - do not destroy source image in your host program before plug-in is executed
extern "C" PSPI_API int __stdcall pspiStartImageSL(TImgType type, int width, int height, bool externalAlpha);
extern "C" PSPI_API int __stdcall pspiAddImageSL(void *imageScanLine, void *alphaScanLine);
extern "C" PSPI_API int __stdcall pspiFinishImageSL(int imageStride = 0, int alphaStride = 0);
// block dor addding mask scanlines (possibly non-contiguous mask)
extern "C" PSPI_API int __stdcall pspiStartMaskSL(int width, int height, bool useByPi);
extern "C" PSPI_API int __stdcall pspiAddMaskSL(void *maskScanLine);
extern "C" PSPI_API int __stdcall pspiFinishMaskSL(int maskStride = 0);
// release all images - all image buffers including mask are released when application is closed, but sometimes it's necessary to free memory (big images) 
extern "C" PSPI_API int __stdcall pspiReleaseAllImages(void);	
// set call backs of interset
extern "C" PSPI_API int __stdcall pspiSetProgressCallBack(PROGRESSCALLBACK progressProc);
extern "C" PSPI_API int __stdcall pspiSetColorPickerCallBack(COLORPICKERCALLBACK colorPickerProc);
// 
extern "C" PSPI_API int __stdcall pspiPlugInLoad(wchar_t *filter);
extern "C" PSPI_API int __stdcall pspiPlugInAbout(HWND hWnd = 0);
extern "C" PSPI_API int __stdcall pspiPlugInExecute(HWND hWnd = 0);
extern "C" PSPI_API int __stdcall pspiPlugInEnumerate(ENUMCALLBACK enumFunc, bool recurseSubFolders = true);
