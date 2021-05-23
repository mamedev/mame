// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Creation of Apple ProDOS floppy images

#ifndef MAME_FORMATS_FS_PRODOS_H
#define MAME_FORMATS_FS_PRODOS_H

#pragma once

#include "fsmgr.h"

class fs_prodos : public filesystem_manager_t {
public:
	class impl : public filesystem_t {
	public:
		class dir : public idir_t {
		public:
			dir(impl &fs, u16 base_block, u16 key = 0) : m_fs(fs), m_base_block(base_block), m_key(key) { (void)m_key; }
			virtual ~dir() = default;

			virtual void drop_weak_references() override;

			virtual fs_meta_data metadata() override;
			virtual std::vector<fs_dir_entry> contents() override;
			virtual file_t file_get(uint64_t key) override;
			virtual dir_t dir_get(uint64_t key) override;

		private:
			impl &m_fs;
			u16 m_base_block;
			u16 m_key;

			std::pair<fsblk_t::block_t, const u8 *> get_entry_ro(uint64_t key);
			std::pair<fsblk_t::block_t, u8 *> get_entry(uint64_t key);
		};

		class file : public ifile_t {
		public:
			file(impl &fs, const u8 *entry, u16 key);
			virtual ~file() = default;

			virtual void drop_weak_references() override;

			virtual fs_meta_data metadata() override;
			virtual std::vector<u8> read_all() override;
			virtual std::vector<u8> rsrc_read_all() override;

		private:
			impl &m_fs;
			u16 m_key;
			u8 m_entry[39];

			std::vector<uint16_t> get_file_blocks(uint8_t type, u16 block, u32 length);
			std::pair<std::vector<uint16_t>, uint32_t> data_blocks();
			std::pair<std::vector<uint16_t>, uint32_t> rsrc_blocks();
		};

		impl(fsblk_t &blockdev);
		virtual ~impl() = default;

		virtual void format(const fs_meta_data &meta) override;

		virtual fs_meta_data metadata() override;
		virtual dir_t root() override;

		void drop_root_ref();

		static util::arbitrary_datetime prodos_to_dt(u32 date);

	private:
		static const u8 boot[512];

		dir_t m_root;
	};

	fs_prodos() : filesystem_manager_t() {}

	virtual void enumerate_f(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const override;

	virtual bool can_format() const override;
	virtual bool can_read() const override;
	virtual bool can_write() const override;
	virtual bool has_rsrc() const override;
	virtual char directory_separator() const override;

	virtual std::vector<fs_meta_description> volume_meta_description() const override;
	virtual std::vector<fs_meta_description> file_meta_description() const override;
	virtual std::vector<fs_meta_description> directory_meta_description() const override;
};

extern const filesystem_manager_type FS_PRODOS;

#endif
