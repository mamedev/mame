// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d82_dsk.h

    Commodore 8250/SFD-1001 sector disk image format

*********************************************************************/

#ifndef D82_DSK_H_
#define D82_DSK_H_

#include "d80_dsk.h"

class d82_format : public d80_format {
public:
	d82_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

protected:
	static const format file_formats[];
};

extern const floppy_format_type FLOPPY_D82_FORMAT;



#endif
