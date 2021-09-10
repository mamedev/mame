// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*********************************************************************

    formats/esq16_dsk.c

    Formats for 16-bit Ensoniq synthesizers and samplers

    Disk is PC MFM, 80 tracks, double-sided, with 10 sectors per track

*********************************************************************/

#include "esq16_dsk.h"

#include "ioprocs.h"


const floppy_image_format_t::desc_e esqimg_format::esq_10_desc[] = {
	{ MFM, 0x4e, 80 },
	{ MFM, 0x00, 12 },
	{ RAW, 0x5224, 3 },
	{ MFM, 0xfc, 1 },
	{ MFM, 0x4e, 50 },
	{ MFM, 0x00, 12 },
	{ SECTOR_LOOP_START, 0, 9 },
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

esqimg_format::esqimg_format()
{
}

const char *esqimg_format::name() const
{
	return "esq16";
}

const char *esqimg_format::description() const
{
	return "Ensoniq VFX-SD/SD-1/EPS-16 floppy disk image";
}

const char *esqimg_format::extensions() const
{
	return "img";
}

bool esqimg_format::supports_save() const
{
	return true;
}

void esqimg_format::find_size(util::random_read &io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count)
{
	uint64_t size;
	if(!io.length(size)) {
		track_count = 80;
		head_count = 2;
		sector_count = 10;

		uint32_t expected_size = 512 * track_count*head_count*sector_count;
		if(size == expected_size) {
			return;
		}
	}
	track_count = head_count = sector_count = 0;
}

int esqimg_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	uint8_t track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	if(track_count)
		return 50;
	return 0;
}

bool esqimg_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	uint8_t track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	uint8_t sectdata[10*512];
	desc_s sectors[10];
	for(int i=0; i<sector_count; i++) {
		sectors[i].data = sectdata + 512*i;
		sectors[i].size = 512;
		sectors[i].sector_id = i;
	}

	int track_size = sector_count*512;
	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			size_t actual;
			io.read_at((track*head_count + head)*track_size, sectdata, track_size, actual);
			generate_track(esq_10_desc, track, head, sectors, sector_count, 110528, image);
		}
	}

	image->set_variant(floppy_image::DSDD);

	return true;
}

bool esqimg_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image)
{
	int track_count, head_count, sector_count;
	get_geometry_mfm_pc(image, 2000, track_count, head_count, sector_count);

	if(track_count != 80)
		track_count = 80;

	// Happens for a fully unformatted floppy
	if(!head_count)
		head_count = 2;

	if(sector_count != 10)
		sector_count = 10;

	uint8_t sectdata[11*512];
	int track_size = sector_count*512;

	for(int track=0; track < track_count; track++) {
		for(int head=0; head < head_count; head++) {
			get_track_data_mfm_pc(track, head, image, 2000, 512, sector_count, sectdata);
			size_t actual;
			io.write_at((track*head_count + head)*track_size, sectdata, track_size, actual);
		}
	}

	return true;
}

const floppy_format_type FLOPPY_ESQIMG_FORMAT = &floppy_image_format_creator<esqimg_format>;
