// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*********************************************************************

    formats/esq16_dsk.h

    Formats for 16-bit Ensoniq synthesizers and samplers

    Disk is PC MFM, 80 tracks, double-sided, with 10 sectors per track

*********************************************************************/
#ifndef MAME_FORMATS_ESQ16_DSK_H
#define MAME_FORMATS_ESQ16_DSK_H

#pragma once

#include "flopimg.h"

class esqimg_format : public floppy_image_format_t
{
public:
	esqimg_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

	static const desc_e esq_10_desc[];

private:
	void find_size(io_generic *io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count);
};

extern const floppy_format_type FLOPPY_ESQIMG_FORMAT;

#endif // MAME_FORMATS_ESQ16_DSK_H
