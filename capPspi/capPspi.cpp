// capPsp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "pspiHost.h"
#include <string>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/highgui/highgui_c.h"
using namespace cv;
static int counter;
//-----------------------------------------------------------
void __stdcall enumerator(const char *piCategory, const char *piName, const char *piEntry, const wchar_t *filterPath)
{
	std::wstring file_name = std::wstring(filterPath);
	std::cout << piCategory << "; " << piName << "; " << piEntry<<"; ";
	std::wcout<<file_name<<L"\n";
	counter++;
}
//-----------------------------------------------------------
int main(int argc, char *argv[])
{
	argc--;
	argv++;
	if (argc != 3) { fprintf(stderr, " Bad arguments! Calling sequence:\n"
								   " capPspi -command <input_image> <8bf-filter>\n"
                                   " <input_image> -> path to input image \n"
                                   " <8bf-filter> -> path to 8bf filter or path to folder for enumerate command\n"
								   " commands: -execolor, -exeunchanged, -enumerate, -about\n" ) ; exit(1); }
	String command = argv[0];
	String input_filename = argv[1];
	String filter_path = argv[2];
	std::wstring filter(filter_path.begin(), filter_path.end());
	int retCode;
	Mat srcImage;
	if (command == "-execolor")
		srcImage = imread(input_filename,  IMREAD_COLOR);
	else if (command == "-exeunchanged")
		srcImage = imread(input_filename,  IMREAD_UNCHANGED);
	else if (command == "-enumerate")
		{
		// test enumeration
		pspiSetPath((wchar_t*)filter.c_str());
		pspiPlugInEnumerate(enumerator, true);
		return 0;
		}
	else if (command == "-about")
		{
		retCode = pspiPlugInLoad((wchar_t*)(filter.c_str()));
		HWND wh = (HWND)cvGetWindowHandle("Image");
		if (retCode == 0)
			retCode = pspiPlugInAbout( 0);

		return  retCode;
		}
	else
		{
		srcImage = imread(input_filename,  IMREAD_COLOR);		
		}
	namedWindow("Inp image", WINDOW_AUTOSIZE);  // Create a window for display.
	imshow("Inp image", srcImage);               // Show our image inside it.
	waitKey(3);
	counter = 0;
	HWND wh = (HWND)cvGetWindowHandle("Image");
	//
	TImgType type;
	if (srcImage.channels() == 4)
		type = PSPIW_IMT_BGRA;
	else if (srcImage.channels() == 3)
		type = PSPIW_IMT_BGR;
	else if (srcImage.channels() == 1)
		type = PSPIW_IMT_GRAY;
	pspiSetImage(type, srcImage.cols, srcImage.rows, (void*)srcImage.data, srcImage.step);	
	/* test external alpha	if (srcImage.channels() == 4)
	Mat bgra[4];
	split(srcImage, bgra);
	Mat srcAlpha = bgra[3];
	pspiSetImage(PSPIW_IMT_BGR, imInput.cols, imInput.rows, (void*)imInput.data, imInput.step, (void*)imAlpha.data, imAlpha.step);	
	*/
	std::wcout << filter;
	retCode = pspiPlugInLoad((wchar_t*)(filter.c_str()));
	if (retCode	== 0)
		retCode = pspiPlugInExecute(wh);
	imshow("Out image", srcImage);               // Show our image.
	waitKey(0);
	return retCode;
}

