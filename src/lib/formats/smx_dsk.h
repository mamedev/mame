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

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_SMX_FORMAT;

#endif
