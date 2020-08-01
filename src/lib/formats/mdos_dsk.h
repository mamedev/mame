// license:BSD-3-Clause
// copyright-holders:68bit
/*
 * mdos_dsk.h  -  Motorola MDOS compatible disk images
 */
#ifndef MAME_FORMATS_MDOS_DSK_H
#define MAME_FORMATS_MDOS_DSK_H

#pragma once

#include "flopimg.h"
#include "wd177x_dsk.h"

LEGACY_FLOPPY_OPTIONS_EXTERN(mdos);

class mdos_format : public wd177x_format
{
public:
	mdos_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual int find_size(io_generic *io, uint32_t form_factor) override;
	virtual const wd177x_format::format &get_track_format(const format &f, int head, int track) override;

private:
	static const format formats[];
	static const format formats_head1[];
};

extern const floppy_format_type FLOPPY_MDOS_FORMAT;

#endif // MAME_FORMATS_MDOS_DSK_H
