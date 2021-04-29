// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Creation of Oric Jasmin floppy images

#ifndef MAME_FORMATS_FS_ORIC_JASMIN_H
#define MAME_FORMATS_FS_ORIC_JASMIN_H

#pragma once

#include "fsmgr.h"

class fs_oric_jasmin : public filesystem_manager_t {
public:
	class impl : public filesystem_t {
	public:
		impl(fsblk_t &blockdev) : filesystem_t(blockdev, 256) {}
		virtual ~impl() = default;
		
		virtual void format(const fs_meta_data &meta) override;
		virtual fs_meta_data metadata() override;
	};

	fs_oric_jasmin() : filesystem_manager_t() {}

	virtual void enumerate_f(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const override;

	virtual bool can_format() const override;
	virtual bool can_read() const override;
	virtual bool can_write() const override;
	virtual bool has_subdirectories() const override;

	virtual std::vector<fs_meta_description> volume_meta_description() const override;
	virtual std::vector<fs_meta_description> file_meta_description() const override;

	static bool validate_filename(std::string name);
};

extern const filesystem_manager_type FS_ORIC_JASMIN;

#endif
