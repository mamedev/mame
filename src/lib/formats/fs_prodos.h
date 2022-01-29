// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Creation of Apple ProDOS floppy images

#ifndef MAME_FORMATS_FS_PRODOS_H
#define MAME_FORMATS_FS_PRODOS_H

#pragma once

#include "fsmgr.h"

namespace fs {

class prodos_image : public manager_t {
public:
	class impl : public filesystem_t {
	public:
		class root_dir : public idir_t {
		public:
			root_dir(impl &fs, u16 base_block) : m_fs(fs), m_base_block(base_block) { }
			virtual ~root_dir() = default;

			virtual void drop_weak_references() override;

			virtual meta_data metadata() override;
			virtual std::vector<dir_entry> contents() override;
			virtual file_t file_get(u64 key) override;
			virtual dir_t dir_get(u64 key) override;

		protected:
			impl &m_fs;
			u16 m_base_block;

			std::pair<fsblk_t::block_t, const u8 *> get_entry_ro(u64 key);
			std::pair<fsblk_t::block_t, u8 *> get_entry(u64 key);
		};

		class dir : public root_dir {
		public:
			dir(impl &fs, const u8 *entry, u16 base_block, u16 key, root_dir *parent_dir);
			virtual ~dir() = default;

			virtual meta_data metadata() override;

		protected:
			root_dir *m_parent_dir;
			u16 m_key;
			u8 m_entry[39];
		};

		class file : public ifile_t {
		public:
			file(impl &fs, const u8 *entry, u16 key, root_dir *parent_dir);
			virtual ~file() = default;

			virtual void drop_weak_references() override;

			virtual meta_data metadata() override;
			virtual std::vector<u8> read_all() override;
			virtual std::vector<u8> rsrc_read_all() override;

		private:
			impl &m_fs;
			root_dir *m_parent_dir;
			u16 m_key;
			u8 m_entry[39];

			std::vector<u8> any_read_all(u8 type, u16 block, u32 length);
		};

		impl(fsblk_t &blockdev);
		virtual ~impl() = default;

		virtual void format(const meta_data &meta) override;

		virtual meta_data metadata() override;
		virtual dir_t root() override;

		void drop_root_ref();

		static util::arbitrary_datetime prodos_to_dt(u32 date);
		std::vector<dir_entry> contents(u16 block);

	private:
		static const u8 boot[512];

		dir_t m_root;
	};

	prodos_image() : manager_t() {}

	virtual const char *name() const override;
	virtual const char *description() const override;

	virtual void enumerate_f(floppy_enumerator &fe, u32 form_factor, const std::vector<u32> &variants) const override;
	virtual std::unique_ptr<filesystem_t> mount(fsblk_t &blockdev) const override;

	virtual bool can_format() const override;
	virtual bool can_read() const override;
	virtual bool can_write() const override;
	virtual bool has_rsrc() const override;
	virtual char directory_separator() const override;

	virtual std::vector<meta_description> volume_meta_description() const override;
	virtual std::vector<meta_description> file_meta_description() const override;
	virtual std::vector<meta_description> directory_meta_description() const override;
};

extern const prodos_image PRODOS;

} // namespace fs

#endif
