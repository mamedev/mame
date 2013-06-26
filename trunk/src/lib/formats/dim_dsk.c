/*********************************************************************

    formats/dim_dsk.c

    DIM disk images

*********************************************************************/

#include <string.h>

#include "dim_dsk.h"
#include "basicdsk.h"

FLOPPY_IDENTIFY(dim_dsk_identify)
{
	UINT8 dim_header[16];

	floppy_image_read(floppy, (UINT8*)dim_header,0xab,16);

	if(strncmp((const char*)dim_header,"DIFC HEADER",11) == 0)
		*vote = 100;
	else
		*vote = 0;
	return FLOPPY_ERROR_SUCCESS;
}

FLOPPY_CONSTRUCT(dim_dsk_construct)
{
	struct basicdsk_geometry geometry;
	// DIM disk image header, most of this is guesswork
	int tracks = 77;
	int heads = 2;
	int sectors = 8;  // per track
	int sectorlen = 1024;
	int firstsector = 0x01;
	UINT8 format_tmp;
	int x;
	UINT16 temp;

	if(params)
	{
		// create
		return FLOPPY_ERROR_UNSUPPORTED;
	}

	// Offset + 0 : disk format type (1 byte):
	//  0 = 2HD / 2HDA (8 sector/track, 1024 bytes/sector, GAP#3 = 0x74)
	//  1 = 2HS        (9 sector/track, 1024 bytes/sector, GAP#3 = 0x39)
	//  2 = 2HC        (15 sector/track, 512 bytes/sector, GAP#3 = 0x54)
	//  3 = 2HDE(68)   (9 sector/track, 1024 bytes/sector, GAP#3 = 0x39)
	//  9 = 2HQ        (18 sector/track, 512 bytes/sector, GAP#3 = 0x54)
	//  17 = N88-BASIC (26 sector/track, 256 bytes/sector, GAP#3 = 0x33)
	//              or (26 sector/track, 128 bytes/sector, GAP#3 = 0x1a)
	//
	floppy_image_read(floppy, &format_tmp,0,1);

	switch(format_tmp)
	{
	case 0x00:
		sectors = 8;
		sectorlen = 1024;
		break;
	case 0x01:
	case 0x03:
		sectors = 9;
		sectorlen = 1024;
		break;
	case 0x02:
		sectors = 15;
		sectorlen = 512;
		break;
	case 0x09:
		sectors = 18;
		sectorlen = 512;
		break;
	case 0x11:
		sectors = 26;
		sectorlen = 256;
		break;
	}
	tracks = 0;
	for (x=0;x<86;x++)
	{
		floppy_image_read(floppy,&temp,(x*2)+1,2);
		if(temp == 0x0101)
			tracks++;
	}
	// TODO: expand on this basic implementation

	LOG_FORMATS("FDD: DIM image loaded - type %i, %i tracks, %i sectors per track, %i bytes per sector\n", format_tmp,tracks, sectors,sectorlen);

	memset(&geometry, 0, sizeof(geometry));
	geometry.heads = heads;
	geometry.first_sector_id = firstsector;
	geometry.sector_length = sectorlen;
	geometry.tracks = tracks+1;
	geometry.sectors = sectors;
	geometry.offset = 0x100;
	return basicdsk_construct(floppy, &geometry);
}
