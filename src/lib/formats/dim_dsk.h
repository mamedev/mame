// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/dim_dsk.h

    DIM disk images

*********************************************************************/
#ifndef MAME_FORMATS_DIM_DSK_H
#define MAME_FORMATS_DIM_DSK_H

#pragma once

#include "flopimg.h"

/**************************************************************************/

FLOPPY_IDENTIFY(dim_dsk_identify);
FLOPPY_CONSTRUCT(dim_dsk_construct);


class dim_format : public floppy_image_format_t
{
public:
	dim_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_DIM_FORMAT;

#endif // MAME_FORMATS_DIM_DSK_H
