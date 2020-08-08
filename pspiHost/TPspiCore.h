#pragma once
#include <windows.h>
#include <windowsx.h>
#include <PIGeneral.h>
#include "PIAcquire.h"
#include "PIAbout.h"
#include "PIFilter.h"
#include <PIProperties.h>
#include "pspiGlobals.h"
#include "pspiStructs.h"
#include <unordered_map> 
#define signatureSize	4
static char cSig[signatureSize] = {'s', 'p', 'H', 'O'};
const int maxChans = 5;
using namespace std;
//
class TPspiCore
{
	private:
		int chanOrder[maxChans];
		bool loadPIPLResources(HINSTANCE DLL_Handle);
		bool loadPIMIResources(HINSTANCE module);
		void enumResourcesSingle(wstring &filter, ENUMCALLBACK enumFunc);
		void enumResourcesPath(wstring &piFolder, ENUMCALLBACK enumFunc, bool recurseSubFolders);
		void prepareSuites(void);
		void prepareFilter(void);
		void buffer2Scanlines(void *buffer, LPBYTE *scan, int height, int stride, LPBYTE *cpyScan = 0);
		void *resizeBuffer(void *data, int &rowBytes, Rect rect, int loPlane, int hiPlane, int &prevSize);
		int resizeImage(SpspiImage *src, SpspiImage *res, float sampleRate, int rectW, int rectH);
		bool image2Buffer(SpspiImage *image, void *data, Rect plugRect, int rowBytes, int loPlane, int hiPlane, SpspiImage *res = NULL, float sampleRate = 0.0f);
		bool buffer2Image(SpspiImage *image, void *data, Rect plugRect, int rowBytes, int loPlane, int hiPlane);
		void dst2Src(void);
		void releaseImage(SpspiImage *img, bool dispose);
		//void releaseMask(SpspiImage *mask);
		int filterStage;
		unordered_map <unsigned long, SpspiHandle*> handleMap;
		// pointer to host record
		SpspiHostRecord *hRecordPtr;
		// filter record
		FilterRecord fRecord;
		Str255 errString;
		//
		SPBasicSuite bSuite;
		WriteDescriptorProcs wDescriptorProcs;
		ReadDescriptorProcs rDescriptorProcs;
		PIDescriptorParameters pDescriptorParameters;
		//
		BigDocumentStruct bigDoc;
		// procs
		BufferProcs bProcs;
		PropertyProcs pProcs;
		ResourceProcs rProcs;
		HandleProcs hProcs;
		ChannelPortProcs cProcs;
		ImageServicesProcs iProcs;
		PlatformData pData;
		//
		SpspiAdvanceState aState;
	public:
		SpspiImage SrcImage;
		SpspiImage DstImage;
		SpspiImage ResImage;
		SpspiImage SrcMask;
		SpspiImage ResMask;
		SpspiHostRecord HostRecord;
		PROGRESSCALLBACK ProgressCallBack;
		COLORPICKERCALLBACK ColorPickerCallback;
		// const/desc
		TPspiCore();
		~TPspiCore();
		// methods 
		void ProcessAdvanceState(void);
		int SetPath(const wchar_t *filterFolder);
		void ReleaseAllImages(void);
		int SetImage(TImgType type, int width, int height, void *imageBuff, int imageStride, void *alphaBuff, int alphaStride);
		int SetMask(int width, int height, void *maskBuff, int maskStride, bool useByPi);
		int StartImageSL(TImgType type, int width, int height, bool externalAplha);
		int AddImageSL(int scanIndex, void *imageScanLine, void *alphaScanLine);
		int FinishImageSL(int imageStride, int alphaStride);
		int StartMaskSL(int width, int height, bool useByPi);
		int AddMaskSL(int scanIndex, void *maskScanLine);
		int FinishMaskSL(int maskStride);
		int PlugInLoad(const wchar_t *filter);
		int PlugInAbout(HWND hWnd);
		int PlugInExecute(HWND hWnd);
		int PlugInEnumerate(ENUMCALLBACK enumFunc, bool recurseSubFolders);
		// inline
		inline float Fixed2Float(Fixed f_number);
		inline Fixed FixRatio(short numer, short denom);
		inline bool EqRects(Rect *r1, Rect *r2);
};

