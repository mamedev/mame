// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    (Floppy) image command-line manager

***************************************************************************/

#include "image_handler.h"

#include "formats/fsblk.h"

#include "corestr.h"
#include "hashing.h"
#include "ioprocs.h"
#include "path.h"
#include "strformat.h"

#include "osdcomm.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>


static formats_table formats;

struct command_info
{
	const char *name;
	const char *params;
	const char *desc;
};

static const command_info s_command_usage[] =
{
	{
		"identify", "<inputfile> [<inputfile> ...]",
		"Identify an image format"
	},
	{
		"flopconvert", "[input_format|auto] output_format <inputfile> <outputfile>",
		"Convert a floppy image"
	},
	{
		"flopcreate", "output_format filesystem <outputfile>",
		"Create a preformatted floppy image"
	},
	{
		"flopdir", "input_format filesystem <image>",
		"List the contents of a floppy image"
	},
	{
		"flophashes", "input_format filesystem <image>",
		"List hashes for each file on a floppy image"
	},
	{
		"flopblocks", "input_format filesystem <image>",
		"Enumerate blocks used by each file/directory on a floppy image"
	},
	{
		"flopread", "input_format filesystem <image> <path> <outputfile>",
		"Extract a file from a floppy image"
	},
	{
		"flopwrite", "input_format filesystem <image> <inputfile> <path>",
		"Write a file into a floppy image"
	},
	{
		"flopchmeta", "format filesystem <image> [<path>] [-<name> <value> ...]",
		"Change metadata for a file, directory or volume on a floppy image"
	},
	{
		"floprename", "input_format filesystem <image> <oldpath> <newpath>",
		"Rename a file or directory on a floppy image"
	},
	{
		"flopremove", "input_format filesystem <image> <path>",
		"Remove a file or empty directory from a floppy image"
	},
	{
		"hddir", "filesystem <image>",
		"List the contents of a hard disk image"
	},
	{
		"hdhashes", "filesystem <image>",
		"List hashes for each file on a hard disk image"
	},
	{
		"hdblocks", "filesystem <image>",
		"Enumerate blocks used by each file/directory on a hard disk image"
	},
	{
		"hdread", "filesystem <image> <path> <outputfile>",
		"Extract a file from a hard disk image"
	},
	{
		"hdwrite", "filesystem <image> <inputfile> <path>",
		"Write a file into a hard disk image"
	},
	{
		"help", "[subject|.ext|all]",
		"Display help for supported commands, filesystems and/or formats"
	},
	{
		"version", "",
		"Display the current version of floptool"
	}
};

static void display_usage(FILE *f, const char *first_argument, const char *cmd_name)
{
	std::string exe_name(core_filename_extract_base(first_argument));

	fprintf(f, "Usage: \n");
	for(const command_info &info : s_command_usage) {
		if(!cmd_name || !strcmp(info.name, cmd_name))
			fprintf(f, "       %s %s %-*s -- %s\n", exe_name.c_str(), info.name, int(69 - strlen(info.name)), info.params, info.desc);
	}
}

static void display_one_format(const floppy_image_format_t &format, int sk)
{
	printf("  %-*s     r%c - %s [%s]\n", sk, format.name(), format.supports_save() ? 'w' : '-', format.description(), format.extensions());
}

static void display_formats()
{
	int sk = 0;
	for(const auto &e : formats.floppy_format_info_by_key) {
		int sz = e.first.size();
		if(sz > sk)
			sk = sz;
	}

	printf("Supported floppy formats:\n\n");
	for(const auto &e : formats.floppy_format_info_by_category)
		if(!e.second.empty()) {
			printf("%s:\n", e.first.c_str());
			for(auto *fif : e.second)
				display_one_format(*fif->m_format, sk);
		}
}

static int display_formats_by_extension(const char *ext)
{
	int sk = 0;
	for(const auto &e : formats.floppy_format_info_by_key) {
		int sz = e.first.size();
		if(sz > sk)
			sk = sz;
	}

	bool found = false;
	for(const auto &e : formats.floppy_format_info_by_category) {
		bool foundcat = false;
		for(auto *fif : e.second) {
			if(fif->m_format->extension_matches(ext)) {
				if(!found) {
					printf("Floppy formats matching %s:\n\n", ext);
					found = true;
				}
				if(!foundcat) {
					printf("%s:\n", e.first.c_str());
					foundcat = true;
				}
				display_one_format(*fif->m_format, sk);
			}
		}
	}

	if(!found) {
		fprintf(stderr, "No floppy formats matching \"%s\" found.\n", ext);
		return 1;
	}

	return 0;
}

