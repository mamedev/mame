// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    fs_isis.h

    Intel ISIS-II filesystem handling

*********************************************************************/

#ifndef MAME_FORMATS_FS_ISIS_H
#define MAME_FORMATS_FS_ISIS_H

#pragma once

#include "fsmgr.h"

namespace fs {

	class isis_image : public manager_t {
	public:
		isis_image() : manager_t() {}

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

	extern const isis_image ISIS;
} // namespace fs

#endif /* MAME_FORMATS_FS_ISIS_H */
