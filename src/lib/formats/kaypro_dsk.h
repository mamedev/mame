// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Robbbert
/*********************************************************************

    formats/kaypro_dsk.h

    Kaypro disk image format

*********************************************************************/

#ifndef KAYPRO_DSK_H_
#define KAYPRO_DSK_H_

#include "upd765_dsk.h"

class kayproii_format : public upd765_format {
public:
	kayproii_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class kaypro2x_format : public upd765_format {
public:
	kaypro2x_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_KAYPROII_FORMAT;
extern const floppy_format_type FLOPPY_KAYPRO2X_FORMAT;

#endif
