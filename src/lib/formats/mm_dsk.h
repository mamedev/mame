// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/mm_dsk.h

    MikroMikko formats

*********************************************************************/

#ifndef MM_DSK_H_
#define MM_DSK_H_

#include "upd765_dsk.h"

class mm1_format : public upd765_format {
public:
	mm1_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

class mm2_format : public upd765_format {
public:
	mm2_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_MM1_FORMAT;
extern const floppy_format_type FLOPPY_MM2_FORMAT;

#endif