static void display_format_info(const floppy_format_info &fmtinfo)
{
	printf("Format '%s' (%s)\n  Identifying extensions: %s\n  Saving: %s\n",
			fmtinfo.m_format->name(),
			fmtinfo.m_format->description(),
			fmtinfo.m_format->extensions(),
			fmtinfo.m_format->supports_save() ? "supported" : "not supported");
}

static void display_filesystems()
{
	int sk = 0;
	for(const auto &e : formats.filesystem_format_by_key) {
		int sz = e.first.size();
		if(sz > sk)
			sk = sz;
	}

	for(const auto &e : formats.floppy_create_info_by_key) {
		int sz = e.first.size();
		if(sz > sk)
			sk = sz;
	}

	printf("Supported filesystems (with floppy formatting names):\n\n");
	for(const auto &e : formats.filesystem_format_by_category)
		if(!e.second.empty()) {
			printf("%s:\n", e.first.c_str());
			for(const auto &f : e.second) {
				printf("  %-*s %c%c%c %c%c%c - %s\n",
						sk,
						f->m_manager->name(),
						f->m_floppy || f->m_floppy_raw ? 'F' : '-',
						f->m_hd ? 'H' : '-',
						f->m_cd ? 'C' : '-',
						f->m_manager->can_format() || f->m_floppy_raw ? 'f' : '-',
						f->m_manager->can_read() ? 'r' : '-',
						f->m_manager->can_write() ? 'w' : '-',
						f->m_manager->description());
				for(auto &f2 : f->m_floppy_create)
					printf("    %-*s         - %s\n",
							sk,
							f2->m_name,
							f2->m_description);
			}
		}
}

static void display_metadata_info(const std::vector<fs::meta_description> &metad)
{
	for(const auto &d : metad)
		printf("    %s [r%s] - %s\n",
				fs::meta_data::entry_name(d.m_name),
				d.m_ro ? "" : "w",
				d.m_tooltip);
}

static void display_filesystem_info(const filesystem_format &fs)
{
	const bool can_format = fs.m_manager->can_format() || fs.m_floppy_raw;
	printf("%s '%s' (%s)\n  Reading: %s\n  Writing: %s\n  Formatting: %s\n",
			fs.m_floppy_raw ? "Raw format" : "Filesystem",
			fs.m_manager->name(),
			fs.m_manager->description(),
			fs.m_manager->can_read() ? "supported" : "not supported",
			fs.m_manager->can_write() ? "supported" : "not supported",
			can_format ? "supported" : "not supported");
	if(can_format)
		for(auto &f2 : fs.m_floppy_create)
			printf("    '%s' - %s\n", f2->m_name, f2->m_description);

	const auto vmetad = fs.m_manager->volume_meta_description();
	if(!vmetad.empty()) {
		printf("  Volume metadata:\n");
		display_metadata_info(vmetad);
	}

	const auto fmetad = fs.m_manager->file_meta_description();
	if(!fmetad.empty()) {
		printf("  File metadata:\n");
		display_metadata_info(fmetad);
	}

	const auto dmetad = fs.m_manager->directory_meta_description();
	if(!dmetad.empty()) {
		printf("  Directory metadata:\n");
		display_metadata_info(dmetad);
	}
}

static void display_standard_usage(char *argv[])
{
	/* Usage */
	printf("floptool - Generic floppy image manipulation tool for use with MAME\n\n");
	display_usage(stdout, argv[0], nullptr);
}

