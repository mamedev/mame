// license:BSD-3-Clause
// copyright-holders:Felipe Corrêa da Silva Sanches
/*********************************************************************

    formats/ds2717_dsk.h

    Consul 2717 DS 2717 disk images

*********************************************************************/
#ifndef MAME_FORMATS_DS2717_DSK_H
#define MAME_FORMATS_DS2717_DSK_H

#pragma once

#include "upd765_dsk.h"

class ds2717_format : public upd765_format
{
public:
	ds2717_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

private:
	static constexpr unsigned TRACK_SIZE = 26 * 128;

	static const format formats[];
};

extern const ds2717_format FLOPPY_DS2717_FORMAT;

#endif // MAME_FORMATS_DS2717_DSK_H
