// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    main.c

    Floptool command line front end

    20/07/2011 Initial version by Miodrag Milanovic

***************************************************************************/

#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <cstdarg>
#include <cassert>
#include <map>

#include "../emu/emucore.h"
#include "corestr.h"
#include "osdcomm.h"

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

#include "formats/all.h"
#include "formats/fs_unformatted.h"
#include "formats/fsblk_vec.h"

emu_fatalerror::emu_fatalerror(util::format_argument_pack<std::ostream> const &args)
	: emu_fatalerror(0, args)
{
}

emu_fatalerror::emu_fatalerror(int _exitcode, util::format_argument_pack<std::ostream> const &args)
	: m_text(util::string_format(args))
	, m_code(_exitcode)
{
}

struct fs_info {
	const filesystem_manager_t *m_manager;
	floppy_format_type m_type;
	u32 m_image_size;
	const char *m_name;
	u32 m_key;
	const char *m_description;

	fs_info(const filesystem_manager_t *manager, floppy_format_type type, u32 image_size, const char *name, const char *description) :
		m_manager(manager),
		m_type(type),
		m_image_size(image_size),
		m_name(name),
		m_key(0),
		m_description(description)
	{}

	fs_info(const char *name, u32 key, const char *description) :
		m_manager(nullptr),
		m_type(nullptr),
		m_image_size(0),
		m_name(name),
		m_key(key),
		m_description(description)
	{}
};

struct iofile_ram {
	std::vector<u8> *data;
	int64_t pos;
};

static void ram_closeproc(void *file)
{
	auto f = (iofile_ram *)file;
	delete f;
}

static int ram_seekproc(void *file, int64_t offset, int whence)
{
	auto f = (iofile_ram *)file;
	switch(whence) {
	case SEEK_SET: f->pos = offset; break;
	case SEEK_CUR: f->pos += offset; break;
	case SEEK_END: f->pos = f->data->size() + offset; break;
	}

	if(whence == SEEK_CUR)
		f->pos = std::max<int64_t>(f->pos, 0);
	else
		f->pos = std::clamp<int64_t>(f->pos, 0, f->data->size());
	return 0;
}

static size_t ram_readproc(void *file, void *buffer, size_t length)
{
	auto f = (iofile_ram *)file;
	size_t l = std::min<std::common_type_t<size_t, int64_t> >(length, f->data->size() - f->pos);
	memcpy(buffer, f->data->data() + f->pos, l);
	return l;
}

static size_t ram_writeproc(void *file, const void *buffer, size_t length)
{
	auto f = (iofile_ram *)file;
	size_t l = std::max<std::common_type_t<size_t, int64_t> >(f->pos + length, f->data->size());
	f->data->resize(l);
	memcpy(f->data->data() + f->pos, buffer, length);
	return length;
}

static uint64_t ram_filesizeproc(void *file)
{
	auto f = (iofile_ram *)file;
	return f->data->size();
}


static const io_procs iop_ram = {
	ram_closeproc,
	ram_seekproc,
	ram_readproc,
	ram_writeproc,
	ram_filesizeproc
};

static io_generic *ram_open(std::vector<u8> &data)
{
	iofile_ram *f = new iofile_ram;
	f->data = &data;
	f->pos = 0;
	return new io_generic({ &iop_ram, f });
}

std::map<std::string, std::vector<floppy_image_format_t *>> formats_by_category;
std::map<std::string, floppy_image_format_t *> formats_by_key;

std::map<std::string, std::vector<fs_info>> fs_by_category;
std::map<std::string, fs_info> fs_by_key;

static std::vector<uint32_t> variants;

struct enumerator;

struct fs_enum : public filesystem_manager_t::floppy_enumerator {
	enumerator *m_en;
	fs_enum(enumerator *en) : filesystem_manager_t::floppy_enumerator(), m_en(en) {};

	void reg(const fs_info &fsi) const;
	virtual void add(const filesystem_manager_t *manager, floppy_format_type type, u32 image_size, const char *name, const char *description) override;
	virtual void add_raw(const char *name, u32 key, const char *description) override;
};

struct enumerator : public mame_formats_enumerator {
	fs_enum fse;

	enumerator() : mame_formats_enumerator(), fse(this) {}

