// license:GPL-2.0+
// copyright-holders:Nigel Barnes
/***************************************************************************

    Acorn FileStore

    Disk image formats

***************************************************************************/

#pragma once

#ifndef __AFS_DSK_H__
#define __AFS_DSK_H__

#include "wd177x_dsk.h"


class afs_format : public wd177x_format
{
public:
	afs_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};


extern const floppy_format_type FLOPPY_AFS_FORMAT;

#endif // __AFS_DSK_H__
