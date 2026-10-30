// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/*********************************************************************

    formats/scl_dsk.h

    SCL disk images

*********************************************************************/
#ifndef MAME_FORMATS_SCL_DSK_H
#define MAME_FORMATS_SCL_DSK_H

#pragma once

#include "wd177x_dsk.h"

class scl_format : public wd177x_format {
public:
	scl_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];

	struct scl_entry {
		char name[8];
		uint8_t type;
		uint8_t start[2];
		uint8_t length[2];
		uint8_t sec_count;
	};

	static constexpr unsigned MAX_FILES = 128;
	static constexpr unsigned SECTOR_SIZE = 256;
	static constexpr unsigned SECTORS_PER_TRACK = 16;
	static constexpr unsigned MAX_TRACKS = 160; // 80 tracks * 2 sides
};

extern const scl_format FLOPPY_SCL_FORMAT;

#endif // MAME_FORMATS_SCL_DSK_H
