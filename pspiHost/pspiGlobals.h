#pragma once
#include <string>
// constants
const int PSPIW_ERR_FILTER_NOT_LOADED   = 1;
const int PSPIW_ERR_FILTER_BAD_PROC		= 2;
const int PSPIW_ERR_FILTER_ABOUT_ERROR  = 3;
const int PSPIW_ERR_FILTER_DUMMY_PROC	= 4;
const int PSPIW_ERR_FILTER_CANCELED		= 5;
const int PSPIW_ERR_FILTER_CRASHED		= 6;
const int PSPIW_ERR_FILTER_INVALID		= 7;
const int PSPIW_ERR_IMAGE_INVALID		= 10;
const int PSPIW_ERR_MEMORY_ALLOC		= 11;
const int PSPIW_ERR_INIT_PATH_EMPTY		= 12;
const int PSPIW_ERR_WORK_PATH_EMPTY		= 13;
const int PSPIW_ERR_BAD_PARAM			= 14;
// enums
enum TImgType {PSPIW_IMT_BGR = 0, PSPIW_IMT_BGRA, PSPIW_IMT_RGB, PSPIW_IMT_RGBA, PSPIW_IMT_GRAY, PSPIW_IMT_GRAYA };
// typedefs
typedef void (__stdcall *ENUMCALLBACK)(const char *, const char *, const char *, const wchar_t *);
typedef void (__stdcall *PROGRESSCALLBACK)(unsigned int, unsigned int);
typedef bool (__stdcall *COLORPICKERCALLBACK)(unsigned int);

