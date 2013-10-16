// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/ep64_dsk.h

    Enterprise Sixty Four disk image format

*********************************************************************/

#ifndef EP64_DSK_H_
#define EP64_DSK_H_

#include "wd177x_dsk.h"

class ep64_format : public wd177x_format {
public:
	ep64_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_EP64_FORMAT;

#endif
