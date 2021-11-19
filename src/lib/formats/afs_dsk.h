// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Acorn FileStore

    Disk image formats

***************************************************************************/
#ifndef MAME_FORMATS_AFS_DSK_H
#define MAME_FORMATS_AFS_DSK_H

#pragma once

#include "wd177x_dsk.h"


class afs_format : public wd177x_format
{
public:
	afs_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};


extern const floppy_format_type FLOPPY_AFS_FORMAT;

#endif // MAME_FORMATS_AFS_DSK_H
