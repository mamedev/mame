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

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

	static const desc_e esq_10_desc[];

private:
	static void find_size(util::random_read &io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count);
};

extern const esqimg_format FLOPPY_ESQIMG_FORMAT;

#endif // MAME_FORMATS_ESQ16_DSK_H
