// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Image generic handler class and helpers

#include "image_handler.h"

#include "formats/all.h"
#include "formats/fsblk_vec.h"
#include "formats/fs_unformatted.h"

#include "ioprocs.h"
#include "ioprocsfill.h"
#include "ioprocsvec.h"
#include "strformat.h"


// Format enumeration

namespace {
	struct enumerator : public mame_formats_enumerator {
		formats_table *m_table;
		std::string m_category;

		enumerator(formats_table *table) : mame_formats_enumerator(), m_table(table), m_category("None") {}

		virtual ~enumerator() = default;
		virtual void add(const cassette_image::Format *const *formats) {}

		virtual void category(const char *name) {
			m_category = name;
		}

		virtual void add(const floppy_image_format_t &format) {
			m_table->floppy_format_infos.emplace_back(std::make_unique<floppy_format_info>(&format, m_category));
		}

		virtual void add(const fs::manager_t &fs) {
			m_table->filesystem_formats.emplace_back(std::make_unique<filesystem_format>(&fs, m_category));
		}
	};

	struct fs_enum : public fs::manager_t::floppy_enumerator {
		filesystem_format *m_format;

		fs_enum(filesystem_format *format) : m_format(format) {}

		virtual void add(const floppy_image_format_t &type, u32 image_size, const char *name, const char *description) override {
			m_format->m_floppy = true;
			m_format->m_floppy_create.emplace_back(std::make_unique<floppy_create_info>(m_format->m_manager, &type, image_size, name, description));
		}

		virtual void add_raw(const char *name, u32 key, const char *description) override {
			m_format->m_floppy_raw = true;
			m_format->m_floppy_create.emplace_back(std::make_unique<floppy_create_info>(name, key, description));
		}
	};
}

void formats_table::init()
{
	std::vector<uint32_t> variants;

	enumerator en(this);
	mame_formats_full_list(en);

	for(auto &f : filesystem_formats) {
		fs_enum fen(f.get());
		f->m_manager->enumerate_f(fen, floppy_image::FF_UNKNOWN, variants);
	}

	for(auto &f : floppy_format_infos) {
		auto *ff = f.get();
		std::string key = ff->m_format->name();
		auto i = floppy_format_info_by_key.find(key);
		if(i != floppy_format_info_by_key.end()) {
			fprintf(stderr, "Collision on floppy format name %s between \"%s\" and \"%s\".\n",
					ff->m_format->name(),
					ff->m_format->description(),
					i->second->m_format->description());
			exit(1);
		}

		floppy_format_info_by_key[key] = ff;
		floppy_format_info_by_category[ff->m_category].push_back(ff);
	}

	for(auto &f : filesystem_formats) {
		auto *ff = f.get();
		std::string key = ff->m_manager->name();
		auto i = filesystem_format_by_key.find(key);
		if(i != filesystem_format_by_key.end()) {
			fprintf(stderr, "Collision on filesystem name %s between \"%s\" and \"%s\".\n",
					ff->m_manager->name(),
					ff->m_manager->description(),
					i->second->m_manager->description());
			exit(1);
		}

		filesystem_format_by_key[key] = ff;
		filesystem_format_by_category[ff->m_category].push_back(ff);

		for(auto &f2 : ff->m_floppy_create) {
			auto *ff2 = f2.get();
			key = ff2->m_name;
			auto i = floppy_create_info_by_key.find(key);
			if(i != floppy_create_info_by_key.end()) {
				fprintf(stderr, "Collision on floppy create name %s between \"%s\" and \"%s\".\n",
						ff2->m_name,
						ff2->m_description,
						i->second->m_description);
				exit(1);
			}

			floppy_create_info_by_key[key] = ff2;
		}
	}
}

const floppy_format_info *formats_table::find_floppy_format_info_by_key(const std::string &key) const
{
	auto i = floppy_format_info_by_key.find(key);
	return i == floppy_format_info_by_key.end() ? nullptr : i->second;
}

const filesystem_format *formats_table::find_filesystem_format_by_key(const std::string &key) const
{
	auto i = filesystem_format_by_key.find(key);
	return i == filesystem_format_by_key.end() ? nullptr : i->second;
}

