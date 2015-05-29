// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************

    formats/coupedsk.c

    SAM Coupe disk image formats

**************************************************************************/

#include <assert.h>

#include "formats/coupedsk.h"
#include "flopimg.h"

const floppy_image_format_t::desc_e mgt_format::desc_10[] = {
	{ MFM, 0x4e, 60 },
	{ SECTOR_LOOP_START, 0, 9 },
	{   MFM, 0x00, 12 },
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
	{   MFM, 0x4e, 24 },
	{ SECTOR_LOOP_END },
	{ MFM, 0x4e, 210 },
	{ END }
};

mgt_format::mgt_format()
{
}

const char *mgt_format::name() const
{
	return "mgt";
}

const char *mgt_format::description() const
{
	return "Sam Coupe MGT image format";
}

const char *mgt_format::extensions() const
{
	return "mgt,dsk";
}

bool mgt_format::supports_save() const
{
	return true;
}

int mgt_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT64 size = io_generic_size(io);

	if(/*size == 737280 || */ size == 819200)
		return 50;

	return 0;
}

bool mgt_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT64 size = io_generic_size(io);
	int sector_count = size == 737280 ? 9 : 10;

	UINT8 sectdata[10*512];
	desc_s sectors[10];
	for(int i=0; i<sector_count; i++) {
		sectors[i].data = sectdata + 512*i;
		sectors[i].size = 512;
		sectors[i].sector_id = i + 1;
	}

	int track_size = sector_count*512;
	for(int head=0; head < 2; head++) {
		for(int track=0; track < 80; track++) {
			io_generic_read(io, sectdata, (track*2+head)*track_size, track_size);
			generate_track(desc_10, track, head, sectors, sector_count+1, 100000, image);
		}
	}

	image->set_variant(floppy_image::DSDD);
	return true;
}

bool mgt_format::save(io_generic *io, floppy_image *image)
{
	int track_count, head_count, sector_count;
	get_geometry_mfm_pc(image, 2000, track_count, head_count, sector_count);

	if(sector_count > 10)
		sector_count = 10;
	else if(sector_count < 9)
		sector_count = 9;

	UINT8 sectdata[10*512];
	int track_size = sector_count*512;

	for(int head=0; head < 2; head++) {
		for(int track=0; track < 80; track++) {
			get_track_data_mfm_pc(track, head, image, 2000, 512, sector_count, sectdata);
			io_generic_write(io, sectdata, (head*80 + track)*track_size, track_size);
		}
	}

	return true;
}

const floppy_format_type FLOPPY_MGT_FORMAT = &floppy_image_format_creator<mgt_format>;
