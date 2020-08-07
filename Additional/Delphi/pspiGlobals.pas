unit pspiGlobals;

interface

uses
  Windows;

// constants
const PSPIW_ERR_FILTER_NOT_LOADED   = 1;
const PSPIW_ERR_FILTER_BAD_PROC     = 2;
const PSPIW_ERR_FILTER_ABOUT_ERROR  = 3;
const PSPIW_ERR_FILTER_DUMMY_PROC   = 4;
const PSPIW_ERR_FILTER_CANCELED	    = 5;
const PSPIW_ERR_FILTER_CRASHED	    = 6;
const PSPIW_ERR_FILTER_INVALID	    = 7;
const PSPIW_ERR_IMAGE_INVALID	    = 10;
const PSPIW_ERR_MEMORY_ALLOC	    = 11;
const PSPIW_ERR_INIT_PATH_EMPTY	    = 12;
const PSPIW_ERR_WORK_PATH_EMPTY     = 13;
const PSPIW_ERR_BAD_PARAM	    = 14;

// enums
type TImgType = (PSPIW_IMT_BGR = 0, PSPIW_IMT_BGRA, PSPIW_IMT_RGB, PSPIW_IMT_RGBA, PSPIW_IMT_GRAY, PSPIW_IMT_GRAYA);

// typedefs
type 
   ENUMCALLBACK = procedure(p1: PAnsiChar; p2: PAnsiChar; p3: PAnsiChar; p4: PWideChar); stdcall;
   PROGRESSCALLBACK = procedure(p1: DWORD; p2: DWORD); stdcall;
   COLORPICKERCALLBACK = function(var p1: DWORD): LongBool; stdcall;


implementation

end.
