// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/abcfd2_dsk.h

    Scandia Metric ABC FD2 disk image formats

*********************************************************************/

#ifndef ABC_FD2_DSK_H_
#define ABC_FD2_DSK_H_

#include "wd177x_dsk.h"

class abc_fd2_format : public wd177x_format {
public:
	abc_fd2_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_ABC_FD2_FORMAT;

#endif
