#pragma once
#include <windows.h>
#include <windowsx.h>
#include <PIGeneral.h>
#include "PIAcquire.h"
#include "PIAbout.h"
#include "PIFilter.h"
#include <PIProperties.h>
#include "pspiHelper.hpp"
typedef void(*ASPptr)(void*);
typedef bool(*CPRptr)(void*, uint32 &);
typedef void(*PGRptr)(void *, uint32, uint32);
ASPptr SuitesASP;
CPRptr SuitesCPR;
PGRptr SuitesPGR;
void *SuitesCorePtr;
static FilterRecord *SuitesFPR;
static SpspiHostRecord *SuitesHostPR;
static unordered_map<unsigned long, SpspiHandle*> *SuitesHMP;
static int *SuitesChanOrder;
static int SuitesDisplChans;
static SpspiImage cpsRes;
//---------------------------------------------------------------------------
// Check if handle exists
//---------------------------------------------------------------------------
bool AllocatedBySuite(Handle handle)
{
	unsigned long ulh = (unsigned long)handle;
	return !(SuitesHMP->find(ulh) == SuitesHMP->end());
}
//---------------------------------------------------------------------------
// clean the house
//---------------------------------------------------------------------------
void ReleaseSuiteData(void)
{
	//TODO: clean the house!
}
//---------------------------------------------------------------------------
//
// Allocate BP routine  PSP
//
//---------------------------------------------------------------------------
OSErr DoAllocateBuffer(int32 size, BufferID* bufferID)
{
 HANDLE h = GlobalAlloc(GMEM_MOVEABLE, size);
 if (h == 0)
    return memFullErr;
 *bufferID = BufferID(h);
 return noErr;
}
//---------------------------------------------------------------------------
//
// Lock buffer routine PSP
//
//---------------------------------------------------------------------------
Ptr DoLockBuffer(BufferID b, Boolean moveHigh)
{
	//return (Ptr) b;
    return Ptr(GlobalLock(HANDLE(b)));
}
//---------------------------------------------------------------------------
//
// Unlock buffer routine PSP
//
//---------------------------------------------------------------------------
void DoUnlockBuffer(BufferID b)
{
	GlobalUnlock(HANDLE(b));
}
//---------------------------------------------------------------------------
//
// Free buffer routine PSP
//
//---------------------------------------------------------------------------
void DoFreeBuffer(BufferID b)
{
	DoUnlockBuffer(b);
	GlobalFree(HANDLE(b));
}
//---------------------------------------------------------------------------
//
// Buffer space routine PSP
//
//---------------------------------------------------------------------------
int32 DoBufferSpace(void)
{
  return SuitesFPR->maxSpace;  // BufferSpace
}
//---------------------------------------------------------------------------
//
// Buffer space routine PSP
//
//---------------------------------------------------------------------------
OSErr DoBufferReserve (int32 size)
{
  // reserve space - dummy 
  return noErr; 
}
//---------------------------------------------------------------------------
OSErr DoAllocateBuffer64(int64 size, BufferID *bufferID)
{
	HANDLE h = GlobalAlloc(GMEM_MOVEABLE, size);
	if (h == 0)
	    return memFullErr;
	 *bufferID = BufferID(h);
	 return noErr;
}
//---------------------------------------------------------------------------
int64 DoBufferSpace64(void)
{
	return SuitesFPR->maxSpace64;
}
//---------------------------------------------------------------------------
//
// Abort routine  PSP
//
//---------------------------------------------------------------------------
Boolean DoTestAbort(void)
{
 return 0x00;
}
//---------------------------------------------------------------------------
void  DoHostProc (int16 selector, intptr_t *data)
{
int ii= 0;
}
//---------------------------------------------------------------------------
//
// Prcess event
//
//---------------------------------------------------------------------------
void DoProcessEvent (void *event)
{
int ii = 0;
}
//---------------------------------------------------------------------------
//
// Acauire  routine  PSP
//
//---------------------------------------------------------------------------
SPErr DoAcquireSuite( const char *name, int32 version, const void **suite )
{
 // dummy for now
 return errPlugInHostInsufficient;
}
//---------------------------------------------------------------------------
//
// Count PIR routine  PSP
//
//---------------------------------------------------------------------------
int16 DoCountPIResources(ResType)
{
 return 0;
}
//---------------------------------------------------------------------------
//
// Handle PIR routine  PSP
//
//---------------------------------------------------------------------------
Handle DoGetPIResource(ResType, int16)
{
	return 0;
}
//---------------------------------------------------------------------------
//
// Delete PIR routine  PSP
//
//---------------------------------------------------------------------------
void DoDeletePIResource(ResType, int16)
{
	return;
}
//---------------------------------------------------------------------------
//
// Add PIR routine  PSP
//
//---------------------------------------------------------------------------
OSErr DoAddPIResource(ResType, Handle)
{
	return noErr;
}
//---------------------------------------------------------------------------
//
// New PIH routine  PSP
//
//---------------------------------------------------------------------------
Handle DoNewPIHandle(int32 size)
{
	SpspiHandle *mHand = new SpspiHandle;
	if (mHand)
	{
		mHand->hPointer = (Ptr) GlobalAllocPtr (GHND, size);
		mHand->hSize = size;
	}
	else
		return NULL;
	//pair< unsigned long, SpspiHandle*> result;
	SuitesHMP->insert(make_pair((unsigned int)mHand, mHand));
	return (Handle)mHand;
}
//---------------------------------------------------------------------------
//
// Dispose PIH routine  PSP
//
//---------------------------------------------------------------------------
void DoDisposePIHandle(Handle handle)
{
	if (!AllocatedBySuite(handle))
	    {
		if (GlobalSize ((HGLOBAL) handle) > 0)
			GlobalFreePtr ((Ptr)handle);
		else if (!IsBadReadPtr (handle, sizeof (HGLOBAL *)) &&  GlobalSize (*(HGLOBAL *) handle) > 0)
			GlobalFree (*(HGLOBAL *) handle);
		}
}
//---------------------------------------------------------------------------
//
// Get PIH  routine  PSP
//
//---------------------------------------------------------------------------
int32 DoGetPIHandleSize(Handle handle)
{
	int32 size;
	if (!AllocatedBySuite(handle))
    {
		if ((size = (int32)GlobalSize ((HGLOBAL) handle)) > 0)
			return size;	
		else if (!IsBadReadPtr (handle, sizeof (HGLOBAL *)) && (size = (int32)GlobalSize (*(HGLOBAL *) handle)) > 0)
	       return (int32)GlobalSize (handle);
    }
	return 0;
}
//---------------------------------------------------------------------------
//
// Set PIH routine  PSP
//
//---------------------------------------------------------------------------
OSErr DoSetPIHandleSize(Handle handle, int32 newSize)
{
	if (!AllocatedBySuite(handle))
	{
		if (GlobalSize((HGLOBAL)handle) > 0)
		{
			if (GlobalReAlloc((HGLOBAL)handle, newSize, 0) != (HGLOBAL)handle)
				return nilHandleErr;
			return noErr;
		}
		else if (!IsBadReadPtr(handle, sizeof(HGLOBAL *)) && GlobalSize(*(HGLOBAL *)handle) > 0)
		{
			*((HGLOBAL *)handle) = GlobalReAlloc(*(HGLOBAL *)handle, newSize, 0);
			return noErr;
		}
		else
			return nilHandleErr;
	}
 return noErr;
}
//---------------------------------------------------------------------------
//
// Lock PIH routine  PSP
//
//---------------------------------------------------------------------------
Ptr DoLockPIHandle(Handle handle, Boolean moveHigh)
{
	if (!AllocatedBySuite(handle))
	{
	    if (GlobalSize ((HGLOBAL) handle) > 0)
			return (Ptr)GlobalLock ((HGLOBAL) handle);
		else if (!IsBadReadPtr (handle, sizeof (HGLOBAL *)) && GlobalSize (*(HGLOBAL *) handle) > 0)
			return (Ptr)GlobalLock (*(HGLOBAL *) handle);
		else
		{
			if (!IsBadReadPtr(handle, sizeof(Ptr)) && !IsBadWritePtr(*(Ptr *)handle, 8))
				return *(Ptr *)handle;
			else
				return NULL;
	    }
	}
	return ((SpspiHandle *) handle)->hPointer;
}
//---------------------------------------------------------------------------
//
// Unlock PIH routine  PSP
//
//---------------------------------------------------------------------------
void DoUnlockPIHandle(Handle handle)
{
	if (!AllocatedBySuite(handle))
    {
		if (GlobalSize ((HGLOBAL) handle) > 0)
		   GlobalUnlock ((HGLOBAL) handle);
		else if (!IsBadReadPtr (handle, sizeof (HGLOBAL *)) && GlobalSize (*(HGLOBAL *) handle) > 0)
		   GlobalUnlock (*(HGLOBAL *) handle);
	}
}
//---------------------------------------------------------------------------
//
// Recover Space routine  PSP
//
//---------------------------------------------------------------------------
void DoRecoverSpace(int32 size)
{
	return;
}
//---------------------------------------------------------------------------
//
// Dispose regular routine  PSP
//
//---------------------------------------------------------------------------
void DoDisposeRegular(Handle handle)
{
	if (!AllocatedBySuite(handle))
    {
		if (GlobalSize ((HGLOBAL) handle) > 0)
			GlobalFree ((HGLOBAL) handle);
		else if (!IsBadReadPtr (handle, sizeof (HGLOBAL *)) && GlobalSize (*(HGLOBAL *) handle) > 0)
			GlobalFree (*(HGLOBAL *) handle);
	}
}
//---------------------------------------------------------------------------
//
// Color Service routine  PSP
//
//---------------------------------------------------------------------------
OSErr DoColorServices (ColorServicesInfo *info)
{
 if (info->infoSize < sizeof (ColorServicesInfo))
    return errPlugInHostInsufficient;
  switch (info->selector)
    {
    case plugIncolorServicesChooseColor:
		{
			uint32 colorPick;
			if (SuitesCPR(SuitesCorePtr, colorPick))
			{
				info->colorComponents[SuitesChanOrder[0]] = BYTE((colorPick << 8) >> 24);
				info->colorComponents[SuitesChanOrder[1]] = BYTE((colorPick << 16) >> 24);
				info->colorComponents[SuitesChanOrder[2]] = BYTE((colorPick << 24) >> 24);
			}
		}
         return noErr;       
    case plugIncolorServicesConvertColor:
         break;
    case plugIncolorServicesSamplePoint:
         break;
    case plugIncolorServicesGetSpecialColor:
         break;
    default:
         break;
    }
  return errPlugInHostInsufficient;
}
//---------------------------------------------------------------------------
//
// Display Pixels routine  PSP
//
//---------------------------------------------------------------------------
OSErr DoDisplayPixels(const PSPixelMap *source, const VRect *srcRect, int32 dstRow, int32 dstCol, void *platformContext)
{
	//TODO: posredi kad je u pitanju alpha kanal
	HDC hdc = (HDC) platformContext, hmemdc;
	void * hbitmap, *holdbm;
	struct {
	    BITMAPINFOHEADER bi;
	    union {
	      WORD bmiIndices[256];
	      DWORD bmiMasks[3];
		  RGBQUAD bmiColors[256];
	    } u;
	  } bmi;
	LPBYTE bits;
	int bypp = 0;
	LPBYTE src, dst;
	//char rgb[3];
	int w = srcRect->right - srcRect->left, h = srcRect->bottom - srcRect->top;
	int bpl;
	/* Some plug-ins call displayPixels with bogus parameters */
	if (hdc == NULL || source->rowBytes == 0 || source->baseAddr == 0)
	    return filterBadParameters;
	bmi.bi.biSize = sizeof (BITMAPINFOHEADER);
	bmi.bi.biWidth = w;
	bmi.bi.biHeight = -h;
	bmi.bi.biPlanes = 1;
	switch (source->imageMode)
	{
	case plugInModeGrayScale:
		bypp = 1;
		bmi.bi.biBitCount = 8;
		for (int i = 0; i < 255; i++)
		{
			bmi.u.bmiColors[i].rgbBlue =
		    bmi.u.bmiColors[i].rgbGreen =
		    bmi.u.bmiColors[i].rgbRed = i;
		    bmi.u.bmiColors[i].rgbReserved = 0;
		}
		break;
    case plugInModeRGBColor:
		bypp = SuitesDisplChans;
		bmi.bi.biBitCount = bypp * 8;
	    break;
    default:
	    break;
    }
	bmi.bi.biCompression = BI_RGB;
	bmi.bi.biSizeImage = 0;
	bmi.bi.biXPelsPerMeter = bmi.bi.biYPelsPerMeter = 0;
	bmi.bi.biClrUsed = 0;
	bmi.bi.biClrImportant = 0;

	if ((hbitmap = CreateDIBSection (hdc, (BITMAPINFO *) &bmi, DIB_RGB_COLORS, (void **) &bits, NULL, 0)) == NULL)
    {
	      return noErr;
    }
	bpl = ((w*bypp - 1)/4 + 1) * 4;
	if (bypp == 1)
	{
		for (int y = srcRect->top; y < srcRect->bottom; y++)
		{
			memmove(bits + bpl * y, ((char *)source->baseAddr) + srcRect->left + source->rowBytes*y, w);
		}
	}
	else
	{
		if (source->colBytes == 1)
		{
			/*
			for (int y = 0; y < h; y++)
			{
				src = ((LPBYTE)source->baseAddr) + y * source->rowBytes; // +srcRect->left;
				dst = bits + bpl*y;
				for (int x = 0; x < w; x++)
				{
					for (int k = 0; k < SuitesDisplChans; k++)
					{
						LPBYTE src_ptr = src + k * source->planeBytes;
						dst[SuitesChanOrder[k]] = src_ptr[k];
					}
					//rgb[0]= *src;
		            //rgb[1] = *LPBYTE(src + source->planeBytes);
		            //rgb[2] = *LPBYTE(src + 2*source->planeBytes);
		            //dst[0] = rgb[SuitesChanOrder[0]];
		            //dst[1] = rgb[SuitesChanOrder[1]];
		            //dst[2] = rgb[SuitesChanOrder[2]];
					// move forward
					dst += bypp;
					src += source->colBytes;
				}
			}
			*/
			// suggested by Irfan Škiljan
			for (int32 k = 0; k < SuitesDisplChans; k++)
			{			
		        uint8* inPixel = (uint8*)source->baseAddr + ((size_t)k * (size_t)source->planeBytes);
		        for (int32 y = 0; y < h; y++)
		        {
	                dst = bits + bpl*y;
	                for (int32 x = 0; x < w; x++)
	                {
		                dst[x*bypp + SuitesChanOrder[k]] = *inPixel;
		                inPixel++;
	                }	
				}
			}
		}
		else
		{
			for (int y = 0; y < h; y++)
			{
				src = ((LPBYTE)source->baseAddr) + y* source->rowBytes; // +srcRect->left * source->colBytes;
				dst = bits + bpl * y;
				for (int x = 0; x < w; x++)
				{
					/* take care of channels order -> dest DIB can be BGR, source can be RGB */
					for (int k = 0; k < SuitesDisplChans; k++)
						dst[SuitesChanOrder[k]] = src[k];
					//TODO: checkerboard for alpha channel
					// move forward
					dst += bypp;
					src += source->colBytes;
				}
			}
		}
	}
	hmemdc = CreateCompatibleDC(hdc);
	holdbm = SelectObject(hmemdc, hbitmap);
	// bitblt
	if (!BitBlt(hdc, dstCol, dstRow, bmi.bi.biWidth, -bmi.bi.biHeight, hmemdc, 0, 0, SRCCOPY))
		{
		if (GetLastError() != ERROR_SUCCESS)
		{
			// say something on console???
			//return ioErr;
		}
	}
	SelectObject(hmemdc, holdbm);
	DeleteDC(hmemdc);
	DeleteObject(hbitmap);
	return noErr;
}
//---------------------------------------------------------------------------
//
// Progress routine  PSP
//
//---------------------------------------------------------------------------
void DoProgressProc (uint32 done, uint32 total)
{
  SuitesPGR(SuitesCorePtr, done, total);
}
//---------------------------------------------------------------------------
//
// Suite GP routine  PSP
//
//---------------------------------------------------------------------------
OSErr DoSuiteGetProperty (PIType signature, PIType key, int32 index,
                         intptr_t *simpleProperty, Handle *complexProperty)
{
 return noErr;
}
//---------------------------------------------------------------------------
//
// Suite SP routine  PSP
//
//---------------------------------------------------------------------------
OSErr DoSuiteSetProperty (PIType signature, PIType key, int32 index,
        				intptr_t simpleProperty,  Handle complexProperty)
{
 return noErr;
}
//---------------------------------------------------------------------------
//
// Read Pixels routine  PSP
//
//---------------------------------------------------------------------------
static OSErr DoReadPixels (ChannelReadPort			port,
                         const PSScaling       *scaling,
                         const VRect           *writeRect,
                         const PixelMemoryDesc *destination,
                         VRect                 *wroteRect)
{
	if (destination->depth != 8)
		return errUnsupportedDepth;
	if ((destination->bitOffset % 8) != 0)  // the offsets must be aligned to byte. 
		return errUnsupportedBitOffset;
	if ((destination->colBits % 8) != 0)
		return errUnsupportedColBits;
	if ((destination->rowBits % 8) != 0)
		return errUnsupportedRowBits;

	int channel = (int32)port;

	if (channel < ctUnspecified || channel > ctSelectionMask) // < gray || > selectionMask
		return errUnknownPort;
	VRect srcRect = scaling->sourceRect;
	VRect dstRect = scaling->destinationRect;

	int srcWidth = srcRect.right - srcRect.left;
	int srcHeight = srcRect.bottom - srcRect.top;
	int dstWidth = dstRect.right - dstRect.left;
	int dstHeight = dstRect.bottom - dstRect.top;
	bool isSelection = (channel == ctSelectionMask);
	SpspiImage *source = SuitesHostPR->srcImage;
	if (isSelection)
	{
		/*
		if (srcWidth == dstWidth && srcHeight == dstHeight)
				{
					FillSelectionMask(destination, mask, srcRect);
				}
				else if (dstWidth < srcWidth || dstHeight < srcHeight) // scale down
				{
					if ((scaledSelectionMask == null) || scaledSelectionMask.Width != dstWidth || scaledSelectionMask.Height != dstHeight)
					{
						if (scaledSelectionMask != null)
						{
							scaledSelectionMask.Dispose();
							scaledSelectionMask = null;
						}

						try
						{
							scaledSelectionMask = new Surface8(dstWidth, dstHeight);
							scaledSelectionMask.SuperSampleFitSurface(mask);
						}
						catch (OutOfMemoryException)
						{
							return memFullErr;
						}
					}

					FillSelectionMask(destination, scaledSelectionMask, dstRect);
				}
				else if (dstWidth > srcWidth || dstHeight > srcHeight) // scale up
				{
					if ((scaledSelectionMask == null) || scaledSelectionMask.Width != dstWidth || scaledSelectionMask.Height != dstHeight)
					{
						if (scaledSelectionMask != null)
						{
							scaledSelectionMask.Dispose();
							scaledSelectionMask = null;
						}

						try
						{
							scaledSelectionMask = new Surface8(dstWidth, dstHeight);
							scaledSelectionMask.BicubicFitSurface(mask);
						}
						catch (OutOfMemoryException)
						{
							return PSError.memFullErr;
						}
					}

					FillSelectionMask(destination, scaledSelectionMask, dstRect);
				}
			*/

			}
			else
			{
				if (srcWidth == dstWidth && srcHeight == dstHeight)
					hlpCopyChannelData(SuitesHostPR, SuitesChanOrder, channel, destination, source, srcRect);
				else if (dstWidth != srcWidth || dstHeight != srcHeight) // scale image
				{
					if ((cpsRes.imageBuff == NULL) || cpsRes.width != dstWidth || cpsRes.height != dstHeight)
					{
						if (cpsRes.imageBuff)
							hlpReleaseImage(&cpsRes, true);
						hlpSetupImageFromMaster(source, &cpsRes, dstWidth, dstHeight);
					}
					hlpCopyChannelData(SuitesHostPR, SuitesChanOrder, channel, destination, &cpsRes, dstRect);
				}
			}
			wroteRect = &dstRect;
 return noErr;
}
//---------------------------------------------------------------------------
//
// Write Base Pixels routine  PSP
//
//---------------------------------------------------------------------------

