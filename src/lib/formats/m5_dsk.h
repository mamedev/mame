// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/m5_dsk.h

    sord m5 format

*********************************************************************/

#ifndef M5_DSK_H_
#define M5_DSK_H_

#include "upd765_dsk.h"

class m5_format : public upd765_format {
public:
	m5_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_M5_FORMAT;

#endif
