// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    (Floppy) image command-line manager

***************************************************************************/

#include "image_handler.h"

#include "corefile.h"
#include "corestr.h"
#include "ioprocs.h"
#include "strformat.h"

#include "osdcomm.h"

#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>


static formats_table formats;

static void display_usage(const char *first_argument)
{
	std::string exe_name(core_filename_extract_base(first_argument));

	fprintf(stderr, "Usage: \n");
	fprintf(stderr, "       %s identify <inputfile> [<inputfile> ...]                                 -- Identify an image format\n", exe_name.c_str());
	fprintf(stderr, "       %s flopconvert [input_format|auto] output_format <inputfile> <outputfile> -- Convert a floppy image\n", exe_name.c_str());
	fprintf(stderr, "       %s flopcreate output_format filesystem <outputfile>                       -- Create a preformatted floppy image\n", exe_name.c_str());
	fprintf(stderr, "       %s flopdir input_format filesystem <image>                                -- List the contents of a floppy image\n", exe_name.c_str());
	fprintf(stderr, "       %s flopread input_format filesystem <image> <path> <outputfile>           -- Extract a file from a floppy image\n", exe_name.c_str());
	fprintf(stderr, "       %s flopwrite input_format filesystem <image> <inputfile> <path>           -- Write a file into a floppy image\n", exe_name.c_str());
	fprintf(stderr, "       %s hddir filesystem <image>                                               -- List the contents of a hard disk image\n", exe_name.c_str());
	fprintf(stderr, "       %s hdread filesystem <image> <path> <outputfile>                          -- Extract a file from a hard disk image\n", exe_name.c_str());
	fprintf(stderr, "       %s hdwrite filesystem <image> <inputfile> <path>                          -- Write a file into a hard disk image\n", exe_name.c_str());
	fprintf(stderr, "       %s version                                                                -- Display the current version of floptool\n", exe_name.c_str());
}

