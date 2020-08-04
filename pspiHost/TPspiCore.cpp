#include "stdafx.h"
#include <string>
#include <vector>
#include <experimental/filesystem>
#include "TPspiCore.h"
#include "pspiSuites.hpp"
namespace fsys = experimental::filesystem;
static vector<string> GFilterData;
static bool GFilterSupCases[7];
//void *GThisPtr;
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
//-----------------------------------------------------------------
// trivial constructor
//-----------------------------------------------------------------
TPspiCore::TPspiCore()
{
	 int buff_size = 65535;
	 HostRecord.initialEnvPath.resize(buff_size);
	 GetEnvironmentVariableW((LPCWSTR)L"PATH", (LPWSTR)&(HostRecord.initialEnvPath[0]), buff_size);
	 ColorPickerCallback = 0;
	 ProgressCallBack = 0;
	 hRecordPtr = &HostRecord;
	 SuitesFPR = &fRecord;
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
						  GFilterSupCases[i] = (fici_ptr[i].inputHandling > 0x00);
					  }
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
//---------------------------------------------------------------------------
// static members
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
//-----------------------------------------------------------------
// private methods section
//-----------------------------------------------------------------
void TPspiCore::releaseImage(SpspiImage *img, bool dispose)
{
	if (img->width)
	{
		if (img->imageScan)
			delete []img->imageScan;
		if (img->alphaScan)
			delete []img->alphaScan;
		img->imageScan = 0;
		img->alphaScan = 0;
		if (img->imageBuff && dispose)
			free(img->imageBuff);
		if (img->alphaBuff && dispose)
			free(img->alphaBuff);
		img->imageBuff = 0;
		img->alphaBuff = 0;
		img->width = 0;
		img->height = 0;
		img->imageStride = 0;
		img->alphaStride = 0;
		img->imageScan = 0;
		img->alphaScan = 0;
	}
}
//-----------------------------------------------------------------
void TPspiCore::releaseMask(SpspiMask *mask)
{
	if (mask->width) 
		{
		if (mask->maskScan)
			delete []mask->maskScan;
		mask->maskBuff = 0;
		mask->maskScan = 0;
		mask->width = 0;
		mask->height = 0;
		mask->maskStride = 0;
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
	 // big doc
	 // bigDoc;
}
//---------------------------------------------------------------------------
void  TPspiCore::prepareFilter(void)
{
 int mask_size;
 unsigned int serNumber = 12345;
 int FData = 0;
 BYTE f_color[3], b_color[3];

 f_color[chanOrder[0]]  = BYTE(((hRecordPtr->foregroundColor)<<8)>>24);
 f_color[chanOrder[1]]  = BYTE(((hRecordPtr->foregroundColor)<<16)>>24);
 f_color[chanOrder[2]]  = BYTE(((hRecordPtr->foregroundColor)<<24)>>24);


 b_color[chanOrder[0]]  = BYTE(((hRecordPtr->backgroundColor)<<8)>>24);
 b_color[chanOrder[1]]  = BYTE(((hRecordPtr->backgroundColor)<<16)>>24);
 b_color[chanOrder[2]]  = BYTE(((hRecordPtr->backgroundColor)<<24)>>24);
 //
 //******** FILTER RECORD *******
 memset(&fRecord, 0, sizeof(fRecord));
 //
 // fill up filter record    -  idemo po redu
 //
 fRecord.serialNumber = 0;               // Host's serial number, to allow copy protected plug-in modules.
 fRecord.abortProc = DoTestAbort;        // The plug-in module may call this no-argument...
 if (hRecordPtr->progressProc)
     fRecord.progressProc = (ProgressProc)(hRecordPtr->progressProc);  // The plug-in module may call this two-argument...
 else
     fRecord.progressProc = (ProgressProc)DoProgressProc;  // The plug-in module may call this two-argument...
 fRecord.parameters = NULL;              // A handle, initialized to NIL by Photoshop.
 // imageSize -> depreciated -> check BigDocumentStruct::imageSize32
 // size of complete image if selection is not floating
 fRecord.imageSize.v = hRecordPtr->srcImage->height;  // Size of image v
 fRecord.imageSize.h = hRecordPtr->srcImage->width;   // Size of image h
 fRecord.planes =  hRecordPtr->srcImage->channels + hRecordPtr->srcImage->alphaChans;    // planes   
 //--- filter rect
 if (hRecordPtr->hasBoundingRectangle)
    {
    fRecord.filterRect.top    = hRecordPtr->roiRect.top;      // Rectangle to filter top
    fRecord.filterRect.left   = hRecordPtr->roiRect.left;     // Rectangle to filter left
    fRecord.filterRect.bottom = hRecordPtr->roiRect.bottom;   // Rectangle to filter bottom
    fRecord.filterRect.right  = hRecordPtr->roiRect.right;    // Rectangle to filter right
    }
 else
    {
    fRecord.filterRect.top    = 0;                  // Rectangle to filter top
    fRecord.filterRect.left   = 0;                  // Rectangle to filter left
    fRecord.filterRect.bottom = hRecordPtr->srcImage->height;    // Rectangle to filter bottom
    fRecord.filterRect.right = hRecordPtr->srcImage->width;      // Rectangle to filter right
    }
 // current background - depreciated
 fRecord.background.red   = b_color[0];
 fRecord.background.green = b_color[1];
 fRecord.background.blue  = b_color[2];
 // current foreground - depreciated
 fRecord.foreground.red   = f_color[0];
 fRecord.foreground.green  = f_color[1];
 fRecord.foreground.blue   = f_color[2];
 //TODO: calculate available space
 //fRecord.maxSpace64 =  100000000;
 fRecord.maxSpace =   100000000;         // maximum total space???
 //fRecord.bufferSpace = fRecord.maxSpace;
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
  if (hRecordPtr->srcMask == 0)
    {
    fRecord.haveMask = false;
	fRecord.filterCase = filterCaseFlatImageNoSelection;
    fRecord.maskData = NULL;
	}
 else
    {
    fRecord.haveMask = true;
    fRecord.filterCase = filterCaseFlatImageWithSelection;
    mask_size =  hRecordPtr->srcMask->height * hRecordPtr->srcMask->width;
    fRecord.maskData = NULL;
    }
// test
 //fRecord.filterCase = filterCaseEditableTransparencyNoSelection;
 fRecord.backColor[0]  = b_color[0];
 fRecord.backColor[1]  = b_color[1];
 fRecord.backColor[2]  = b_color[2];
 fRecord.backColor[3]  = 0xff;
 fRecord.foreColor[0]  = f_color[0];
 fRecord.foreColor[1]  = f_color[1];
 fRecord.foreColor[2]  = f_color[2];
 fRecord.foreColor[3]  = 0xff;
 //fRecord.hostSig = 0x02020202;                   // host signature
 memcpy ((char *) &fRecord.hostSig, &cSig[0], 4);
 fRecord.hostProc =  DoHostProc;                  // host proc - ne treba ???
 fRecord.imageMode = plugInModeRGBColor;         // color mode
 fRecord.imageHRes = FixRatio(96, 1);            // pixels per inch
 fRecord.imageVRes = fRecord.imageHRes;
 //fRecord.floatCoord; 	   -  Top left coordinate of selection
 fRecord.wholeSize.v = hRecordPtr->srcImage->height;  //  Size of image selection is floating over
 fRecord.wholeSize.h = hRecordPtr->srcImage->width;   //  Size of image selection is floating over
 //fRecord.monitor;	   -  Information on current monitor
 fRecord.platformData = &pData;		// Platform specific information.
 fRecord.bufferProcs =  &bProcs;		// The host buffer procedures.
 fRecord.resourceProcs = &rProcs;       // The host plug-in resource procedures.
 fRecord.handleProcs =   &hProcs;  	    // Platform independent handle manipulation.
 fRecord.processEvent  = DoProcessEvent;     // Pass event to the application.
 fRecord.displayPixels = DoDisplayPixels;  // Display dithered pixels.
 //fRecord.supportsDummyChannels;     -  Does the host support dummy channels?
 //fRecord.supportsAlternateLayouts;    -  Does the host support alternate data layouts.
 fRecord.wantLayout = piLayoutTraditional;  // The layout to use for the data...
 //
 //
 //fRecord.filterCase = 0;			// Filter case.... definition moved to mask if block
 fRecord.dummyPlaneValue = -1;	    //  0..255 = fill value -1 = leave undefined...
 //fRecord.premiereHook;		       - A hook for Premiere...
 fRecord.advanceState = DoAdvanceState;	// Advance from start to continue...
 fRecord.supportsAbsolute = 1;	// Does the host support absolute plane indexing?
 fRecord.wantsAbsolute = false;	       // Does the plug-in want absolute plane indexing? (input only)
 fRecord.getPropertyObsolete = DoSuiteGetProperty;	 // Use the suite if available
 fRecord.cannotUndo = 0;		      // If set to TRUE, then undo will not be enabled for this command.
 fRecord.supportsPadding = false;	       // Does the host support requests outside the image area?
 fRecord.inputPadding  = plugInWantsErrorOnBoundsException;        // Instructions for padding the input.
 fRecord.outputPadding = plugInWantsErrorOnBoundsException;        // Instructions for padding the output.
 fRecord.maskPadding   = plugInWantsErrorOnBoundsException;		   // Padding instructions for the mask.
 fRecord.samplingSupport = true;     // Does the host support sampling the input and mask?
 //fRecord.reservedByte;		       - Alignment.
 fRecord.inputRate = FixRatio(1, 1);            // Input sample rate.
 fRecord.maskRate = FixRatio(1, 1);           // Mask sample rate.
 fRecord.colorServices = DoColorServices;          // Routine to access color services.
 fRecord.inLayerPlanes =  fRecord.planes;
 // for alpha channel
 fRecord.inTransparencyMask = hRecordPtr->srcImage->alphaChans;
 //fRecord.inLayerMasks = 1;
 //fRecord.inInvertedLayerMasks;
 //fRecord.inNonLayerPlanes;
 fRecord.outLayerPlanes = fRecord.inLayerPlanes;
 fRecord.outTransparencyMask = fRecord.inTransparencyMask;
 //fRecord.outLayerMasks = 1;
 //fRecord.outInvertedLayerMasks;
 //fRecord.outNonLayerPlanes;
 fRecord.absLayerPlanes = fRecord.inLayerPlanes;
 fRecord.absTransparencyMask = fRecord.inTransparencyMask;
 //fRecord.absLayerMasks = 1;
 //fRecord.absInvertedLayerMasks;
 //fRecord.absNonLayerPlanes;
 //fRecord.inPreDummyPlanes;	- Extra planes to allocate in the input.
 //fRecord.inPostDummyPlanes;
 //fRecord.outPreDummyPlanes;	- Extra planes to allocate in the output.
 //fRecord.outPostDummyPlanes;
 fRecord.inColumnBytes = fRecord.planes;         // Step between input columns.
 fRecord.inPlaneBytes = 1;		    // Step between input planes.
 fRecord.outColumnBytes = fRecord.planes;        // Step between output pcolumns.
 fRecord.outPlaneBytes =  1;		    // Step between input planes.
 //
 // **** New in 3.0.4. ****
 //
 fRecord.imageServicesProcs = NULL; //&iProcs; // Suite of image processing callbacks.
 fRecord.propertyProcs = &pProcs;        //Routines to query and set document and view properties...
 // tilling
 fRecord.inTileHeight = fRecord.imageSize.v;	
 fRecord.inTileWidth  = fRecord.imageSize.h;
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
 //
 // **** New in 4.0 ****
 //
 fRecord.descriptorParameters = &pDescriptorParameters;	// For recording and playback
 fRecord.errorString = &errString;      // For silent and errReportString
 fRecord.channelPortProcs = &cProcs;     // Suite for passing pixels through channel ports.
  fRecord.documentInfo = NULL;	         // The document info for the document being filtered.
 //
 // **** New in 5.0 ****
 //
 //fRecord.sSPBasic = &bSuite;     	// SuitePea basic suite
 fRecord.plugInRef = NULL;       	// plugin reference used by SuitePea
 fRecord.depth = 8;                 // bit depth per channel (1,8,16)
 //
 // **** New in 6.0 ****
 //
 fRecord.iCCprofileData = NULL;		// Handle containing the ICC profile for the image. (NULL if none)
 fRecord.iCCprofileSize = 0;		// size of profile.
 //fRecord.canUseICCProfiles;	        // non-zero if the host can export ICC profiles...
 // test for big doc
 // fRecord.bigDocumentData = &bigDoc;
 }
//---------------------------------------------------------------------------
// resize buffer 
//---------------------------------------------------------------------------
void * TPspiCore::resizeBuffer(void *data, int &rowBytes, Rect rect, int loPlane, int hiPlane, int &prevSize)
{
	void *res_ptr = 0;
	if ((rect.right <= 0) || (rect.bottom <= 0))
	{
		if (data)
		{
		free(data);
		data = 0;
		rowBytes = 0;
		}
		return res_ptr;
	}
	int nplanes = hiPlane - loPlane + 1;
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	int stride = nplanes * w;
	int size = nplanes * w * h;
	if (size == prevSize)	// same size, do not resize
		return data;
	if (data)
		free(data);
	res_ptr = malloc (size);
	rowBytes = stride;
	prevSize = size;
	return res_ptr;
}
//---------------------------------------------------------------------------
//
// step calculator for crap resize
//
//---------------------------------------------------------------------------
int TPspiCore::resizeImage(SpspiImage *src, float sampleRate, int rectW, int rectH)
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
	if (ResImage.imageBuff == 0 || scaleWidth != ResImage.width || scaleHeight != ResImage.height)
	{
		if (ResImage.imageBuff)
			releaseImage(&ResImage, true);
		// create resample image
		ResImage.width = scaleWidth;
		ResImage.height = scaleHeight;
		ResImage.channels  = src->channels;
		ResImage.alphaChans = src->alphaChans;
		ResImage.imageStride = src->channels * scaleWidth;
		ResImage.alphaStride = src->alphaChans *scaleWidth;
		ResImage.imageBuff = malloc(scaleHeight * ResImage.imageStride);
		ResImage.imageScan = new LPBYTE[scaleHeight];
		// fill res image scanlines
		LPBYTE ptr = (LPBYTE)ResImage.imageBuff;
		for (int i = 0; i < scaleHeight; i++)
		{
			ResImage.imageScan[i] = ptr;
			memset(ResImage.imageScan[i], 0, ResImage.imageStride);
			ptr = ptr + ResImage.imageStride;
		}
		// image has separated alpha channel - create res alpha
		if (src->alphaBuff)
		{
			ResImage.alphaBuff = malloc(scaleHeight * ResImage.alphaStride);
			ResImage.alphaScan = new LPBYTE[scaleHeight];	
			ptr = (LPBYTE)ResImage.alphaBuff;
			for (int i = 0; i < scaleHeight; i++)
			{
				ResImage.alphaScan[i] = ptr;
				memset(ResImage.alphaScan[i], 0, ResImage.alphaStride);
				ptr = ptr + ResImage.alphaStride;
			}
		}
		else
		{
			ResImage.alphaScan = 0;
			ResImage.alphaStride = 0;
		}
	}
	// calculate crap resample step
	int step_y = src->width / scaleWidth;
	int step_x = src->height / scaleHeight;
	// perform crap resampler	
	LPBYTE src_ip, src_ap = 0, dst_ip = 0, dst_ap = 0;
	for (int i = 0; i < ResImage.height; i++)
	{
		if ((i * step_y) >= src->height)
			break;
		src_ip = src->imageScan[i * step_y];
		dst_ip = ResImage.imageScan[i];
		if (src->alphaBuff)
		{
			src_ap = src->alphaScan[i * step_y];
			dst_ap = ResImage.alphaScan[i];
		}
		for (int j = 0; j < ResImage.width; j++)
		{
			if ((j *step_x) >= src->width)
				break;
			memcpy(dst_ip, src_ip, ResImage.channels);
			dst_ip += ResImage.channels;
			src_ip += src->channels * step_x;
			if (src->alphaBuff)
			{
				dst_ap[0] = src_ap[0];
				dst_ap += ResImage.alphaChans;
				src_ap += src->alphaChans * step_x;
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
bool TPspiCore::image2Buffer(SpspiImage *image, void *data, Rect plugRect, int rowBytes, int loPlane, int hiPlane)
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
	LPBYTE *alphaScan = image->alphaScan;
	float sample_rate = Fixed2Float(fRecord.inputRate);
	if (fRecord.inData == data)
	{
		step = resizeImage(image, Fixed2Float(fRecord.inputRate), w, h);
		if (step > 1)
		{
			imageScan = ResImage.imageScan;
			alphaScan = ResImage.alphaScan;
			srw = ResImage.width;
			srh = ResImage.height;
		}
	}
	int top, left, bottom, right;
	top  = rect.top;
	left = rect.left;
	bottom = min(rect.bottom, srh);
	right =  min(rect.right, srw);
	int nplanes = hiPlane - loPlane + 1;
	int hip = (alphaScan != NULL) ? hiPlane - image->alphaChans : hiPlane;
	LPBYTE src_ptr, plug_ptr, alpha_ptr = 0;
	plug_ptr = (LPBYTE)data;
	for (int i = top; i < bottom; i++)
	{
		src_ptr = imageScan[i];
		src_ptr = src_ptr + image->channels * left;
		if (alphaScan)	// external alpha channel
		{
			alpha_ptr = alphaScan[i];
			alpha_ptr = alpha_ptr + image->alphaChans * left;
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
				plug_ptr[chanOrder[plc]] = alpha_ptr[k];
				plc++;
				}
			// move forward
			plug_ptr  += nplanes;
			src_ptr   += image->channels;
			alpha_ptr += image->alphaChans;
		}
	//plug_ptr = (LPBYTE)data + rowBytes;
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
	LPBYTE plug_ptr, src_ptr, alpha_ptr = 0;
	LPBYTE *imageScan = image->imageScan;
	LPBYTE *alphaScan = image->alphaScan;
	int hip = (alphaScan != NULL) ? hiPlane - image->alphaChans : hiPlane;
	// copy buff to image
	plug_ptr = (LPBYTE)data;
	for (int i = top; i < bottom; i++)
	{
		src_ptr = imageScan[i];
		src_ptr = src_ptr + image->channels * left;
		if (alphaScan)	// external alpha channel
		{
			alpha_ptr = alphaScan[i];
			alpha_ptr = alpha_ptr + image->alphaChans * left;
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
				alpha_ptr[k] = plug_ptr[chanOrder[plc]];
				plc++;
				}
			// move forward
			plug_ptr  += nplanes;
			src_ptr   += image->channels;
			alpha_ptr += image->alphaChans;
		}
	}
	return true;
}
//---------------------------------------------------------------------------
// dst to src - finalize filter output 
//---------------------------------------------------------------------------
void TPspiCore::dst2Src(void)
{
	for (int i = 0; i < hRecordPtr->srcImage->height; i++)
	{
		memcpy(hRecordPtr->srcImage->imageScan[i], hRecordPtr->dstImage->imageScan[i], hRecordPtr->srcImage->imageStride);
		if (hRecordPtr->srcImage->alphaScan)
			memcpy(hRecordPtr->srcImage->alphaScan[i], hRecordPtr->dstImage->alphaScan[i], hRecordPtr->srcImage->alphaStride);
	}
}
//---------------------------------------------------------------------------
// Public section
//---------------------------------------------------------------------------
void TPspiCore::ProcessAdvanceState(void)
{
	buffer2Image(hRecordPtr->dstImage, fRecord.outData, aState.lastOutRect, aState.lastOutRowBytes, aState.lastOutLoPlane, aState.lastOutHiPlane);
	// resize input buffer
	fRecord.inData = resizeBuffer(fRecord.inData, fRecord.inRowBytes, fRecord.inRect, fRecord.inLoPlane, fRecord.inHiPlane, aState.inSize);
	// copy src to input buffer
	if (aState.inSize > 0)
		aState.inBuffOK = image2Buffer(hRecordPtr->srcImage, fRecord.inData, fRecord.inRect, fRecord.inRowBytes, fRecord.inLoPlane, fRecord.inHiPlane);
	// check lastOutRect
	if (!(aState.lastOutRect.left == fRecord.outRect.left
		&& aState.lastOutRect.right == fRecord.outRect.right
		&& aState.lastOutRect.top == fRecord.outRect.top
		&& aState.lastOutRect.bottom == fRecord.outRect.bottom))
	{
		// resize output buffer
		fRecord.outData = resizeBuffer(fRecord.outData, fRecord.outRowBytes, fRecord.outRect, fRecord.outLoPlane, fRecord.outHiPlane, aState.outSize);
		// copy tgt to output buffer
		if (aState.outSize)
			aState.outBuffOK = image2Buffer(hRecordPtr->dstImage, fRecord.outData, fRecord.outRect, fRecord.outRowBytes, fRecord.outLoPlane, fRecord.outHiPlane);
	}
	if (!aState.inBuffOK)
	{
		if (fRecord.inData)
			free(fRecord.inData);
		fRecord.inData = 0;
		aState.inSize = 0;
	}
	if (!aState.outBuffOK)
	{
		if (fRecord.outData)
			free(fRecord.outData);
		fRecord.outData = 0;
		aState.outSize = 0;
		aState.lastOutRect = Rect();
		aState.lastOutRowBytes = 0;
		aState.lastOutLoPlane = 0;
		aState.lastOutHiPlane = 0;
	}
	else
	{
		// store previous out values
		aState.lastOutRect = fRecord.outRect;
		aState.lastOutRowBytes = fRecord.outRowBytes;
		aState.lastOutLoPlane = fRecord.outLoPlane;
		aState.lastOutHiPlane = fRecord.outHiPlane;
	}
	/* TODO: tute imamo problem
	if (fRecord.haveMask)
		Mask2PlugIn(fRecord.maskRect); */
}
//-----------------------------------------------------------------
// set path
//-----------------------------------------------------------------
int TPspiCore::SetPath(wchar_t *filterFolder)
{
	if (HostRecord.initialEnvPath.empty())
		return PSPIW_ERR_INIT_PATH_EMPTY;
	HostRecord.workingEnvPath = HostRecord.initialEnvPath;
	HostRecord.piPath = wstring(filterFolder);
	HostRecord.workingEnvPath += L";" + HostRecord.piPath;
	SetEnvironmentVariableW((LPCWSTR)L"PATH", (LPWSTR)&HostRecord.workingEnvPath[0]);
 return 0;
}
//-----------------------------------------------------------------
void TPspiCore::ReleaseAllImages(void)
{
	releaseImage(&SrcImage, false);	// shared
	releaseImage(&DstImage, true);	// not-shared
	releaseImage(&ResImage, true);
	releaseMask(&SrcMask);
}
//-----------------------------------------------------------------
// set working image by buffer
//-----------------------------------------------------------------
int TPspiCore::SetImage(TImgType type, int width, int height, void *imageBuff, int imageStride, void *alphaBuff, int alphaStride)
{
	ReleaseAllImages();
	for (int i = 0; i < maxChans; i++)		// max channels == 5 (RGBA + external alpha channel)...for simplicity only.
		chanOrder[i] = i;
	switch (type)
	{
		case PSPIW_IMT_BGR:
			SrcImage.channels = DstImage.channels = 3;
			SrcImage.alphaChans = DstImage.alphaChans = 0;
			chanOrder[0] = 2;
			chanOrder[2] = 0;
			break;
		case PSPIW_IMT_BGRA:
			SrcImage.channels = DstImage.channels = 4;
			SrcImage.alphaChans = DstImage.alphaChans = 0;
			chanOrder[0] = 2;
			chanOrder[2] = 0;
			break;
		case PSPIW_IMT_RGB:
			SrcImage.channels = DstImage.channels = 3;
			SrcImage.alphaChans = DstImage.alphaChans = 0;
			break;
		case PSPIW_IMT_RGBA:
			SrcImage.channels = DstImage.channels = 4;
			SrcImage.alphaChans = DstImage.alphaChans = 1;
			break;
		case PSPIW_IMT_GRAYA:
			SrcImage.channels = DstImage.channels = 2;
			SrcImage.alphaChans = DstImage.alphaChans = 1;
			break;
		default:
			SrcImage.channels = DstImage.channels = 1;
			SrcImage.alphaChans = DstImage.alphaChans = 0;
			break;
	}
	SrcImage.width = DstImage.width = width;
	SrcImage.height = DstImage.height = height;
	SrcImage.imageStride = DstImage.imageStride = imageStride;
	SrcImage.alphaStride = DstImage.alphaStride = alphaStride;
	// fill source image scanlines
	SrcImage.imageScan = new LPBYTE[height];
	LPBYTE ptr = (LPBYTE)imageBuff;
	for (int i = 0; i < height; i++)
	{
		SrcImage.imageScan[i] = ptr;
		ptr = ptr + imageStride;
	}
    // image has separated alpha channel
	if (alphaBuff)
	{
		SrcImage.alphaChans++;  // increase alpha channels
		SrcImage.alphaScan = new LPBYTE[height];
		ptr = (LPBYTE)alphaBuff;
		for (int i = 0; i < height; i++)
		{
			SrcImage.alphaScan[i] = ptr;
			ptr = ptr + alphaStride;
		}
	}
	else
	{
		SrcImage.alphaScan = 0;
		SrcImage.alphaStride = 0;
	}
	// create and set destination image - copy of source
	DstImage.imageBuff = malloc(height * imageStride);
	DstImage.imageScan = new LPBYTE[height];
	// fill destination image scanlines
	ptr = (LPBYTE)DstImage.imageBuff;
	for (int i = 0; i < height; i++)
	{
		DstImage.imageScan[i] = ptr;
		memcpy(DstImage.imageScan[i], SrcImage.imageScan[i], imageStride);
		ptr = ptr + imageStride;
	}
    // image has separated alpha channel - copy to dest
	if (alphaBuff)
	{
		DstImage.alphaChans++;	// increase alpha channels
		DstImage.alphaBuff = malloc(height * alphaStride);
		DstImage.alphaScan = new LPBYTE[height];	
		ptr = (LPBYTE)DstImage.alphaBuff;
		for (int i = 0; i < height; i++)
		{
			DstImage.alphaScan[i] = ptr;
			memcpy(DstImage.alphaScan[i], SrcImage.alphaScan[i], alphaStride);
			ptr = ptr + alphaStride;
		}
	}
	else
	{
		DstImage.alphaScan = 0;
		DstImage.alphaStride = 0;
	}
	// set suites static vars
	SuitesDisplChans = SrcImage.channels + SrcImage.alphaChans;		// + external alpha if any
	return 0;
}
//-----------------------------------------------------------------
// set working mask by buffer
//-----------------------------------------------------------------
int TPspiCore::SetMask(int width, int height, void *maskBuff, int maskStride, bool useByPi)
{
	releaseMask(&SrcMask);
	SrcMask.width = width;
	SrcMask.height = height;
	SrcMask.maskStride = maskStride;
	SrcMask.useByPi = useByPi;
	// fill mask scanlines
	SrcMask.maskScan = new LPBYTE[height];
	LPBYTE ptr = (LPBYTE)maskBuff;
	for (int i = 0; i < height; i++)
	{
		SrcMask.maskScan[i] = ptr;
		ptr = ptr + maskStride;
	}
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
	switch (type)
	{
		case PSPIW_IMT_BGR:
			SrcImage.channels = DstImage.channels = 3;
			SrcImage.alphaChans = DstImage.alphaChans = 0;
			chanOrder[0] = 2;
			chanOrder[2] = 0;
			break;
		case PSPIW_IMT_BGRA:
			SrcImage.channels = DstImage.channels = 4;
			SrcImage.alphaChans = DstImage.alphaChans = 0;
			chanOrder[0] = 2;
			chanOrder[2] = 0;
			break;
		case PSPIW_IMT_RGB:
			SrcImage.channels = DstImage.channels = 3;
			SrcImage.alphaChans = DstImage.alphaChans = 0;
			break;
		case PSPIW_IMT_RGBA:
			SrcImage.channels = DstImage.channels = 4;
			SrcImage.alphaChans = DstImage.alphaChans = 1;
			break;
		case PSPIW_IMT_GRAYA:
			SrcImage.channels = DstImage.channels = 2;
			SrcImage.alphaChans = DstImage.alphaChans = 1;
			break;
		default:
			SrcImage.channels = DstImage.channels = 1;
			SrcImage.alphaChans = DstImage.alphaChans = 0;
			break;
	}
	SrcImage.width = DstImage.width = width;
	SrcImage.height = DstImage.height = height;
	// create array for image scanlines
	SrcImage.imageScan = new LPBYTE[height];
	DstImage.imageScan = new LPBYTE[height];
    // image has separated alpha channel
	if (externalAlpha)
	{
		SrcImage.alphaChans++;  // increase alpha channels
		SrcImage.alphaScan = new LPBYTE[height];
		DstImage.alphaChans++;
		DstImage.alphaScan = new LPBYTE[height];
	}
	else
	{
		SrcImage.alphaScan = 0;
		SrcImage.alphaStride = 0;
		DstImage.alphaScan = 0;
		DstImage.alphaStride = 0;
	}
	return 0;
}
//-----------------------------------------------------------------
// Add image scanline to prepared container
//-----------------------------------------------------------------
int TPspiCore:: AddImageSL(int scanIndex, void *imageScanLine,  void *alphaScanLine)
{
	if (SrcImage.width == 0)
		return PSPIW_ERR_IMAGE_INVALID;
	if (scanIndex >= SrcImage.height)
		return PSPIW_ERR_BAD_PARAM;
	if (alphaScanLine && SrcImage.alphaScan == NULL)
		return PSPIW_ERR_BAD_PARAM;
	SrcImage.imageScan[scanIndex] = (LPBYTE)imageScanLine;
	if (alphaScanLine)
		SrcImage.alphaScan[scanIndex] = (LPBYTE)alphaScanLine;
	return 0;
}
//-----------------------------------------------------------------
// Finish image creation
//-----------------------------------------------------------------
int TPspiCore::FinishImageSL(int imageStride, int alphaStride)
{
	if (SrcImage.width == 0)
		return PSPIW_ERR_IMAGE_INVALID;
	if (DstImage.width == 0)
		return PSPIW_ERR_IMAGE_INVALID;
	if (imageStride != 0)
	{
		SrcImage.imageStride = imageStride;
		DstImage.imageStride = imageStride;
	}
	else
		DstImage.imageStride = DstImage.width * DstImage.channels;	// only dest stride as we don't know src alignement
	DstImage.imageBuff = malloc(DstImage.height * DstImage.imageStride);
	LPBYTE ptr = (LPBYTE)DstImage.imageBuff;
	for (int i = 0; i < DstImage.height; i++)
	{
		DstImage.imageScan[i] = ptr;
		memcpy(DstImage.imageScan[i], SrcImage.imageScan[i], DstImage.imageStride);
		ptr = ptr + DstImage.imageStride;
	}
	if (alphaStride != 0)
	{
		SrcImage.alphaStride = alphaStride;
		DstImage.alphaStride = alphaStride;
	}
	else
	{
		DstImage.alphaStride = DstImage.width;	// only dest stride as we don't know src alignement
	}
	if (DstImage.alphaScan)
	{
		DstImage.alphaBuff = malloc(DstImage.height * DstImage.alphaStride);
		ptr = (LPBYTE)DstImage.alphaBuff;
		for (int i = 0; i < DstImage.height; i++)
		{
			DstImage.alphaScan[i] = ptr;
			memcpy(DstImage.alphaScan[i], SrcImage.alphaScan[i], DstImage.alphaStride);
			ptr = ptr + DstImage.alphaStride;
		}
	}
	return 0;
}
//-----------------------------------------------------------------
// start mask creation  (for adding scanlines)
//-----------------------------------------------------------------
int TPspiCore::StartMaskSL(int width, int height, bool useByPi)
{
	if (DstImage.width == 0)
		return PSPIW_ERR_IMAGE_INVALID;
	if (DstImage.width != width || DstImage.height != height)
		return PSPIW_ERR_BAD_PARAM;
	releaseMask(&SrcMask);
	SrcMask.width = width;
	SrcMask.height = height;
	SrcMask.useByPi = useByPi;
	SrcMask.maskScan = new LPBYTE[height];
	return 0;
}
//-----------------------------------------------------------------
// Add mask scanline to prepared container
//-----------------------------------------------------------------
int TPspiCore::AddMaskSL(int scanIndex, void *maskScanLine)
{
	if (SrcMask.width == 0)
		return PSPIW_ERR_IMAGE_INVALID;
	if (scanIndex >= SrcMask.height)
		return PSPIW_ERR_BAD_PARAM;
	SrcMask.maskScan[scanIndex] = (LPBYTE)maskScanLine;
	return 0;
}
//-----------------------------------------------------------------
// Finish Add mask scanline to prepared surface
//-----------------------------------------------------------------
int TPspiCore::FinishMaskSL(int maskStride)
{
	if (maskStride != 0)
		SrcMask.maskStride = maskStride;
	return 0;
}
//-----------------------------------------------------------------
// Load plugin
//-----------------------------------------------------------------
int TPspiCore::PlugInLoad(wchar_t *filter)
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
		return PSPIW_ERR_FILTER_NOT_LOADED;
	}
	if (!loadPIPLResources(HostRecord.dllHandle))
	{
		if (!loadPIMIResources(HostRecord.dllHandle))
			return PSPIW_ERR_FILTER_INVALID;
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
		return PSPIW_ERR_FILTER_INVALID;
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
		return PSPIW_ERR_FILTER_NOT_LOADED;
	int rc = 0;
	PLUGINPROC PluginMainProc;
	PluginMainProc = (PLUGINPROC)GetProcAddress(HostRecord.dllHandle, HostRecord.piEntrypointName.c_str());
	if (!PluginMainProc)
	{
		return PSPIW_ERR_FILTER_BAD_PROC;
	}
	AboutRecord aRecord;
	PlatformData pData;
	aRecord.platformData = &pData;
	int16 result = 1;
	intptr_t* dataPtr = NULL;
	SPBasicSuite bSuite;
	pData.hwnd = (intptr_t) hWnd;
	memset(&aRecord, 0, sizeof(aRecord));
	aRecord.platformData = &pData;
	 // uups, input, acquire about fails...
	memset(&bSuite, 0, sizeof(bSuite));
	bSuite.AcquireSuite = 0;// AcquireSuite;
	aRecord.sSPBasic = 0;   // &bSuite;
	aRecord.plugInRef = 0;  // Application;
	try
	{
		PluginMainProc(plugInSelectorAbout, &aRecord, dataPtr, &result);
		if (result)
			rc = PSPIW_ERR_FILTER_DUMMY_PROC;
	}
	catch(...)
	{
		rc = PSPIW_ERR_FILTER_ABOUT_ERROR;
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
		return PSPIW_ERR_IMAGE_INVALID;
	// check if plug-in is loaded
	if (!HostRecord.filterLoaded)
		return PSPIW_ERR_FILTER_NOT_LOADED;
	aState.Init();
	HostRecord.srcImage = &SrcImage;
	HostRecord.dstImage = &DstImage;
	HostRecord.hWnd = hWnd;
	if (SrcMask.width)
		HostRecord.srcMask = &SrcMask;
	else
		HostRecord.srcMask = 0;
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
		return PSPIW_ERR_FILTER_BAD_PROC;
	}
	try
	{
	   filterStage = 0;
	   //fRecord.parameters = DoNewPIHandle(100000); // test
	   PluginMainProc(filterSelectorParameters, &fRecord, dataPtr, &result);
	   if (result ==  noErr)
		  {
		  filterStage = 1;
		  PluginMainProc(filterSelectorPrepare, &fRecord, dataPtr, &result);
		  if (result == noErr)
			 {
			 filterStage = 2;
			 PluginMainProc(filterSelectorStart, &fRecord, dataPtr, &result);
			 if (result == noErr)
				{
				filterStage = 3;
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
					filterStage = 4;
					PluginMainProc(filterSelectorFinish, &fRecord, dataPtr, &finish_result);
					DoAdvanceState();
					}
				}
			 }
		  }
	   if (result != noErr)
		  rc = PSPIW_ERR_FILTER_CANCELED;
	   }
	 catch (...)
	   {
	   rc = PSPIW_ERR_FILTER_CRASHED;
	   }
if (result == noErr)
	{
	dst2Src();
	}
//TODO: release suite handles 
 return rc;
}
//---------------------------------------------------------------------------
// Enumerate Filters from directory using call back function
//---------------------------------------------------------------------------
int TPspiCore::PlugInEnumerate(ENUMCALLBACK enumFunc, bool recurseSubFolders)
{
	if (HostRecord.piPath.empty())
		return  PSPIW_ERR_WORK_PATH_EMPTY;
	enumResourcesPath(HostRecord.piPath, enumFunc, recurseSubFolders);
	return 0;
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








