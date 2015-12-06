// license:BSD-3-Clause
// copyright-holders:Robbbert
/*********************************************************************

    formats/camplynx_dsk.h

    Camputers Lynx disk image format

*********************************************************************/

#ifndef CAMPLYNX_DSK_H_
#define CAMPLYNX_DSK_H_

#include "wd177x_dsk.h"

class camplynx_format : public wd177x_format {
public:
	camplynx_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_CAMPLYNX_FORMAT;

#endif
