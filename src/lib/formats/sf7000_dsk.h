// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/sf7000_dsk.h

    sf7000 format

*********************************************************************/

#ifndef SF7000_DSK_H_
#define SF7000_DSK_H_

#include "upd765_dsk.h"

class sf7000_format : public upd765_format {
public:
	sf7000_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_SF7000_FORMAT;

#endif
