// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/abc800_dsk.h

    Luxor ABC 830/832/834/838 disk image formats

*********************************************************************/

#ifndef ABC800_DSK_H_
#define ABC800_DSK_H_

#include "wd177x_dsk.h"

class abc800_format : public wd177x_format {
public:
	abc800_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_ABC800_FORMAT;

#endif
