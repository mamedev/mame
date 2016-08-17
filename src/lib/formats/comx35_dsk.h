// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/comx35_dsk.h

    COMX-35 disk image format

*********************************************************************/

#ifndef COMX35_DSK_H_
#define COMX35_DSK_H_

#include "wd177x_dsk.h"

class comx35_format : public wd177x_format {
public:
	comx35_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_COMX35_FORMAT;

#endif
