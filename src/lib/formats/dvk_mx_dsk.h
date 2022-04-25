// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/dvk_mx_dsk.h

*********************************************************************/
#ifndef MAME_FORMATS_DVK_MX_DSK_H
#define MAME_FORMATS_DVK_MX_DSK_H

#pragma once

#include "flopimg.h"
#include "imageutl.h"

class dvk_mx_format : public floppy_image_format_t
{
public:
	dvk_mx_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

	static const desc_e dvk_mx_old_desc[];
	static const desc_e dvk_mx_new_desc[];

private:
	static void find_size(util::random_read &io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count);
};

extern const dvk_mx_format FLOPPY_DVK_MX_FORMAT;

#endif // MAME_FORMATS_DVK_MX_DSK_H
