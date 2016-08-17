// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*********************************************************************

    formats/pc98_dsk.h

    PC disk images

*********************************************************************/

#ifndef PC98_DSK_H
#define PC98_DSK_H

#include "flopimg.h"
#include "upd765_dsk.h"

/**************************************************************************/


class pc98_format : public upd765_format
{
public:
	pc98_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_PC98_FORMAT;

#endif /* PC_DSK_H */
