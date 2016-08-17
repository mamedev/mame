// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    formats/pc_dsk.h

    PC disk images

*********************************************************************/

#ifndef PC_DSK_H
#define PC_DSK_H

#include "flopimg.h"
#include "upd765_dsk.h"

/**************************************************************************/

LEGACY_FLOPPY_OPTIONS_EXTERN(pc);


class pc_format : public upd765_format
{
public:
	pc_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_PC_FORMAT;

#endif /* PC_DSK_H */