const floppy_create_info *formats_table::find_floppy_create_info_by_key(const std::string &key) const
{
	auto i = floppy_create_info_by_key.find(key);
	return i == floppy_create_info_by_key.end() ? nullptr : i->second;
}


// Image handling

std::vector<u8> image_handler::fload(std::string path)
{
	char msg[4096];
	sprintf(msg, "Error opening %s for reading", path.c_str());
	auto fi = fopen(path.c_str(), "rb");
	if(!fi) {
		perror(msg);
		exit(1);
	}
	fseek(fi, 0, SEEK_END);
	long size = ftell(fi);
	std::vector<u8> filedata(size);
	fseek(fi, 0, SEEK_SET);
	fread(filedata.data(), filedata.size(), 1, fi);
	fclose(fi);

	return filedata;
}

std::vector<u8> image_handler::fload_rsrc(std::string path)
{
	auto filedata = fload(path);
	const u8 *head = filedata.data();

	using fs::filesystem_t;
	if(filesystem_t::r32b(head+0x00) == 0x00051607 &&
	   filesystem_t::r32b(head+0x04) == 0x00020000) {
		u16 nent = filesystem_t::r16b(head+0x18);
		for(u16 i=0; i != nent; i++) {
			const u8 *e = head + 12*i;
			if(filesystem_t::r32b(e+0) == 2) {
				u32 start = filesystem_t::r32b(e+4);
				u32 len = filesystem_t::r32b(e+8);
				filedata.erase(filedata.begin(), filedata.begin() + start);
				filedata.erase(filedata.begin() + len, filedata.end());
				return filedata;
			}
		}
	}
	filedata.clear();
	return filedata;
}

void image_handler::fsave(std::string path, const std::vector<u8> &data)
{
	char msg[4096];
	sprintf(msg, "Error opening %s for writing", path.c_str());
	auto fo = fopen(path.c_str(), "wb");
	if(!fo) {
		perror(msg);
		exit(1);
	}

	fwrite(data.data(), data.size(), 1, fo);
	fclose(fo);
}

void image_handler::fsave_rsrc(std::string path, const std::vector<u8> &data)
{
	u8 head[0x2a];

	using fs::filesystem_t;
	filesystem_t::w32b(head+0x00, 0x00051607);  // Magic
	filesystem_t::w32b(head+0x04, 0x00020000);  // Version
	filesystem_t::fill(head+0x08, 0, 16);       // Filler
	filesystem_t::w16b(head+0x18, 1);           // Number of entries
	filesystem_t::w32b(head+0x1a, 2);           // Resource fork
	filesystem_t::w32b(head+0x22, 0x2a);        // Offset in the file
	filesystem_t::w32b(head+0x26, data.size()); // Length

	char msg[4096];
	sprintf(msg, "Error opening %s for writing", path.c_str());
	auto fo = fopen(path.c_str(), "wb");
	if(!fo) {
		perror(msg);
		exit(1);
	}

	fwrite(head, sizeof(head), 1, fo);
	fwrite(data.data(), data.size(), 1, fo);
	fclose(fo);
}

image_handler::image_handler() : m_floppy_image(84, 2, floppy_image::FF_UNKNOWN)
{
}

void image_handler::set_on_disk_path(std::string path)
{
	m_on_disk_path = path;
}

std::vector<std::pair<u8, const floppy_format_info *>> image_handler::identify(const formats_table &formats)
{
	std::vector<std::pair<u8, const floppy_format_info *>> res;
	std::vector<uint32_t> variants;

	FILE *f = fopen(m_on_disk_path.c_str(), "rb");
	if (!f) {
		std::string msg = util::string_format("Error opening %s for reading", m_on_disk_path);
		perror(msg.c_str());
		return res;
	}

	auto io = util::stdio_read(f, 0xff);

	for(const auto &e : formats.floppy_format_info_by_key) {
		u8 score = e.second->m_format->identify(*io, floppy_image::FF_UNKNOWN, variants);
		if(score && e.second->m_format->extension_matches(m_on_disk_path.c_str()))
			score |= floppy_image_format_t::FIFID_EXT;
		if(score)
			res.emplace_back(std::make_pair(score, e.second));
	}

	return res;
}

