// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/c3040_dsk.h

    Commodore 3040 sector disk image format

*********************************************************************/

#ifndef C3040_DSK_H_
#define C3040_DSK_H_

#include "d64_dsk.h"

class c3040_format : public d64_format {
public:
	c3040_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

protected:
	virtual int get_sectors_per_track(const format &f, int track) override { return c3040_sectors_per_track[track]; }
	virtual floppy_image_format_t::desc_e* get_sector_desc(const format &f, int &current_size, int sector_count, UINT8 id1, UINT8 id2, int gap_2) override;
	virtual int get_gap2(const format &f, int head, int track) override { return c3040_gap2[track]; }
	virtual void fix_end_gap(floppy_image_format_t::desc_e* desc, int remaining_size) override;

	static const format file_formats[];

	static const int c3040_gap2[];
	static const int c3040_sectors_per_track[];
};

extern const floppy_format_type FLOPPY_C3040_FORMAT;



#endif
