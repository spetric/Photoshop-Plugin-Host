#pragma once
#include <string>
// constants
const int PSPI_ERR_FILTER_NOT_LOADED    = 1;
const int PSPI_ERR_FILTER_BAD_PROC		= 2;
const int PSPI_ERR_FILTER_ABOUT_ERROR   = 3;
const int PSPI_ERR_FILTER_DUMMY_PROC	= 4;
const int PSPI_ERR_FILTER_CANCELED		= 5;
const int PSPI_ERR_FILTER_CRASHED		= 6;
const int PSPI_ERR_FILTER_INVALID		= 7;
const int PSPI_ERR_IMAGE_INVALID		= 10;
const int PSPI_ERR_MEMORY_ALLOC			= 11;
const int PSPI_ERR_INIT_PATH_EMPTY		= 12;
const int PSPI_ERR_WORK_PATH_EMPTY		= 13;
const int PSPI_ERR_BAD_PARAM			= 14;
const int PSPI_ERR_BAD_IMAGE_TYPE		= 15;
// enums
enum TImgType {PSPI_IMG_TYPE_BGR = 0, PSPI_IMG_TYPE_BGRA, PSPI_IMG_TYPE_RGB, PSPI_IMG_TYPE_RGBA, PSPI_IMG_TYPE_GRAY, PSPI_IMG_TYPE_GRAYA };
enum TImgOrientation {PSPI_IMG_ORIENTATION_ASIS = 0, PSPI_IMG_ORIENTATION_INVERT};
// typedefs
typedef void (__stdcall *ENUMCALLBACK)(const char *, const char *, const char *, const wchar_t *);
typedef void (__stdcall *PROGRESSCALLBACK)(unsigned int, unsigned int);
typedef bool (__stdcall *COLORPICKERCALLBACK)(unsigned int &);

