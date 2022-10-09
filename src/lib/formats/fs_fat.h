// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    fs_fat.h

    PC FAT disk images

***************************************************************************/

#ifndef MAME_FORMATS_FS_FAT_H
#define MAME_FORMATS_FS_FAT_H

#pragma once

#include "fsmgr.h"
#include <optional>
#include <string_view>

namespace fs {

// ======================> fat_image

class fat_image : public manager_t {
public:
	fat_image() = default;

	virtual bool can_format() const override;
	virtual bool can_read() const override;
	virtual bool can_write() const override;
	virtual bool has_rsrc() const override;
	virtual char directory_separator() const override;

	virtual std::vector<meta_description> volume_meta_description() const override;
	virtual std::vector<meta_description> file_meta_description() const override;
	virtual std::vector<meta_description> directory_meta_description() const override;

protected:
	static std::unique_ptr<filesystem_t> mount_partition(fsblk_t &blockdev, u32 starting_sector, u32 sector_count, u8 bits_per_fat_entry);
};


// ======================> fat_image

class pc_fat_image : public fat_image {
public:
	pc_fat_image() = default;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual void enumerate_f(floppy_enumerator &fe) const override;
	virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const override;
};


extern const pc_fat_image PC_FAT;

} // namespace fs

#endif // MAME_FORMATS_FS_FAT_H
