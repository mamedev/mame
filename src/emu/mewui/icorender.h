// license:BSD-3-Clause
// copyright-holders:Dankan1890;Victor Laskin
/***************************************************************************

    mewui/icorender.h

    ICOns file loader.

    Original code by Victor Laskin (victor.laskin@gmail.com)
    http://vitiy.info/Code/ico.cpp

    Revised for MEWUI by dankan1890.

***************************************************************************/
#pragma once

#ifndef __MEWUI_ICORENDER_H__
#define __MEWUI_ICORENDER_H__

// These next two structs represent how the icon information is stored
// in an ICO file.
typedef struct
{
	UINT8   bWidth;                // Width of the image
	UINT8   bHeight;               // Height of the image (times 2)
	UINT8   bColorCount;           // Number of colors in image (0 if >=8bpp)
	UINT8   bReserved;             // Reserved
	UINT16  wPlanes;               // Color Planes
	UINT16  wBitCount;             // Bits per pixel
	UINT32  dwBytesInRes;          // how many bytes in this resource?
	UINT32  dwImageOffset;         // where in the file is this image
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
	UINT16  idReserved;            // Reserved
	UINT16  idType;                // resource type (1 for icons)
	UINT16  idCount;               // how many images?
	//ICONDIRENTRY  idEntries[1];  // the entries for each image
} ICONDIR, *LPICONDIR;

// size - 40 bytes
typedef struct {
	UINT32  biSize;
	UINT32  biWidth;
	UINT32  biHeight;   // Icon Height (added height of XOR-Bitmap and AND-Bitmap)
	UINT16  biPlanes;
	UINT16  biBitCount;
	UINT32  biCompression;
	INT32   biSizeImage;
	UINT32  biXPelsPerMeter;
	UINT32  biYPelsPerMeter;
	UINT32  biClrUsed;
	UINT32  biClrImportant;
} s_BITMAPINFOHEADER, *s_PBITMAPINFOHEADER;

// 46 bytes
typedef struct{
	s_BITMAPINFOHEADER  icHeader;       // DIB header
	UINT32              icColors[1];    // Color table (short 4 bytes) //RGBQUAD
	UINT8               icXOR[1];       // DIB bits for XOR mask
	UINT8               icAND[1];       // DIB bits for AND mask
} ICONIMAGE, *LPICONIMAGE;

//-------------------------------------------------
//  load an ICO file into a bitmap
//-------------------------------------------------

