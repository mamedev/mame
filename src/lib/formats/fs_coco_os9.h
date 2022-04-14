// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    fs_coco_os9.h

    Management of CoCo OS-9 floppy images

***************************************************************************/

#ifndef MAME_FORMATS_FS_COCO_OS9_H
#define MAME_FORMATS_FS_COCO_OS9_H

#pragma once

#include "fsmgr.h"
#include <optional>
#include <string_view>

namespace fs {

// ======================> coco_os9_image

class coco_os9_image : public manager_t {
public:
	class volume_header
	{
	public:
		volume_header(fsblk_t::block_t &&block);

		u32 total_sectors() const           { return m_block.r24b(0); }
		u8  track_size_in_sectors() const   { return m_block.r8(3); }
		u16 allocation_bitmap_bytes() const { return m_block.r16b(4); }
		u16 cluster_size() const            { return m_block.r16b(6); }
		u32 root_dir_lsn() const            { return m_block.r24b(8); }
		u16 owner_id() const                { return m_block.r16b(11); }
		u16 disk_id() const                 { return m_block.r16b(14); }
		u8  format_flags() const            { return m_block.r8(16); }
		u16 sectors_per_track() const       { return m_block.r16b(17); }
		u32 bootstrap_lsn() const           { return m_block.r24b(21); }
		u16 bootstrap_size() const          { return m_block.r16b(24); }
		util::arbitrary_datetime creation_date() const { return from_os9_date(m_block.r24b(26), m_block.r16b(29)); }
		u16 sector_size() const             { u16 result = m_block.r16b(104); return result != 0 ? result : 256; }
		u8 sides() const                    { return (format_flags() & 0x01) ? 2 : 1; }
		bool double_density() const         { return (format_flags() & 0x02) != 0; }
		bool double_track() const           { return (format_flags() & 0x04) != 0; }
		bool quad_track_density() const     { return (format_flags() & 0x08) != 0; }
		bool octal_track_density() const    { return (format_flags() & 0x10) != 0; }

		std::string name() const;

	private:
		fsblk_t::block_t    m_block;
	};

	class file_header
	{
	public:
		file_header(fsblk_t::block_t &&block);

		u8  attributes() const          { return m_block.r8(0); }
		u16 owner_id() const            { return m_block.r16b(1); }
		u8  link_count() const          { return m_block.r8(8); }
		u32 file_size() const           { return m_block.r32b(9); }
		util::arbitrary_datetime creation_date() const;
		bool is_directory() const       { return (attributes() & 0x80) != 0; }
		bool is_non_sharable() const    { return (attributes() & 0x40) != 0; }
		bool is_public_execute() const  { return (attributes() & 0x20) != 0; }
		bool is_public_write() const    { return (attributes() & 0x10) != 0; }
		bool is_public_read() const     { return (attributes() & 0x08) != 0; }
		bool is_user_execute() const    { return (attributes() & 0x04) != 0; }
		bool is_user_write() const      { return (attributes() & 0x02) != 0; }
		bool is_user_read() const       { return (attributes() & 0x01) != 0; }

		meta_data metadata() const;
		int get_sector_map_entry_count() const;
		void get_sector_map_entry(int entry_number, u32 &start_lsn, u16 &count) const;

	private:
		fsblk_t::block_t    m_block;
	};

	coco_os9_image() : manager_t() {}

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
	std::vector<meta_description> entity_meta_description() const;

private:
	class impl : public filesystem_t {
	public:
		class file : public ifile_t {
		public:
			file(impl &fs, file_header &&file_header);
			virtual ~file() = default;

			virtual void drop_weak_references() override;

			virtual meta_data metadata() override;
			virtual std::vector<u8> read_all() override;

		private:
			impl &      m_fs;
			file_header m_file_header;
		};

		class directory : public idir_t {
		public:
			directory(impl &i, file_header &&file_header);
			virtual ~directory() = default;

			virtual void drop_weak_references() override;
			virtual meta_data metadata() override;
			virtual std::vector<dir_entry> contents() override;
			virtual file_t file_get(u64 key) override;
			virtual dir_t dir_get(u64 key) override;

		private:
			impl &      m_fs;
			file_header m_file_header;
		};

		impl(fsblk_t &blockdev, volume_header &&header);
		virtual ~impl() = default;

		virtual meta_data metadata() override;
		virtual dir_t root() override;
		virtual void format(const meta_data &meta) override;

	private:
		volume_header   m_volume_header;
		dir_t           m_root;

		directory *open_directory(u32 lsn);
		void drop_root_ref();
		std::vector<u8> read_file_data(const file_header &header) const;
	};

	static std::string pick_os9_string(std::string_view raw_string);
	static std::string to_os9_string(std::string_view s, size_t length);
	static util::arbitrary_datetime from_os9_date(u32 os9_date, u16 os9_time = 0);
	static std::tuple<u32, u16> to_os9_date(const util::arbitrary_datetime &datetime);
	static u32 pick_integer_be(const u8 *data, int length);
	static bool validate_filename(std::string_view name);
	static bool is_ignored_filename(std::string_view name);
};

extern const coco_os9_image COCO_OS9;

} // namespace fs

#endif // MAME_FORMATS_FS_COCO_OS9_H