bool image_handler::floppy_load(const floppy_format_info *format)
{
	std::vector<uint32_t> variants;
	FILE *f = fopen(m_on_disk_path.c_str(), "rb");
	if (!f) {
		std::string msg = util::string_format("Error opening %s for reading", m_on_disk_path);
		perror(msg.c_str());
		return true;
	}

	auto io = util::stdio_read(f, 0xff);

	return !format->m_format->load(*io, floppy_image::FF_UNKNOWN, variants, &m_floppy_image);
}

bool image_handler::floppy_save(const floppy_format_info *format)
{
	std::vector<uint32_t> variants;
	std::string msg = util::string_format("Error opening %s for writing", m_on_disk_path);
	FILE *f = fopen(m_on_disk_path.c_str(), "wb");
	if (!f) {
		perror(msg.c_str());
		return true;
	}

	auto io = util::stdio_read_write(f, 0xff);

	return !format->m_format->save(*io, variants, &m_floppy_image);
}

void image_handler::floppy_create(const floppy_create_info *format, fs::meta_data meta)
{
	if(format->m_type) {
		std::vector<uint32_t> variants;
		std::vector<u8> img(format->m_image_size);
		fs::fsblk_vec_t blockdev(img);
		auto fs = format->m_manager->mount(blockdev);
		fs->format(meta);

		auto io = util::ram_read(img.data(), img.size(), 0xff);
		format->m_type->load(*io, floppy_image::FF_UNKNOWN, variants, &m_floppy_image);
	} else {
		fs::unformatted_image::format(format->m_key, &m_floppy_image);
	}
}

bool image_handler::floppy_mount_fs(const filesystem_format *format)
{
	m_floppy_fs_converter = nullptr;
	for(const auto &ci : format->m_floppy_create) {
		if(ci->m_type != m_floppy_fs_converter) {
			std::vector<uint32_t> variants;
			m_floppy_fs_converter = ci->m_type;
			m_sector_image.clear();
			util::random_read_write_fill_wrapper<util::vector_read_write_adapter<u8>, 0xff> io(m_sector_image);
			m_floppy_fs_converter->save(io, variants, &m_floppy_image);
		}

		if(ci->m_image_size == m_sector_image.size())
			goto success;
	}
	m_floppy_fs_converter = nullptr;
	m_sector_image.clear();
	return true;

 success:
	m_fsblk.reset(new fs::fsblk_vec_t(m_sector_image));
	m_fsm = format->m_manager;
	m_fs = m_fsm->mount(*m_fsblk);
	return false;
}

bool image_handler::hd_mount_fs(const filesystem_format *format)
{
	// Should use the chd mechanisms, one thing at a time...

	m_sector_image = fload(m_on_disk_path);
	m_fsblk.reset(new fs::fsblk_vec_t(m_sector_image));
	m_fsm = format->m_manager;
	m_fs = m_fsm->mount(*m_fsblk);
	return false;
}

void image_handler::fs_to_floppy()
{
	std::vector<uint32_t> variants;
	auto io = util::ram_read(m_sector_image.data(), m_sector_image.size(), 0xff);
	m_floppy_fs_converter->load(*io, floppy_image::FF_UNKNOWN, variants, &m_floppy_image);
}

std::vector<std::string> image_handler::path_split(std::string path) const
{
	std::string opath = path;
	std::vector<std::string> rpath;
	if(m_fsm->has_subdirectories()) {
		std::string element;
		char sep = m_fsm->directory_separator();
		for(char c : opath) {
			if(c == sep) {
				if(!element.empty()) {
					rpath.push_back(element);
					element.clear();
				}
			} else
				element += c;
		}
		if(!element.empty())
			rpath.push_back(element);

	} else
		rpath.push_back(opath);

	return rpath;
}

bool image_handler::fexists(std::string path)
{
	auto f = fopen(path.c_str(), "rb");
	if(f != nullptr) {
		fclose(f);
		return true;
	}
	return false;
}


std::string image_handler::path_make_rsrc(std::string path)
{
	auto p = path.end();
	while(p != path.begin() && p[-1] != '/')
		p--;
	std::string rpath(path.begin(), p);
	rpath += "._";
	rpath += std::string(p, path.end());
	return rpath;
}