void render_load_ico(bitmap_argb32 &bitmap, emu_file &file, const char *dirname, const char *filename)
{
	INT32 width = 0;
	INT32 height = 0;

	// deallocate previous bitmap
	bitmap.reset();

	// define file's full name
	std::string fname;

	if (!dirname)
		fname.assign(filename);
	else
		fname.assign(dirname).append(PATH_SEPARATOR).append(filename);

	file_error filerr = file.open(fname.c_str());

	if (filerr != FILERR_NONE)
		return;

	// allocates a buffer for the image
	UINT64 size = file.size();
	UINT8 *buffer = global_alloc_array(UINT8, size + 1);

	// read data from the file and set them in the buffer
	file.read(buffer, size);

	LPICONDIR icoDir = (LPICONDIR)buffer;
	int iconsCount = icoDir->idCount;

	if (icoDir->idReserved != 0 || icoDir->idType != 1 || iconsCount == 0 || iconsCount > 20)
	{
		file.close();
		global_free_array(buffer);
		return;
	}

	UINT8* cursor = buffer;
	cursor += 6;
	ICONDIRENTRY* dirEntry = (ICONDIRENTRY*)(cursor);
	int maxSize = 0;
	int offset = 0;
	int maxBitCount = 0;
	for (int i = 0; i < iconsCount; i++, dirEntry++)
	{
		int w = dirEntry->bWidth;
		int h = dirEntry->bHeight;
		int bitCount = dirEntry->wBitCount;
		if (w * h > maxSize || bitCount > maxBitCount) // we choose icon with max resolution
		{
			width = w;
			height = h;
			offset = dirEntry->dwImageOffset;
			maxSize = w * h;
		}
	}

	if (offset == 0) return;

	cursor = buffer;
	cursor += offset;
	ICONIMAGE* icon = (ICONIMAGE*)(cursor);
	int realBitsCount = (int)icon->icHeader.biBitCount;
	bool hasAndMask = (realBitsCount < 32) && (height != icon->icHeader.biHeight);

	cursor += 40;
	bitmap.allocate(width, height);

	// rgba + vertical swap
	if (realBitsCount >= 32)
	{
		for (int x = 0; x < width; x++)
			for (int y = 0; y < height; y++)
			{
				int shift2 = 4 * (x + (height - y - 1) * width);
				bitmap.pix32(y, x) = rgb_t(cursor[shift2 + 3], cursor[shift2 + 2], cursor[shift2 + 1], cursor[shift2]);
			}
	}
	else if (realBitsCount == 24)
		for (int x = 0; x < width; x++)
			for (int y = 0; y < height; y++)
			{
				int shift2 = 3 * (x + (height - y - 1) * width);
				bitmap.pix32(y, x) = rgb_t(255, cursor[shift2 + 2], cursor[shift2 + 1], cursor[shift2]);
			}
	else if (realBitsCount == 8)  // 256 colors
	{
		// 256 color table
		UINT8 *colors = cursor;
		cursor += 256 * 4;
		for (int x = 0; x < width; x++)
			for (int y = 0; y < height; y++)
			{
				int shift2 = (x + (height - y - 1) * width);
				int index = 4 * cursor[shift2];
				bitmap.pix32(y, x) = rgb_t(255, colors[index + 2], colors[index + 1], colors[index]);
			}
	}
	else if (realBitsCount == 4)  // 16 colors
	{
		// 16 color table
		UINT8 *colors = cursor;
		cursor += 16 * 4;
		for (int x = 0; x < width; x++)
			for (int y = 0; y < height; y++)
			{
				int shift2 = (x + (height - y - 1) * width);
				UINT8 index = cursor[shift2 / 2];
				if (shift2 % 2 == 0)
					index = (index >> 4) & 0xF;
				else
					index = index & 0xF;
				index *= 4;
				bitmap.pix32(y, x) = rgb_t(255, colors[index + 2], colors[index + 1], colors[index]);
			}
	}
	else if (realBitsCount == 1)  // 2 colors
	{
		// 2 color table
		UINT8 *colors = cursor;
		cursor += 2 * 4;
		int boundary = width; // !!! 32 bit boundary (http://www.daubnet.com/en/file-format-ico)
		while (boundary % 32 != 0) boundary++;

		for (int x = 0; x < width; x++)
			for (int y = 0; y < height; y++)
			{
				int shift2 = (x + (height - y - 1) * boundary);
				UINT8 index = cursor[shift2 / 8];

				// select 1 bit only
				UINT8 bit = 7 - (x % 8);
				index = (index >> bit) & 0x01;
				index *= 4;
				bitmap.pix32(y, x) = rgb_t(255, colors[index + 2], colors[index + 1], colors[index]);
			}
	}

	// Read AND mask after base color data - 1 BIT MASK
	if (hasAndMask)
	{
		int boundary = width * realBitsCount; // !!! 32 bit boundary (http://www.daubnet.com/en/file-format-ico)
		while (boundary % 32 != 0) boundary++;
		cursor += boundary * height / 8;

		boundary = width;
		while (boundary % 32 != 0) boundary++;

		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
			{
				UINT8 bit = 7 - (x % 8);
				int shift2 = (x + (height - y - 1) * boundary) / 8;
				int mask = (0x01 & ((UINT8)cursor[shift2] >> bit));
				rgb_t colors = bitmap.pix32(y, x);
				UINT8 alpha = colors.a();
				alpha *= 1 - mask;
				colors.set_a(alpha);
				bitmap.pix32(y, x) = colors;
			}
	}
	file.close();
	global_free_array(buffer);
}

#endif /* __MEWUI_ICORENDER_H__ */
