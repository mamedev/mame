#ifndef __ATOM_DSK_H__
#define __ATOM_DSK_H__

#include "flopimg.h"
#include "wd177x_dsk.h"

class atom_format : public wd177x_format
{
public:
	atom_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_ATOM_FORMAT;

#endif
