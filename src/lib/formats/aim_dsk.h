// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/aim_dsk.h

    AIM disk images

*********************************************************************/
#ifndef MAME_FORMATS_AIM_DSK_H
#define MAME_FORMATS_AIM_DSK_H

#pragma once

#include "flopimg.h"

/**************************************************************************/

class aim_format : public floppy_image_format_t
{
public:
	aim_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override { return false; }
};

extern const aim_format FLOPPY_AIM_FORMAT;

#endif // MAME_FORMATS_AIM_DSK_H
