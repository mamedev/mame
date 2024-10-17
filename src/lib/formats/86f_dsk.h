// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Carl
/*********************************************************************

    formats/86f_dsk.h

    86f disk images

*********************************************************************/
#ifndef MAME_FORMATS_86F_DSK_H
#define MAME_FORMATS_86F_DSK_H

#pragma once

#include "flopimg.h"

// workaround for conflict with glibc/bits/endian.h
#undef BIG_ENDIAN

class _86f_format : public floppy_image_format_t
{
public:
	_86f_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	//virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
	virtual bool supports_save() const noexcept override;

private:
	void generate_track_from_bitstream_with_weak(int track, int head, const uint8_t *trackbuf, const uint8_t *weak, int index_cell, int track_size, floppy_image &image) const;
	enum
	{
		SURFACE_DESC = 1,
		TYPE_MASK = 6,
		TYPE_DD = 0,
		TYPE_HD = 2,
		TYPE_ED = 4,
		TYPE_ED2000 = 6,
		TWO_SIDES = 8,
		WRITE_PROTECT = 0x10,
		RPM_MASK = 0x60,
		RPM_0 = 0,
		RPM_1 = 0x20,
		RPM_15 = 0x40,
		RPM_2 = 0x60,
		EXTRA_BC = 0x80,
		ZONED_RPM = 0x100,
		ZONE_PREA2_1 = 0,
		ZONE_PREA2_2 = 0x200,
		ZONE_A2 = 0x400,
		ZONE_C64 = 0x600,
		BIG_ENDIAN = 0x800,
		RPM_FAST = 0x1000,
		TOTAL_BC = 0x1000
	};
};

extern const _86f_format FLOPPY_86F_FORMAT;

#endif // MAME_FORMATS_86F_DSK_H
