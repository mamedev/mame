/*********************************************************************

    formats/kaypro_dsk.h

    Kaypro disk image format

*********************************************************************/

#ifndef KAYPRO_DSK_H_
#define KAYPRO_DSK_H_

#include "wd177x_dsk.h"

class kayproii_format : public wd177x_format {
public:
	kayproii_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

class kaypro2x_format : public wd177x_format {
public:
	kaypro2x_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_KAYPROII_FORMAT;
extern const floppy_format_type FLOPPY_KAYPRO2X_FORMAT;

#endif