static OSErr DoWriteBasePixels (ChannelWritePort       port,
                              const VRect           *writeRect,
                              const PixelMemoryDesc *source)
{
 return errPlugInHostInsufficient;
}
//---------------------------------------------------------------------------
//
// ReadPortForWritePort routine  PSP
//
//---------------------------------------------------------------------------
static OSErr DoReadPortForWritePort (ChannelReadPort *readPort,
                                   ChannelWritePort writePort)
{
 return errPlugInHostInsufficient;
}
//---------------------------------------------------------------------------
//
// 1D routine  PSP
//
//---------------------------------------------------------------------------
static OSErr DoInterpolate1D (PSImagePlane *source,
                                    PSImagePlane *destination,
                                    Rect         *area,
                                    Fixed        *coords,
                                    int16         method)
{
  return memFullErr;
}
//---------------------------------------------------------------------------
//
// 2D routine  PSP
//
//---------------------------------------------------------------------------
static OSErr DoInterpolate2D (PSImagePlane *source,
                                    PSImagePlane *destination,
                                    Rect         *area,
                                    Fixed        *coords,
                                    int16         method)
{
  return memFullErr;
}
//---------------------------------------------------------------------------
//
// 1D multi routine  PSP
//
//---------------------------------------------------------------------------
static OSErr DoInterpolate1Dmulti (PSImageMultiPlane *source,
                                          PSImageMultiPlane *destination,
                                          Rect              *area,
                                          Fixed             *coords,
                                          int16              method)
{
  return memFullErr;
}
//---------------------------------------------------------------------------
//
// 2D multi routine  PSP
//
//---------------------------------------------------------------------------
static OSErr DoInterpolate2Dmulti (PSImageMultiPlane *source,
                                          PSImageMultiPlane *destination,
                                          Rect              *area,
                                          Fixed             *coords,
                                          int16              method)
{
  return memFullErr;
}
//---------------------------------------------------------------------------
//
// 1D multi routine 32  PSP
//
//---------------------------------------------------------------------------
static OSErr DoInterpolate1Dmulti32(PSImageMultiPlane32 *source,
										PSImageMultiPlane32 *destination,
										VRect *area,
										int64 *coords,	// s47.16 numbers
										int16 method)
{
  return memFullErr;
}
//---------------------------------------------------------------------------
//
// 1D multi routine 32  PSP
//
//---------------------------------------------------------------------------
static OSErr DoInterpolate2Dmulti32(PSImageMultiPlane32 *source,
										PSImageMultiPlane32 *destination,
										VRect *area,
										int64 *coords,	// s47.16 numbers
										int16 method)
{
  return memFullErr;
}
//---------------------------------------------------------------------------
//
// Advance State routine
//
//---------------------------------------------------------------------------
OSErr DoAdvanceState(void)
{
	SuitesASP(SuitesCorePtr);
    return noErr;
}







