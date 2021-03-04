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

#include "corestr.h"
#include "osdcomm.h"

#include "formats/all.h"

std::map<std::string, std::vector<floppy_image_format_t *>> formats_by_category;
std::map<std::string, floppy_image_format_t *> formats_by_key;

struct enumerator : public mame_formats_enumerator {
	virtual ~enumerator() = default;
	virtual void add(const cassette_image::Format *const *formats) {}

	std::vector<floppy_image_format_t *> *cf = nullptr;
	virtual void category(const char *name) {
		auto i = formats_by_category.find(name);
		if(i != formats_by_category.end()) {
			fprintf(stderr, "Collision on category name %s\n", name);
			exit(1);
		}
		cf = &formats_by_category[name];
	}

	virtual void add(floppy_format_type format) {
		auto f = format();
		std::string key = f->name();
		auto i = formats_by_key.find(key);
		if(i != formats_by_key.end()) {
			fprintf(stderr, "Collision on key %s between \"%s\" and \"%s\".\n",
					key.c_str(),
					i->second->description(),
					f->description());
			exit(1);
		}
		cf->push_back(f);
		formats_by_key[key] = f;
	}
};

void CLIB_DECL ATTR_PRINTF(1,2) logerror(const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vprintf(format, arg);
	va_end(arg);
}

static std::vector<uint32_t> variants;

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

static void display_usage()
{
	fprintf(stderr, "Usage: \n");
	fprintf(stderr, "       floptool.exe identify <inputfile> [<inputfile> ...]\n");
	fprintf(stderr, "       floptool.exe convert [input_format|auto] output_format <inputfile> <outputfile>\n");
}

static void display_formats()
{
	int sk = 0;
	for(const auto &e : formats_by_key) {
		int sz = e.first.size();
		if(sz > sk)
			sk = sz;
	}

	fprintf(stderr, "Supported formats:\n\n");
	for(const auto &e : formats_by_category)
		if(!e.second.empty()) {
			fprintf(stderr, "%s:\n", e.first.c_str());
			for(floppy_image_format_t *fif : e.second)
				fprintf(stderr, "  %-*s - %s [%s]\n", sk, fif->name(), fif->description(), fif->extensions());
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
	else {
		fprintf(stderr, "Unknown command '%s'\n\n", argv[1]);
		display_usage();
		return 1;
	}
}
