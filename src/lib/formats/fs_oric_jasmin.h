// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Management of Oric Jasmin floppy images

#ifndef MAME_FORMATS_FS_ORIC_JASMIN_H
#define MAME_FORMATS_FS_ORIC_JASMIN_H

#pragma once

#include "fsmgr.h"

namespace fs {

class oric_jasmin_image : public manager_t {
public:
	oric_jasmin_image() : manager_t() {}

	virtual const char *name() const override;
	virtual const char *description() const override;

	virtual void enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const override;
	virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const override;

	virtual bool can_format() const override;
	virtual bool can_read() const override;
	virtual bool can_write() const override;
	virtual bool has_rsrc() const override;

	virtual std::vector<meta_description> volume_meta_description() const override;
	virtual std::vector<meta_description> file_meta_description() const override;

	static bool validate_filename(std::string name);
};

extern const oric_jasmin_image ORIC_JASMIN;

} // namespace fs

#endif
