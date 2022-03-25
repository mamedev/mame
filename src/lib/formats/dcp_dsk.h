// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    formats/dcp_dsk.h

    PC98 DCP & DCU disk images

*********************************************************************/
#ifndef MAME_FORMATS_DCP_DSK_H
#define MAME_FORMATS_DCP_DSK_H

#pragma once

#include "flopimg.h"


class dcp_format : public floppy_image_format_t
{
public:
	dcp_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const dcp_format FLOPPY_DCP_FORMAT;

#endif // MAME_FORMATS_DCP_DSK_H
