// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/*********************************************************************

Heath h17disk  disk image format

The Heath hard-sectored disk format for the H8 and H89 systems with the
H17 controller on the H8 and the H-88-1 controller on the H89.

*********************************************************************/
#ifndef MAME_FORMATS_H17DISK_H
#define MAME_FORMATS_H17DISK_H

#pragma once

#include "flopimg.h"

class heath_h17d_format : public floppy_image_format_t
{
public:
	heath_h17d_format();

	int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

	const char *name() const noexcept override { return "h17disk"; }
	const char *description() const noexcept override { return "Heath H17D disk image"; }
	const char *extensions() const noexcept override { return "h17,h17d,h17disk"; }
	bool supports_save() const noexcept override { return false; }

protected:

	void fm_reverse_byte_w(std::vector<uint32_t> &buffer, uint8_t val) const;
};

extern const heath_h17d_format FLOPPY_H17D_FORMAT;

#endif // MAME_FORMATS_H17DISK_H
