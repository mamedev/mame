// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/adam_dsk.h

    Coleco Adam disk image format

*********************************************************************/

#ifndef ADAM_DSK_H_
#define ADAM_DSK_H_

#include "wd177x_dsk.h"

class adam_format : public wd177x_format {
public:
	adam_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_ADAM_FORMAT;

#endif
