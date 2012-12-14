/*********************************************************************

    formats/d81_dsk.h

    Commodore 1581 disk image format

*********************************************************************/

#ifndef D81_DSK_H_
#define D81_DSK_H_

#include "wd177x_dsk.h"

class d81_format : public wd177x_format {
public:
	d81_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_D81_FORMAT;

#endif
