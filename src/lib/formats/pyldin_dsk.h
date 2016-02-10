// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/pyldin_dsk.h

    pyldin format

*********************************************************************/

#ifndef PYLDIN_DSK_H_
#define PYLDIN_DSK_H_

#include "upd765_dsk.h"

class pyldin_format : public upd765_format {
public:
	pyldin_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_PYLDIN_FORMAT;

#endif
