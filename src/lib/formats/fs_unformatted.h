// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Creation of unformatted floppy images

#ifndef MAME_FORMATS_FS_UNFORMATTED_H
#define MAME_FORMATS_FS_UNFORMATTED_H

#pragma once

#include "fsmgr.h"

class fs_unformatted : public filesystem_manager_t {
public:
	fs_unformatted() : filesystem_manager_t() {}

	virtual void enumerate(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual void floppy_instantiate_raw(u32 key, floppy_image *image) const override;

private:
	enum {
		FSI_NONE,

		FSI_8_SSSD,
		FSI_8_DSSD,
		FSI_8_DSDD,

		FSI_525_SSSD,
		FSI_525_SSDD,
		FSI_525_SSQD,
		FSI_525_DSSD,
		FSI_525_DSDD,
		FSI_525_DSQD,
		FSI_525_DSHD,

		FSI_35_SSDD,
		FSI_35_DSDD,
		FSI_35_DSHD,
		FSI_35_DSED,

		FSI_3_DSDD,
		FSI_3_SSDD,
	};
};

extern const filesystem_manager_type FS_UNFORMATTED;

#endif
