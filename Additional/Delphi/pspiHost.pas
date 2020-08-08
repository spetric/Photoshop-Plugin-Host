unit pspiHost;

interface

uses pspiGlobals, Windows;

          
type
  int = Integer;

// get psipHost version
function pspiGetVersion(): PAnsiChar stdcall; 

// set path
function pspiSetPath(filterFolder: Pointer): int stdcall;

// set roi
function pspiSetRoi(top: int = 0; left: int = 0; bottom: int = 0; right: int = 0): int  stdcall;
// set image orientation
function pspiSetImageOrientation(orientation: TImgOrientation) int stdcall;

// set image using contiguous memory buffer pointer
// note: source image is shared - do not destroy source image in your host program before plug-in is executed
function pspiSetImage(imgtype: TImgType; width: int; height: int; imageBuff: Pointer; imageStride: int; alphaBuff: Pointer = nil; alphaStride: int = 0): int stdcall;

// set mask using contiguous memory buffer pointer
// note: source mask is shared - do not destroy source maske in your host program before plug-in is executed
function pspiSetMask(width: int; height: int; maskBuff: Pointer; maskStride: int;  useMaskByPi: Boolean = true): int stdcall;

// block for adding image scanlines (possibly non-contiguous image)
// note: source image is shared - do not destroy source image in your host program before plug-in is executed
function pspiStartImageSL(imgtype: TImgType; width: int; height: int; externalAlpha: Boolean = false): int  stdcall;
function pspiAddImageSL(imageScanLine: Pointer; alphaScanLine: Pointer = nil): int stdcall;
function pspiFinishImageSL(imageStride: int = 0; alphaStride: int = 0): int  stdcall;

// block dor addding mask scanlines (possibly non-contiguous mask)
// note: source mask is shared - do not destroy source maske in your host program before plug-in is executed
function pspiStartMaskSL(width: int; height: int; useMaskByPi: Boolean = true): int stdcall;
function pspiAddMaskSL(maskScanLine: Pointer): int stdcall;
function pspiFinishMaskSL(maskStride: int = 0): int stdcall;

// release all images - all image buffers including mask are released when application is closed; but sometimes it's necessary to free memory (big images) 
function pspiReleaseAllImages(): int stdcall;

// set call backs of interset
function pspiSetProgressCallBack(progressProc: PROGRESSCALLBACK): int stdcall;
function pspiSetColorPickerCallBack(colorPickerProc: COLORPICKERCALLBACK): int stdcall;

// plug-in related 
function pspiPlugInLoad(filter: Pointer): int stdcall;
function pspiPlugInAbout(hWnd: HWND = 0): int stdcall;
function pspiPlugInExecute(hWnd: HWND = 0): int stdcall;
function pspiPlugInEnumerate(enumFunc: ENUMCALLBACK; recurseSubFolders: Boolean = true): int stdcall;

implementation

function pspiGetVersion(); external 'pspiHost.dll';
function pspiSetPath(filterFolder: Pointer); external 'pspiHost.dll';
function pspiSetRoi(top: int = 0; left: int = 0; bottom: int = 0; right: int = 0); external 'pspiHost.dll';
function pspiSetImageOrientation(orientation: TImgOrientation) external 'pspiHost.dll';
function pspiSetImage(imgtype: TImgType; width: int; height: int; imageBuff: Pointer; imageStride: int; alphaBuff: Pointer = nil; alphaStride: int = 0); external 'pspiHost.dll';
function pspiSetMask(width: int; height: int; maskBuff: Pointer; maskStride: int;  useMaskByPi: Boolean = true); external 'pspiHost.dll';
function pspiStartImageSL(imgtype: TImgType; width: int; height: int; externalAlpha: Boolean = false); external 'pspiHost.dll';
function pspiAddImageSL(imageScanLine: Pointer; alphaScanLine: Pointer = nil); external 'pspiHost.dll';
function pspiFinishImageSL(imageStride: int = 0; alphaStride: int = 0); external 'pspiHost.dll';
function pspiStartMaskSL(width: int; height: int; useMaskByPi: Boolean = true); external 'pspiHost.dll';
function pspiAddMaskSL(maskScanLine: Pointer); external 'pspiHost.dll';
function pspiFinishMaskSL(maskStride: int = 0); external 'pspiHost.dll';
function pspiReleaseAllImages(); external 'pspiHost.dll';
function pspiSetProgressCallBack(progressProc: PROGRESSCALLBACK); external 'pspiHost.dll';
function pspiSetColorPickerCallBack(colorPickerProc: COLORPICKERCALLBACK); external 'pspiHost.dll';
function pspiPlugInLoad(filter: Pointer); external 'pspiHost.dll';
function pspiPlugInAbout(hWnd: HWND = 0); external 'pspiHost.dll';
function pspiPlugInExecute(hWnd: HWND = 0); external 'pspiHost.dll';
function pspiPlugInEnumerate(enumFunc: ENUMCALLBACK; recurseSubFolders: Boolean = true); external 'pspiHost.dll';

end.