static int identify(int argc, char *argv[])
{
	// Need to detect CHDs too

	if(argc<3) {
		fprintf(stderr, "Missing name of file to identify.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	int sz = 0;
	for(int i=2; i<argc; i++) {
		int len = strlen(argv[i]);
		if(len > sz)
			sz = len;
	}

	for(int i=2; i<argc; i++) {
		image_handler ih;
		ih.set_on_disk_path(argv[i]);
		auto scores = ih.identify(formats);
		if(scores.empty())
			printf("%-*s : Unknown format\n", sz, argv[i]);
		int sz2 = 0;
		for(const auto &e : scores) {
			int len = strlen(e.second->m_format->name());
			if(len > sz2)
				sz2 = len;
		}

		bool first = true;
		for(const auto &e : scores) {
			printf("%-*s %c %c%c%c%c%c - %-*s %s\n", sz, first ? argv[i] : "", first ? ':' : ' ',
				   e.first & 0x10 ? '+' : '.',
				   e.first & 0x08 ? '+' : '.',
				   e.first & 0x04 ? '+' : '.',
				   e.first & 0x02 ? '+' : '.',
				   e.first & 0x01 ? '+' : '.',
				   sz2, e.second->m_format->name(), e.second->m_format->description());
			first = false;
		}
	}
	return 0;
}

static const floppy_format_info *find_floppy_source_format(const char *name, image_handler &ih)
{
	const floppy_format_info *source_format;
	if(!core_stricmp(name, "auto")) {
		auto scores = ih.identify(formats);
		if(scores.empty()) {
			fprintf(stderr, "Error: Could not identify the format of file %s\n", ih.get_on_disk_path().c_str());
			return nullptr;
		}
		if(scores.size() >= 2 && scores[0].first == scores[1].first) {
			fprintf(stderr, "Ambiguous source format.  Possible formats:\n");
			int sz = 0;
			for(const auto &e : scores) {
				int len = strlen(e.second->m_format->name());
				if(len > sz)
					sz = len;
			}

			for(const auto &e : scores)
				printf("  %3d - %-*s %s\n", e.first, sz, e.second->m_format->name(), e.second->m_format->description());
			return nullptr;
		}
		source_format = scores[0].second;

	} else {
		source_format = formats.find_floppy_format_info_by_key(name);
		if(!source_format) {
			fprintf(stderr, "Error: Format '%s' unknown\n", name);
			return nullptr;

		}
	}
	return source_format;
}

static int flopconvert(int argc, char *argv[])
{
	if(argc!=6) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[4]);

	const floppy_format_info *source_format = find_floppy_source_format(argv[2], ih);
	if(!source_format)
		return 1;

	const floppy_format_info *dest_format = formats.find_floppy_format_info_by_key(argv[3]);
	if(!dest_format) {
		fprintf(stderr, "Error: Format '%s' unknown\n", argv[3]);
		return 1;
	}
	if(!dest_format->m_format->supports_save()) {
		fprintf(stderr, "Error: Saving to format '%s' unsupported\n", argv[3]);
		return 1;
	}

	if(ih.floppy_load(*source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	ih.set_on_disk_path(argv[5]);

	if(ih.floppy_save(*dest_format)) {
		fprintf(stderr, "Error: Saving as format '%s' failed\n", dest_format->m_format->name());
		return 1;
	}

	return 0;
}

static fs::meta_data extract_meta_data(int &argc, char *argv[])
{
	fs::meta_data result;

	int new_argc = 0;
	for (int i = 0; i < argc; i++)
	{
		std::optional<fs::meta_name> attrname;
		if (argv[i][0] == '-' && i < argc - 1)
			attrname = fs::meta_data::from_entry_name(&argv[i][1]);

		if (attrname)
		{
			// we found a metadata variable; set it
			result.set(*attrname, argv[i + 1]);
			i++;
		}
		else
		{
			// we didn't; update argv
			argv[new_argc++] = argv[i];
		}
	}

	// we're done; update the argument count
	argc = new_argc;
	return result;
}

static int flopcreate(int argc, char *argv[])
{
	fs::meta_data meta = extract_meta_data(argc, argv);

	if(argc!=5) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	auto dest_format = formats.find_floppy_format_info_by_key(argv[2]);
	if(!dest_format) {
		fprintf(stderr, "Error: Floppy format '%s' unknown\n", argv[2]);
		return 1;
	}
	if(!dest_format->m_format->supports_save()) {
		fprintf(stderr, "Error: Saving to format '%s' unsupported\n", argv[2]);
		return 1;
	}

	auto create_fs = formats.find_floppy_create_info_by_key(argv[3]);
	if(!create_fs) {
		fprintf(stderr, "Error: Floppy creation format '%s' unknown\n", argv[3]);
		return 1;
	}
	if(create_fs->m_manager && !create_fs->m_manager->can_format()) {
		fprintf(stderr, "Error: Floppy creation format '%s' does not support creating new images\n", argv[3]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[4]);

	if(ih.floppy_create(*create_fs, meta)) {
		fprintf(stderr, "Error: Floppy creation failed for format '%s'\n", create_fs->m_manager->name());
		return 1;
	}
	return ih.floppy_save(*dest_format);
}

static void dir_scan(fs::filesystem_t *fs, u32 depth, const std::vector<std::string> &path, std::vector<std::vector<std::string>> &entries, const std::unordered_map<fs::meta_name, size_t> &nmap, size_t nc, const std::vector<fs::meta_description> &dmetad, const std::vector<fs::meta_description> &fmetad)
{
	std::string head;
	for(u32 i = 0; i != depth; i++)
		head += "  ";
	auto [err, contents] = fs->directory_contents(path);
	if(err)
		return; // FIXME: don't swallow errors
	for(const auto &c : contents) {
		size_t id = entries.size();
		entries.resize(id+1);
		entries[id].resize(nc);
		switch(c.m_type) {
		case fs::dir_entry_type::dir: {
			for(const auto &m : dmetad) {
				if(!c.m_meta.has(m.m_name))
					continue;
				size_t slot = nmap.find(m.m_name)->second;
				std::string val = c.m_meta.get(m.m_name).as_string();
				if(slot == 0)
					val = head + "dir  " + val;
				entries[id][slot] = val;
			}
			auto npath = path;
			npath.push_back(c.m_name);
			dir_scan(fs, depth+1, npath, entries, nmap, nc, dmetad, fmetad);
			break;
		}
		case fs::dir_entry_type::file: {
			for(const auto &m : fmetad) {
				if(!c.m_meta.has(m.m_name))
					continue;
				size_t slot = nmap.find(m.m_name)->second;
				std::string val = c.m_meta.get(m.m_name).as_string();
				if(slot == 0)
					val = head + "file " + val;
				entries[id][slot] = val;
			}
			break;
		}
		}
	}
}

static void dir_report(const std::vector<std::vector<std::string>> &entries, size_t nc)
{
	std::vector<u32> sizes(nc);

	for(const auto &e : entries)
		for(unsigned int i=0; i != nc; i++)
			sizes[i] = std::max<u32>(sizes[i], e[i].size());

	for(const auto &e : entries) {
		std::string l;
		for(unsigned int i=0; i != nc; i++) {
			if(i)
				l += ' ';
			l += util::string_format("%-*s", sizes[i], e[i]);
		}
		printf("%s\n", l.c_str());
	}
}

static int generic_dir(image_handler &ih)
{
	auto [fsm, fs] = ih.get_fs();
	auto vmetad = fsm->volume_meta_description();
	auto fmetad = fsm->file_meta_description();
	auto dmetad = fsm->directory_meta_description();

	auto vmeta = fs->volume_metadata();
	if(!vmeta.empty()) {
		std::string vinf = "Volume:";
		for(const auto &e : vmetad)
			vinf += util::string_format(" %s=%s", fs::meta_data::entry_name(e.m_name), vmeta.get(e.m_name).as_string());
		printf("%s\n\n", vinf.c_str());
	}

	std::vector<fs::meta_name> names;
	names.push_back(fs::meta_name::name);
	for(const auto &e : fmetad)
		if(e.m_name != fs::meta_name::name)
			names.push_back(e.m_name);
	for(const auto &e : dmetad)
		if(std::find(names.begin(), names.end(), e.m_name) == names.end())
			names.push_back(e.m_name);

	std::unordered_map<fs::meta_name, size_t> nmap;
	for(size_t i = 0; i != names.size(); i++)
		nmap[names[i]] = i;

	std::vector<std::vector<std::string>> entries;

	entries.resize(1);
	for(fs::meta_name n : names)
		entries[0].push_back(fs::meta_data::entry_name(n));

	dir_scan(fs, 0, std::vector<std::string>(), entries, nmap, names.size(), dmetad, fmetad);

	dir_report(entries, names.size());

	return 0;
}

static int flopdir(int argc, char *argv[])
{
	if(argc!=5) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[4]);

	const floppy_format_info *source_format = find_floppy_source_format(argv[2], ih);
	if(!source_format)
		return 1;

	auto fs = formats.find_filesystem_format_by_key(argv[3]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[3]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_read()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement reading\n", argv[3]);
		return 1;
	}

	if(ih.floppy_load(*source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	if(ih.floppy_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_dir(ih);
}

static int hddir(int argc, char *argv[])
{
	if(argc!=4) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[3]);

	auto fs = formats.find_filesystem_format_by_key(argv[2]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[2]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_read()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement reading\n", argv[2]);
		return 1;
	}

	if(ih.hd_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_dir(ih);
}


static std::error_condition dir_scan_hashes(fs::filesystem_t *fs, u32 depth, const std::vector<std::string> &path, std::vector<std::vector<std::string>> &entries)
{
	std::string head;
	for(u32 i = 0; i != depth; i++)
		head += "  ";
	auto [err, contents] = fs->directory_contents(path);
	if(err)
		return err;
	for(const auto &c : contents) {
		size_t id = entries.size();
		entries.resize(id+1);
		entries[id].resize(entries[0].size());
		auto npath = path;
		npath.push_back(c.m_name);
		switch(c.m_type) {
		case fs::dir_entry_type::dir: {
			entries[id][0] = head + "dir  " + c.m_name;
			if(std::error_condition err = dir_scan_hashes(fs, depth+1, npath, entries))
				entries[id][3] = err.message();
			break;
		}
		case fs::dir_entry_type::file: {
			entries[id][0] = head + "file " + c.m_name;
			{
				auto [err, dfork] = fs->file_read(npath);
				if(err)
					entries[id][3] = err.message();
				else {
					entries[id][1] = std::to_string(dfork.size());
					entries[id][2] = util::crc32_creator::simple(dfork.data(), dfork.size()).as_string();
					entries[id][3] = util::sha1_creator::simple(dfork.data(), dfork.size()).as_string();
				}
			}
			if(entries[id].size() > 4) {
				auto [err2, rfork] = fs->file_rsrc_read(npath);
				if(err2)
					entries[id][6] = err2.message();
				else if(!rfork.empty()) {
					entries[id][4] = std::to_string(rfork.size());
					entries[id][5] = util::crc32_creator::simple(rfork.data(), rfork.size()).as_string();
					entries[id][6] = util::sha1_creator::simple(rfork.data(), rfork.size()).as_string();
				}
			}
			break;
		}
		}
	}
	return std::error_condition();
}

static int generic_hashes(image_handler &ih)
{
	auto [fsm, fs] = ih.get_fs();

	std::vector<std::vector<std::string>> entries;

	entries.resize(1);
	entries[0].push_back("name");
	entries[0].push_back("length");
	entries[0].push_back("crc32");
	entries[0].push_back("sha1");
	if(fsm->has_rsrc()) {
		entries[0].push_back("rsrc_length");
		entries[0].push_back("rsrc_crc32");
		entries[0].push_back("rsrc_sha1");
	}

	std::error_condition err = dir_scan_hashes(fs, 0, std::vector<std::string>(), entries);
	if(err) {
		fprintf(stderr, "Error scanning directory: %s\n", err.message().c_str());
		return 1;
	}

	dir_report(entries, entries[0].size());

	return 0;
}

static int flophashes(int argc, char *argv[])
{
	if(argc!=5) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[4]);

	const floppy_format_info *source_format = find_floppy_source_format(argv[2], ih);
	if(!source_format)
		return 1;

	auto fs = formats.find_filesystem_format_by_key(argv[3]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[3]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_read()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement reading\n", argv[3]);
		return 1;
	}

	if(ih.floppy_load(*source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	if(ih.floppy_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_hashes(ih);
}

static std::string blocks_to_string(const std::vector<u32> &blocks)
{
	std::string str;
	for(u32 i = 0; i != blocks.size(); i++) {
		if(i != 0)
			str += ",";
		str += std::to_string(blocks[i]);
		if(i+1 < blocks.size()) {
			using s32 = std::int32_t;
			s32 delta = blocks[i+1] - blocks[i];
			if(delta == 1 || delta == -1) {
				u32 j = i+1;
				while(j+1 != blocks.size() && s32(blocks[j+1] - blocks[j]) == delta)
					j++;
				str += "-" + std::to_string(blocks[j]);
				i = j;
			}			
		}
	}
	return str;
}

static std::error_condition dir_scan_blocks(fs::filesystem_t *fs, u32 depth, const std::vector<std::string> &path, std::vector<std::vector<std::string>> &entries)
{
	std::string head;
	for(u32 i = 0; i != depth; i++)
		head += "  ";
	auto [err, contents] = fs->directory_contents(path);
	if(err)
		return err;
	for(const auto &c : contents) {
		size_t id = entries.size();
		entries.resize(id+1);
		entries[id].resize(entries[0].size());
		auto npath = path;
		npath.push_back(c.m_name);
		auto [err, alloc_blocks, data_blocks] = fs->enum_blocks(npath);
		switch(c.m_type) {
		case fs::dir_entry_type::dir: {
			entries[id][0] = head + "dir  " + c.m_name;
			entries[id][1] = blocks_to_string(alloc_blocks);
			if(data_blocks.empty())
				entries[id][2] = err ? err.message() : "nil";
			else
				entries[id][2] = blocks_to_string(data_blocks);
			if(std::error_condition err2 = dir_scan_blocks(fs, depth+1, npath, entries)) {
				entries.resize(id+2);
				entries[id+1].resize(entries[0].size());
				entries[id+1][2] = err2.message();
			}
			break;
		}
		case fs::dir_entry_type::file: {
			entries[id][0] = head + "file " + c.m_name;
			entries[id][1] = blocks_to_string(alloc_blocks);
			if(data_blocks.empty())
				entries[id][2] = err ? err.message() : "nil";
			else {
				entries[id][2] = blocks_to_string(data_blocks);
				if(err) {
					entries.resize(id+2);
					entries[id+1].resize(entries[0].size());
					entries[id+1][2] = err.message();
				}
			}
			break;
		}
		}
	}
	return std::error_condition();
}

static int generic_blocks(image_handler &ih)
{
	auto [fsm, fs] = ih.get_fs();

	auto [err0, alloc_blocks, data_blocks] = fs->enum_blocks(std::vector<std::string>());
	if(err0 && alloc_blocks.empty() && data_blocks.empty()) {
		fprintf(stderr, "Error scanning root directory: %s\n", err0.message().c_str());
		return 1;
	}

	std::vector<std::vector<std::string>> entries(2);
	entries[0].push_back("name");
	entries[0].push_back("alloc_blocks");
	entries[0].push_back("data_blocks");
	entries[1].push_back("root");
	entries[1].push_back(blocks_to_string(alloc_blocks));
	entries[1].push_back(blocks_to_string(data_blocks));

	std::error_condition err = dir_scan_blocks(fs, 0, std::vector<std::string>(), entries);
	if(err) {
		fprintf(stderr, "Error scanning directory: %s\n", err.message().c_str());
		return 1;
	}

	dir_report(entries, entries[0].size());

	return 0;
}

static int flopblocks(int argc, char *argv[])
{
	if(argc!=5) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[4]);

	const floppy_format_info *source_format = find_floppy_source_format(argv[2], ih);
	if(!source_format)
		return 1;

	auto fs = formats.find_filesystem_format_by_key(argv[3]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[3]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_read()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement reading\n", argv[3]);
		return 1;
	}

	if(ih.floppy_load(*source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	if(ih.floppy_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_blocks(ih);
}

static int hdhashes(int argc, char *argv[])
{
	if(argc!=4) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[3]);

	auto fs = formats.find_filesystem_format_by_key(argv[2]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[2]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_read()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement reading\n", argv[2]);
		return 1;
	}

	if(ih.hd_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_hashes(ih);
}

static int hdblocks(int argc, char *argv[])
{
	if(argc!=4) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[3]);

	auto fs = formats.find_filesystem_format_by_key(argv[2]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[2]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_read()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement reading\n", argv[2]);
		return 1;
	}

	if(ih.hd_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_blocks(ih);
}


static int generic_read(image_handler &ih, const char *srcpath, const char *dstpath)
{
	auto [fsm, fs] = ih.get_fs();
	std::ignore = fsm;

	std::vector<std::string> path = ih.path_split(srcpath);
	auto [err, dfork] = fs->file_read(path);
	if(err) {
		fprintf(stderr, "File reading failed: %s\n", err.message().c_str());
		return 1;
	}

	image_handler::fsave(dstpath, dfork);

	auto [err2, rfork] = fs->file_rsrc_read(path);
	if(!err2 && !rfork.empty())
		image_handler::fsave_rsrc(image_handler::path_make_rsrc(dstpath), rfork);

	return 0;
}


static int flopread(int argc, char *argv[])
{
	if(argc!=7) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[4]);

	const floppy_format_info *source_format = find_floppy_source_format(argv[2], ih);
	if(!source_format)
		return 1;

	auto fs = formats.find_filesystem_format_by_key(argv[3]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[3]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_read()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement reading\n", argv[3]);
		return 1;
	}

	if(ih.floppy_load(*source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	if(ih.floppy_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_read(ih, argv[5], argv[6]);
}

static int hdread(int argc, char *argv[])
{
	if(argc!=6) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[3]);

	auto fs = formats.find_filesystem_format_by_key(argv[2]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[2]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_read()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement reading\n", argv[2]);
		return 1;
	}

	if(ih.hd_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_read(ih, argv[4], argv[5]);
}



static int generic_write(image_handler &ih, const char *srcpath, const char *dstpath)
{
	auto [fsm, fs] = ih.get_fs();

	std::vector<std::string> path = ih.path_split(dstpath);
	auto [err, meta] = fs->metadata(path);
	std::ignore = meta;

	if(err == fs::error::not_found) {
		fs::meta_data meta;
		meta.set(fs::meta_name::name, path.back());
		auto dpath = path;
		dpath.pop_back();
		err = fs->file_create(dpath, meta);
		if(err) {
			fprintf(stderr, "File creation failed: %s\n", err.message().c_str());
			return 1;
		}
	}

	auto dfork = image_handler::fload(srcpath);
	err = fs->file_write(path, dfork);
	if(err) {
		fprintf(stderr, "File writing failed: %s\n", err.message().c_str());
		return 1;
	}

	if(fsm->has_rsrc()) {
		std::string rpath = image_handler::path_make_rsrc(dstpath);

		if(image_handler::fexists(rpath)) {
			auto rfork = image_handler::fload_rsrc(rpath);
			if(!rfork.empty()) {
				err = fs->file_rsrc_write(path, rfork);
				fprintf(stderr, "File resource fork writing failed: %s\n", err.message().c_str());
				return 1;
			}
		}
	}

	return 0;
}


static int flopwrite(int argc, char *argv[])
{
	if(argc!=7) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[4]);

	const floppy_format_info *source_format = find_floppy_source_format(argv[2], ih);
	if(!source_format)
		return 1;

	auto fs = formats.find_filesystem_format_by_key(argv[3]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[3]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_write()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement writing\n", argv[3]);
		return 1;
	}

	if(ih.floppy_load(*source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	if(ih.floppy_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	int err = generic_write(ih, argv[5], argv[6]);
	if(err)
		return err;

	ih.fs_to_floppy();
	if(ih.floppy_save(*source_format))
		return 1;

	return 0;
}

static int hdwrite(int argc, char *argv[])
{
	if(argc!=6) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[3]);

	auto fs = formats.find_filesystem_format_by_key(argv[2]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[2]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_write()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement writing\n", argv[2]);
		return 1;
	}

	if(ih.hd_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	// FIXME: this doesn't actually save anything
	return generic_write(ih, argv[4], argv[5]);
}


static int generic_chmeta(image_handler &ih, fs::meta_data &&meta, bool isfile, const char *srcpath)
{
	auto [fsm, fs] = ih.get_fs();

	const auto metad = isfile ? fsm->file_meta_description() : fsm->volume_meta_description();
	for(const auto &nv : meta.meta) {
		auto it = std::find_if(metad.begin(), metad.end(), [&nv] (const fs::meta_description &desc) { return desc.m_name == nv.first; });
		if(it == metad.end()) {
			fprintf(stderr, "Error: Filesystem '%s' does not provide %s metadata for %s\n",
					fsm->name(),
					fs::meta_data::entry_name(nv.first),
					isfile ? "files" : "volumes");
			return 1;
		}
		if(it->m_ro) {
			fprintf(stderr, "Error: Filesystem '%s' does not permit changing %s metadata on a %s\n",
					fsm->name(),
					fs::meta_data::entry_name(nv.first),
					isfile ? "file" : "volume");
			return 1;
		}
		if(it->m_validator && !it->m_validator(nv.second)) {
			fprintf(stderr, "Error: Filesystem '%s' does not recognize '%s' as valid %s metadata (%s)\n",
					fsm->name(),
					nv.second.as_string().c_str(),
					fs::meta_data::entry_name(nv.first),
					it->m_tooltip);
			return 1;
		}
	}

	std::error_condition err;
	if(isfile) {
		std::vector<std::string> path = ih.path_split(srcpath);
		err = fs->metadata_change(path, meta);
	} else
		err = fs->volume_metadata_change(meta);

	if(err) {
		fprintf(stderr, "%s metadata change failed: %s\n", isfile ? "File" : "Volume", err.message().c_str());
		return 1;
	}

	return 0;
}

static int flopchmeta(int argc, char *argv[])
{
	fs::meta_data meta = extract_meta_data(argc, argv);

	if(argc!=5 && argc!=6) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[4]);

	const floppy_format_info *source_format = find_floppy_source_format(argv[2], ih);
	if(!source_format)
		return 1;

	auto fs = formats.find_filesystem_format_by_key(argv[3]);
	if(!fs || !fs->m_manager) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[3]);
		return 1;
	}

	if(ih.floppy_load(*source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	if(ih.floppy_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	int err = generic_chmeta(ih, std::move(meta), argc==6, argc==6 ? argv[5] : "");
	if(err)
		return err;

	ih.fs_to_floppy();
	if(ih.floppy_save(*source_format))
		return 1;

	return 0;
}


static int floprename(int argc, char *argv[])
{
	if(argc!=7) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[4]);

	const floppy_format_info *source_format = find_floppy_source_format(argv[2], ih);
	if(!source_format)
		return 1;

	auto fs = formats.find_filesystem_format_by_key(argv[3]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[3]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_write()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement writing\n", argv[2]);
		return 1;
	}

	if(ih.floppy_load(*source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	if(ih.floppy_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	std::vector<std::string> opath = ih.path_split(argv[5]);
	std::vector<std::string> npath = ih.path_split(argv[6]);
	std::error_condition err = ih.get_fs().second->rename(opath, npath);
	if(err) {
		fprintf(stderr, "Renaming failed: %s\n", err.message().c_str());
		return 1;
	}

	ih.fs_to_floppy();
	if(ih.floppy_save(*source_format))
		return 1;

	return 0;
}

static int flopremove(int argc, char *argv[])
{
	if(argc!=6) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	image_handler ih;
	ih.set_on_disk_path(argv[4]);

	const floppy_format_info *source_format = find_floppy_source_format(argv[2], ih);
	if(!source_format)
		return 1;

	auto fs = formats.find_filesystem_format_by_key(argv[3]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[3]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_write()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement writing\n", argv[2]);
		return 1;
	}

	if(ih.floppy_load(*source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	if(ih.floppy_mount_fs(*fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	std::vector<std::string> path = ih.path_split(argv[5]);
	std::error_condition err = ih.get_fs().second->remove(path);
	if(err) {
		fprintf(stderr, "Deletion failed: %s\n", err.message().c_str());
		return 1;
	}

	ih.fs_to_floppy();
	if(ih.floppy_save(*source_format))
		return 1;

	return 0;
}

static int help(int argc, char *argv[])
{
	if(argc!=2 && argc!=3) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(stderr, argv[0], argv[1]);
		return 1;
	}

	if(argc==2 || !strcmp(argv[2], "commands")) {
		display_standard_usage(argv);
		return 0;
	}

	if(!strcmp(argv[2], "all")) {
		display_standard_usage(argv);
		printf("\n");
		display_formats();
		printf("\n\n");
		display_filesystems();
		return 0;
	}

	if(argv[2][0] == '.')
		return display_formats_by_extension(argv[2]);

	if(!strcmp(argv[2], "formats")) {
		display_formats();
		return 0;
	}

	if(!strcmp(argv[2], "filesystems")) {
		display_filesystems();
		return 0;
	}

	if(std::find_if(std::begin(s_command_usage), std::end(s_command_usage), [argv] (const command_info &info) { return !strcmp(info.name, argv[2]); }) != std::end(s_command_usage)) {
		display_usage(stdout, argv[0], argv[2]);
		return 0;
	}

	auto fmtinfo = formats.find_floppy_format_info_by_key(argv[2]);
	if(fmtinfo) {
		display_format_info(*fmtinfo);
		return 0;
	}

	auto fs = formats.find_filesystem_format_by_key(argv[2]);
	if(!fs) {
		auto create_fs = formats.find_floppy_create_info_by_key(argv[2]);
		if(create_fs)
			fs = formats.find_filesystem_format_by_key(create_fs->m_manager->name());
	}
	if(fs) {
		display_filesystem_info(*fs);
		return 0;
	}

	fprintf(stderr, "No help available for \"%s\".\n", argv[2]);
	return 1;
}

static int version(int argc, char *argv[])
{
	extern const char build_version[];
	printf("%s\n", build_version);
	return 0;
}

int CLIB_DECL main(int argc, char *argv[])
{
	formats.init();

	if(argc == 1) {
		display_standard_usage(argv);
		return 0;
	}

	try {
		if(!core_stricmp("identify", argv[1]))
			return identify(argc, argv);
		else if(!core_stricmp("flopconvert", argv[1]))
			return flopconvert(argc, argv);
		else if(!core_stricmp("flopcreate", argv[1]))
			return flopcreate(argc, argv);
		else if(!core_stricmp("flopdir", argv[1]))
			return flopdir(argc, argv);
		else if(!core_stricmp("flophashes", argv[1]))
			return flophashes(argc, argv);
		else if(!core_stricmp("flopblocks", argv[1]))
			return flopblocks(argc, argv);
		else if(!core_stricmp("flopread", argv[1]))
			return flopread(argc, argv);
		else if(!core_stricmp("flopwrite", argv[1]))
			return flopwrite(argc, argv);
		else if(!core_stricmp("flopchmeta", argv[1]))
			return flopchmeta(argc, argv);
		else if(!core_stricmp("floprename", argv[1]))
			return floprename(argc, argv);
		else if(!core_stricmp("flopremove", argv[1]))
			return flopremove(argc, argv);
		else if(!core_stricmp("hddir", argv[1]))
			return hddir(argc, argv);
		else if(!core_stricmp("hdhashes", argv[1]))
			return hdhashes(argc, argv);
		else if(!core_stricmp("hdblocks", argv[1]))
			return hdblocks(argc, argv);
		else if(!core_stricmp("hdread", argv[1]))
			return hdread(argc, argv);
		else if(!core_stricmp("hdwrite", argv[1]))
			return hdwrite(argc, argv);
		else if(!core_stricmp("help", argv[1]))
			return help(argc, argv);
		else if (!core_stricmp("version", argv[1]))
			return version(argc, argv);
		else {
			fprintf(stderr, "Unknown command '%s'\n\n", argv[1]);
			display_usage(stderr, argv[0], nullptr);
			return 1;
		}
	} catch(const std::exception &err) {
		fprintf(stderr, "Error: %s\n", err.what());
		return 1;
	}
}
