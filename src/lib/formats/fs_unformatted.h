// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Creation of unformatted floppy images

#ifndef MAME_FORMATS_FS_UNFORMATTED_H
#define MAME_FORMATS_FS_UNFORMATTED_H

#pragma once

#include "fsmgr.h"

class floppy_image;

namespace fs {

class unformatted_image : public manager_t {
public:
	enum {
		FSI_NONE,

		FSI_8_SSSD,
		FSI_8_SS32,
		FSI_8_DSSD,
		FSI_8_DS32,
		FSI_8_DSDD,
		FSI_8_DD32,

		FSI_525_SSSD,
		FSI_525_SS10,
		FSI_525_SS16,
		FSI_525_SSDD,
		FSI_525_SD10,
		FSI_525_SD16,
		FSI_525_SSQD,
		FSI_525_SQ10,
		FSI_525_SQ16,
		FSI_525_DSSD,
		FSI_525_DS10,
		FSI_525_DS16,
		FSI_525_DSDD,
		FSI_525_DD10,
		FSI_525_DD16,
		FSI_525_DSQD,
		FSI_525_DQ10,
		FSI_525_DQ16,
		FSI_525_DSHD,

		FSI_35_SSDD,
		FSI_35_DSDD,
		FSI_35_DSHD,
		FSI_35_DSED,

		FSI_3_DSDD,
		FSI_3_SSDD,
	};

	unformatted_image() : manager_t() {}

	virtual const char *name() const override;
	virtual const char *description() const override;

	static void format(u32 key, floppy_image *image);

	virtual void enumerate_f(floppy_enumerator &fe) const override;
	virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const override;

	virtual bool can_format() const override;
	virtual bool can_read() const override;
	virtual bool can_write() const override;
	virtual bool has_rsrc() const override;
};

extern const unformatted_image UNFORMATTED;

} // namespace fs

#endif