	virtual ~enumerator() = default;
	virtual void add(const cassette_image::Format *const *formats) {}

	std::vector<floppy_image_format_t *> *cf = nullptr;
	std::vector<fs_info> *cfs = nullptr;
	virtual void category(const char *name) {
		auto i = formats_by_category.find(name);
		if(i != formats_by_category.end()) {
			fprintf(stderr, "Collision on category name %s\n", name);
			exit(1);
		}
		cf = &formats_by_category[name];
		cfs = &fs_by_category[name];
	}

	virtual void add(floppy_format_type format) {
		auto f = format();
		std::string key = f->name();
		auto i = formats_by_key.find(key);
		if(i != formats_by_key.end()) {
			fprintf(stderr, "Collision on format key %s between \"%s\" and \"%s\".\n",
					key.c_str(),
					i->second->description(),
					f->description());
			exit(1);
		}
		cf->push_back(f);
		formats_by_key[key] = f;
	}

	virtual void add(filesystem_manager_type fs) {
		auto ff = fs();
		ff->enumerate_f(fse, floppy_image::FF_UNKNOWN, variants);
	}
};

void fs_enum::reg(const fs_info &fsi) const
{
	std::string key = fsi.m_name;
	auto i = fs_by_key.find(key);
	if(i != fs_by_key.end()) {
		fprintf(stderr, "Collision on fs key %s between \"%s\" and \"%s\".\n",
				key.c_str(),
				i->second.m_description,
				fsi.m_description);
		exit(1);
	}
	m_en->cfs->push_back(fsi);
	fs_by_key.emplace(key, fsi);
}

void fs_enum::add(const filesystem_manager_t *manager, floppy_format_type type, u32 image_size, const char *name, const char *description)
{
	fs_info fsi(manager, type, image_size, name, description);
	reg(fsi);
}

void fs_enum::add_raw(const char *name, u32 key, const char *description)
{
	fs_info fsi(name, key, description);
	reg(fsi);
}

void CLIB_DECL ATTR_PRINTF(1,2) logerror(const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vprintf(format, arg);
	va_end(arg);
}


static void init_formats()
{
	enumerator en;
	mame_formats_full_list(en);
}

static floppy_image_format_t *find_format_by_name(const char *name)
{
	auto i = formats_by_key.find(name);
	if(i == formats_by_key.end())
		return nullptr;
	return i->second;
}

static floppy_image_format_t *find_format_by_identify(io_generic *image)
{
	int best = 0;
	floppy_image_format_t *best_fif = nullptr;

	for(const auto &e : formats_by_key) {
		floppy_image_format_t *fif = e.second;
		int score = fif->identify(image, floppy_image::FF_UNKNOWN, variants);
		if(score > best) {
			best = score;
			best_fif = fif;
		}
	}
	return best_fif;
}

static const fs_info *find_fs_by_name(const char *name)
{
	auto i = fs_by_key.find(name);
	if(i == fs_by_key.end())
		return nullptr;
	return &i->second;
}


static void display_usage()
{
	fprintf(stderr, "Usage: \n");
	fprintf(stderr, "       floptool.exe identify <inputfile> [<inputfile> ...]\n");
	fprintf(stderr, "       floptool.exe convert [input_format|auto] output_format <inputfile> <outputfile>\n");
	fprintf(stderr, "       floptool.exe create output_format filesystem <outputfile>\n");
	fprintf(stderr, "       floptool.exe dir input_format filesystem <inputfile>\n");
}

static void display_formats()
{
	int sk = 0;
	for(const auto &e : formats_by_key) {
		int sz = e.first.size();
		if(sz > sk)
			sk = sz;
	}
	for(const auto &e : fs_by_key) {
		int sz = e.first.size();
		if(sz > sk)
			sk = sz;
	}

	fprintf(stderr, "Supported formats:\n\n");
	for(const auto &e : formats_by_category)
		if(!e.second.empty()) {
			fprintf(stderr, "%s:\n", e.first.c_str());
			for(floppy_image_format_t *fif : e.second)
				fprintf(stderr, "  %-*s     - %s [%s]\n", sk, fif->name(), fif->description(), fif->extensions());
		}

	fprintf(stderr, "\n\n");
	fprintf(stderr, "Supported filesystems:\n\n");
	for(const auto &e : fs_by_category)
		if(!e.second.empty()) {
			fprintf(stderr, "%s:\n", e.first.c_str());
			for(const fs_info &fs : e.second)
				fprintf(stderr, "  %-*s %c%c%c - %s\n",
						sk,
						fs.m_name,
						!fs.m_manager || fs.m_manager->can_format() ? 'f' : '-',
						fs.m_manager && fs.m_manager->can_read() ? 'r' : '-',
						fs.m_manager && fs.m_manager->can_write() ? 'w' : '-',
						fs.m_description);
		}
}

