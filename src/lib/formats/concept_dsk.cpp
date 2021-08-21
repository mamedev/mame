// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/*********************************************************************

    formats/concept_dsk.c

    Formats for Corvus Concept

    5.25" DSDD is PC MFM, 77 tracks, double-sided, with 9 sectors per track

*********************************************************************/

#include "formats/concept_dsk.h"

#include "ioprocs.h"


/* 9 sectors / track, 512 bytes per sector */
const floppy_image_format_t::desc_e cc525dsdd_format::cc_9_desc[] = {
	{ MFM, 0x4e, 80 },
	{ MFM, 0x00, 12 },
	{ RAW, 0x5224, 3 },
	{ MFM, 0xfc, 1 },
	{ MFM, 0x4e, 50 },
	{ MFM, 0x00, 12 },
	{ SECTOR_LOOP_START, 1, 9 },
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

cc525dsdd_format::cc525dsdd_format()
{
}

const char *cc525dsdd_format::name() const
{
	return "concept";
}

const char *cc525dsdd_format::description() const
{
	return "Corvus Concept 5.25\" DSDD floppy disk image";
}

const char *cc525dsdd_format::extensions() const
{
	return "img";
}

bool cc525dsdd_format::supports_save() const
{
	return true;
}

void cc525dsdd_format::find_size(util::random_read &io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count)
{
	track_count = 77;
	head_count = 2;
	sector_count = 9;

	uint32_t const expected_size = 512 * track_count*head_count*sector_count;

	uint64_t size;
	if (!io.length(size) && (size == expected_size))
		return;

	track_count = head_count = sector_count = 0;
}

int cc525dsdd_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants)
{
	uint8_t track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	if (track_count)
		return 50;

	return 0;
}

bool cc525dsdd_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image)
{
	uint8_t track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	uint8_t sectdata[10*512];
	desc_s sectors[10];
	for (int i=0; i<sector_count; i++) {
		sectors[i+1].data = sectdata + 512*i;
		sectors[i+1].size = 512;
		sectors[i+1].sector_id = i+1;
	}

	int track_size = sector_count*512;
	for (int track=0; track < track_count; track++) {
		for (int head=0; head < head_count; head++) {
			size_t actual;
			io.read_at((track*head_count + head) * track_size, sectdata, track_size, actual);
			generate_track(cc_9_desc, track, head, sectors, sector_count, 100000, image);
		}
	}

	image->set_variant(floppy_image::DSDD);

	return true;
}

bool cc525dsdd_format::save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image)
{
	int track_count, head_count, sector_count;
	get_geometry_mfm_pc(image, 2000, track_count, head_count, sector_count);

	if (track_count != 77)
		track_count = 77;

	// Happens for a fully unformatted floppy
	if (!head_count)
		head_count = 2;

	if (sector_count != 9)
		sector_count = 9;

	uint8_t sectdata[9*512];
	int track_size = sector_count*512;

	for (int track=0; track < track_count; track++) {
		for (int head=0; head < head_count; head++) {
			get_track_data_mfm_pc(track, head, image, 2000, 512, sector_count, sectdata);
			size_t actual;
			io.write_at((track*head_count + head)*track_size, sectdata, track_size, actual);
		}
	}

	return true;
}

const floppy_format_type FLOPPY_CONCEPT_525DSDD_FORMAT = &floppy_image_format_creator<cc525dsdd_format>;
