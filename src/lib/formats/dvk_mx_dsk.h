// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/dvk_mx_dsk.h

*********************************************************************/
#ifndef MAME_FORMATS_DVK_MX_DSK_H
#define MAME_FORMATS_DVK_MX_DSK_H

#pragma once

#include "flopimg.h"

class dvk_mx_format : public floppy_image_format_t
{
public:
	dvk_mx_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	virtual const char *name() const noexcept override { return "mx"; }
	virtual const char *description() const noexcept override { return "DVK MX disk image"; }
	virtual const char *extensions() const noexcept override { return "mx"; }
	virtual bool supports_save() const noexcept override { return true; }

	static const desc_e dvk_mx_old_desc[];
	static const desc_e dvk_mx_new_desc[];

private:
	static constexpr unsigned MX_SECTORS = 11;
	static constexpr unsigned MX_SECTOR_SIZE = 256;

	static void find_size(util::random_read &io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count);
	static bool get_next_sector(const std::vector<bool> &bitstream , uint8_t *sector_data);
};

extern const dvk_mx_format FLOPPY_DVK_MX_FORMAT;

#endif // MAME_FORMATS_DVK_MX_DSK_H
