// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    fs_coco_rsdos.h

    Management of CoCo "RS-DOS" floppy images

***************************************************************************/

#ifndef MAME_FORMATS_FS_COCO_RSDOS_H
#define MAME_FORMATS_FS_COCO_RSDOS_H

#pragma once

#include "fsmgr.h"
#include <optional>
#include <string_view>

namespace fs {

class coco_rsdos_image : public manager_t {
public:
	coco_rsdos_image() : manager_t() {}

	virtual const char *name() const override;
	virtual const char *description() const override;

	virtual void enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const override;
	virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const override;

	virtual bool can_format() const override;
	virtual bool can_read() const override;
	virtual bool can_write() const override;
	virtual bool has_rsrc() const override;

	virtual std::vector<meta_description> file_meta_description() const override;

private:
	class impl : public filesystem_t {
	public:
		class root_dir : public idir_t {
		public:
			root_dir(impl &i) : m_fs(i) {}
			virtual ~root_dir() = default;

			virtual void drop_weak_references() override;
			virtual meta_data metadata() override;
			virtual std::vector<dir_entry> contents() override;
			virtual file_t file_get(u64 key) override;
			virtual dir_t dir_get(u64 key) override;

		private:
			impl &m_fs;
		};

		struct rsdos_dirent
		{
			char    m_filename[11];
			u8      m_filetype;
			u8      m_asciiflag;
			u8      m_first_granule;
			u8      m_last_sector_bytes_msb;
			u8      m_last_sector_bytes_lsb;
		};

		struct rsdos_dirent_sector
		{
			struct
			{
				rsdos_dirent    m_dirent;
				u8              m_unused[16];
			} m_entries[4];
		};

		class granule_iterator {
		public:
			granule_iterator(impl &fs, const rsdos_dirent &dirent);
			bool next(u8 &granule, u16 &byte_count);

		private:
			fsblk_t::block_t    m_granule_map;
			std::optional<u8>   m_current_granule;
			u8                  m_maximum_granules;
			u16                 m_last_sector_bytes;
		};

		class file : public ifile_t {
		public:
			file(impl &fs, rsdos_dirent &&dirent);
			virtual ~file() = default;

			virtual void drop_weak_references() override;

			virtual meta_data metadata() override;
			virtual std::vector<u8> read_all() override;

		private:
			impl &          m_fs;
			rsdos_dirent    m_dirent;
		};

		impl(fsblk_t &blockdev);
		virtual ~impl() = default;

		virtual meta_data metadata() override;
		virtual dir_t root() override;

	private:
		dir_t m_root;

		void drop_root_ref();
		fsblk_t::block_t read_sector(int track, int sector) const;
		u8 maximum_granules() const;
		static std::string get_filename_from_dirent(const rsdos_dirent &dirent);
	};

	static bool validate_filename(std::string_view name);
};

extern const coco_rsdos_image COCO_RSDOS;

} // namespace fs

#endif // MAME_FORMATS_FS_COCO_RSDOS_H
