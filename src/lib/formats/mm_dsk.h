// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/mm_dsk.h

    MikroMikko formats

*********************************************************************/
#ifndef MAME_FORMATS_MM_DSK_H
#define MAME_FORMATS_MM_DSK_H

#pragma once

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

extern const mm1_format FLOPPY_MM1_FORMAT;
extern const mm2_format FLOPPY_MM2_FORMAT;

#endif // MAME_FORMATS_MM_DSK_H
