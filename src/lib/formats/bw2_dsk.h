// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/bw2_dsk.h

    Bondwell 2 format

*********************************************************************/

#ifndef BW2_DSK_H_
#define BW2_DSK_H_

#include "upd765_dsk.h"

class bw2_format : public upd765_format {
public:
	bw2_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_BW2_FORMAT;

#endif
