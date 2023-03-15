// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_FORMATS_MFI_DSK_H
#define MAME_FORMATS_MFI_DSK_H

#pragma once

#include "flopimg.h"

class mfi_format : public floppy_image_format_t
{
public:
	mfi_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	enum {
		TIME_MASK = 0x0fffffff,
		MG_MASK   = 0xf0000000,
		MG_SHIFT  = 28,

		OLD_MG_A  = (0 << MG_SHIFT),
		OLD_MG_B  = (1 << MG_SHIFT),
		OLD_MG_N  = (2 << MG_SHIFT),
		OLD_MG_D  = (3 << MG_SHIFT),

		MG_F      = (0 << MG_SHIFT),    //!< - 0, MG_F -> Flux orientation change
		MG_N      = (1 << MG_SHIFT),    //!< - 1, MG_N -> Non-magnetized zone (neutral)
		MG_D      = (2 << MG_SHIFT),    //!< - 2, MG_D -> Damaged zone, reads as neutral but cannot be changed by writing
		MG_E      = (3 << MG_SHIFT),    //!< - 3, MG_E -> End of zone

		RESOLUTION_SHIFT = 30,
		CYLINDER_MASK = 0x3fffffff
	};

	static const char sign_old[16];
	static const char sign[16];

	struct header {
		char sign[16];
		unsigned int cyl_count, head_count;
		unsigned int form_factor, variant;
	};

	struct entry {
		unsigned int offset, compressed_size, uncompressed_size, write_splice;
	};
};

extern const mfi_format FLOPPY_MFI_FORMAT;

#endif // MAME_FORMATS_MFI_DSK_H
