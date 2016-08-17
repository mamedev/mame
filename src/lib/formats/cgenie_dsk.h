// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie

    Disk image format

***************************************************************************/

#pragma once

#ifndef __CGENIE_DSK_H__
#define __CGENIE_DSK_H__

#include "wd177x_dsk.h"

class cgenie_format : public wd177x_format
{
public:
	cgenie_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

protected:
	virtual int get_track_dam_fm(const format &f, int head, int track) override;
	virtual int get_track_dam_mfm(const format &f, int head, int track) override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_CGENIE_FORMAT;

#endif // __CGENIE_DSK_H__
