# Photoshop-Plugin-Host
Plugin host for 8bf Photoshop filters

## pspiHost.dll
This dll engine enables loading and executing 8bf filters from user application.  
The code is written in VisualStudio 2017 C++ for Windows platform. Host engine can be used also from Embarcadero C++ with created libs (implib -a/ mkexp -p) and also from Delphi, but conversion of pspiHost.h and pspiGlobals.h to respective pas files needs to be done.

Engine can be built in 32/64-bit mode and it supports 32/64-bit filters respectively.
Before compiling psipHost, you must download Adobe Photoshop SDK and copy following SDK directories in photoshopapi directory under pspiHost:
- photoshop
- pica_sp
- resources

Currently supported image formats:
- RGB
- BGR
- RGBA
- BGRA
- GRAYSCALE
- RGB + external alpha channel
- BGR + external alpha channel
- GRAYSCALE + external alpha channel.

Image can be passed to engine using single contiguous image buffer, or by adding scalines (for possibly non-contiguous image buffers). 

Currently not supported in host engine:
- padding
- parameters saving/restoring
- filters requiring SuitePea
- descriptorParameters handling
- ICCProfile
- Big document support (prepared but not yet implemented)

### Required
- C++ compiler (tested on Visual Studio 2017) 
- Adobe Photoshop SDK

### pspiHost APIs:
- ```pspiGetVersion(void);``` get current version: returns pointer to 3 charactes describing pspiHost version (currently 0.5).
- ```pspiSetPath(wchar_t *filterFolder);``` sets path to filters directory (usually some 8bf collection).
- ```pspiSetRoi(int top = 0, int left = 0, int bottom = 0, int right = 0);``` sets ROI (region of interest) to be filtered (rectangle).
- ```pspiSetImage(TImgType type, int width, int height, void *imageBuff, int imageStride, void *alphaBuff = 0, int alphaStride = 0);``` sets source image by passing contiguous image buffer pointer. If image has external alpha channel, pointer to alpha buffer can be passed as well. Also, you must pass image type, image width, height and image stride value and alpha stride value if alpha buffer is not-null.  
- ```pspiSetMask(int width, int height, void *maskBuff, int maskStride, bool useMaskByPi = true);``` sets 8-bit single channel grayscale mask. Contiguous buffer must be passed, as well as mask width, height (the same size as image), mask stride value and boolean value useMakByPi. This value tells if the mask will be used by plug-in (if supported by plug-in) or only for blending filtered and source image. 
- ```pspiStartImageSL(TImgType type, int width, int height, bool externalAlpha = false);``` prepares host image container for accepting source image scanlines. This option deals with images with non-contiguous memory or with images with unknown scanline alignement, so stride value can not be calculated.
- ```pspiAddImageSL(void *imageScanLine, void *alphaScanLine = 0);``` adds one scanline at the time to host image container. This API must be called in a loop and y scanlines are usually added in bottom-up order (last scanline is added first).
- ```pspiFinishImageSL(int imageStride = 0, int alphaStride = 0);``` finishes scanline addition and completely sets host image container. If values for stride are not passed, destination image is not aligned to source image scanline boundary. Destination image strides are callculated by pspiHost.
- ```pspiStartMaskSL(int width, int height, bool useMaskByPi = true);``` similar to pspiStartImageSL, but deals with mask.
- ```pspiAddMaskSL(void *maskScanLine);``` similar to pspiAddImageSL
- ```pspiFinishMaskSL(int maskStride = 0);``` similar to pspiFinsihImageSL
- ```pspiReleaseAllImages(void);``` releases all images and mask memory. When image to be filtered is passed to pspiHost, a copy of source image is created (dest). Source image is shared and dest image is owned by pspiHost. You don't need to call this API as it's called by host on exit, but if you're working with big images, sometimes it may come handy.
- ```pspiSetProgressCallBack(PROGRESSCALLBACK progressProc);``` sets progress procedure call-back. You must pass your function that will display filtering progress (optional). Function definition: ```typedef void (__stdcall *PROGRESSCALLBACK)(unsigned int, unsigned int);```. Funcion parameters are: done and total.
- ```pspiSetColorPickerCallBack(COLORPICKERCALLBACK colorPickerProc);``` if plug-in requires color service (color picker), you must pass your function that can deal with color picking (optional). Function definition: ```typedef bool (__stdcall *COLORPICKERCALLBACK)(unsigned int &);```. Picked color can be 0x00rrggbb or 0x00bbggrr depending on your application, but it must suite image type (RGB or BGR). 
- ```pspiPlugInLoad(wchar_t *filter);``` loads 8bf filter. This API must be called before filter execution.
- ```pspiPlugInAbout(HWND hWnd = 0);``` displays about window of loaded plugin. It's recommanded to pass your application windows handle.
- ```pspiPlugInExecute(HWND hWnd = 0);``` executes loaded plugin. It's recommanded to pass your application windows handle.
- ```pspiPlugInEnumerate(ENUMCALLBACK enumFunc, bool recurseSubFolders = true);``` routine for enumerating plugins in directory previously set by ```pspiSetPath(wchar_t *filterFolder);```. Function definition: ```typedef void (__stdcall *ENUMCALLBACK)(const char *, const char *, const char *, const wchar_t *);```. Function paremeters are: filter category, filter name, filter entrypoint name and filter full path.

### Important notes
Source image from your application (one that needs to be filtered) passed to pspiHost using pspiSetImage, or by pspiStartImageSL-pspiAddImageSL-pspiFinishImageSL block is shared (image buffer is shared). You must not delete this image in your application before executing plug-in. Otherwise, pspiHost will crash. The same stands for the source mask passed to pspiHost.

Mask and ROI can be used in conjunction, the result is intersection of mask and ROI. 

## capPspi
Console application for testing engine.

### Required
- C++ compiler (tested on Visual Studio 2017) 
- OpenCV







