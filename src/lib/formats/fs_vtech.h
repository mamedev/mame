// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Management of VTech images

#ifndef MAME_FORMATS_FS_VTECH_H
#define MAME_FORMATS_FS_VTECH_H

#pragma once

#include "fsmgr.h"

class fs_vtech : public filesystem_manager_t {
public:
	class impl : public filesystem_t {
	public:
		class root_dir : public idir_t {
		public:
			root_dir(impl &i) : m_fs(i) {}
			virtual ~root_dir() = default;

			virtual void drop_weak_references() override;

			virtual fs_meta_data metadata() override;
			virtual void metadata_change(const fs_meta_data &info) override;
			virtual std::vector<fs_dir_entry> contents() override;
			virtual file_t file_get(uint64_t key) override;
			virtual dir_t dir_get(uint64_t key) override;
			virtual file_t file_create(const fs_meta_data &info) override;
			virtual void file_delete(uint64_t key) override;

			void update_file(uint16_t key, const uint8_t *entry);

		private:
			impl &m_fs;

			std::pair<fsblk_t::block_t, uint32_t> get_dir_block(uint64_t key);
		};

		class file : public ifile_t {
		public:
			file(impl &fs, root_dir *dir, const uint8_t *entry, uint16_t key);
			virtual ~file() = default;

			virtual void drop_weak_references() override;

			virtual fs_meta_data metadata() override;
			virtual void metadata_change(const fs_meta_data &info) override;
			virtual std::vector<uint8_t> read_all() override;
			virtual void replace(const std::vector<uint8_t> &data) override;

		private:
			impl &m_fs;
			root_dir *m_dir;
			uint16_t m_key;
			uint8_t m_entry[18];
		};

		impl(fsblk_t &blockdev);
		virtual ~impl() = default;

		virtual void format(const fs_meta_data &meta) override;
		virtual fs_meta_data metadata() override;
		virtual void metadata_change(const fs_meta_data &info) override;
		virtual dir_t root() override;

		void drop_root_ref();

		std::vector<std::pair<uint8_t, uint8_t>> allocate_blocks(uint32_t count);
		void free_blocks(const std::vector<std::pair<uint8_t, uint8_t>> &blocks);
		uint32_t free_block_count();

	private:
		dir_t m_root;
	};

	fs_vtech() : filesystem_manager_t() {}

	virtual const char *name() const override;
	virtual const char *description() const override;

	virtual void enumerate_f(floppy_enumerator &fe, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const override;

	virtual bool can_format() const override;
	virtual bool can_read() const override;
	virtual bool can_write() const override;
	virtual bool has_rsrc() const override;

	virtual std::vector<fs_meta_description> volume_meta_description() const override;
	virtual std::vector<fs_meta_description> file_meta_description() const override;
};

extern const fs_vtech FS_VTECH;

#endif
