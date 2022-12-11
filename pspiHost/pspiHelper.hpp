#pragma once
#include <windows.h>
#include <windowsx.h>
//--------------------------------------------------------
// release image 
//--------------------------------------------------------
void hlpReleaseImage(SpspiImage *img, bool dispose)
{
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
}
//--------------------------------------------------------
// setup (create) image from master template
//--------------------------------------------------------
void hlpSetupImageFromMaster(SpspiImage *master, SpspiImage *res, int width, int height)
{
	// setup result image
	res->width = width;
	res->height = height;
	res->channels  = master->channels;
	res->exaChannels = master->exaChannels;
	res->imageStride = master->channels * width;
	res->exaStride = master->exaChannels *width;
	res->imageBuff = GlobalAlloc(GMEM_FIXED, height * res->imageStride);
	res->imageScan = new LPBYTE[height];
	// fill result image scanlines
	LPBYTE ptr = (LPBYTE)res->imageBuff;
	for (int i = 0; i < height; i++)
	{
		res->imageScan[i] = ptr;
		memset(res->imageScan[i], 0, res->imageStride);
		ptr = ptr + res->imageStride;
	}
	// image has separated alpha channel - create result alpha
	if (master->exaBuff)
	{
		res->exaBuff = GlobalAlloc(GMEM_FIXED, height* res->exaStride);
		res->exaScan = new LPBYTE[height];	
		ptr = (LPBYTE)res->exaBuff;
		for (int i = 0; i < height; i++)
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
}
//--------------------------------------------------------
// very naive resizer...just for testing
//--------------------------------------------------------
void hlpNaiveResize(SpspiImage *source, SpspiImage *dest)
{
	int sw = source->width;
	int sh = source->height;
	int dw = dest->width;
	int dh = dest->height;
	int imchans = source->channels;
	int exchans = source->exaChannels;
	float factw = (float)sw / dw;
	float facth = (float)sh / dh;
	int si, sj;
	BYTE *src_ptr, *dst_ptr, *src_pix;
	BYTE *src_ap = 0, *dst_ap = 0, *src_ap_pix;
	for (int i = 0; i < dh; i++)
	{
		si = (int)(i * facth);
		if (si >= sh)
			continue;
		src_ptr = source->imageScan[si];
		dst_ptr = dest->imageScan[i];		
		if (source->exaBuff)
		{
			src_ap = source->exaScan[si];
			dst_ap = dest->exaScan[i];
		}
		for (int j = 0; j < dw; j++)
		{
			sj = (int)(j * factw);
			if (sj >= sw)
				continue;
			src_pix = src_ptr + sj;
			for (int k = 0; k < imchans; k++)
				dst_ptr[k] = src_pix[k];
			dst_ptr += imchans;	
			if (source->exaBuff)
			{
				src_ap_pix = src_ptr + sj;
				for (int k = 0; k < exchans; k++)
					dst_ap[k] = src_ap[k];
				dst_ap += exchans;
			}
		}
	}
}
//---------------------------------------------------------------------------
// Copy channel data (for channel port suite)
//---------------------------------------------------------------------------
void hlpCopyChannelData(SpspiHostRecord *SuitesHostPR, int *SuitesChanOrder, int channel, const PixelMemoryDesc *destiniation, SpspiImage *source, VRect srcRect)
{
	BYTE* dst_base_ptr = (BYTE*)destiniation->data;
	int stride = destiniation->rowBits / 8;
	int bpp = destiniation->colBits / 8;
	int offset = destiniation->bitOffset / 8;
	BYTE *src_ptr, *dst_ptr;
	int chanPos;
	// channel position 
	switch (channel)
	{
	case ctRed:
		chanPos = 0;
		break;
	case ctGreen:
		chanPos = 1;
		break;
	case ctBlue:
		chanPos = 2;
		break;
	case ctTransparency:
		chanPos = 3;
		break;
	}
	// copy bytes to destination
	// TODO: add external alpha support!!!!!
	switch (SuitesHostPR->imgType)
	{
	case PSPI_IMG_TYPE_GRAY:
		// what about GRAYA ???
		for (int i = srcRect.top; i < srcRect.bottom; i++)
		{
			src_ptr = source->imageScan[i];
			src_ptr = src_ptr + source->channels * srcRect.left;
			dst_ptr = dst_base_ptr + (i * stride) + offset;
			for (int j = srcRect.left; j < srcRect.right; j++)
			{
				*dst_ptr = *src_ptr;
				src_ptr += source->channels;
				dst_ptr += bpp;
			}
		}
		break;
		// BGRA
		// RGBA
	default:
		for (int i = srcRect.top; i < srcRect.bottom; i++)
		{
			src_ptr = source->imageScan[i];
			src_ptr = src_ptr + source->channels * srcRect.left;
			dst_ptr = dst_base_ptr + (i * stride) + offset;
			for (int j = srcRect.left; j < srcRect.right; j++)
			{
				dst_ptr[SuitesChanOrder[chanPos]] = src_ptr[chanPos];
				src_ptr += source->channels;
				dst_ptr += bpp;
			}
		}
		break;
	}
}
