// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d80_dsk.h

    Commodore 8050 sector disk image format

*********************************************************************/
#ifndef MAME_FORMATS_D80_DSK_H
#define MAME_FORMATS_D80_DSK_H

#pragma once

#include "d64_dsk.h"

class d80_format : public d64_format
{
public:
	d80_format();
	d80_format(const format *formats);

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

protected:
	const format *formats;

	virtual int get_physical_track(const format &f, int head, int track) override;
	virtual uint32_t get_cell_size(const format &f, int track) override;
	virtual int get_sectors_per_track(const format &f, int track) override;
	virtual int get_disk_id_offset(const format &f) override;
	virtual floppy_image_format_t::desc_e* get_sector_desc(const format &f, int &current_size, int sector_count, uint8_t id1, uint8_t id2, int gap_2) override;
	virtual void fix_end_gap(floppy_image_format_t::desc_e* desc, int remaining_size) override;

	static const format file_formats[];

	static const uint32_t d80_cell_size[];
	static const int d80_speed_zone[];
	static const int d80_sectors_per_track[];
};

extern const floppy_format_type FLOPPY_D80_FORMAT;

#endif // MAME_FORMATS_D80_DSK_H
