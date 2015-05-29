// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/smx_dsk.h

    Specialist MX disk images

*********************************************************************/

#ifndef SMX_DSK_H_
#define SMX_DSK_H_

#include "wd177x_dsk.h"

class smx_format : public wd177x_format {
public:
	smx_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_SMX_FORMAT;

#endif
