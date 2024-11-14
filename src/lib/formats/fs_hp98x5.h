// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    fs_hp98x5.h

    HP 9825, 9831 & 9845 filesystem handling

*********************************************************************/

#ifndef MAME_FORMATS_FS_HP98X5_H
#define MAME_FORMATS_FS_HP98X5_H

#pragma once

#include "fsmgr.h"

namespace fs {

	class hp9825_image : public manager_t {
	public:
		hp9825_image() : manager_t() {}

		virtual const char *name() const override;
		virtual const char *description() const override;

		virtual void enumerate_f(floppy_enumerator &fe) const override;

		virtual bool can_format() const override;
		virtual bool can_read() const override;
		virtual bool can_write() const override;
		virtual bool has_rsrc() const override;

		virtual std::vector<meta_description> volume_meta_description() const override;
		virtual std::vector<meta_description> file_meta_description() const override;

		virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const override;

	};

	class hp9831_image : public manager_t {
	public:
		hp9831_image() : manager_t() {}

		virtual const char *name() const override;
		virtual const char *description() const override;

		virtual void enumerate_f(floppy_enumerator &fe) const override;

		virtual bool can_format() const override;
		virtual bool can_read() const override;
		virtual bool can_write() const override;
		virtual bool has_rsrc() const override;

		virtual std::vector<meta_description> volume_meta_description() const override;
		virtual std::vector<meta_description> file_meta_description() const override;

		virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const override;

	};

	class hp9845_image : public manager_t {
	public:
		hp9845_image() : manager_t() {}

		virtual const char *name() const override;
		virtual const char *description() const override;

		virtual void enumerate_f(floppy_enumerator &fe) const override;

		virtual bool can_format() const override;
		virtual bool can_read() const override;
		virtual bool can_write() const override;
		virtual bool has_rsrc() const override;

		virtual std::vector<meta_description> volume_meta_description() const override;
		virtual std::vector<meta_description> file_meta_description() const override;

		virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const override;

	};

	extern const hp9825_image HP9825;
	extern const hp9831_image HP9831;
	extern const hp9845_image HP9845;
} // namespace fs

#endif /* MAME_FORMATS_FS_HP98X5_H */
