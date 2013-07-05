/*********************************************************************

    formats/mbee_dsk.h

    Microbee disk image format

*********************************************************************/

#ifndef MBEE_DSK_H_
#define MBEE_DSK_H_

#include "wd177x_dsk.h"

class mbee_format : public wd177x_format {
public:
	mbee_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_MBEE_FORMAT;

#endif
