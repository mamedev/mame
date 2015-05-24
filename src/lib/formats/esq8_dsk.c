// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*********************************************************************

    formats/esq8_dsk.c

    Formats for 8-bit Ensoniq synthesizers and samplers

    Disk is PC MFM, 40 tracks, single (Mirage) or double (SQ-80) sided,
    with 6 sectors per track.
    Sectors 0-4 are 1024 bytes, sector 5 is 512 bytes

*********************************************************************/

#include <assert.h>

#include "flopimg.h"
#include "formats/esq8_dsk.h"

const floppy_image_format_t::desc_e esq8img_format::esq_6_desc[] = {
	{ MFM, 0x4e, 80 },
	{ MFM, 0x00, 12 },
	{ RAW, 0x5224, 3 },
	{ MFM, 0xfc, 1 },
	{ MFM, 0x4e, 50 },
	{ MFM, 0x00, 12 },
	{ SECTOR_LOOP_START, 0, 5 },
	{   CRC_CCITT_START, 1 },
	{     RAW, 0x4489, 3 },
	{     MFM, 0xfe, 1 },
	{     TRACK_ID },
	{     HEAD_ID },
	{     SECTOR_ID },
	{     SIZE_ID },
	{   CRC_END, 1 },
	{   CRC, 1 },
	{   MFM, 0x4e, 22 },
	{   MFM, 0x00, 12 },
	{   CRC_CCITT_START, 2 },
	{     RAW, 0x4489, 3 },
	{     MFM, 0xfb, 1 },
	{     SECTOR_DATA, -1 },
	{   CRC_END, 2 },
	{   CRC, 2 },
	{   MFM, 0x4e, 84 },
	{   MFM, 0x00, 12 },
	{ SECTOR_LOOP_END },
	{ MFM, 0x4e, 170 },
	{ END }
};

esq8img_format::esq8img_format()
{
}

const char *esq8img_format::name() const
{
	return "esq8";
}

const char *esq8img_format::description() const
{
	return "Ensoniq Mirage/SQ-80 floppy disk image";
}

const char *esq8img_format::extensions() const
{
	return "img";
}

bool esq8img_format::supports_save() const
{
	return true;
}

void esq8img_format::find_size(io_generic *io, int &track_count, int &head_count, int &sector_count)
{
	UINT64 size = io_generic_size(io);
	track_count = 80;
	head_count = 1;
	sector_count = 6;

	if (size == 5632 * 80)
	{
		return;
	}

	track_count = head_count = sector_count = 0;
}

int esq8img_format::identify(io_generic *io, UINT32 form_factor)
{
	int track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	if(track_count)
		return 50;
	return 0;
}

bool esq8img_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	UINT8 sectdata[(5*1024) + 512];
	desc_s sectors[6];
	for(int i=0; i<sector_count; i++) {
		if (i < 5)
		{
			sectors[i].data = sectdata + (1024*i);  // 5 1024 byte sectors
			sectors[i].size = 1024;
			sectors[i].sector_id = i;
		}
		else
		{
			sectors[i].data = sectdata + (5*1024);  // 1 512 byte sector
			sectors[i].size = 512;
			sectors[i].sector_id = i;
		}
	}

	int track_size = (5*1024) + 512;

	for(int track=0; track < track_count; track++)
	{
		for(int head=0; head < head_count; head++)
		{
			io_generic_read(io, sectdata, (track*head_count + head)*track_size, track_size);
			generate_track(esq_6_desc, track, head, sectors, sector_count, 109376, image);
		}
	}

	image->set_variant(floppy_image::DSDD);

	return true;
}

bool esq8img_format::save(io_generic *io, floppy_image *image)
{
	int track_count, head_count, sector_count;
	get_geometry_mfm_pc(image, 2000, track_count, head_count, sector_count);

	if(track_count != 80)
		track_count = 80;

	// Happens for a fully unformatted floppy
	if(!head_count)
		head_count = 1;

	if(sector_count != 6)
		sector_count = 6;

	UINT8 sectdata[10*512];
	int track_size = (5*1024) + 512;

	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			get_track_data_mfm_pc(track, head, image, 2000, 512, sector_count, sectdata);
			io_generic_write(io, sectdata, (track*head_count + head)*track_size, track_size);
		}
	}

	return true;
}

const floppy_format_type FLOPPY_ESQ8IMG_FORMAT = &floppy_image_format_creator<esq8img_format>;
