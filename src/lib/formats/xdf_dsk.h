// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/xdf_dsk.h

    x68k bare-bones formats

*********************************************************************/

#ifndef XDF_DSK_H_
#define XDF_DSK_H_

#include "upd765_dsk.h"

class xdf_format : public upd765_format {
public:
	xdf_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_XDF_FORMAT;

#endif