static void display_full_usage()
{
	/* Usage */
	fprintf(stderr, "floptool - Generic floppy image manipulation tool for use with MAME\n\n");
	display_usage();
	fprintf(stderr, "\n");
	display_formats();
	fprintf(stderr, "\nExample usage:\n");
	fprintf(stderr, "        floptool.exe identify image.dsk\n\n");

}

static int identify(int argc, char *argv[])
{
	if (argc<3) {
		fprintf(stderr, "Missing name of file to identify.\n\n");
		display_usage();
		return 1;
	}

	for(int i=2; i<argc; i++) {
		char msg[4096];
		sprintf(msg, "Error opening %s for reading", argv[i]);
		FILE *f = fopen(argv[i], "rb");
		if (!f) {
			perror(msg);
			return 1;
		}
		io_generic io;
		io.file = f;
		io.procs = &stdio_ioprocs_noclose;
		io.filler = 0xff;

		floppy_image_format_t *best_fif = find_format_by_identify(&io);
		if (best_fif)
			printf("%s : %s\n", argv[i], best_fif->description());
		else
			printf("%s : Unknown format\n", argv[i]);
		fclose(f);
	}
	return 0;
}

static int convert(int argc, char *argv[])
{
	if (argc!=6) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage();
		return 1;
	}

	floppy_image_format_t *source_format, *dest_format;

	char msg[4096];
	sprintf(msg, "Error opening %s for reading", argv[4]);
	FILE *f = fopen(argv[4], "rb");
	if (!f) {
		perror(msg);
		return 1;
	}
	io_generic source_io;
	source_io.file = f;
	source_io.procs = &stdio_ioprocs_noclose;
	source_io.filler = 0xff;

	if(!core_stricmp(argv[2], "auto")) {
		source_format = find_format_by_identify(&source_io);
		if(!source_format) {
			fprintf(stderr, "Error: Could not identify the format of file %s\n", argv[4]);
			return 1;
		}

	} else {
		source_format = find_format_by_name(argv[2]);
		if(!source_format) {
			fprintf(stderr, "Error: Format '%s' unknown\n", argv[2]);
			return 1;
		}
	}

	dest_format = find_format_by_name(argv[3]);
	if(!dest_format) {
		fprintf(stderr, "Error: Format '%s' unknown\n", argv[3]);
		return 1;
	}
	if(!dest_format->supports_save()) {
		fprintf(stderr, "Error: saving to format '%s' unsupported\n", argv[3]);
		return 1;
	}

	sprintf(msg, "Error opening %s for writing", argv[5]);
	f = fopen(argv[5], "wb");
	if (!f) {
		perror(msg);
		return 1;
	}
	io_generic dest_io;
	dest_io.file = f;
	dest_io.procs = &stdio_ioprocs_noclose;
	dest_io.filler = 0xff;

	floppy_image image(84, 2, floppy_image::FF_UNKNOWN);
	if(!source_format->load(&source_io, floppy_image::FF_UNKNOWN, variants, &image)) {
		fprintf(stderr, "Error: parsing input file as '%s' failed\n", source_format->name());
		return 1;
	}

	if(!dest_format->save(&dest_io, variants, &image)) {
		fprintf(stderr, "Error: writing output file as '%s' failed\n", dest_format->name());
		return 1;
	}

	fclose((FILE *)source_io.file);
	fclose((FILE *)dest_io.file);

	return 0;
}

