// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/cqm_dsk.h

    CopyQM disk images

*********************************************************************/
#ifndef MAME_FORMATS_CQM_DSK_H
#define MAME_FORMATS_CQM_DSK_H

#pragma once

#include "flopimg.h"

class cqm_format : public floppy_image_format_t
{
public:
	cqm_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
};

extern const cqm_format FLOPPY_CQM_FORMAT;

#endif // MAME_FORMATS_CQM_DSK_H
