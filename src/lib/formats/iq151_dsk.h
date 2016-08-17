// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/iq151_dsk.h

    iq151 format

*********************************************************************/

#ifndef IQ151_DSK_H_
#define IQ151_DSK_H_

#include "upd765_dsk.h"

class iq151_format : public upd765_format {
public:
	iq151_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_IQ151_FORMAT;

#endif