static int create(int argc, char *argv[])
{
	if (argc!=5) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage();
		return 1;
	}

	auto dest_format = find_format_by_name(argv[2]);
	if(!dest_format) {
		fprintf(stderr, "Error: Format '%s' unknown\n", argv[3]);
		return 1;
	}

	auto source_fs = find_fs_by_name(argv[3]);
	if(!source_fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[2]);
		return 1;
	}

	floppy_image image(84, 2, floppy_image::FF_UNKNOWN);

	if(source_fs->m_type) {
		auto metav = source_fs->m_manager->volume_meta_description();
		fs_meta_data meta;
		for(const auto &e : metav)
			if(!e.m_ro)
				meta[e.m_name] = e.m_default;

		std::vector<u8> img(source_fs->m_image_size);
		fsblk_vec_t blockdev(img);
		auto fs = source_fs->m_manager->mount(blockdev);
		fs->format(meta);

		auto iog = ram_open(img);
		auto source_format = source_fs->m_type();
		source_format->load(iog, floppy_image::FF_UNKNOWN, variants, &image);
		delete source_format;
		delete iog;

	} else
		fs_unformatted::format(source_fs->m_key, &image);

	char msg[4096];
	sprintf(msg, "Error opening %s for writing", argv[4]);
	auto f = fopen(argv[4], "wb");
	if (!f) {
		perror(msg);
		return 1;
	}

	io_generic dest_io;
	dest_io.file = f;
	dest_io.procs = &stdio_ioprocs_noclose;
	dest_io.filler = 0xff;

	if(!dest_format->save(&dest_io, variants, &image)) {
		fprintf(stderr, "Error: writing output file as '%s' failed\n", dest_format->name());
		return 1;
	}

	fclose((FILE *)dest_io.file);

	return 0;
}

static int dir(int argc, char *argv[])
{
	if (argc!=5) {
		fprintf(stderr, "Incorrect number of arguments.\n\n");
		display_usage();
		return 1;
	}

	auto format = find_format_by_name(argv[2]);
	if(!format) {
		fprintf(stderr, "Error: Format '%s' unknown\n", argv[3]);
		return 1;
	}

	auto fs = find_fs_by_name(argv[3]);
	if(!fs) {
		fprintf(stderr, "Error: Filesystem '%s' unknown\n", argv[2]);
		return 1;
	}

	if(!fs->m_manager || !fs->m_manager->can_read()) {
		fprintf(stderr, "Error: Filesystem '%s' does not implement reading\n", argv[2]);
		return 1;
	}

	char msg[4096];
	sprintf(msg, "Error opening %s for reading", argv[4]);
	FILE *f = fopen(argv[4], "rb");
	if (!f) {
		perror(msg);
		return 1;
	}
	io_generic io;
	io.file = f;
	io.procs = &stdio_ioprocs_noclose;
	io.filler = 0xff;

	floppy_image image(84, 2, floppy_image::FF_UNKNOWN);
	if(!format->load(&io, floppy_image::FF_UNKNOWN, variants, &image)) {
		fprintf(stderr, "Error: parsing input file as '%s' failed\n", format->name());
		return 1;
	}

	std::vector<u8> img;
	auto iog = ram_open(img);
	auto load_format = fs->m_type();
	load_format->save(iog, variants, &image);
	delete load_format;
	delete iog;

	fsblk_vec_t blockdev(img);
	auto load_fs = fs->m_manager->mount(blockdev);
	auto vmetad = fs->m_manager->volume_meta_description();
	auto vmeta = load_fs->metadata();

	if(!vmeta.empty()) {
		std::string vinf = "Volume:";
		for(const auto &e : vmetad)
			vinf += util::string_format(" %s=%s", fs_meta_get_name(e.m_name), fs_meta_to_string(e.m_type, vmeta[e.m_name]));
		printf("%s\n", vinf.c_str());
	}

	return 0;
}


int CLIB_DECL main(int argc, char *argv[])
{
	init_formats();

	if (argc == 1) {
		display_full_usage();
		return 0;
	}

	if (!core_stricmp("identify", argv[1]))
		return identify(argc, argv);
	else if (!core_stricmp("convert", argv[1]))
		return convert(argc, argv);
	else if (!core_stricmp("create", argv[1]))
		return create(argc, argv);
	else if (!core_stricmp("dir", argv[1]))
		return dir(argc, argv);
	else {
		fprintf(stderr, "Unknown command '%s'\n\n", argv[1]);
		display_usage();
		return 1;
	}
}
