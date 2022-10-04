#include "stdafx.h"
#include <string>
#include <vector>
#include <iostream>
#include <experimental/filesystem>
#include "TPspiCore.h"
#include "pspiSuites.hpp"
//#include "uxtheme.h"
namespace fsys = experimental::filesystem;
static vector<string> GFilterData;
static bool GFilterInputHandling[7];
static bool GFilterOutputHandling[7];
//static bool GDontCopySrc2Dst;
//void *GThisPtr;
static HMODULE GHModule;
static HHOOK GHookEx;
static int GFilterStage;
static HWND GAppHandle;
//-----------------------------------------------------------------
// extern declarations for accesing meber moethods from pspiSuites.hpp
//-----------------------------------------------------------------
extern "C" void AdvanceStateProcessPtr(void* object)
{
    static_cast<TPspiCore*>( object)->ProcessAdvanceState();
}
//-----------------------------------------------------------------
extern "C" bool ColorPickerProcessPtr(void* object, uint32 &color)
{
	if (static_cast<TPspiCore*>(object)->ColorPickerCallback == NULL)
		return false;
    return static_cast<TPspiCore*>( object)->ColorPickerCallback(color);
}
//-----------------------------------------------------------------
extern "C" void ProgressProcessPtr(void* object, uint32 done, uint32 total)
{
	if (static_cast<TPspiCore*>(object)->ProgressCallBack == NULL)
		return;
    static_cast<TPspiCore*>( object)->ProgressCallBack(done, total);
}
//---------------------------------------------------------------------------
// Block for unstyling - for Embarcadero VCL styles affecting plug-in windows
// I don't have time for this crap...
//---------------------------------------------------------------------------
/*
LRESULT CALLBACK spHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	int test = 0;
	if (nCode < 0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);

	if (nCode == HCBT_ACTIVATE)
	{
		HWND hWnd = (HWND)wParam;
		HWND hPar = GetParent(hWnd);
		if (GFilterStage > 0 &&	GFilterStage < 4)
		{
			if (hWnd)
			{
				SetWindowTheme(hWnd, L" ", L" ");
				// DEBUG OUTPUT
				cout << hWnd << "\n";
				cout << hPar << "\n";
			}
		}	
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
*/
//---------------------------------------------------------------------------
// PIPL enumerator - non-class function
//---------------------------------------------------------------------------
BOOL CALLBACK spEnumPIPL(HINSTANCE module, LPCWSTR type, LPCWSTR name, LONG param)
{
 GFilterData.clear();
 bool loadedPiPL = false;
 int so_far = 0;
 string entrypointname;
 string piname;
 string picategory;
 long priority;
 int32 version;
 LPBYTE pipl;
 PIProperty *prop;
 int count;
 enum { IsUnknown, IsAcquisition, IsFilter } kind = IsUnknown;
 //
 // Take a walk
 //
 HRSRC hrsrc = FindResourceW(module, name, type);
 HGLOBAL hpipl = LoadResource(module, hrsrc);
 if (hpipl != 0)
    {
	pipl =  (LPBYTE)LockResource(hpipl);
    if (pipl)
       {
	   int16 fb = *(int16*)pipl;		// PSP sign -> should be 1
	   version = *(int32*)(pipl + 2);   // version
	   count = *(int32*)(pipl + 6);
       prop = (PIProperty *) (pipl + 10);
       //PIPLHeader *header = (PIPLHeader *)pipl;
       //pipl += sizeof *header;
       //
       // We only recognize version 0.
       //
       if (version == 0)
          {
          loadedPiPL = true;
          //
          // Enumerate through the properties, grabbing the data we need.
          //
          for (int p = 0 ; p < count ; p += 1)
              {
              /*
              PIProperty *prop = (PIProperty *)pipl;
              pipl += sizeof *prop;
              const char *data = pipl;
              pipl += (prop->propertyLength+3)&~3; */
              switch (prop->propertyKey)
                 {
                 case PIKindProperty:
                      {
                      if (memcmp (prop->propertyData, "MFB8", 4) == 0)
                         kind = IsFilter;
                      /*
                      else if (memcmp(prop->propertyData, "MAB8", 4) == 0)
                         kind = IsAcquisition; */
                      else
                         loadedPiPL = false;
                      }
					  break;
                 case PIVersionProperty:
                      {
                      int *versionp = (int *)(prop->propertyData);
                      int version_major = *versionp >> 16;
                      int version_minor = *versionp & 0x0FFFF;
                      if (version_major > 4 || (version_major == 4 && version_minor > 0))
                         loadedPiPL = false;
                      }
                 case PIPriorityProperty:
                      {
                      priority = *((long *)prop->propertyData);
                      }
                      break;
                 case PIRequiredHostProperty:
	                  /* 8BIM
	 				  {
                      loadedPiPL = false;
                      }
					  */
                      break;
                 case PICategoryProperty:
                      {
                      unsigned char strlen = *(unsigned char *)prop->propertyData;
                      const char *str = prop->propertyData+1;
                      char *tname = new char[strlen+1];
                      //memcpy(&FilterString[so_far], str, strlen);
                      //FilterString[strlen] = '\0';
                      //so_far += strlen;
                      memcpy(tname, str, strlen);
                      tname[strlen] = '\0';
                      picategory = string(tname);
                      delete [] tname;
                      }
                      break;
                 case PINameProperty:
                      {
                      unsigned char strlen = *(unsigned char *)prop->propertyData;
					  const char *str = prop->propertyData +1;
                      char *tname = new char[strlen+1];
                      memcpy(tname, str, strlen);
                      tname[strlen] = '\0';
                      piname = string(tname);
                      delete [] tname;
                      }
                      break;
				 case PIImageModesProperty:
					 /*
					 {
					 unsigned int mode = (unsigned int)*(prop->propertyData);
					 if (mode < plugInModeRGBColor)
						  loadedPiPL = false; // test
					 }
					 */
					 break;
                 case PIWin32X86CodeProperty:
				 case PIWin64X86CodeProperty: 	
                      {
                      entrypointname = string(prop->propertyData);
                      }
                      break;
                 case PIFilterCaseInfoProperty:
                      {
					  // fici key -> 7 * 4 bytes
                      FilterCaseInfo *fici_ptr;
                      fici_ptr = (FilterCaseInfo *)prop->propertyData;
					  for (int i = 0; i < 7; i++)
					  {
						  GFilterInputHandling[i] = (fici_ptr[i].inputHandling > 0x00);
						  GFilterOutputHandling[i] = (fici_ptr[i].outputHandling > 0x00);
					  }
					  // check flags1 (only for 0, should be the same for all...probably)
					  //GDontCopySrc2Dst = fici_ptr[0].flags1 & PIFilterDontCopyToDestinationBit;
                      }
                      break;
                 }
              if (!loadedPiPL) break;
              prop = (PIProperty *) ((&prop->propertyData) + prop->propertyLength);
              }
           }
        }
    }
if (loadedPiPL)
{
	GFilterData.push_back(picategory);
	GFilterData.push_back(piname);
	GFilterData.push_back(entrypointname);
}
return loadedPiPL;
}
//-----------------------------------------------------------------
// CLASS members
// trivial constructor
//-----------------------------------------------------------------
TPspiCore::TPspiCore()
{
	 GHModule = GetModuleHandle("pspiHost.dll");
	 GFilterStage = 0;
	 GAppHandle = 0;
	 //freopen("output.log","w",stdout); <- collect log
	 int buff_size = 65535;
	 HostRecord.initialEnvPath.resize(buff_size);
	 GetEnvironmentVariableW((LPCWSTR)L"PATH", (LPWSTR)&(HostRecord.initialEnvPath[0]), buff_size);
	 ColorPickerCallback = 0;
	 ProgressCallBack = 0;
	 hRecordPtr = &HostRecord;
	 SuitesFPR = &fRecord;
	 SuitesHostPR = hRecordPtr;
	 SuitesASP = AdvanceStateProcessPtr;
	 SuitesCPR = ColorPickerProcessPtr;
	 SuitesPGR = ProgressProcessPtr;
	 SuitesHMP = &handleMap;
	 SuitesCorePtr = this;
	 SuitesChanOrder = &chanOrder[0];
}
//-----------------------------------------------------------------
// destructor
//-----------------------------------------------------------------
TPspiCore::~TPspiCore()
{
	if (!HostRecord.initialEnvPath.empty())
		SetEnvironmentVariableW((LPCWSTR)L"PATH", &HostRecord.initialEnvPath[0]);
	ReleaseAllImages();
	if (HostRecord.dllHandle)
	{
		try
		{
			FreeLibrary(HostRecord.dllHandle);
		}
		catch (...)
		{
			// never mind
		}
	}
}
//-----------------------------------------------------------------
// private methods section
//-----------------------------------------------------------------
void TPspiCore::releaseImage(SpspiImage *img, bool dispose)
{
	hlpReleaseImage(img, dispose);
/*
	if (img->width)
	{
		if (img->imageScan)
			delete []img->imageScan;
		if (img->exaScan)
			delete []img->exaScan;
		if (img->imageBuff && dispose)
			GlobalFree(img->imageBuff);
		if (img->exaBuff && dispose)
			GlobalFree(img->exaBuff);
		img->imageScan = 0;
		img->exaScan = 0;
		img->imageBuff = 0;
		img->exaBuff = 0;
		img->width = 0;
		img->height = 0;
		img->imageStride = 0;
		img->exaStride = 0;
		img->channels = 0;
		img->exaChannels = 0;
	}
*/
}
//-----------------------------------------------------------------
// set windowsHookEx
//-----------------------------------------------------------------
/*
void TPspiCore::setUnstyler(void)
{
	 GHookEx = SetWindowsHookEx(WH_CBT, (HOOKPROC)spHookProc, GHModule, 0);
	 DWORD erc = GetLastError();
	 if (erc > 0)
		 return;

}
*/
//-----------------------------------------------------------------
void TPspiCore::getAviailableSpace(int &space32, int64 &space64)
{
	// Let's start with 1GB
	space32 = 1024 * 1024 * 1024;
	space64 = (int64)space32;
	//
	MEMORYSTATUSEX statex;
	memset(&statex, 0, sizeof(statex));
	statex.dwLength = sizeof (statex);
	if (GlobalMemoryStatusEx(&statex))
	{
		space64 = statex.ullAvailVirtual;
		if (statex.ullAvailVirtual < INT_MAX)
			space32 = (int)statex.ullAvailVirtual;
		else
			space32 = 0x7FFFFFFF;	// 2GB max
	}
}
//-----------------------------------------------------------------
bool TPspiCore::loadPIPLResources(HINSTANCE DLL_Handle)
{
 if (DLL_Handle)
	return EnumResourceNamesW(DLL_Handle, L"PiPL", reinterpret_cast <ENUMRESNAMEPROCW> (spEnumPIPL), 0);
 else
    return false;
}
//---------------------------------------------------------------------------
bool TPspiCore::loadPIMIResources(HINSTANCE module)
{
 GFilterData.clear();
 bool loadedPiMI = false;
 string delim = "\n";
 string entrypointname;
 string piname;
 string picategory;
 //long priority;
 LPBYTE pimi;
 //long version;
  //
 // Šetaj šetnju kroz handlove.
 //
 for (int resID = 1 ; ; resID += 1)
	 {
	 string resID_str = to_string(resID);
	 HRSRC hrsrc = FindResource(module, resID_str.c_str(), "PiMI");
     if (hrsrc == 0)
        break;
     else
       {
       HGLOBAL hpimi = LoadResource(module, hrsrc);
       if (hpimi == 0)
          break;
       else
          {
          pimi = (LPBYTE)LockResource(hpimi);
          if (pimi == NULL)
             break;
          else
             {
             // _8BFM data
             HRSRC bfm_res = FindResource(module, resID_str.c_str(), "_8BFM");
             if (bfm_res != 0)
                {
                // OK, skip version and grab category name
                pimi += 2;
                //
                // Next comes the name of the interface.
                //
                picategory = to_string(*pimi);
                HGLOBAL bfm_hand = LoadResource(module, bfm_res);
                if (bfm_hand == 0)
                   break;
                LPBYTE bfm_pimi = (LPBYTE)LockResource(bfm_hand);
                // OK, let's skip version
                bfm_pimi += 2;
                piname = to_string(*bfm_pimi);
                entrypointname = "ENTRYPOINT"+to_string(resID);
				GFilterData.push_back(picategory);
				GFilterData.push_back(piname);
				GFilterData.push_back(entrypointname);
                loadedPiMI = true;
                }
             }
           }
        }
    }
return loadedPiMI;
}
//---------------------------------------------------------------------------
// Enumerate Filter Resources Single
//---------------------------------------------------------------------------
void TPspiCore::enumResourcesSingle(wstring &filter, ENUMCALLBACK enumFunc)
{
    HINSTANCE dllHandle;
	bool lRet;
	try
	{
		dllHandle = LoadLibraryW(filter.c_str());
	}
	catch (...)
	{
		return;
	}
	lRet = loadPIPLResources(dllHandle);
	if (!lRet)
		lRet = loadPIMIResources(dllHandle);		
	try
	{
		FreeLibrary(dllHandle);
	}
	catch (...)
	{
		// never mind
	}
	// prepare output
	if (lRet)
	{
		if (GFilterData.size() == 3)
			enumFunc(GFilterData[0].c_str(), GFilterData[1].c_str(),  GFilterData[2].c_str(), filter.c_str());
		else
		{
			string empty = "";
			enumFunc(empty.c_str(), empty.c_str(), empty.c_str(), filter.c_str());
		}
	}
}
//---------------------------------------------------------------------------
//
// Enumerate Filter Resources Path
//
//---------------------------------------------------------------------------
void TPspiCore::enumResourcesPath(wstring &piFolder, ENUMCALLBACK enumFunc, bool recurseSubFolders)
{
	wstring file_path, extension;
	if (recurseSubFolders)
	{
		auto d = fsys::recursive_directory_iterator(piFolder);	
		for (auto& p : d)
		{
			file_path = p.path();
			extension = p.path().extension();
			transform(extension.begin(), extension.end(), extension.begin(),  towlower);
			if (extension == L".8bf" && enumFunc)
				enumResourcesSingle(file_path, enumFunc);
		}
	}
	else
	{
		auto d = fsys::directory_iterator(piFolder);
		for (auto& p : d)
		{
			file_path = p.path();
			extension = p.path().extension();
			transform(extension.begin(), extension.end(), extension.begin(),  towlower);
			if (extension == L".8bf" && enumFunc)
				enumResourcesSingle(file_path, enumFunc);
		}
	}
 return;
}
//---------------------------------------------------------------------------
void  TPspiCore::prepareSuites(void)
{
	 // property procs
	 pProcs.propertyProcsVersion = kCurrentPropertyProcsVersion;
	 pProcs.numPropertyProcs = kCurrentPropertyProcsCount;
	 pProcs.getPropertyProc = DoSuiteGetProperty;
	 pProcs.setPropertyProc = DoSuiteSetProperty;
	 // resource procs
	 rProcs.resourceProcsVersion =   kCurrentResourceProcsVersion;
	 rProcs.numResourceProcs = kCurrentResourceProcsCount;
	 rProcs.countProc = DoCountPIResources;
	 rProcs.getProc = DoGetPIResource;
	 rProcs.deleteProc = DoDeletePIResource;
	 rProcs.addProc = DoAddPIResource;
	 // handle procs
	 hProcs.handleProcsVersion =    kCurrentHandleProcsVersion;
	 hProcs.numHandleProcs     = kCurrentHandleProcsCount;
	 hProcs.newProc		=	  DoNewPIHandle;
	 hProcs.disposeProc =     DoDisposePIHandle;
	 hProcs.getSizeProc =     DoGetPIHandleSize;
	 hProcs.setSizeProc =     DoSetPIHandleSize;
	 hProcs.lockProc	=        DoLockPIHandle;
	 hProcs.unlockProc  =      DoUnlockPIHandle;
	 hProcs.recoverSpaceProc =  DoRecoverSpace;
	 hProcs.disposeRegularHandleProc =  DoDisposeRegular;
	 //
	 // channel port procs
	 cProcs.channelPortProcsVersion = kCurrentChannelPortProcsVersion;
	 cProcs.numChannelPortProcs = kCurrentChannelPortProcsCount;
	 cProcs.readPixelsProc = DoReadPixels;
	 cProcs.writeBasePixelsProc = DoWriteBasePixels;
	 cProcs.readPortForWritePortProc = DoReadPortForWritePort;
	 //
	 // image service procs
	 iProcs.imageServicesProcsVersion = kCurrentImageServicesProcsVersion;
	 iProcs.numImageServicesProcs = kCurrentImageServicesProcsCount;
	 iProcs.interpolate1DProc = DoInterpolate1D;
	 iProcs.interpolate2DProc = DoInterpolate2D;
	 iProcs.interpolate1DMultiProc = DoInterpolate1Dmulti;
	 iProcs.interpolate2DMultiProc = DoInterpolate2Dmulti;
	 iProcs.interpolate1DMulti32Proc = DoInterpolate1Dmulti32;
	 iProcs.interpolate2DMulti32Proc = DoInterpolate2Dmulti32;
	 // platform data
	 pData.hwnd = (intptr_t)hRecordPtr->hWnd;
	 // descriptor procs
	 memset(&wDescriptorProcs, 0, sizeof(wDescriptorProcs));
	 memset(&rDescriptorProcs, 0, sizeof(rDescriptorProcs));
	 pDescriptorParameters.descriptorParametersVersion = kCurrentDescriptorParametersVersion;
	 pDescriptorParameters.playInfo = 0;
	 pDescriptorParameters.recordInfo = 0;
	 pDescriptorParameters.descriptor = 0;
	 pDescriptorParameters.writeDescriptorProcs = 0; // &wDescriptorProcs;
	 pDescriptorParameters.readDescriptorProcs = 0; // &rDescriptorProcs;
	 // buffer procs
	 bProcs.bufferProcsVersion = kCurrentBufferProcsVersion;;
	 bProcs.numBufferProcs = kCurrentBufferProcsCount;
	 bProcs.allocateProc = DoAllocateBuffer;
	 bProcs.lockProc = DoLockBuffer;
	 bProcs.unlockProc = DoUnlockBuffer;
	 bProcs.freeProc = DoFreeBuffer;
	 bProcs.spaceProc = DoBufferSpace;
	 bProcs.reserveProc = DoBufferReserve;
	 bProcs.allocateProc64 = DoAllocateBuffer64;
	 bProcs.spaceProc64 = DoBufferSpace64; 
	 // big doc - new stuff
	 // NOTE: it plugin uses big doc struct, 	bigDoc.PluginUsing32BitCoordinates will be non-zero
	 // we will set data for this structure, but not use it now
	 memset(&bigDoc, 0, sizeof(bigDoc));
	 //--- filter rect
	 if (!hRecordPtr->roiRect.IsEmpty())
     {
		bigDoc.filterRect32.top  = hRecordPtr->roiRect.top;
		bigDoc.filterRect32.left = hRecordPtr->roiRect.left;
		bigDoc.filterRect32.bottom = hRecordPtr->roiRect.bottom;
		bigDoc.filterRect32.right = hRecordPtr->roiRect.right;
     }
 	 else
     {
		bigDoc.filterRect32.top = 0;
		bigDoc.filterRect32.left = 0;
		bigDoc.filterRect32.bottom = hRecordPtr->srcImage->height;
		bigDoc.filterRect32.right = hRecordPtr->srcImage->width;
     }
	 bigDoc.imageSize32.h = hRecordPtr->srcImage->width;
	 bigDoc.imageSize32.v = hRecordPtr->srcImage->height;
	 //
	 bigDoc.wholeSize32.h = hRecordPtr->srcImage->width;
	 bigDoc.wholeSize32.v = hRecordPtr->srcImage->height;
	 }
