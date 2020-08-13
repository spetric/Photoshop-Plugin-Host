#pragma once
#ifdef PSPIHOST_EXPORTS
#define PSPI_API __declspec(dllexport)
#else
#define PSPI_API __declspec(dllimport)
#endif
#include <windows.h>
#include "pspiGlobals.h"
// get psipHost version
extern "C" PSPI_API const char* pspiGetVersion(void);
// set path
extern "C" PSPI_API int pspiSetPath(const wchar_t *filterFolder);
// set roi
extern "C" PSPI_API int pspiSetRoi(int top = 0, int left = 0, int bottom = 0, int right = 0);
// set image orientation
extern "C" PSPI_API int pspiSetImageOrientation(TImgOrientation orientation);
// set image using contiguous memory buffer pointer
// note: source image is shared - do not destroy source image in your host program before plug-in is executed
extern "C" PSPI_API int pspiSetImage(TImgType type, int width, int height, void *imageBuff, int imageStride, void *alphaBuff = 0, int alphaStride = 0);
// set mask using contiguous memory buffer pointer
// note: source mask is shared - do not destroy source maske in your host program before plug-in is executed
extern "C" PSPI_API int pspiSetMask(int width = 0, int height = 0, void *maskBuff = 0, int maskStride = 0, bool useMaskByPi = true);
// block for adding image scanlines (possibly non-contiguous image)
// note: source image is shared - do not destroy source image in your host program before plug-in is executed
extern "C" PSPI_API int pspiStartImageSL(TImgType type, int width, int height, bool externalAlpha = false);
extern "C" PSPI_API int pspiAddImageSL(void *imageScanLine, void *alphaScanLine = 0);
extern "C" PSPI_API int pspiFinishImageSL(int imageStride = 0, int alphaStride = 0);
// block dor addding mask scanlines (possibly non-contiguous mask)
// note: source mask is shared - do not destroy source maske in your host program before plug-in is executed
extern "C" PSPI_API int pspiStartMaskSL(int width, int height, bool useMaskByPi = true);
extern "C" PSPI_API int pspiAddMaskSL(void *maskScanLine);
extern "C" PSPI_API int pspiFinishMaskSL(int maskStride = 0);
// release all images - all image buffers including mask are released when application is closed, but sometimes it's necessary to free memory (big images) 
extern "C" PSPI_API int pspiReleaseAllImages(void);	
// set call backs of interset
extern "C" PSPI_API int pspiSetProgressCallBack(PROGRESSCALLBACK progressProc);
extern "C" PSPI_API int pspiSetColorPickerCallBack(COLORPICKERCALLBACK colorPickerProc);
// plug-in related 
extern "C" PSPI_API int pspiPlugInLoad(const wchar_t *filter);
extern "C" PSPI_API int pspiPlugInAbout(HWND hWnd = 0);
extern "C" PSPI_API int pspiPlugInExecute(HWND hWnd = 0);
extern "C" PSPI_API int pspiPlugInEnumerate(ENUMCALLBACK enumFunc, bool recurseSubFolders = true);