static void display_formats()
{
	int sk = 0;
	for(const auto &e : formats.floppy_format_info_by_key) {
		int sz = e.first.size();
		if(sz > sk)
			sk = sz;
	}

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

	fprintf(stderr, "Supported floppy formats:\n\n");
	for(const auto &e : formats.floppy_format_info_by_category)
		if(!e.second.empty()) {
			fprintf(stderr, "%s:\n", e.first.c_str());
			for(auto *fif : e.second)
				fprintf(stderr, "  %-*s     r%c - %s [%s]\n", sk, fif->m_format->name(), fif->m_format->supports_save() ? 'w' : '-', fif->m_format->description(), fif->m_format->extensions());
		}

	fprintf(stderr, "\n\n");
	fprintf(stderr, "Supported filesystems (with floppy formatting names):\n\n");
	for(const auto &e : formats.filesystem_format_by_category)
		if(!e.second.empty()) {
			fprintf(stderr, "%s:\n", e.first.c_str());
			for(const auto &f : e.second) {
				fprintf(stderr, "  %-*s %c%c%c %c%c%c - %s\n",
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
					fprintf(stderr, "    %-*s         - %s\n",
							sk,
							f2->m_name,
							f2->m_description);
			}
		}
}

static void display_full_usage(char *argv[])
{
	/* Usage */
	fprintf(stderr, "floptool - Generic floppy image manipulation tool for use with MAME\n\n");
	display_usage(argv[0]);
	fprintf(stderr, "\n");
	display_formats();
}

static int identify(int argc, char *argv[])
{
	// Need to detect CHDs too

	if(argc<3) {
		fprintf(stderr, "Missing name of file to identify.\n\n");
		display_usage(argv[0]);
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
			printf("%-*s %c %3d - %-*s %s\n", sz, first ? argv[i] : "", first ? ':' : ' ', e.first, sz2, e.second->m_format->name(), e.second->m_format->description());
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
		display_usage(argv[0]);
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

	if(ih.floppy_load(source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	ih.set_on_disk_path(argv[5]);

	if(ih.floppy_save(dest_format)) {
		fprintf(stderr, "Error: Saving as format '%s' failed\n", dest_format->m_format->name());
		return 1;
	}

	return 0;
}

static int flopcreate(int argc, char *argv[])
{
	if(argc!=5) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(argv[0]);
		return 1;
	}

	auto dest_format = formats.find_floppy_format_info_by_key(argv[2]);
	if(!dest_format) {
		fprintf(stderr, "Error: Floppy format '%s' unknown\n", argv[3]);
		return 1;
	}
	if(!dest_format->m_format->supports_save()) {
		fprintf(stderr, "Error: Saving to format '%s' unsupported\n", argv[3]);
		return 1;
	}

	auto create_fs = formats.find_floppy_create_info_by_key(argv[3]);
	if(!create_fs) {
		fprintf(stderr, "Error: Floppy creation format '%s' unknown\n", argv[3]);
		return 1;
	}
	if(!create_fs->m_manager->can_format()) {
		fprintf(stderr, "Error: Floppy creation format '%s' does not support creating new images\n", argv[3]);
		return 1;
	}

	fs::meta_data meta;
	image_handler ih;
	ih.set_on_disk_path(argv[4]);

	ih.floppy_create(create_fs, meta);
	return ih.floppy_save(dest_format);
}

static void dir_scan(u32 depth, fs::filesystem_t::dir_t dir, std::vector<std::vector<std::string>> &entries, const std::unordered_map<fs::meta_name, size_t> &nmap, size_t nc, const std::vector<fs::meta_description> &dmetad, const std::vector<fs::meta_description> &fmetad)
{
	std::string head;
	for(u32 i = 0; i != depth; i++)
		head += "  ";
	auto contents = dir.contents();
	for(const auto &c : contents) {
		size_t id = entries.size();
		entries.resize(id+1);
		entries[id].resize(nc);
		switch(c.m_type) {
		case fs::dir_entry_type::dir: {
			auto subdir = dir.dir_get(c.m_key);
			auto meta = subdir.metadata();
			if (!meta.has(fs::meta_name::name))
				meta.set(fs::meta_name::name, c.m_name);
			for(const auto &m : dmetad) {
				if(!meta.has(m.m_name))
					continue;
				size_t slot = nmap.find(m.m_name)->second;
				std::string val = fs::meta_value::to_string(m.m_type, meta.get(m.m_name));
				if(slot == 0)
					val = head + "dir  " + val;
				entries[id][slot] = val;
			}
			dir_scan(depth+1, subdir, entries, nmap, nc, dmetad, fmetad);
			break;
		}
		case fs::dir_entry_type::file:
		case fs::dir_entry_type::system_file: {
			auto file = dir.file_get(c.m_key);
			auto meta = file.metadata();
			if (!meta.has(fs::meta_name::name))
				meta.set(fs::meta_name::name, c.m_name);
			for(const auto &m : fmetad) {
				if(!meta.has(m.m_name))
					continue;
				size_t slot = nmap.find(m.m_name)->second;
				std::string val = fs::meta_value::to_string(m.m_type, meta.get(m.m_name));
				if(slot == 0)
					val = head + (c.m_type == fs::dir_entry_type::system_file ? "sys  " : "file ") + val;
				entries[id][slot] = val;
			}
			break;
		}
		}
	}
}

static int generic_dir(image_handler &ih)
{
	auto [fsm, fs] = ih.get_fs();
	auto vmetad = fsm->volume_meta_description();
	auto fmetad = fsm->file_meta_description();
	auto dmetad = fsm->directory_meta_description();

	auto vmeta = fs->metadata();
	if(!vmeta.empty()) {
		std::string vinf = "Volume:";
		for(const auto &e : vmetad)
			vinf += util::string_format(" %s=%s", fs::meta_data::entry_name(e.m_name), fs::meta_value::to_string(e.m_type, vmeta.get(e.m_name)));
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

	auto root = fs->root();
	std::vector<std::vector<std::string>> entries;

	entries.resize(1);
	for(fs::meta_name n : names)
		entries[0].push_back(fs::meta_data::entry_name(n));

	dir_scan(0, root, entries, nmap, names.size(), dmetad, fmetad);

	std::vector<u32> sizes(names.size());

	for(const auto &e : entries)
		for(unsigned int i=0; i != names.size(); i++)
			sizes[i] = std::max<u32>(sizes[i], e[i].size());

	for(const auto &e : entries) {
		std::string l;
		for(unsigned int i=0; i != names.size(); i++) {
			if(i)
				l += ' ';
			l += util::string_format("%-*s", sizes[i], e[i]);
		}
		printf("%s\n", l.c_str());
	}

	return 0;
}

static int flopdir(int argc, char *argv[])
{
	if(argc!=5) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(argv[0]);
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

	if(ih.floppy_load(source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	if(ih.floppy_mount_fs(fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_dir(ih);
}

static int hddir(int argc, char *argv[])
{
	if(argc!=4) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(argv[0]);
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

	if(ih.hd_mount_fs(fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_dir(ih);
}


static int generic_read(image_handler &ih, const char *srcpath, const char *dstpath)
{
	auto [fsm, fs] = ih.get_fs();

	std::vector<std::string> path = ih.path_split(srcpath);

	auto dir = fs->root();
	std::string apath;
	for(unsigned int i = 0; i < path.size() - 1; i++) {
		auto c = dir.contents();
		unsigned int j;
		for(j = 0; j != c.size(); j++)
			if(c[j].m_name == path[i])
				break;
		if(j == c.size()) {
			fprintf(stderr, "Error: directory %s%c%s not found\n", apath.c_str(), fsm->directory_separator(), path[i].c_str());
			return 1;
		}
		if(c[j].m_type != fs::dir_entry_type::dir) {
			fprintf(stderr, "Error: %s%c%s is not a directory\n", apath.c_str(), fsm->directory_separator(), path[i].c_str());
			return 1;
		}
		dir = dir.dir_get(c[j].m_key);
		apath += fsm->directory_separator() + path[i];
	}

	auto c = dir.contents();
	unsigned int j;
	for(j = 0; j != c.size(); j++)
		if(c[j].m_name == path.back())
			break;
	if(j == c.size()) {
		fprintf(stderr, "Error: file %s%c%s not found\n", apath.c_str(), fsm->directory_separator(), path.back().c_str());
		return 1;
	}
	auto file = dir.file_get(c[j].m_key);
	auto meta = file.metadata();

	if(!meta.has(fs::meta_name::length)) {
		fprintf(stderr, "Error: %s%c%s is not a readable file\n", apath.c_str(), fsm->directory_separator(), path.back().c_str());
		return 1;
	}

	image_handler::fsave(dstpath, file.read_all());

	bool has_rsrc = fsm->has_rsrc() && meta.has(fs::meta_name::rsrc_length);

	if(has_rsrc)
		image_handler::fsave_rsrc(image_handler::path_make_rsrc(dstpath), file.rsrc_read_all());

	return 0;
}


static int flopread(int argc, char *argv[])
{
	if(argc!=7) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(argv[0]);
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

	if(ih.floppy_load(source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	if(ih.floppy_mount_fs(fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_read(ih, argv[5], argv[6]);
}

static int hdread(int argc, char *argv[])
{
	if(argc!=6) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(argv[0]);
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

	if(ih.hd_mount_fs(fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_read(ih, argv[4], argv[5]);
}



static int generic_write(image_handler &ih, const char *srcpath, const char *dstpath)
{
	auto [fsm, fs] = ih.get_fs();

	std::vector<std::string> path = ih.path_split(dstpath);

	auto dir = fs->root();
	std::string apath;
	for(unsigned int i = 0; i < path.size() - 1; i++) {
		auto c = dir.contents();
		unsigned int j;
		for(j = 0; j != c.size(); j++)
			if(c[j].m_name == path[i])
				break;
		if(j == c.size()) {
			fprintf(stderr, "Error: directory %s%c%s not found\n", apath.c_str(), fsm->directory_separator(), path[i].c_str());
			return 1;
		}
		if(c[j].m_type != fs::dir_entry_type::dir) {
			fprintf(stderr, "Error: %s%c%s is not a directory\n", apath.c_str(), fsm->directory_separator(), path[i].c_str());
			return 1;
		}
		dir = dir.dir_get(c[j].m_key);
		apath += fsm->directory_separator() + path[i];
	}


	fs::meta_data meta;
	meta.set(fs::meta_name::name, path.back());

	auto file = dir.file_create(meta);
	auto filedata = image_handler::fload(srcpath);
	file.replace(filedata);

	bool has_rsrc = fsm->has_rsrc();

	if(has_rsrc) {
		std::string rpath = image_handler::path_make_rsrc(dstpath);

		if(image_handler::fexists(rpath)) {
			filedata = image_handler::fload_rsrc(rpath);
			if(!filedata.empty())
				file.rsrc_replace(filedata);
		}
	}

	return 0;
}


static int flopwrite(int argc, char *argv[])
{
	if(argc!=7) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(argv[0]);
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

	if(ih.floppy_load(source_format)) {
		fprintf(stderr, "Error: Loading as format '%s' failed\n", source_format->m_format->name());
		return 1;
	}

	if(ih.floppy_mount_fs(fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	int err = generic_write(ih, argv[5], argv[6]);
	if(err)
		return err;

	ih.fs_to_floppy();
	if(ih.floppy_save(source_format))
		return 1;

	return 0;
}

static int hdwrite(int argc, char *argv[])
{
	if(argc!=6) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage(argv[0]);
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

	if(ih.hd_mount_fs(fs)) {
		fprintf(stderr, "Error: Parsing as filesystem '%s' failed\n", fs->m_manager->name());
		return 1;
	}

	return generic_write(ih, argv[4], argv[5]);
}

static int version(int argc, char *argv[])
{
	extern const char build_version[];
	fprintf(stdout, "%s\n", build_version);
	return 0;
}

int CLIB_DECL main(int argc, char *argv[])
{
	formats.init();

	if(argc == 1) {
		display_full_usage(argv);
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
		else if(!core_stricmp("flopread", argv[1]))
			return flopread(argc, argv);
		else if(!core_stricmp("flopwrite", argv[1]))
			return flopwrite(argc, argv);
		else if(!core_stricmp("hddir", argv[1]))
			return hddir(argc, argv);
		else if(!core_stricmp("hdread", argv[1]))
			return hdread(argc, argv);
		else if(!core_stricmp("hdwrite", argv[1]))
			return hdwrite(argc, argv);
		else if (!core_stricmp("version", argv[1]))
			return version(argc, argv);
		else {
			fprintf(stderr, "Unknown command '%s'\n\n", argv[1]);
			display_usage(argv[0]);
			return 1;
		}
	} catch(const std::exception &err) {
		fprintf(stderr, "Error: %s", err.what());
		return 1;
	}
}
