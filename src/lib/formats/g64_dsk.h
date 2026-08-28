// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/g64_dsk.h

    Commodore 1541/1571 GCR disk image format

*********************************************************************/
#ifndef MAME_FORMATS_G64_DSK_H
#define MAME_FORMATS_G64_DSK_H

#pragma once

#include "flopimg.h"

class g64_format : public floppy_image_format_t
{
public:
	g64_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
	virtual bool supports_save() const noexcept override { return true; }

protected:
	enum
	{
		POS_SIGNATURE = 0x0,
		POS_VERSION = 0x8,
		POS_TRACK_COUNT = 0x9,
		POS_MAX_TRACK_SIZE = 0xa,
		POS_TRACK_OFFSET = 0xc
	};

	enum
	{
		TRACK_LENGTH = 0x1ef8,
		TRACK_COUNT = 84
	};

	static const uint32_t c1541_cell_size[];

	static int speed_zone(int cylinder);
	static uint32_t raw_track_size(int speed_zone);
	static void generate_empty_track(int cylinder, int head, uint8_t fill, floppy_image &image);
	static bool is_empty_track(const std::vector<bool> &trackbuf);
	static int generate_bitstream(int track, int head, int speed_zone, std::vector<bool> &trackbuf, const floppy_image &image);

	// encodes one track's GCR bitstream for save(); returns -1 on a genuine
	// zone-detection error, 0 if the track has no data to write (skip), 1
	// if packed/zone were filled in. Shared by save()'s size-probe pass and
	// its write pass so both agree exactly on what a given track encodes to.
	static int encode_track(int track, int head, const floppy_image &image, std::vector<uint8_t> &packed, int &zone);
};

extern const g64_format FLOPPY_G64_FORMAT;

#endif // MAME_FORMATS_G64_DSK_H
