// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Image generic handler class and helpers

#ifndef MAME_TOOLS_IMAGE_HANDLER_H
#define MAME_TOOLS_IMAGE_HANDLER_H

#pragma once

#include "formats/fsmgr.h"
#include "harddisk.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>


using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

struct floppy_format_info {
	const floppy_image_format_t *m_format;
	std::string m_category;

	floppy_format_info(const floppy_image_format_t *format, std::string category) : m_format(format), m_category(category) {}
};

struct floppy_create_info {
	const fs::manager_t *m_manager;

	const floppy_image_format_t *m_type;
	u32 m_image_size;
	u32 m_key;
	const char *m_name;
	const char *m_description;

	floppy_create_info(const fs::manager_t *manager, const floppy_image_format_t *type, u32 image_size, const char *name, const char *description) :
		m_manager(manager), m_type(type), m_image_size(image_size), m_key(0), m_name(name), m_description(description)
	{ }

	floppy_create_info(const char *name, u32 key, const char *description) :
		m_manager(nullptr), m_type(nullptr), m_image_size(0), m_key(key), m_name(name), m_description(description)
	{ }
};

struct filesystem_format {
	const fs::manager_t *m_manager;
	std::vector<std::unique_ptr<floppy_create_info>> m_floppy_create;
	std::string m_category;
	bool m_floppy, m_floppy_raw, m_hd, m_cd;

	filesystem_format(const fs::manager_t *manager, std::string category) : m_manager(manager), m_category(category), m_floppy(false), m_floppy_raw(false), m_hd(false), m_cd(false) {}
};

struct formats_table {
	std::vector<std::unique_ptr<floppy_format_info>>               floppy_format_infos;
	std::vector<std::unique_ptr<filesystem_format>>                filesystem_formats;

	std::map<std::string, const floppy_format_info *>              floppy_format_info_by_key;
	std::map<std::string, const filesystem_format *>               filesystem_format_by_key;
	std::map<std::string, const floppy_create_info *>              floppy_create_info_by_key;

	std::map<std::string, std::vector<const floppy_format_info *>> floppy_format_info_by_category;
	std::map<std::string, std::vector<const filesystem_format *>>  filesystem_format_by_category;

	void init();

	const floppy_format_info *find_floppy_format_info_by_key(const std::string &key) const;
	const filesystem_format *find_filesystem_format_by_key(const std::string &key) const;
	const floppy_create_info *find_floppy_create_info_by_key(const std::string &key) const;
};

class image_handler {
public:
	image_handler();

	void set_on_disk_path(std::string path);
	const std::string &get_on_disk_path() const { return m_on_disk_path; }

	std::vector<std::pair<u8, const floppy_format_info *>> identify(const formats_table &formats);

	bool floppy_load(const floppy_format_info *format);
	bool floppy_save(const floppy_format_info *format);

	void floppy_create(const floppy_create_info *format, fs::meta_data meta);
	bool floppy_mount_fs(const filesystem_format *format);
	bool hd_mount_fs(const filesystem_format *format);
	void fs_to_floppy();

	std::pair<const fs::manager_t *, fs::filesystem_t *> get_fs() const { return std::make_pair(m_fsm, m_fs.get()); }

	std::vector<std::string> path_split(std::string path) const;

	static std::vector<u8> fload(std::string path);
	static std::vector<u8> fload_rsrc(std::string path);
	static void fsave(std::string path, const std::vector<u8> &data);
	static void fsave_rsrc(std::string path, const std::vector<u8> &data);
	static bool fexists(std::string path);
	static std::string path_make_rsrc(std::string path);

private:
	std::string m_on_disk_path;

	floppy_image m_floppy_image;

	const floppy_image_format_t *m_floppy_fs_converter = nullptr;
	std::vector<u8> m_sector_image;
	std::unique_ptr<fs::fsblk_t> m_fsblk;
	const fs::manager_t *m_fsm = nullptr;
	std::unique_ptr<fs::filesystem_t> m_fs;

};

#endif // MAME_TOOLS_IMAGE_HANDLER_H
