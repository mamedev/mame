// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Hector Minidisc

    Disk image format

***************************************************************************/

#pragma once

#ifndef __HMD_DSK_H__
#define __HMD_DSK_H__

#include "upd765_dsk.h"

class hmd_format : public upd765_format
{
public:
	hmd_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_HMD_FORMAT;

#endif // __HMD_DSK_H__
