// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    formats/ds9_dsk.cpp

    Floppies used by Agat-9 840KB controller

    http://agatcomp.ru/Reading/docs/es5323.txt

    https://github.com/sintech/AGAT/blob/master/docs/agat-840k-format.txt

    http://www.torlus.com/floppy/forum/viewtopic.php?f=19&t=1385

************************************************************************/

#include "formats/ds9_dsk.h"

#include "ioprocs.h"

#include <cstring>


// exactly 6500 bytes
const floppy_image_format_t::desc_e ds9_format::ds9_desc[] = {
	/* 01 */ { MFM, 0xaa, 32 },             // GAP1
	/* 02 */ { SECTOR_LOOP_START, 0, 20 },  // 21 sectors
	/* 03 */ {   RAWBITS, 0x8924, 16 },     // sync mark: xA4, 2 us zero level interval, 0xFF
	/* 04 */ {   RAWBITS, 0x5555, 16 },
	/* 05 */ {   MFM, 0x95, 1 },            // address field prologue
	/* 06 */ {   MFM, 0x6a, 1 },
	/* 07 */ {   MFM, 0xfe, 1 },            // volume number
	/* 08 */ {   OFFSET_ID },
	/* 09 */ {   SECTOR_ID },
	/* 10 */ {   MFM, 0x5a, 1 },            // address field epilogue
	/* 11 */ {   MFM, 0xaa, 5 },            // GAP2 (min 4 bytes)
	/* 12 */ {   RAWBITS, 0x8924, 16 },     // sync mark
	/* 13 */ {   RAWBITS, 0x5555, 16 },
	/* 14 */ {   MFM, 0x6a, 1 },            // data field prologue
	/* 15 */ {   MFM, 0x95, 1 },
	/* 16 */ {   SECTOR_DATA_DS9, -1 },
	/* 17 */ {   MFM, 0x5a, 1 },            // data field epilogue
	/* 18 */ {   MFM, 0xaa, 33 },           // GAP3
	/* 19 */ { SECTOR_LOOP_END },
	/* 20 */ { END }
};

ds9_format::ds9_format()
{
}

const char *ds9_format::name() const noexcept
{
	return "a9dsk";
}

const char *ds9_format::description() const noexcept
{
	return "Agat-9 840K floppy image";
}

const char *ds9_format::extensions() const noexcept
{
	return "ds9";
}

void ds9_format::find_size(util::random_read &io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count)
{
	head_count = 2;
	track_count = 80;
	sector_count = 21;
	uint32_t const expected_size = 256 * track_count * head_count * sector_count;

	uint64_t size;
	if (!io.length(size) && (size >= expected_size)) // standard format has 860160 bytes
		return;

	track_count = head_count = sector_count = 0;
}

int ds9_format::identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const
{
	uint8_t track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);

	if (track_count)
		return FIFID_SIZE;

	return 0;
}

bool ds9_format::load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const
{
	uint8_t track_count, head_count, sector_count;
	find_size(io, track_count, head_count, sector_count);
	if (track_count == 0) return false;

	uint8_t sectdata[21 * 256];
	desc_s sectors[21];
	for (int i = 0; i < sector_count; i++)
	{
		sectors[i].data = sectdata + 256 * i;
		sectors[i].size = 256;
		sectors[i].sector_id = i;
	}

	int track_size = sector_count * 256;
	for (int track = 0; track < track_count; track++)
	{
		for (int head = 0; head < head_count; head++)
		{
			/*auto const [err, actual] =*/ read_at(io, (track * head_count + head) * track_size, sectdata, track_size); // FIXME: check for errors and premature EOF
			generate_track(ds9_desc, track, head, sectors, sector_count, 104000, image);
		}
	}

	image.set_variant(floppy_image::DSQD);

	return true;
}

const ds9_format FLOPPY_DS9_FORMAT;
