// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/cpis_dsk.h

    Telenova Compis disk images

*********************************************************************/

#ifndef CPIS_DSK_H
#define CPIS_DSK_H

#include "upd765_dsk.h"

class cpis_format : public upd765_format {
public:
	cpis_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_CPIS_FORMAT;

#endif /* CPIS_DSK_H */
