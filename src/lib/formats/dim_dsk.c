// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/dim_dsk.c

    DIM disk images

*********************************************************************/

#include <string.h>
#include <assert.h>

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


/*********************************************************************

    formats/dim_dsk.c

    DIM disk images

*********************************************************************/

#include "dim_dsk.h"

dim_format::dim_format()
{
}

const char *dim_format::name() const
{
	return "dim";
}

const char *dim_format::description() const
{
	return "DIM disk image";
}

const char *dim_format::extensions() const
{
	return "dim";
}

int dim_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 h[16];

	io_generic_read(io, h, 0xab, 16);

	if(strncmp((const char *)h, "DIFC HEADER", 11) == 0)
		return 100;

	return 0;
}

bool dim_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int offset = 0x100;
	UINT8 h;
	UINT8 track_total = 77;
	int cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;

	io_generic_read(io, &h, 0, 1);

	int spt, gap3, bps, size;
	switch(h) {
	case 0:
	default:
		spt = 8;
		gap3 = 0x74;
		size = 3;
		break;
	case 1:
		spt = 9;
		gap3 = 0x39;
		size = 3;
		break;
	case 3:
		spt = 9;
		gap3 = 0x39;
		size = 3;
		break;
	case 2:
		spt = 15;
		gap3 = 0x54;
		size = 2;
		break;
	case 9:
		spt = 18;
		gap3 = 0x54;
		size = 2;
		break;
	case 17:
		spt = 26;
		gap3 = 0x33;
		size = 1;
		break;
	}
	bps = 128 << size;

	for(int track=0; track < track_total; track++)
		for(int head=0; head < 2; head++) {
			desc_pc_sector sects[30];
			UINT8 sect_data[10000];
			int sdatapos = 0;
			for(int i=0; i<spt; i++) {
				sects[i].track       = track;
				sects[i].head        = head;
				if(h == 1)  // handle 2HS sector layout
				{
					if(i == 0 && track == 0)
						sects[i].sector      = i+1;
					else
						sects[i].sector      = i+10;
				}
				else
					sects[i].sector      = i+1;
				sects[i].size        = size;
				sects[i].actual_size = bps;
				sects[i].deleted     = false;
				sects[i].bad_crc     = false;
				sects[i].data        = &sect_data[sdatapos];
				io_generic_read(io, sects[i].data, offset, bps);
				offset += bps;
				sdatapos += bps;
			}

			build_pc_track_mfm(track, head, image, cell_count, spt, sects, gap3);
		}

	return true;
}


bool dim_format::save(io_generic *io, floppy_image *image)
{
	return false;
}

bool dim_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_DIM_FORMAT = &floppy_image_format_creator<dim_format>;
