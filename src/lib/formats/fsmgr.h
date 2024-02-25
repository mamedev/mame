// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem management code for floppy, hd and cdrom images

#ifndef MAME_FORMATS_FSMGR_H
#define MAME_FORMATS_FSMGR_H

#pragma once

#include "fsmeta.h"

#include <memory>
#include <vector>

class floppy_image_format_t;

namespace fs {

// declared in fsblk.h
class filesystem_t;
class fsblk_t;

using u32 = uint32_t;

class manager_t {
public:
	struct floppy_enumerator {
		floppy_enumerator(u32 form_factor, const std::vector<u32> &variants);
		virtual ~floppy_enumerator() = default;

		void add(const floppy_image_format_t &type, u32 form_factor, u32 variant, u32 image_size, const char *name, const char *description);
		virtual void add_raw(const char *name, u32 key, const char *description) = 0;

		u32 form_factor() const { return m_form_factor; }
		const std::vector<u32> &variants() const { return m_variants; }

	protected:
		virtual void add_format(const floppy_image_format_t &type, u32 image_size, const char *name, const char *description) = 0;

	private:
		u32                         m_form_factor;
		const std::vector<u32> &    m_variants;
	};

	struct hd_enumerator {
		virtual ~hd_enumerator() = default;

		virtual void add(const manager_t *manager, const char *name, const char *description) = 0;
	};

	struct cdrom_enumerator {
		virtual ~cdrom_enumerator() = default;

		virtual void add(const manager_t *manager, const char *name, const char *description) = 0;
	};

	virtual ~manager_t() = default;

	virtual const char *name() const = 0;
	virtual const char *description() const = 0;

	virtual void enumerate_f(floppy_enumerator &fe) const;
	virtual void enumerate_h(hd_enumerator &he) const;
	virtual void enumerate_c(cdrom_enumerator &ce) const;

	virtual bool can_format() const = 0;
	virtual bool can_read() const = 0;
	virtual bool can_write() const = 0;
	virtual bool has_rsrc() const = 0;
	virtual char directory_separator() const;

	bool has_subdirectories() const { return directory_separator() != 0; }

	virtual std::vector<meta_description> volume_meta_description() const;
	virtual std::vector<meta_description> file_meta_description() const;
	virtual std::vector<meta_description> directory_meta_description() const;

	// Create a filesystem object from a block device
	virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const = 0;

	static bool has(u32 form_factor, const std::vector<u32> &variants, u32 ff, u32 variant);
	static bool has_variant(const std::vector<u32> &variants, u32 variant);

protected:
	manager_t() = default;
};

} // namespace fs

#endif
