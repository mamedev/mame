// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d81_dsk.h

    Commodore 1581 disk image format

*********************************************************************/

#ifndef D81_DSK_H_
#define D81_DSK_H_

#include "wd177x_dsk.h"

class d81_format : public wd177x_format {
public:
	d81_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

	virtual floppy_image_format_t::desc_e* get_desc_mfm(const format &f, int &current_size, int &end_gap_index);

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_D81_FORMAT;

#endif
