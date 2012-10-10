/*********************************************************************

    formats/sf7000_dsk.h

    sf7000 format

*********************************************************************/

#ifndef SF7000_DSK_H_
#define SF7000_DSK_H_

#include "upd765_dsk.h"

class sf7000_format : public upd765_format {
public:
	sf7000_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_SF7000_FORMAT;

#endif
