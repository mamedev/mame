// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    main.c

    Floptool command line front end

    20/07/2011 Initial version by Miodrag Milanovic

***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <assert.h>

#include "corestr.h"

#include "formats/mfi_dsk.h"
#include "formats/dfi_dsk.h"
#include "formats/ipf_dsk.h"

#include "formats/hxcmfm_dsk.h"
#include "formats/ami_dsk.h"

#include "formats/st_dsk.h"
#include "formats/pasti_dsk.h"

#include "formats/dsk_dsk.h"

#include "formats/d88_dsk.h"
#include "formats/imd_dsk.h"
#include "formats/td0_dsk.h"
#include "formats/cqm_dsk.h"
#include "formats/pc_dsk.h"
#include "formats/naslite_dsk.h"

#include "formats/ap_dsk35.h"
#include "formats/ap2_dsk.h"

#include "formats/oric_dsk.h"

#include "formats/applix_dsk.h"

static floppy_format_type floppy_formats[] = {
	FLOPPY_MFI_FORMAT,
	FLOPPY_DFI_FORMAT,
	FLOPPY_IPF_FORMAT,

	FLOPPY_MFM_FORMAT,
	FLOPPY_ADF_FORMAT,

	FLOPPY_ST_FORMAT,
	FLOPPY_MSA_FORMAT,
	FLOPPY_PASTI_FORMAT,

	FLOPPY_DSK_FORMAT,

	FLOPPY_D88_FORMAT,
	FLOPPY_IMD_FORMAT,
	FLOPPY_TD0_FORMAT,
	FLOPPY_CQM_FORMAT,
	FLOPPY_PC_FORMAT,
	FLOPPY_NASLITE_FORMAT,

	FLOPPY_DC42_FORMAT,
	FLOPPY_A216S_FORMAT,
	FLOPPY_RWTS18_FORMAT,
	FLOPPY_EDD_FORMAT,

	FLOPPY_ORIC_DSK_FORMAT,

	FLOPPY_APPLIX_FORMAT,
};

void CLIB_DECL ATTR_PRINTF(1,2) logerror(const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vprintf(format, arg);
	va_end(arg);
}

enum { FORMAT_COUNT = ARRAY_LENGTH(floppy_formats) };

static floppy_image_format_t *formats[FORMAT_COUNT];

static void init_formats()
{
	for(int i=0; i != FORMAT_COUNT; i++)
		formats[i] = floppy_formats[i]();
}

static floppy_image_format_t *find_format_by_name(const char *name)
{
	for(int i=0; i != FORMAT_COUNT; i++)
		if(!core_stricmp(name, formats[i]->name()))
			return formats[i];
	return nullptr;
}

static floppy_image_format_t *find_format_by_identify(io_generic *image)
{
	int best = 0;
	floppy_image_format_t *best_fif = nullptr;

	for(int i = 0; i != FORMAT_COUNT; i++) {
		floppy_image_format_t *fif = formats[i];
		int score = fif->identify(image, floppy_image::FF_UNKNOWN);
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
	fprintf(stderr, "Supported formats:\n\n");
	for(int i = 0; i != FORMAT_COUNT; i++)
	{
		floppy_image_format_t *fif = formats[i];
		fprintf(stderr, "%15s - %s [%s]\n", fif->name(), fif->description(), fif->extensions());
	}
}

static void display_full_usage()
{
	/* Usage */
	fprintf(stderr, "floptool - Generic floppy image manipulation tool for use with MESS\n\n");
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
	if(!source_format->load(&source_io, floppy_image::FF_UNKNOWN, &image)) {
		fprintf(stderr, "Error: parsing input file as '%s' failed\n", source_format->name());
		return 1;
	}

	if(!dest_format->save(&dest_io, &image)) {
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
