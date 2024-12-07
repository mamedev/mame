// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Carl
/*********************************************************************

    formats/86f_dsk.h

    86f disk images

*********************************************************************/
#ifndef MAME_FORMATS_86F_DSK_H
#define MAME_FORMATS_86F_DSK_H

#pragma once

#include "flopimg.h"

class _86f_format : public floppy_image_format_t
{
public:
	_86f_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	//virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
	virtual bool supports_save() const noexcept override;

private:
	void generate_track_from_bitstream_with_weak(int track, int head, const uint8_t *trackbuf, const uint8_t *weak, int index_cell, int track_size, floppy_image &image) const;
};

extern const _86f_format FLOPPY_86F_FORMAT;

#endif // MAME_FORMATS_86F_DSK_H