//---------------------------------------------------------------------------
void  TPspiCore::prepareFilter(void)
{
	int filterInputCase;
	bool skipAlpha;
	BYTE f_color[3], b_color[3];
	// set initial foreground color
	f_color[chanOrder[0]] = BYTE(((hRecordPtr->foregroundColor) << 8) >> 24);
	f_color[chanOrder[1]] = BYTE(((hRecordPtr->foregroundColor) << 16) >> 24);
	f_color[chanOrder[2]] = BYTE(((hRecordPtr->foregroundColor) << 24) >> 24);
	// set initial background color
	b_color[chanOrder[0]] = BYTE(((hRecordPtr->backgroundColor) << 8) >> 24);
	b_color[chanOrder[1]] = BYTE(((hRecordPtr->backgroundColor) << 16) >> 24);
	b_color[chanOrder[2]] = BYTE(((hRecordPtr->backgroundColor) << 24) >> 24);
	//-----------------------------------
	// Nullify complete filter record
	//-----------------------------------
	memset(&fRecord, 0, sizeof(fRecord));
	//-----------------------------------
	// fill up filter record fields
	//-----------------------------------
	fRecord.serialNumber = 0;					        // Host's serial number, to allow copy protected plug-in modules.
	memcpy((char *)&fRecord.hostSig, &cSig[0], 4);		// host signature - that's me
	// size of complete image if selection is not floating
	fRecord.imageSize.v = hRecordPtr->srcImage->height;	// Size of image v
	fRecord.imageSize.h = hRecordPtr->srcImage->width;		// Size of image h
	fRecord.planes = hRecordPtr->colorChannels + hRecordPtr->alphaChannels;    // planes   
	// filter rect
	if (!hRecordPtr->roiRect.IsEmpty())
	{
		fRecord.filterRect.top = hRecordPtr->roiRect.top;      // Rectangle to filter top
		fRecord.filterRect.left = hRecordPtr->roiRect.left;     // Rectangle to filter left
		fRecord.filterRect.bottom = hRecordPtr->roiRect.bottom;   // Rectangle to filter bottom
		fRecord.filterRect.right = hRecordPtr->roiRect.right;    // Rectangle to filter right
	}
	else
	{
		fRecord.filterRect.top = 0;                  // Rectangle to filter top
		fRecord.filterRect.left = 0;                  // Rectangle to filter left
		fRecord.filterRect.bottom = hRecordPtr->srcImage->height;    // Rectangle to filter bottom
		fRecord.filterRect.right = hRecordPtr->srcImage->width;      // Rectangle to filter right
	}
	// current background 
	fRecord.background.red = b_color[0];
	fRecord.background.green = b_color[1];
	fRecord.background.blue = b_color[2];
	// current foreground 
	fRecord.foreground.red = f_color[0];
	fRecord.foreground.green = f_color[1];
	fRecord.foreground.blue = f_color[2];
	// back color
	fRecord.backColor[0] = b_color[0];
	fRecord.backColor[1] = b_color[1];
	fRecord.backColor[2] = b_color[2];
	fRecord.backColor[3] = 0xff;
	// fore color
	fRecord.foreColor[0] = f_color[0];
	fRecord.foreColor[1] = f_color[1];
	fRecord.foreColor[2] = f_color[2];
	fRecord.foreColor[3] = 0xff;
	// let's calcualte available space
	getAviailableSpace(fRecord.maxSpace, fRecord.maxSpace64);
	fRecord.bufferSpace = fRecord.maxSpace;
	fRecord.bufferSpace64 = fRecord.maxSpace64;
	//fRecord.inRect = Rect();    
	//fRecord.inLoPlane = 0;
	//fRecord.inHiPlane = 2;
	//fRecord.outRect = Rect();
	//fRecord.floatCoord.h = 50;
	//fRecord.floatCoord.v = 50;
	//
	//
	//fRecord.outLoPlane = 0;
	//fRecord.outHiPlane = 2;
	//fRecord.inData = NULL;  // pointer to input buffer
	//fRecord.inRowBytes = 0;  // inrow bytes
	//fRecord.outData = NULL;   // pointer to output buffer
	//fRecord.outRowBytes = 0;       
	//fRecord.maskData = NULL;
	//fRecord.isFloating = false;
	//fRecord.autoMask = false; 
	//
	// Shitty code...buf it will probably work
	//
	skipAlpha = false;
	if (hRecordPtr->mask == 0 || !(hRecordPtr->useMaskByPi))
	{
		if (hRecordPtr->alphaChannels == 0)
		{
			filterInputCase = filterCaseFlatImageNoSelection;
			skipAlpha = true;
		}
		else
		{
			filterInputCase = filterCaseEditableTransparencyNoSelection;
			if (!GFilterInputHandling[filterInputCase])
				filterInputCase = filterCaseProtectedTransparencyNoSelection;
			if (!GFilterInputHandling[filterInputCase])
			{
				filterInputCase = filterCaseFlatImageNoSelection;
				skipAlpha = true;
			}
		}
		fRecord.haveMask = false;
	}
	else
	{
		if (hRecordPtr->alphaChannels == 0)
		{
			filterInputCase = filterCaseFlatImageWithSelection;
			skipAlpha = true;
		}
		else
		{
			filterInputCase = filterCaseEditableTransparencyWithSelection;
			if (!GFilterInputHandling[filterInputCase])
				filterInputCase = filterCaseProtectedTransparencyWithSelection;
			if (!GFilterInputHandling[filterInputCase])
			{
				filterInputCase = filterCaseFlatImageWithSelection;
				skipAlpha = true;
			}
		}
		if (!GFilterInputHandling[filterInputCase])
		{
			filterInputCase = filterCaseFlatImageNoSelection;  // go to lowest possible case
			fRecord.haveMask = false;
			skipAlpha = true;
		}
		else
			fRecord.haveMask = true;
	}
	fRecord.filterCase = filterInputCase;
	switch (hRecordPtr->imgType)
	{
		case PSPI_IMG_TYPE_GRAY:
		case PSPI_IMG_TYPE_GRAYA:
			fRecord.imageMode = plugInModeGrayScale;
			fRecord.depth = 8;
			break;
		default:
			fRecord.imageMode = plugInModeRGBColor;         // color mode
			fRecord.depth = 8;
			break;
	}
	//
	// now let's set planes and layers stuff
	//
	if (skipAlpha)
	{
		fRecord.inLayerPlanes = 0;	
		fRecord.inTransparencyMask = 0; 
		fRecord.inNonLayerPlanes = hRecordPtr->colorChannels;
		fRecord.outLayerPlanes = fRecord.inLayerPlanes;
		fRecord.outTransparencyMask = fRecord.inTransparencyMask;
		fRecord.outNonLayerPlanes = fRecord.inNonLayerPlanes;
		fRecord.outColumnBytes = fRecord.inColumnBytes;
		fRecord.absNonLayerPlanes = hRecordPtr->colorChannels + hRecordPtr->alphaChannels;
		fRecord.planes = hRecordPtr->colorChannels;
	}
	else
	{
		fRecord.inLayerPlanes = hRecordPtr->colorChannels;
		fRecord.inTransparencyMask = hRecordPtr->alphaChannels;
		fRecord.inNonLayerPlanes = 0;
		fRecord.absNonLayerPlanes = fRecord.inNonLayerPlanes;
		fRecord.absTransparencyMask = fRecord.inTransparencyMask;
		fRecord.absLayerMasks		= fRecord.inLayerMasks;
		fRecord.absInvertedLayerMasks = fRecord.inInvertedLayerMasks;
		fRecord.planes = hRecordPtr->colorChannels + hRecordPtr->alphaChannels;
		if (filterInputCase == filterCaseProtectedTransparencyNoSelection || filterInputCase == filterCaseProtectedTransparencyWithSelection)
		{
			fRecord.planes = hRecordPtr->colorChannels;
			fRecord.outLayerPlanes = 0;
			fRecord.outTransparencyMask = 0;
			fRecord.outNonLayerPlanes = hRecordPtr->colorChannels;
			fRecord.outColumnBytes = hRecordPtr->colorChannels;
		}
		else
		{
			fRecord.outLayerPlanes = fRecord.inLayerPlanes;
			fRecord.outTransparencyMask = fRecord.inTransparencyMask;
			fRecord.outNonLayerPlanes = fRecord.inNonLayerPlanes;
			fRecord.outColumnBytes = fRecord.inColumnBytes;
		}
	}
	// rest of the stuff
	fRecord.inPlaneBytes = 1;
	fRecord.outPlaneBytes = 1;
	// 
	fRecord.imageHRes = FixRatio(96, 1);            // pixels per inch
	fRecord.imageVRes = fRecord.imageHRes;
	//fRecord.floatCoord; 	   -  Top left coordinate of selection
	fRecord.wholeSize.v = hRecordPtr->srcImage->height;  //  Size of image selection is floating over
	fRecord.wholeSize.h = hRecordPtr->srcImage->width;   //  Size of image selection is floating over
	//fRecord.monitor;	   -  Information on current monitor
	//
	// suites and precesses
	//
	fRecord.hostProc = DoHostProc;		// host proc - ne treba ???
	fRecord.platformData = &pData;		// Platform specific information.
	fRecord.bufferProcs = &bProcs;		// The host buffer procedures.
	fRecord.resourceProcs = &rProcs;       // The host plug-in resource procedures.
	fRecord.handleProcs = &hProcs;  	    // Platform independent handle manipulation.
	fRecord.propertyProcs = &pProcs;        //Routines to query and set document and view properties...
	fRecord.processEvent = DoProcessEvent;     // Pass event to the application.
	fRecord.displayPixels = DoDisplayPixels;  // Display dithered pixels.
	fRecord.advanceState = DoAdvanceState;	// Advance from start to continue...
	fRecord.getPropertyObsolete = DoSuiteGetProperty;	 // Use the suite if available
	fRecord.colorServices = DoColorServices;  // Routine to access color services.
	fRecord.descriptorParameters = &pDescriptorParameters;	// For recording and playback
	fRecord.errorString = &errString;       // For silent and errReportString
	fRecord.channelPortProcs = &cProcs;     // Suite for passing pixels through channel ports.
	fRecord.imageServicesProcs = NULL; //&iProcs; // Suite of image processing callbacks.
	fRecord.documentInfo = NULL;	         // The document info for the document being filtered.
	fRecord.abortProc = DoTestAbort;        // The plug-in module may call this no-argument...
	fRecord.progressProc = (ProgressProc)DoProgressProc;  // The plug-in module may call this two-argument...
	fRecord.parameters = NULL;              // A handle, initialized to NIL by Photoshop.
	//fRecord.sSPBasic = &bSuite;     	// SuitePea basic suite TODO
	fRecord.plugInRef = NULL;       	// plugin reference used by SuitePea						
	//fRecord.supportsDummyChannels;     -  Does the host support dummy channels?
	//fRecord.supportsAlternateLayouts;    -  Does the host support alternate data layouts.
	fRecord.wantLayout = piLayoutTraditional;  // The layout to use for the data...
	//
	//
	fRecord.dummyPlaneValue = -1;			//  0..255 = fill value -1 = leave undefined...
	//fRecord.premiereHook;				//  A hook for Premiere...nope!
	fRecord.supportsAbsolute = 1;			// Does the host support absolute plane indexing?
	fRecord.wantsAbsolute = false;			// Does the plug-in want absolute plane indexing? (input only)
	fRecord.cannotUndo = false;			// If set to TRUE, then undo will not be enabled for this command.
	fRecord.supportsPadding = false;	    // Does the host support requests outside the image area?
	//fRecord.inputPadding = plugInWantsErrorOnBoundsException;        // Instructions for padding the input.
	//fRecord.outputPadding = plugInWantsErrorOnBoundsException;        // Instructions for padding the output.
	//fRecord.maskPadding = plugInWantsErrorOnBoundsException;		   // Padding instructions for the mask.
	fRecord.samplingSupport = true;     // Does the host support sampling the input and mask?
	// fRecord.reservedByte;		       - Alignment.
	fRecord.inputRate = FixRatio(1, 1);		// Input sample rate.
	fRecord.maskRate = FixRatio(1, 1);			// Mask sample rate.
	// tilling
	fRecord.inTileHeight = fRecord.imageSize.v;
	fRecord.inTileWidth = fRecord.imageSize.h;
	fRecord.inTileOrigin.h = 0;
	fRecord.inTileOrigin.v = 0;
	fRecord.absTileHeight = fRecord.inTileHeight;
	fRecord.absTileWidth = fRecord.inTileWidth;
	fRecord.absTileOrigin;
	fRecord.outTileHeight = fRecord.inTileHeight;
	fRecord.outTileWidth = fRecord.inTileWidth;
	fRecord.outTileOrigin.v = 0;
	fRecord.outTileOrigin.h = 0;
	fRecord.maskTileHeight = fRecord.inTileHeight;	// Tiling for the mask.
	fRecord.maskTileWidth = fRecord.inTileWidth;
	fRecord.maskTileOrigin.h = 0;
	fRecord.maskTileOrigin.v = 0;
	fRecord.iCCprofileData = NULL;		// Handle containing the ICC profile for the image. (NULL if none)
	fRecord.iCCprofileSize = 0;		// size of profile.
	//fRecord.canUseICCProfiles;	        // non-zero if the host can export ICC profiles...
	fRecord.bigDocumentData = 0; // &bigDoc; // not for now...
 }
