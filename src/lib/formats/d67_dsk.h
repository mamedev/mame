// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d67_dsk.h

    Commodore 2040 sector disk image format

*********************************************************************/

#ifndef D67_DSK_H_
#define D67_DSK_H_

#include "d64_dsk.h"

class d67_format : public d64_format {
public:
	d67_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

protected:
	virtual int get_sectors_per_track(const format &f, int track);

	static const format file_formats[];

	static const int d67_sectors_per_track[];
};

extern const floppy_format_type FLOPPY_D67_FORMAT;



#endif
