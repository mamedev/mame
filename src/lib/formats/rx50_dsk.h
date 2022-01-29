// license:GPL-2.0+
// copyright-holders:Karl-Ludwig Deisenhofer
/*********************************************************************

    formats/rx50_dsk.h

    Format for DEC RX50 floppy drive used e.g. by Rainbow 100 and 190

    Disk is PC MFM, 80 tracks, single sided, with 10 sectors per track

based on lib/formats/esq16_dsk.h

*********************************************************************/
#ifndef MAME_FORMATS_RX50_DSK_H
#define MAME_FORMATS_RX50_DSK_H

#pragma once

#include "flopimg.h"

class rx50img_format : public floppy_image_format_t
{
public:
	rx50img_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

	static const desc_e rx50_10_desc[];

private:
	void find_size(util::random_read &io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count);
};

extern const floppy_format_type FLOPPY_RX50IMG_FORMAT;

#endif // MAME_FORMATS_RX50_DSK_H
