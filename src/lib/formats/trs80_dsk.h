// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    TRS-80

    JV1 disk image format

***************************************************************************/

#pragma once

#ifndef __TRS80_DSK_H__
#define __TRS80_DSK_H__

#include "wd177x_dsk.h"

class trs80_format : public wd177x_format
{
public:
	trs80_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

protected:
	virtual int get_track_dam_fm(const format &f, int head, int track);

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_TRS80_FORMAT;

#endif // __TRS80_DSK_H__
