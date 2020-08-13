unit pspiGlobals;

interface

uses
  Windows;

// Result values
const PSPI_ERR_FILTER_NOT_LOADED   = 1;
const PSPI_ERR_FILTER_BAD_PROC     = 2;
const PSPI_ERR_FILTER_ABOUT_ERROR  = 3;
const PSPI_ERR_FILTER_DUMMY_PROC   = 4;
const PSPI_ERR_FILTER_CANCELED     = 5;
const PSPI_ERR_FILTER_CRASHED      = 6;
const PSPI_ERR_FILTER_INVALID      = 7;
const PSPI_ERR_IMAGE_INVALID       = 10;
const PSPI_ERR_MEMORY_ALLOC        = 11;
const PSPI_ERR_INIT_PATH_EMPTY     = 12;
const PSPI_ERR_WORK_PATH_EMPTY     = 13;
const PSPI_ERR_BAD_PARAM           = 14;
const PSPI_ERR_BAD_IMAGE_TYPE      = 15;

// enums
{$MINENUMSIZE 4}
type TImgType = (PSPI_IMG_TYPE_BGR = 0, PSPI_IMG_TYPE_BGRA, PSPI_IMG_TYPE_RGB, PSPI_IMG_TYPE_RGBA, PSPI_IMG_TYPE_GRAY, PSPI_IMG_TYPE_GRAYA);
type TImgOrientation = (PSPI_IMG_ORIENTATION_ASIS = 0, PSPI_IMG_ORIENTATION_INVERT);

// typedefs
type
   TENUMCALLBACK = procedure(p1: PAnsiChar; p2: PAnsiChar; p3: PAnsiChar; p4: PWideChar); cdecl;
   TPROGRESSCALLBACK = procedure(p1: DWORD; p2: DWORD); cdecl;
   TCOLORPICKERCALLBACK = function(var p1: DWORD): LongBool; cdecl;


implementation

end.

