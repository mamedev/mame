// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d71_dsk.h

    Commodore 1571 sector disk image format

*********************************************************************/

#ifndef D71_DSK_H_
#define D71_DSK_H_

#include "d64_dsk.h"

class d71_format : public d64_format {
public:
	d71_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

protected:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_D71_FORMAT;



#endif