//---------------------------------------------------------------------------
// buffer to scanlines
//---------------------------------------------------------------------------
void TPspiCore::buffer2Scanlines(void * buffer, LPBYTE *scan, int height, int stride, LPBYTE *cpyScan)
{
	LPBYTE ptr = (LPBYTE)buffer;
	if (HostRecord.imgOrientation == PSPI_IMG_ORIENTATION_ASIS)
	{
		for (int i = 0; i < height; i++)
		{
			scan[i] = ptr;
			if (cpyScan != NULL)
				memcpy(scan[i], cpyScan[i], stride);
			ptr = ptr + stride;
		}
	}
	else
	{
		for (int i = height - 1; i >= 0; i--)
		{
			scan[i] = ptr;
			if (cpyScan != NULL)
				memcpy(scan[i], cpyScan[i], stride);
			ptr = ptr + stride;
		}
	}
}
//---------------------------------------------------------------------------
// resize buffer 
//---------------------------------------------------------------------------
void *TPspiCore::resizeBuffer(void *data, int &rowBytes, Rect &rect, int loPlane, int hiPlane, int &size)
{
	void *res_ptr = 0;
	if ((rect.right <= 0) || (rect.bottom <= 0))
	{
		if (data)
		{
		GlobalFree(data);
		rowBytes = 0;
		size = 0;
		}
		return 0;
	}
	// some plugins require rect beyond image border - no chance!
	if (rect.bottom > hRecordPtr->srcImage->height)
		rect.bottom = hRecordPtr->srcImage->height;
	if (rect.right > hRecordPtr->srcImage->width)
		rect.right = hRecordPtr->srcImage->width;
	int nplanes = hiPlane - loPlane + 1;
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	rowBytes = nplanes * w;
	//size = nplanes * w * h;
	size = nplanes * w * (h+1);		// suggested by Irfan Škiljan
	if (data)
	{
		if (size == GlobalSize(data))	// same size, do not resize
			return data;
		GlobalFree(data);
	}
	res_ptr = GlobalAlloc(GMEM_FIXED, size);
	return res_ptr;
}
//---------------------------------------------------------------------------
//
// step calculator for crap resize
//
//---------------------------------------------------------------------------
int TPspiCore::resizeImage(SpspiImage *src, SpspiImage *res, float sampleRate, int rectW, int rectH)
{
	float fract_part, int_part;
	int step = 1;
	fract_part = modf (sampleRate , &int_part);	
	step = (int)int_part;			
	if (step == 0)
		step = 1;
	if (step == 1)
		return step;
	else
	{
		// TODO: check if there is call-back to host resize engine
		// if there is resample call back, perform resample and get out
	}
	int scaleWidth  = (int)(src->width / sampleRate);
	int scaleHeight = (int)(src->height / sampleRate);
	//
	if (rectW > scaleWidth)
		scaleWidth = rectW;
	if (rectH > scaleHeight)
		scaleHeight = rectH;
	if (res->imageBuff && scaleWidth == res->width && scaleHeight == res->height)	// already scaled?
		return step;
	if (res->imageBuff)
		releaseImage(res, true);
	// create resample image
	res->width = scaleWidth;
	res->height = scaleHeight;
	res->channels  = src->channels;
	res->exaChannels = src->exaChannels;
	res->imageStride = src->channels * scaleWidth;
	res->exaStride = src->exaChannels *scaleWidth;
	res->imageBuff = GlobalAlloc(GMEM_FIXED, scaleHeight * res->imageStride);
	res->imageScan = new LPBYTE[scaleHeight];
	// fill res image scanlines
	LPBYTE ptr = (LPBYTE)res->imageBuff;
	for (int i = 0; i < scaleHeight; i++)
	{
		res->imageScan[i] = ptr;
		memset(res->imageScan[i], 0, res->imageStride);
		ptr = ptr + res->imageStride;
	}
	// image has separated alpha channel - create res alpha
	if (src->exaBuff)
	{
		res->exaBuff = GlobalAlloc(GMEM_FIXED, scaleHeight * res->exaStride);
		res->exaScan = new LPBYTE[scaleHeight];	
		ptr = (LPBYTE)res->exaBuff;
		for (int i = 0; i < scaleHeight; i++)
		{
			res->exaScan[i] = ptr;
			memset(res->exaScan[i], 0, res->exaStride);
			ptr = ptr + res->exaStride;
		}
	}
	else
	{
		res->exaScan = 0;
		res->exaStride = 0;
	}
	// calculate crap resample step
	int step_y = src->height / scaleHeight;
	int step_x = src->width / scaleWidth;
	// perform crap resampler	
	LPBYTE src_ip, src_ap = 0, dst_ip = 0, dst_ap = 0;
	for (int i = 0; i < res->height; i++)
	{
		if ((i * step_y) >= src->height)
			break;
		src_ip = src->imageScan[i * step_y];
		dst_ip = res->imageScan[i];
		if (src->exaBuff)
		{
			src_ap = src->exaScan[i * step_y];
			dst_ap = res->exaScan[i];
		}
		for (int j = 0; j < res->width; j++)
		{
			if ((j *step_x) >= src->width)
				break;
			memcpy(dst_ip, src_ip, res->channels);
			dst_ip += res->channels;
			src_ip += src->channels * step_x;
			if (src->exaBuff)
			{
				dst_ap[0] = src_ap[0];
				dst_ap += res->exaChannels;
				src_ap += src->exaChannels * step_x;
			}
		}
	}
	return step;
}
//---------------------------------------------------------------------------
//
// image to buffer
//
//---------------------------------------------------------------------------
bool TPspiCore::image2Buffer(SpspiImage *image, void *data, Rect plugRect, int rowBytes, int loPlane, int hiPlane, SpspiImage *res, float sampleRate)
{  
	if ((plugRect.right <= 0) || (plugRect.bottom <= 0))
		return false;
	Rect rect = plugRect;
	// validate rectangle
	if (rect.left < 0)
		rect.left = 0;
	if (rect.top < 0)
		rect.top = 0;
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	int srw = image->width;
	int srh = image->height;
	int step = 1;
	LPBYTE *imageScan = image->imageScan;
	LPBYTE *exaScan = image->exaScan;
	if (res)
	{
		step = resizeImage(image,  res, sampleRate, w, h);
		if (step > 1)
		{
			imageScan = res->imageScan;
			exaScan = res->exaScan;
			srw = res->width;
			srh = res->height;
		}
	}
	int top, left, bottom, right;
	top  = rect.top;
	left = rect.left;
	bottom = min(rect.bottom, srh);
	right =  min(rect.right, srw);
	int nplanes = hiPlane - loPlane + 1;
	int hip = (exaScan != NULL) ? hiPlane - image->exaChannels : hiPlane;
	LPBYTE src_ptr, plug_ptr, exa_ptr = 0;
	plug_ptr = (LPBYTE)data;
	// DEBUG OUTPUT
	/*
	cout << "top = " << top <<  "\n";
	cout << "left = " << left <<  "\n";
	cout << "bottom = " << bottom <<  "\n";
	cout << "right = " << right <<  "\n";
	*/
	// 
	for (int i = top; i < bottom; i++)
	{
		src_ptr = imageScan[i];
		src_ptr = src_ptr + (image->channels * left);
		if (exaScan)	// external alpha channel
		{
			exa_ptr = exaScan[i];
			exa_ptr = exa_ptr + (image->exaChannels * left);
		}
		for (int j = left; j < right; j++)
		{
			int plc = 0;
			for (int k = loPlane; k <= hip; k++)
				{
				plug_ptr[chanOrder[plc]] = src_ptr[plc];
				plc++;
				}
			for (int k = 0; k < (hiPlane - hip); k++)
				{
				plug_ptr[chanOrder[plc]] = exa_ptr[k];
				plc++;
				}
			// move forward
			plug_ptr += nplanes;
			src_ptr  += image->channels;
			exa_ptr  += image->exaChannels;
		}
	}
	return true;
}
//---------------------------------------------------------------------------
//
// buffer to image
//
//---------------------------------------------------------------------------
bool TPspiCore::buffer2Image(SpspiImage *image, void *data, Rect plugRect,  int rowBytes, int loPlane, int hiPlane)
{  
	if ((plugRect.right <= 0) || (plugRect.bottom <= 0))
		return false;
	if (plugRect.left >= image->width || plugRect.top >= image->height)
		return false;
	// validate rectangle
	Rect rect = plugRect;
	if (rect.left < 0)
		rect.left = 0;
	if (rect.top < 0)
		rect.top = 0;
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	int srw = image->width;
	int srh = image->height;
	int top, left, bottom, right;
	top  = rect.top;
	left = rect.left;
	bottom = min(rect.bottom, srh);
	right =  min(rect.right, srw);
	int nplanes = hiPlane - loPlane + 1;
	LPBYTE plug_ptr, src_ptr, exa_ptr = 0;
	LPBYTE *imageScan = image->imageScan;
	LPBYTE *exaScan = image->exaScan;
	int hip = (exaScan != NULL) ? hiPlane - image->exaChannels : hiPlane;
	// copy buff to image
	plug_ptr = (LPBYTE)data;
	for (int i = top; i < bottom; i++)
	{
		src_ptr = imageScan[i];
		src_ptr = src_ptr + image->channels * left;
		if (exaScan)	// external alpha channel
		{
			exa_ptr = exaScan[i];
			exa_ptr = exa_ptr + image->exaChannels * left;
		}
		for (int j = left; j < right; j++)
		{
			int plc = 0;
			for (int k = loPlane; k <= hip; k++)
				{
				src_ptr[plc] = plug_ptr[chanOrder[plc]];
				plc++;
				}
			for (int k = 0; k < (hiPlane - hip); k++)
				{
				exa_ptr[k] = plug_ptr[chanOrder[plc]];
				plc++;
				}
			// move forward
			plug_ptr  += nplanes;
			src_ptr   += image->channels;
			exa_ptr += image->exaChannels;
		}
	}
	return true;
}
//---------------------------------------------------------------------------
// dst to src - finalize filter output 
//---------------------------------------------------------------------------
void TPspiCore::dst2Src(void)
{
	int top = fRecord.filterRect.top;
	int bottom = fRecord.filterRect.bottom;
	int left = fRecord.filterRect.left;
	int right = fRecord.filterRect.right;
	int iStride = (right - left) * hRecordPtr->srcImage->channels;
	int exaStride = (right - left) * hRecordPtr->srcImage->exaChannels;
	bool exaScan = hRecordPtr->srcImage->exaScan;
	LPBYTE src_ptr, dst_ptr, mask_ptr, src_alpha = 0, dst_alpha = 0;
	if (SrcMask.width > 0)			// here is a mask - blend the result
	{
		for (int i = top; i < bottom; i++)
		{
			src_ptr  = hRecordPtr->srcImage->imageScan[i] + left * hRecordPtr->srcImage->channels;
			dst_ptr  = hRecordPtr->dstImage->imageScan[i] + left * hRecordPtr->dstImage->channels;
			mask_ptr = SrcMask.imageScan[i] + left;
			if (exaScan)
			{
				src_alpha = hRecordPtr->srcImage->exaScan[i] + left;
				dst_alpha = hRecordPtr->dstImage->exaScan[i] + left;
			}
			for (int j = left; j < right; j++)
			{
				if (*mask_ptr > 0x00)	// there is something in the mask
				// blend using mask
				for (int k = 0; k < hRecordPtr->srcImage->channels; k++)
				{
					// no floating point arithmetics, only integers
					uint32 mv = (uint32)*mask_ptr + 1;
					uint32 bp = (uint32)src_ptr[k] * (256 - mv) + (uint32)dst_ptr[k] * mv;
					src_ptr[k] = (unsigned char)(bp>>8);
				}
				src_ptr += hRecordPtr->srcImage->channels;
				dst_ptr += hRecordPtr->dstImage->channels;
				mask_ptr += SrcMask.channels;
			}
			// what about external alpha, shell we blend it? Not for now...just copy
			if (exaScan)
				memcpy(src_alpha, dst_alpha, exaStride);
		}
	}
	else
	{
		for (int i = top; i < bottom; i++)
		{
			src_ptr  = hRecordPtr->srcImage->imageScan[i] + left * hRecordPtr->srcImage->channels;
			dst_ptr  = hRecordPtr->dstImage->imageScan[i] + left * hRecordPtr->dstImage->channels;
			if (exaScan)
			{
				src_alpha = hRecordPtr->srcImage->exaScan[i] + left;
				dst_alpha = hRecordPtr->dstImage->exaScan[i] + left;
			}
			memcpy(src_ptr, dst_ptr, iStride);	
			if (exaScan)
				memcpy(src_alpha, dst_alpha, exaStride);
		}
	}
}
//---------------------------------------------------------------------------
// Public section
//---------------------------------------------------------------------------
void TPspiCore::ProcessAdvanceState(void)
{
	if (fRecord.outData)
		buffer2Image(hRecordPtr->dstImage, fRecord.outData, aState.lastOutRect, aState.lastOutRowBytes, aState.lastOutLoPlane, aState.lastOutHiPlane);
	// mask buffer
	int sCord0 = chanOrder[0];
	chanOrder[0] = 0;	// we're using the same method for mask, so if chanOrder[0] = 2 (BGR) we will write outside data block 
	if (fRecord.haveMask)
	{
		if (!EqRects(&(aState.lastMaskRect), &(fRecord.maskRect)))
		{
			fRecord.maskData = resizeBuffer(fRecord.maskData, fRecord.maskRowBytes, fRecord.maskRect, 0, 0, aState.maskSize);
			aState.maskBuffOK = (aState.maskSize > 0);
			if (aState.maskBuffOK)
				aState.maskBuffOK = image2Buffer(hRecordPtr->mask, fRecord.maskData, fRecord.maskRect, fRecord.maskRowBytes, 0, 0, &ResMask, Fixed2Float(fRecord.maskRate));
		}
	}
	chanOrder[0] = sCord0;	// restore channel order 0
	// input buffer
	if (!EqRects(&(aState.lastInRect), &(fRecord.inRect)))
	{
		fRecord.inData = resizeBuffer(fRecord.inData, fRecord.inRowBytes, fRecord.inRect, fRecord.inLoPlane, fRecord.inHiPlane, aState.inSize);
		// copy src to input buffer
		aState.inBuffOK = (aState.inSize > 0);
		if (aState.inBuffOK)
			aState.inBuffOK = image2Buffer(hRecordPtr->srcImage, fRecord.inData, fRecord.inRect, fRecord.inRowBytes, fRecord.inLoPlane, fRecord.inHiPlane, &ResImage, Fixed2Float(fRecord.inputRate));
	}	
	// output buffer
	if (!EqRects(&(aState.lastOutRect), &(fRecord.outRect)))
	{
		// resize output buffer
		fRecord.outData = resizeBuffer(fRecord.outData, fRecord.outRowBytes, fRecord.outRect, fRecord.outLoPlane, fRecord.outHiPlane, aState.outSize);
		// copy tgt to output buffer
		aState.outBuffOK = (aState.outSize > 0);
		if (aState.outBuffOK)
			aState.outBuffOK = image2Buffer(hRecordPtr->dstImage, fRecord.outData, fRecord.outRect, fRecord.outRowBytes, fRecord.outLoPlane, fRecord.outHiPlane);
	}
	if (fRecord.haveMask)
	{
		if (!aState.maskBuffOK)
		{
			if (fRecord.maskData)
				GlobalFree(fRecord.maskData);
			fRecord.maskData = 0;
			aState.maskSize = 0;
			PurgeRect(&aState.lastMaskRect);
		}
		else
		{
			aState.lastMaskRect = fRecord.maskRect;
		}
	}
	if (!aState.inBuffOK)
	{
		if (fRecord.inData)
			GlobalFree(fRecord.inData);
		fRecord.inData = 0;
		fRecord.inRowBytes = 0;
		aState.inSize = 0;
		PurgeRect(&aState.lastInRect);
	}
	else
	{
		aState.lastInRect = fRecord.inRect;
	}
	if (!aState.outBuffOK)
	{
		if (fRecord.outData)
			GlobalFree(fRecord.outData);
		fRecord.outData = 0;
		fRecord.outRowBytes = 0;
		aState.outSize = 0;
		PurgeRect(&aState.lastOutRect);
		aState.lastOutRowBytes = 0;
		aState.lastOutLoPlane = 0;
		aState.lastOutHiPlane = 0;
	}
	else
	{
		aState.lastOutRect = fRecord.outRect;
		aState.lastOutRowBytes = fRecord.outRowBytes;
		aState.lastOutLoPlane = fRecord.outLoPlane;
		aState.lastOutHiPlane = fRecord.outHiPlane;
	}
}
//-----------------------------------------------------------------
// set path
//-----------------------------------------------------------------
int TPspiCore::SetPath(const wchar_t *filterFolder)
{
	if (HostRecord.initialEnvPath.empty())
		return PSPI_ERR_INIT_PATH_EMPTY;
	HostRecord.workingEnvPath = HostRecord.initialEnvPath;
	HostRecord.piPath = wstring(filterFolder);
	HostRecord.workingEnvPath += L";" + HostRecord.piPath;
	SetEnvironmentVariableW((LPCWSTR)L"PATH", (LPWSTR)&HostRecord.workingEnvPath[0]);
 return 0;
}
//-----------------------------------------------------------------
void TPspiCore::ReleaseAllImages(void)
{
	// releae images
	releaseImage(&SrcImage, false);	// shared
	releaseImage(&DstImage, true);	// not-shared
	releaseImage(&ResImage, true);  // not shared
	// release masks
	releaseImage(&SrcMask, false);	// shaed
	releaseImage(&ResMask, true);	// not-shared
}
//-----------------------------------------------------------------
// set working image by buffer
//-----------------------------------------------------------------
int TPspiCore::SetImage(TImgType type, int width, int height, void *imageBuff, int imageStride, void *alphaBuff, int alphaStride)
{
	ReleaseAllImages();
	for (int i = 0; i < maxChans; i++)		// max channels == 5 (RGBA + external alpha channel)...for simplicity only.
		chanOrder[i] = i;
	// set external alpha channels to zero
	SrcImage.exaChannels = DstImage.exaChannels = 0;
	SrcImage.exaStride = DstImage.exaStride = 0;
	SrcImage.exaScan = DstImage.exaScan = 0;
	switch (type)
	{
		case PSPI_IMG_TYPE_BGR:
			SrcImage.channels = DstImage.channels = 3;
			chanOrder[0] = 2;
			chanOrder[2] = 0;
			HostRecord.colorChannels = 3;
			HostRecord.alphaChannels = 0;
			break;
		case PSPI_IMG_TYPE_BGRA:
			SrcImage.channels = DstImage.channels = 4;
			chanOrder[0] = 2;
			chanOrder[2] = 0;
			HostRecord.colorChannels = 3;
			HostRecord.alphaChannels = 1;
			break;
		case PSPI_IMG_TYPE_RGB:
			SrcImage.channels = DstImage.channels = 3;
			HostRecord.colorChannels = 3;
			HostRecord.alphaChannels = 0;
			break;
		case PSPI_IMG_TYPE_RGBA:
			SrcImage.channels = DstImage.channels = 4;
			HostRecord.colorChannels = 3;
			HostRecord.alphaChannels = 1;
			break;
		case PSPI_IMG_TYPE_GRAYA:
			SrcImage.channels = DstImage.channels = 2;
			HostRecord.colorChannels = 1;
			HostRecord.alphaChannels = 1;
			break;
		case PSPI_IMG_TYPE_GRAY:
			SrcImage.channels = DstImage.channels = 1;
			HostRecord.colorChannels = 1;
			HostRecord.alphaChannels = 0;
			break;
		default:
			return PSPI_ERR_BAD_IMAGE_TYPE;
	}
	SrcImage.width = DstImage.width = width;
	SrcImage.height = DstImage.height = height;
	SrcImage.imageStride = DstImage.imageStride = imageStride;
	SrcImage.exaStride = DstImage.exaStride = alphaStride;
	// save buffer poiners
	SrcImage.imageBuff = imageBuff;
	SrcImage.exaBuff = alphaBuff;
	// fill source image scanlines
	SrcImage.imageScan = new LPBYTE[height];
	// fill image scanlines
	buffer2Scanlines(imageBuff, SrcImage.imageScan, height, imageStride);
    // image has separated alpha channel
	if (alphaBuff)
	{
		SrcImage.exaChannels++;			// increase external alpha channels
		SrcImage.exaScan = new LPBYTE[height];
		// fill alpha scanlines
		buffer2Scanlines(alphaBuff, SrcImage.exaScan, height, alphaStride);
	}
	// create and set destination image - copy of source
	DstImage.imageBuff = GlobalAlloc(GMEM_FIXED, height * imageStride);
	DstImage.imageScan = new LPBYTE[height];
	// fill destination image scanlines
	buffer2Scanlines(DstImage.imageBuff, DstImage.imageScan, height, imageStride, SrcImage.imageScan);
    // image has separated alpha channel - copy to dest
	if (alphaBuff)
	{
		DstImage.exaChannels++;	// increase external alpha channels
		DstImage.exaBuff = GlobalAlloc(GMEM_FIXED, height * alphaStride);
		DstImage.exaScan = new LPBYTE[height];	
		buffer2Scanlines(DstImage.exaBuff, DstImage.exaScan, height, alphaStride, SrcImage.exaScan);	
	}
	// set suites static vars
	SuitesDisplChans = SrcImage.channels + SrcImage.exaChannels;		// + external alpha if any
	// add external alpha channels to host record
	HostRecord.alphaChannels += SrcImage.exaChannels;
	HostRecord.imgType = type;
	return 0;
}
//-----------------------------------------------------------------
// set working mask by buffer
//-----------------------------------------------------------------
int TPspiCore::SetMask(int width, int height, void *maskBuff, int maskStride, bool useByPi)
{
	releaseImage(&SrcMask, false);
	releaseImage(&ResMask, true);
	if (width == 0 || height == 0 || maskBuff == 0 || maskStride == 0)	// we just want to clear mask, that's ok 
		return 0;
	SrcMask.width = width;
	SrcMask.height = height;
	SrcMask.imageStride = maskStride;
	SrcMask.channels = 1;
	// save buffer poiner
	SrcMask.imageBuff = maskBuff;
	// fill mask scanlines
	SrcMask.imageScan = new LPBYTE[height];
	buffer2Scanlines(SrcMask.imageBuff, SrcMask.imageScan, height, maskStride);	
	HostRecord.useMaskByPi = useByPi;
	return 0;
}
//-----------------------------------------------------------------
// start image creation  (for adding scanlines)
//-----------------------------------------------------------------
int TPspiCore::StartImageSL(TImgType type, int width, int height, bool externalAlpha)
{
	ReleaseAllImages();
	for (int i = 0; i < maxChans; i++)		// max channels == 5 (RGBA + external alpha channel)...for simplicity only.
		chanOrder[i] = i;
	// set external alpha channels to zero
	SrcImage.exaChannels = DstImage.exaChannels = 0;
	SrcImage.exaScan = DstImage.exaScan = 0;
	SrcImage.exaStride = DstImage.exaStride = 0;
	switch (type)
	{
		case PSPI_IMG_TYPE_BGR:
			SrcImage.channels = DstImage.channels = 3;
			chanOrder[0] = 2;
			chanOrder[2] = 0;
			HostRecord.colorChannels = 3;
			HostRecord.alphaChannels = 0;
			break;
		case PSPI_IMG_TYPE_BGRA:
			SrcImage.channels = DstImage.channels = 4;
			chanOrder[0] = 2;
			chanOrder[2] = 0;
			HostRecord.colorChannels = 3;
			HostRecord.alphaChannels = 1;
			break;
		case PSPI_IMG_TYPE_RGB:
			SrcImage.channels = DstImage.channels = 3;
			HostRecord.colorChannels = 3;
			HostRecord.alphaChannels = 0;
			break;
		case PSPI_IMG_TYPE_RGBA:
			SrcImage.channels = DstImage.channels = 4;
			HostRecord.colorChannels = 3;
			HostRecord.alphaChannels = 1;
			break;
		case PSPI_IMG_TYPE_GRAYA:
			SrcImage.channels = DstImage.channels = 2;
			HostRecord.colorChannels = 1;
			HostRecord.alphaChannels = 1;
			break;
		case PSPI_IMG_TYPE_GRAY:
			SrcImage.channels = DstImage.channels = 1;
			HostRecord.colorChannels = 1;
			HostRecord.alphaChannels = 0;
			break;
		default:
			return PSPI_ERR_BAD_IMAGE_TYPE;
	}
	SrcImage.width = DstImage.width = width;
	SrcImage.height = DstImage.height = height;
	// create array for image scanlines
	SrcImage.imageScan = new LPBYTE[height];
	DstImage.imageScan = new LPBYTE[height];
    // image has separated alpha channel
	if (externalAlpha)
	{
		SrcImage.exaChannels++;  // increase alpha channels
		SrcImage.exaScan = new LPBYTE[height];
		DstImage.exaChannels++;
		DstImage.exaScan = new LPBYTE[height];
	}
	else
	{
	}
	// set suites static vars
	SuitesDisplChans = SrcImage.channels + SrcImage.exaChannels;		// + external alpha if any
	// add external alpha channels to host record
	HostRecord.alphaChannels += SrcImage.exaChannels;
	HostRecord.imgType = type;
	return 0;
}
//-----------------------------------------------------------------
// Add image scanline to prepared container
//-----------------------------------------------------------------
int TPspiCore:: AddImageSL(int scanIndex, void *imageScanLine,  void *alphaScanLine)
{
	if (SrcImage.width == 0)
		return PSPI_ERR_IMAGE_INVALID;
	if (scanIndex >= SrcImage.height)
		return PSPI_ERR_BAD_PARAM;
	if (alphaScanLine && SrcImage.exaScan == NULL)
		return PSPI_ERR_BAD_PARAM;
	SrcImage.imageScan[scanIndex] = (LPBYTE)imageScanLine;
	if (alphaScanLine)
		SrcImage.exaScan[scanIndex] = (LPBYTE)alphaScanLine;
	return 0;
}
//-----------------------------------------------------------------
// Finish image creation
//-----------------------------------------------------------------
int TPspiCore::FinishImageSL(int imageStride, int alphaStride)
{
	if (SrcImage.width == 0)
		return PSPI_ERR_IMAGE_INVALID;
	if (DstImage.width == 0)
		return PSPI_ERR_IMAGE_INVALID;
	if (imageStride != 0)
	{
		SrcImage.imageStride = imageStride;
		DstImage.imageStride = imageStride;
	}
	else
		DstImage.imageStride = DstImage.width * DstImage.channels;	// only dest stride as we don't know src alignement
	DstImage.imageBuff = GlobalAlloc(GMEM_FIXED, DstImage.height * DstImage.imageStride);
	// fill destination image scanlines
	buffer2Scanlines(DstImage.imageBuff, DstImage.imageScan, DstImage.height, DstImage.imageStride, SrcImage.imageScan);
	if (alphaStride != 0)
	{
		SrcImage.exaStride = alphaStride;
		DstImage.exaStride = alphaStride;
	}
	else
	{
		DstImage.exaStride = DstImage.width;	// only dest stride as we don't know src alignement
	}
	if (DstImage.exaScan)
	{
		DstImage.exaBuff = GlobalAlloc(GMEM_FIXED, DstImage.height * DstImage.exaStride);
		buffer2Scanlines(DstImage.exaBuff, DstImage.exaScan, DstImage.height, DstImage.exaStride, SrcImage.exaScan);	
	}
	return 0;
}
//-----------------------------------------------------------------
// start mask creation  (for adding scanlines)
//-----------------------------------------------------------------
int TPspiCore::StartMaskSL(int width, int height, bool useByPi)
{
	if (DstImage.width == 0)
		return PSPI_ERR_IMAGE_INVALID;
	if (DstImage.width != width || DstImage.height != height)
		return PSPI_ERR_BAD_PARAM;
	releaseImage(&SrcMask, false);
	releaseImage(&ResMask, true);
	SrcMask.width = width;
	SrcMask.height = height;
	SrcMask.imageScan = new LPBYTE[height];
	HostRecord.useMaskByPi = useByPi;
	return 0;
}
//-----------------------------------------------------------------
// Add mask scanline to prepared container
//-----------------------------------------------------------------
int TPspiCore::AddMaskSL(int scanIndex, void *maskScanLine)
{
	if (SrcMask.width == 0)
		return PSPI_ERR_IMAGE_INVALID;
	if (scanIndex >= SrcMask.height)
		return PSPI_ERR_BAD_PARAM;
	SrcMask.imageScan[scanIndex] = (LPBYTE)maskScanLine;
	return 0;
}
//-----------------------------------------------------------------
// Finish Add mask scanline to prepared surface
//-----------------------------------------------------------------
int TPspiCore::FinishMaskSL(int maskStride)
{
	if (maskStride != 0)
		SrcMask.imageStride = maskStride;
	return 0;
}
//-----------------------------------------------------------------
// Load plugin
//-----------------------------------------------------------------
int TPspiCore::PlugInLoad(const wchar_t *filter)
{
	HostRecord.filterPathName = filter;
	HostRecord.filterLoaded = false;
	aState.Init();
	if (HostRecord.dllHandle)
	{
		try
		{
			FreeLibrary(HostRecord.dllHandle);
		}
		catch (...)
		{
			// never mind
		}
	}
	try
	{
		HostRecord.dllHandle = LoadLibraryW(filter);
	}
	catch (...)
	{
		return PSPI_ERR_FILTER_NOT_LOADED;
	}
	if (!loadPIPLResources(HostRecord.dllHandle))
	{
		if (!loadPIMIResources(HostRecord.dllHandle))
			return PSPI_ERR_FILTER_INVALID;
	}
	if (GFilterData.size() == 3)
	{
		HostRecord.piCategory = GFilterData[0];
		HostRecord.piName = GFilterData[1];
		HostRecord.piEntrypointName = GFilterData[2];
		HostRecord.filterLoaded = true;
	}
	else
	{
		HostRecord.piCategory = "";
		HostRecord.piName = "";
		HostRecord.piEntrypointName = "";
		return PSPI_ERR_FILTER_INVALID;
	}
	return 0;
}
//-----------------------------------------------------------------
// Show plug-in about
//-----------------------------------------------------------------
int TPspiCore::PlugInAbout(HWND hWnd)
{
	// check if plug-in is loaded
	if (!HostRecord.filterLoaded)
		return PSPI_ERR_FILTER_NOT_LOADED;
	int rc = 0;
	PLUGINPROC PluginMainProc;
	PluginMainProc = (PLUGINPROC)GetProcAddress(HostRecord.dllHandle, HostRecord.piEntrypointName.c_str());
	if (!PluginMainProc)
	{
		return PSPI_ERR_FILTER_BAD_PROC;
	}
	AboutRecord aRecord;
	PlatformData pData;
	int16 result = 1;
	SpspiHandle parHandle;
	parHandle.hPointer = NULL;
	parHandle.hSize = 0;
	intptr_t *dataPtr = (intptr_t*)(&parHandle);
	//SPBasicSuite bSuite;
	//memset(&bSuite, 0, sizeof(bSuite));
	pData.hwnd = (intptr_t) hWnd;
	// zero aboutrecord
	memset(&aRecord, 0, sizeof(aRecord));
	aRecord.platformData = &pData;
	bSuite.AcquireSuite = 0; // AcquireSuite;
	aRecord.sSPBasic = 0;    // bSuite;
	aRecord.plugInRef = 0;   // Application;
	try
	{
		PluginMainProc(plugInSelectorAbout, &aRecord, dataPtr, &result);
		if (result)
			rc = PSPI_ERR_FILTER_DUMMY_PROC;
	}
	catch(...)
	{
		rc = PSPI_ERR_FILTER_ABOUT_ERROR;
	}
return rc;
}
//-----------------------------------------------------------------
// Execute loaded plug-in
//-----------------------------------------------------------------
int TPspiCore::PlugInExecute(HWND hWnd)
{
	// check if image is set
	if (!SrcImage.width || !DstImage.width )
		return PSPI_ERR_IMAGE_INVALID;
	// check if plug-in is loaded
	if (!HostRecord.filterLoaded)
		return PSPI_ERR_FILTER_NOT_LOADED;
	aState.Init();		
	handleMap.clear();	// clear handle map
	HostRecord.srcImage = &SrcImage;
	HostRecord.dstImage = &DstImage;
	HostRecord.hWnd = hWnd;
	if (SrcMask.width)	// for now use src so that we can test it
		HostRecord.mask = &SrcMask;
	else
		HostRecord.mask = 0;
	if (!HostRecord.roiRect.IsEmpty())	// adjust roi if necessary
	{
		if (HostRecord.roiRect.bottom > HostRecord.srcImage->height)
			HostRecord.roiRect.bottom = HostRecord.srcImage->height;
		if (HostRecord.roiRect.right > HostRecord.srcImage->width)
			HostRecord.roiRect.right = HostRecord.srcImage->width;
	}
	// raspaljotka 
	int rc = 0;
	SpspiHandle parHandle;
	//int parHandle = 0;
	//intptr_t *parHandle = (intptr_t*)GlobalAlloc(GMEM_ZEROINIT, sizeof(GHND));	
	parHandle.hPointer = NULL;
	parHandle.hSize = 0;
	// 
	intptr_t *dataPtr = (intptr_t*)(&parHandle);
	int16 result = 0, finish_result = 0;
	prepareSuites();
	prepareFilter();
	PLUGINPROC PluginMainProc;
	PluginMainProc = (PLUGINPROC)GetProcAddress(HostRecord.dllHandle, HostRecord.piEntrypointName.c_str());
	if (!PluginMainProc)
	{
		return PSPI_ERR_FILTER_BAD_PROC;
	}
	GFilterStage = 0;
	GAppHandle = hWnd;
	// setUnstyler(); <-- maybe someone else
	try
	{
	   PluginMainProc(filterSelectorParameters, &fRecord, dataPtr, &result);
	   if (result ==  noErr)
		  {
		  GFilterStage = 1;
		  PluginMainProc(filterSelectorPrepare, &fRecord, dataPtr, &result);
		  if (result == noErr)
			 {
			 GFilterStage = 2;
			 PluginMainProc(filterSelectorStart, &fRecord, dataPtr, &result);
			 if (result == noErr)
				{
				GFilterStage = 3;
				while ((fRecord.outRect.right != 0) || (fRecord.inRect.right != 0)
					   || ((fRecord.haveMask) && (fRecord.maskRect.right != 0)))
					  {
					  DoAdvanceState();
					  PluginMainProc(filterSelectorContinue, &fRecord, dataPtr, &result);
					  if (result != noErr)
						 break;
					  }
				if (result == noErr)
					{
					// always if SelectorStart returned no err.
					GFilterStage = 4;
					PluginMainProc(filterSelectorFinish, &fRecord, dataPtr, &finish_result);
					DoAdvanceState();
					}
				}
			 }
		  }
	   if (result != noErr)
		  rc = PSPI_ERR_FILTER_CANCELED;
	   }
	 catch (...)
	   {
	   rc = PSPI_ERR_FILTER_CRASHED;
	   }
if (result == noErr)
	{
	dst2Src();
	}
//TODO: release suite handles - scroll through handleMap and release allocated handles
handleMap.clear();	// clear handle map
 return rc;
}
//---------------------------------------------------------------------------
// Enumerate Filters from directory using call back function
//---------------------------------------------------------------------------
int TPspiCore::PlugInEnumerate(ENUMCALLBACK enumFunc, bool recurseSubFolders)
{
	if (HostRecord.piPath.empty())
		return  PSPI_ERR_WORK_PATH_EMPTY;
	enumResourcesPath(HostRecord.piPath, enumFunc, recurseSubFolders);
	return 0;
}
//---------------------------------------------------------------------------
// inline members
//---------------------------------------------------------------------------
inline float TPspiCore::Fixed2Float(Fixed f_number)
{
if (f_number == 0)
   return 0;
else
   {
   int denom, number;
   denom = f_number<<16;
   number = f_number>>16;
   return ((float)number + (float)denom/65536);
   }
}
//---------------------------------------------------------------------------
inline bool TPspiCore::EqRects(Rect *r1, Rect *r2)
{
	return ((r1->left == r2->left) && (r1->right == r2->right) && (r1->top == r2->top) && (r1->bottom == r2->bottom));
}
//---------------------------------------------------------------------------
inline void TPspiCore::PurgeRect(Rect *r)
{
	r->top = r->left = r->bottom = r->right = 0;
}
//---------------------------------------------------------------------------
// Routine for fixed format conversion -static 
//---------------------------------------------------------------------------
inline Fixed TPspiCore::FixRatio(short numer, short denom)
{
 if (denom == 0)
    {
    if (numer >=0)
       return   2147483647;
    else
       return  -(int)2147483648;
    }
 else
    {
    return ((long)numer << 16) / denom;
    }
}
//---------------------------------------------------------------------------
// Routine to get plug-in infos -static (suggested by Irfan Škiljan)
//---------------------------------------------------------------------------
int TPspiCore::PlugInGetInfos(char *piCategory,char *piName,char *piEntryName,int bufSize)
{
    // check if plug-in is loaded
    if (!HostRecord.filterLoaded)
       return PSPI_ERR_FILTER_NOT_LOADED;
    if(piCategory != NULL)
      strncpy_s(piCategory, bufSize, HostRecord.piCategory.c_str(), bufSize);
    if(piName != NULL)
      strncpy_s(piName,bufSize, HostRecord.piName.c_str(), bufSize);
    if(piEntryName != NULL)
      strncpy_s(piEntryName,bufSize, HostRecord.piEntrypointName.c_str(), bufSize);
    return 0;
}








