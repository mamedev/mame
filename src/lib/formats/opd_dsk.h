// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Sinclair ZX Spectrum

    Opus Discovery disk image formats

***************************************************************************/

#pragma once

#ifndef OPD_DSK_H
#define OPD_DSK_H

#include "flopimg.h"
#include "wd177x_dsk.h"


class opd_format : public wd177x_format
{
public:
	opd_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};


extern const floppy_format_type FLOPPY_OPD_FORMAT;

#endif // OPD_DSK_H
