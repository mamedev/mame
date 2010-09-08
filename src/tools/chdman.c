/***************************************************************************

    CHD compression frontend

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "osdcore.h"
#include "corefile.h"
#include "chdcd.h"
#include "aviio.h"
#include "avcomp.h"
#include "bitmap.h"
#include "md5.h"
#include "sha1.h"
#include "vbiparse.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>


/***************************************************************************
    CONSTANTS & DEFINES
***************************************************************************/

#define IDE_SECTOR_SIZE			512

#define ENABLE_CUSTOM_CHOMP		0

#define OPERATION_UPDATE		0
#define OPERATION_MERGE			1
#define OPERATION_CHOMP			2

#ifdef PTR64
#define IS_FAKE_AVI_FILE(a)		((UINT64)(a) <= 2)
#else
#define IS_FAKE_AVI_FILE(a)		((UINT32)(a) <= 2)
#endif
#define AVI_FAKE_FILE_22		((avi_file *)1)
#define AVI_FAKE_FILE_32		((avi_file *)2)

#define AVI_FAKE_FRAMERATE		29970000
#define AVI_FAKE_FRAMES			54004
#define AVI_FAKE_WIDTH			720
#define AVI_FAKE_HEIGHT			524
#define AVI_FAKE_CHANNELS		2
#define AVI_FAKE_SAMPLERATE		48000



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _chd_interface_file
{
	osd_file *file;
	UINT64 length;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static chd_error chdman_compress_file(chd_file *chd, const char *rawfile, UINT32 offset);
static chd_error chdman_compress_chd(chd_file *chd, chd_file *source, UINT32 totalhunks);
static chd_error chdman_clone_metadata(chd_file *source, chd_file *dest);



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static clock_t lastprogress = 0;



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    print_big_int - 64-bit int printing with
    commas
-------------------------------------------------*/

static void print_big_int(UINT64 intvalue, char *output)
{
	int chunk;

	chunk = intvalue % 1000;
	intvalue /= 1000;
	if (intvalue != 0)
	{
		print_big_int(intvalue, output);
		strcat(output, ",");
		sprintf(&output[strlen(output)], "%03d", chunk);
	}
	else
		sprintf(&output[strlen(output)], "%d", chunk);
}


/*-------------------------------------------------
    big_int_string - return a string for a big
    integer
-------------------------------------------------*/

static char *big_int_string(UINT64 intvalue)
{
	static char buffer[256];
	buffer[0] = 0;
	print_big_int(intvalue, buffer);
	return buffer;
}


/*-------------------------------------------------
    progress - generic progress callback
-------------------------------------------------*/

static void ATTR_PRINTF(2,3) progress(int forceit, const char *fmt, ...)
{
	clock_t curtime = clock();
	va_list arg;

	/* skip if it hasn't been long enough */
	if (!forceit && curtime - lastprogress < CLOCKS_PER_SEC / 2)
		return;
	lastprogress = curtime;

	/* standard vfprintf stuff here */
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	fflush(stderr);
	va_end(arg);
}


/*-------------------------------------------------
    usage - generic usage error display
-------------------------------------------------*/

static int usage(void)
{
	printf("usage: chdman -info input.chd\n");
	printf("   or: chdman -createraw inputhd.raw output.chd [inputoffs [hunksize]]\n");
	printf("   or: chdman -createhd inputhd.raw output.chd [ident.bin] [inputoffs [cylinders heads sectors [sectorsize [hunksize]]]]\n");
	printf("   or: chdman -createuncomphd inputhd.raw output.chd [ident.bin] [inputoffs [cylinders heads sectors [sectorsize [hunksize]]]]\n");
	printf("   or: chdman -createblankhd output.chd cylinders heads sectors [sectorsize [hunksize]]\n");
	printf("   or: chdman -createcd input.(toc/cue/gdi) output.chd\n");
	printf("   or: chdman -createav input.avi output.chd [firstframe [numframes]]\n");
	printf("   or: chdman -copydata input.chd output.chd\n");
	printf("   or: chdman -extract input.chd output.raw\n");
	printf("   or: chdman -extractcd input.chd output.(toc/cue) output.bin (if toc, no extension if cue)\n");
	printf("   or: chdman -extractav input.chd output.avi [firstframe [numframes]]\n");
	printf("   or: chdman -verify input.chd\n");
	printf("   or: chdman -verifyfix input.chd\n");
	printf("   or: chdman -update input.chd output.chd\n");
	printf("   or: chdman -chomp inout.chd output.chd maxhunk\n");
	printf("   or: chdman -merge parent.chd diff.chd output.chd\n");
	printf("   or: chdman -diff parent.chd compare.chd diff.chd\n");
	printf("   or: chdman -setchs inout.chd cylinders heads sectors\n");
	printf("   or: chdman -fixavdata inout.chd\n");
	printf("   or: chdman -addmetabin inout.chd tag [index] sourcefile\n");
	printf("   or: chdman -addmetatext inout.chd tag [index] sourcefile\n");
	return 1;
}


/*-------------------------------------------------
    get_file_size - get the size of a file
-------------------------------------------------*/

static UINT64 get_file_size(const char *filename)
{
	osd_file *file;
	UINT64 filesize = 0;
	file_error filerr;

	filerr = osd_open(filename, OPEN_FLAG_READ, &file, &filesize);
	if (filerr == FILERR_NONE)
		osd_close(file);
	return filesize;
}


/*-------------------------------------------------
    guess_chs - given a file and an offset,
    compute a best guess CHS value set
-------------------------------------------------*/

static chd_error guess_chs(const char *filename, int offset, int sectorsize, UINT32 *cylinders, UINT32 *heads, UINT32 *sectors, UINT32 *bps)
{
	UINT32 totalsecs, hds, secs;
	UINT64 filesize;

	/* if this is a direct physical drive read, handle it specially */
	if (osd_get_physical_drive_geometry(filename, cylinders, heads, sectors, bps))
		return CHDERR_NONE;

	/* compute the filesize */
	filesize = get_file_size(filename);
	if (filesize <= offset)
	{
		fprintf(stderr, "Invalid file '%s'\n", filename);
		return CHDERR_INVALID_FILE;
	}
	filesize -= offset;

	/* validate the size */
	if (filesize % sectorsize != 0)
	{
		fprintf(stderr, "Can't guess CHS values because data size is not divisible by the sector size\n");
		return CHDERR_INVALID_FILE;
	}
	totalsecs = filesize / sectorsize;

	/* now find a valid value */
	for (secs = 63; secs > 1; secs--)
		if (totalsecs % secs == 0)
		{
			size_t totalhds = totalsecs / secs;
			for (hds = 16; hds > 1; hds--)
				if (totalhds % hds == 0)
				{
					*cylinders = totalhds / hds;
					*heads = hds;
					*sectors = secs;
					*bps = sectorsize;
					return CHDERR_NONE;
				}
		}

	/* ack, it didn't work! */
	fprintf(stderr, "Can't guess CHS values because no logical combination works!\n");
	return CHDERR_INVALID_FILE;
}


/*-------------------------------------------------
    get_chs_from_ident - extract chs from an ident
    information, validate it with the file size

    Note: limited to IDE for now
-------------------------------------------------*/

static chd_error get_chs_from_ident(const char *filename, int offset, const UINT8 *ident, UINT32 identsize, UINT32 *cylinders, UINT32 *heads, UINT32 *sectors, UINT32 *bps)
{
	UINT64 filesize, expected_size;

	filesize = get_file_size(filename);
	if (filesize <= offset)
	{
		fprintf(stderr, "Invalid file '%s'\n", filename);
		return CHDERR_INVALID_FILE;
	}

	if (filesize % 512 != 0)
	{
		fprintf(stderr, "Can't validate CHS values because data size is not divisible by the sector size\n");
		return CHDERR_INVALID_FILE;
	}

	if (identsize < 14)
	{
		fprintf(stderr, "Error: the ident metadata is too short to include geometry information\n");
		return CHDERR_INVALID_FILE;
	}

	*bps = 512;
	*cylinders = ((unsigned char)ident[ 2]) | (((unsigned char )ident[ 3]) << 8);
	*heads     = ((unsigned char)ident[ 6]) | (((unsigned char )ident[ 7]) << 8);
	*sectors   = ((unsigned char)ident[12]) | (((unsigned char )ident[13]) << 8);

	expected_size = (UINT64)*cylinders * (UINT64)*heads * (UINT64)*sectors * 512;
	if (expected_size != filesize - offset)
	{
		fprintf(stderr, "Error: Mismatch between the ident CHS data (%u/%u/%u, %u sectors) and the file size (%u sectors)\n",
				*cylinders, *heads, *sectors, *cylinders * *heads * *sectors,
				(UINT32)((filesize-offset)/512));
		return CHDERR_INVALID_FILE;
	}
	return CHDERR_NONE;
}


/*-------------------------------------------------
    do_createhd - create a new compressed hard
    disk image from a raw file
-------------------------------------------------*/

static int do_createhd(int argc, char *argv[], int param)
{
	UINT32 guess_cylinders = 0, guess_heads = 0, guess_sectors = 0, guess_sectorsize = 0;
	UINT32 cylinders, heads, sectors, sectorsize, hunksize, totalsectors, offset;
	const char *inputfile, *outputfile = NULL;
	chd_error err = CHDERR_NONE;
	UINT32 identdatasize = 0;
	UINT8 *identdata = NULL;
	chd_file *chd = NULL;
	char metadata[256];

	/* if a file is provided for argument 4 (ident filename), then shift the remaining arguments down */
	if (argc >= 5)
	{
		char *scan;

		/* if there are any non-digits in the 'offset', then treat it as a ident file */
		for (scan = argv[4]; *scan != 0; scan++)
			if (!isdigit((UINT8)*scan))
				break;
		if (*scan != 0)
		{
			/* attempt to load the file */
			file_error filerr = core_fload(argv[4], (void **)&identdata, &identdatasize);
			if (filerr != FILERR_NONE)
			{
				fprintf(stderr, "Error opening ident file '%s'\n", argv[4]);
				return 1;
			}

			/* shift the remaining arguments down */
			if (argc > 5)
				memmove(&argv[4], &argv[5], (argc - 5) * sizeof(argv[0]));
			argc--;
		}
	}

	/* require 4-5, or 8-10 args total */
	if (argc != 4 && argc != 5 && argc != 8 && argc != 9 && argc != 10)
		return usage();

	/* extract the first few parameters */
	inputfile = argv[2];
	outputfile = argv[3];
	offset = (argc >= 5) ? atoi(argv[4]) : (get_file_size(inputfile) % IDE_SECTOR_SIZE);

	/* if less than 8 parameters, we need to guess the CHS values */
	if (argc < 8)
	{
		if (identdata != NULL)
			err = get_chs_from_ident(inputfile, offset, identdata, identdatasize, &guess_cylinders, &guess_heads, &guess_sectors, &guess_sectorsize);
		else
			err = guess_chs(inputfile, offset, IDE_SECTOR_SIZE, &guess_cylinders, &guess_heads, &guess_sectors, &guess_sectorsize);
		if (err != CHDERR_NONE)
			goto cleanup;
	}

	/* parse the remaining parameters */
	cylinders = (argc >= 6) ? atoi(argv[5]) : guess_cylinders;
	heads = (argc >= 7) ? atoi(argv[6]) : guess_heads;
	sectors = (argc >= 8) ? atoi(argv[7]) : guess_sectors;
	sectorsize = (argc >= 9) ? atoi(argv[8]) : guess_sectorsize;
	if (sectorsize == 0) sectorsize = IDE_SECTOR_SIZE;
	hunksize = (argc >= 10) ? atoi(argv[9]) : (sectorsize > 4096) ? sectorsize : ((4096 / sectorsize) * sectorsize);
	totalsectors = cylinders * heads * sectors;

	/* print some info */
	printf("Input file:   %s\n", inputfile);
	printf("Output file:  %s\n", outputfile);
	printf("Input offset: %d\n", offset);
	printf("Cylinders:    %d\n", cylinders);
	printf("Heads:        %d\n", heads);
	printf("Sectors:      %d\n", sectors);
	printf("Bytes/sector: %d\n", sectorsize);
	printf("Sectors/hunk: %d\n", hunksize / sectorsize);
	printf("Logical size: %s\n", big_int_string((UINT64)totalsectors * (UINT64)sectorsize));

	/* create the new hard drive */
	err = chd_create(outputfile, (UINT64)totalsectors * (UINT64)sectorsize, hunksize, CHDCOMPRESSION_ZLIB_PLUS, NULL);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error creating CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* open the new hard drive */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* write the metadata */
	sprintf(metadata, HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, sectorsize);
	err = chd_set_metadata(chd, HARD_DISK_METADATA_TAG, 0, metadata, strlen(metadata) + 1, CHD_MDFLAGS_CHECKSUM);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error adding hard disk metadata: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* write the ident if present */
	if (identdata != NULL)
	{
		err = chd_set_metadata(chd, HARD_DISK_IDENT_METADATA_TAG, 0, identdata, identdatasize, CHD_MDFLAGS_CHECKSUM);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error adding hard disk metadata: %s\n", chd_error_string(err));
			goto cleanup;
		}
	}

	/* compress the hard drive */
	err = chdman_compress_file(chd, inputfile, offset);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression: %s\n", chd_error_string(err));

cleanup:
	/* close everything down */
	if (chd != NULL)
		chd_close(chd);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
	if (identdata != NULL)
		free(identdata);
	return (err != CHDERR_NONE);
}

/*-------------------------------------------------
    do_createhd_uncomp - create a new uncompressed hard
    disk image from a raw file
-------------------------------------------------*/

static int do_createhd_uncomp(int argc, char *argv[], int param)
{
	UINT32 guess_cylinders = 0, guess_heads = 0, guess_sectors = 0, guess_sectorsize = 0;
	UINT32 cylinders, heads, sectors, sectorsize, hunksize, totalsectors, offset;
	const char *inputfile, *outputfile = NULL;
	chd_error err = CHDERR_NONE;
	UINT32 identdatasize = 0;
	UINT8 *identdata = NULL;
	chd_file *chd = NULL;
	char metadata[256];
	chd_header header;

	/* if a file is provided for argument 4 (ident filename), then shift the remaining arguments down */
	if (argc >= 5)
	{
		char *scan;

		/* if there are any non-digits in the 'offset', then treat it as a ident file */
		for (scan = argv[4]; *scan != 0; scan++)
			if (!isdigit((UINT8)*scan))
				break;
		if (*scan != 0)
		{
			/* attempt to load the file */
			file_error filerr = core_fload(argv[4], (void **)&identdata, &identdatasize);
			if (filerr != FILERR_NONE)
			{
				fprintf(stderr, "Error opening ident file '%s'\n", argv[4]);
				return 1;
			}

			/* shift the remaining arguments down */
			if (argc > 5)
				memmove(&argv[4], &argv[5], (argc - 5) * sizeof(argv[0]));
			argc--;
		}
	}

	/* require 4-5, or 8-10 args total */
	if (argc != 4 && argc != 5 && argc != 8 && argc != 9 && argc != 10)
		return usage();

	/* extract the first few parameters */
	inputfile = argv[2];
	outputfile = argv[3];
	offset = (argc >= 5) ? atoi(argv[4]) : (get_file_size(inputfile) % IDE_SECTOR_SIZE);

	/* if less than 8 parameters, we need to guess the CHS values */
	if (argc < 8)
	{
		if (identdata != NULL)
			err = get_chs_from_ident(inputfile, offset, identdata, identdatasize, &guess_cylinders, &guess_heads, &guess_sectors, &guess_sectorsize);
		else
			err = guess_chs(inputfile, offset, IDE_SECTOR_SIZE, &guess_cylinders, &guess_heads, &guess_sectors, &guess_sectorsize);
		if (err != CHDERR_NONE)
			goto cleanup;
	}

	/* parse the remaining parameters */
	cylinders = (argc >= 6) ? atoi(argv[5]) : guess_cylinders;
	heads = (argc >= 7) ? atoi(argv[6]) : guess_heads;
	sectors = (argc >= 8) ? atoi(argv[7]) : guess_sectors;
	sectorsize = (argc >= 9) ? atoi(argv[8]) : guess_sectorsize;
	if (sectorsize == 0) sectorsize = IDE_SECTOR_SIZE;
	hunksize = (argc >= 10) ? atoi(argv[9]) : (sectorsize > 4096) ? sectorsize : ((4096 / sectorsize) * sectorsize);
	totalsectors = cylinders * heads * sectors;

	/* print some info */
	printf("Input file:   %s\n", inputfile);
	printf("Output file:  %s\n", outputfile);
	printf("Input offset: %d\n", offset);
	printf("Cylinders:    %d\n", cylinders);
	printf("Heads:        %d\n", heads);
	printf("Sectors:      %d\n", sectors);
	printf("Bytes/sector: %d\n", sectorsize);
	printf("Sectors/hunk: %d\n", hunksize / sectorsize);
	printf("Logical size: %s\n", big_int_string((UINT64)totalsectors * (UINT64)sectorsize));

	/* create the new hard drive */
	err = chd_create(outputfile, (UINT64)totalsectors * (UINT64)sectorsize, hunksize, CHDCOMPRESSION_NONE, NULL);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error creating CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* open the new hard drive */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* write the metadata */
	sprintf(metadata, HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, sectorsize);
	err = chd_set_metadata(chd, HARD_DISK_METADATA_TAG, 0, metadata, strlen(metadata) + 1, CHD_MDFLAGS_CHECKSUM);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error adding hard disk metadata: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* write the ident if present */
	if (identdata != NULL)
	{
		err = chd_set_metadata(chd, HARD_DISK_IDENT_METADATA_TAG, 0, identdata, identdatasize, CHD_MDFLAGS_CHECKSUM);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error adding hard disk metadata: %s\n", chd_error_string(err));
			goto cleanup;
		}
	}

	/* compress the hard drive */
	err = chdman_compress_file(chd, inputfile, offset);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression: %s\n", chd_error_string(err));

	/* make it writeable */
	header = *chd_get_header(chd);
	header.flags |= CHDFLAGS_IS_WRITEABLE;
	err = chd_set_header_file(chd_core_file(chd), &header);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error writing new header: %s\n", chd_error_string(err));

cleanup:
	/* close everything down */
	if (chd != NULL)
		chd_close(chd);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
	if (identdata != NULL)
		free(identdata);
	return (err != CHDERR_NONE);
}

/*-------------------------------------------------
    do_createraw - create a new compressed raw
    image from a raw file
-------------------------------------------------*/

static int do_createraw(int argc, char *argv[], int param)
{
	const char *inputfile, *outputfile;
	UINT32 hunksize, offset;
	UINT64 logicalbytes;
	chd_file *chd = NULL;
	chd_error err;

	/* require 4, 5, or 6 args total */
	if (argc != 4 && argc != 5 && argc != 6)
		return usage();

	/* extract the first few parameters */
	inputfile = argv[2];
	outputfile = argv[3];
	offset = (argc >= 5) ? atoi(argv[4]) : 0;
	hunksize = (argc >= 6) ? atoi(argv[5]) : 4096;
	logicalbytes = get_file_size(inputfile) - offset;

	/* print some info */
	printf("Input file:   %s\n", inputfile);
	printf("Output file:  %s\n", outputfile);
	printf("Input offset: %d\n", offset);
	printf("Bytes/hunk:   %d\n", hunksize);
	printf("Logical size: %s\n", big_int_string(logicalbytes));

	/* create the new CHD */
	err = chd_create(outputfile, logicalbytes, hunksize, CHDCOMPRESSION_ZLIB_PLUS, NULL);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error creating CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* open the new CHD */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* compress the CHD */
	err = chdman_compress_file(chd, inputfile, offset);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression: %s\n", chd_error_string(err));

cleanup:
	/* close everything down */
	if (chd != NULL)
		chd_close(chd);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_createcd - create a new compressed CD
    image from a raw file
-------------------------------------------------*/

static int do_createcd(int argc, char *argv[], int param)
{
	static chdcd_track_input_info track_info;
	static cdrom_toc toc;
	UINT32 hunksize = CD_FRAME_SIZE * CD_FRAMES_PER_HUNK;
	UINT32 sectorsize = CD_FRAME_SIZE;
	const char *inputfile, *outputfile;
	core_file *srcfile = NULL;
	UINT32 origtotalsectors;
	chd_file *chd = NULL;
	UINT8 *cache = NULL;
	UINT32 totalsectors;
	double ratio = 1.0;
	UINT32 totalhunks;
	file_error filerr;
	chd_error err;
	int i;

	/* require 4 args total */
	if (argc != 4)
		return usage();

	/* extract the data */
	inputfile = argv[2];
	outputfile = argv[3];

	/* clear the TOC */
	memset(&toc, 0, sizeof(toc));

	/* allocate a cache */
	cache = (UINT8 *)malloc(hunksize);
	if (cache == NULL)
	{
		fprintf(stderr, "Out of memory allocating temporary buffer\n");
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}

	/* setup the CDROM module and get the disc info */
	err = chdcd_parse_toc(inputfile, &toc, &track_info);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error reading input file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* pad each track to a hunk boundry. cdrom.c will deal with this on the read side */
	for (i = 0; i < toc.numtrks; i++)
	{
		int hunks = (toc.tracks[i].frames + CD_FRAMES_PER_HUNK - 1) / CD_FRAMES_PER_HUNK;
		toc.tracks[i].extraframes = hunks * CD_FRAMES_PER_HUNK - toc.tracks[i].frames;
	}

	/* count up the total number of frames */
	origtotalsectors = totalsectors = 0;
	for (i = 0; i < toc.numtrks; i++)
	{
		origtotalsectors += toc.tracks[i].frames;
		totalsectors += toc.tracks[i].frames + toc.tracks[i].extraframes;
	}
	printf("\nCD-ROM %s has %d tracks and %d total frames\n", inputfile, toc.numtrks, origtotalsectors);

	/* create the new CHD file */
	err = chd_create(outputfile, (UINT64)totalsectors * (UINT64)sectorsize, hunksize, CHDCOMPRESSION_ZLIB_PLUS, NULL);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error creating CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* open the new CHD file */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* write the metadata */
	err = cdrom_write_metadata(chd, &toc);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error adding CD-ROM metadata: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* begin state for writing */
	err = chd_compress_begin(chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error compressing: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* loop over tracks */
	totalhunks = 0;
	for (i = 0; i < toc.numtrks; i++)
	{
		int frames = 0;
		int bytespersector = toc.tracks[i].datasize + toc.tracks[i].subsize;
		int trackhunks = (toc.tracks[i].frames + toc.tracks[i].extraframes) / CD_FRAMES_PER_HUNK;
		UINT64 sourcefileoffset = track_info.offset[i];
		int curhunk;

		/* open the input file for this track */
		filerr = core_fopen(track_info.fname[i], OPEN_FLAG_READ, &srcfile);
		if (filerr != FILERR_NONE)
		{
			fprintf(stderr, "Unable to open file: %s\n", track_info.fname[i]);
			err = CHDERR_FILE_NOT_FOUND;
			goto cleanup;
		}

		printf("Track %02d/%02d (%s:%d,%d frames,%d hunks,swap %d,pregap %d,postgap %d)\n", i+1, toc.numtrks, track_info.fname[i], track_info.offset[i], toc.tracks[i].frames, trackhunks, track_info.swap[i], toc.tracks[i].pregap, toc.tracks[i].postgap);

		/* loop over hunks */
		for (curhunk = 0; curhunk < trackhunks; curhunk++, totalhunks++)
		{
			int secnum;

			progress(FALSE, "Compressing hunk %d/%d... (ratio=%d%%)  \r", totalhunks, chd_get_header(chd)->totalhunks, (int)(ratio * 100));

			/* loop over sectors in this hunk, reading the source data into a fixed start location */
			/* relative to the start; we zero out the buffer ahead of time to ensure that unpopulated */
			/* areas are cleared */
			memset(cache, 0, hunksize);
			for (secnum = 0; secnum < CD_FRAMES_PER_HUNK; secnum++)
			{
				if (frames < toc.tracks[i].frames)
				{
					core_fseek(srcfile, sourcefileoffset, SEEK_SET);
					core_fread(srcfile, &cache[secnum * CD_FRAME_SIZE], bytespersector);

					if (track_info.swap[i])
					{
						int swapindex;

						for (swapindex = 0; swapindex < 2352; swapindex += 2 )
						{
							int swapoffset = ( secnum * CD_FRAME_SIZE ) + swapindex;

							int swaptemp = cache[ swapoffset ];
							cache[ swapoffset ] = cache[ swapoffset + 1 ];
							cache[ swapoffset + 1 ] = swaptemp;
						}
					}
				}

				sourcefileoffset += bytespersector;
				frames++;
			}

			/* compress the current hunk */
			err = chd_compress_hunk(chd, cache, &ratio);
			if (err != CHDERR_NONE)
			{
				fprintf(stderr, "Error during compression: %s\n", chd_error_string(err));
				goto cleanup;
			}
		}

		/* close the file */
		core_fclose(srcfile);
		srcfile = NULL;
	}

	/* cleanup */
	err = chd_compress_finish(chd, TRUE);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression finalization: %s\n", chd_error_string(err));
	else
		progress(TRUE, "Compression complete ... final ratio = %d%%            \n", (int)(100.0 * ratio));

cleanup:
	if (cache != NULL)
		free(cache);
	if (srcfile != NULL)
		core_fclose(srcfile);
	if (chd != NULL)
		chd_close(chd);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    read_avi_frame - read an AVI frame
-------------------------------------------------*/

static avi_error read_avi_frame(avi_file *avi, UINT32 framenum, UINT32 first_sample, bitmap_t *fullbitmap, int interlaced, av_codec_compress_config *avconfig)
{
	const avi_movie_info *info = avi_get_movie_info(avi);
	int interlace_factor = interlaced ? 2 : 1;
	avi_error avierr = AVIERR_NONE;
	int chnum;

	/* loop over channels and read the samples */
	for (chnum = 0; chnum < info->audio_channels; chnum++)
	{
		/* read the sound samples */
		avierr = avi_read_sound_samples(avi, chnum, first_sample, avconfig->samples, avconfig->audio[chnum]);
		if (avierr != AVIERR_NONE)
			goto cleanup;
	}

	/* read the video data when we hit a new frame */
	if (framenum % interlace_factor == 0)
	{
		avierr = avi_read_video_frame_yuy16(avi, framenum / interlace_factor, fullbitmap);
		if (avierr != AVIERR_NONE)
			goto cleanup;
	}

	/* build the fake bitmap */
	bitmap_clone_existing(avconfig->video, fullbitmap);
	if (interlaced)
	{
		avconfig->video->base = BITMAP_ADDR16(avconfig->video, framenum % interlace_factor, 0);
		avconfig->video->rowpixels *= 2;
		avconfig->video->height /= 2;
	}

cleanup:
	return avierr;
}


/*-------------------------------------------------
    fake_avi_frame - fake an AVI frame
-------------------------------------------------*/

static avi_error fake_avi_frame(avi_file *avi, UINT32 framenum, UINT32 first_sample, bitmap_t *fullbitmap, int interlaced, av_codec_compress_config *avconfig)
{
	static int framecounter = 0;
	int leftsamp = (framenum % 200 < 10) ? 10000 : 0;
	int rightsamp = (framenum % 200 >= 100 && framenum % 200 < 110) ? 10000 : 0;
	int interlace_factor = interlaced ? 2 : 1;
	int chnum, sampnum, x, y;
	int whiteflag, line1718;

	/* reset framecounter to 1 on frame 0 */
	if (framenum == 0)
		framecounter = 1;

	/* loop over channels and read the samples */
	for (chnum = 0; chnum < AVI_FAKE_CHANNELS; chnum++)
	{
		int modcheck = AVI_FAKE_SAMPLERATE / ((chnum == 0) ? 110 : 220);
		int samp = (chnum == 0) ? leftsamp : rightsamp;
		INT16 *dest = avconfig->audio[chnum];

		/* store them to the audio buffer */
		for (sampnum = 0; sampnum < avconfig->samples; sampnum++)
			*dest++ = ((first_sample + sampnum) % modcheck < modcheck / 2) ? samp : -samp;
	}

	/* determine what metadata we should generate */
	whiteflag = line1718 = 0;
	if (framenum < 2)
	{
		whiteflag = (framenum == 0);
		line1718 = 0x88ffff;
	}
	else if (framenum >= AVI_FAKE_FRAMES * 2 - 2)
	{
		whiteflag = (framenum == AVI_FAKE_FRAMES * 2 - 2);
		line1718 = 0x80eeee;
	}
	else
	{
		int effnum = framenum - 2;
		if (avi == AVI_FAKE_FILE_22 && effnum % 2 == 0)
			whiteflag = 1;
		else if (avi == AVI_FAKE_FILE_32 && (effnum % 5 == 0 || effnum % 5 == 2))
			whiteflag = 1;
		if (whiteflag)
		{
			line1718 = 0xf80000 | (((framecounter / 10000) % 10) << 16) | (((framecounter / 1000) % 10) << 12) |
						(((framecounter / 100) % 10) << 8) | (((framecounter / 10) % 10) << 4) | (framecounter % 10);
			framecounter++;
		}
	}

	/* build the fake bitmap */
	bitmap_clone_existing(avconfig->video, fullbitmap);
	if (interlaced)
	{
		avconfig->video->base = BITMAP_ADDR16(avconfig->video, framenum % interlace_factor, 0);
		avconfig->video->rowpixels *= 2;
		avconfig->video->height /= 2;
	}

	/* loop over the data and copy it to the cache */
	for (y = 0; y < avconfig->video->height; y++)
	{
		UINT16 *dest = BITMAP_ADDR16(avconfig->video, y, 0);

		/* white flag? */
		if (y == 11 && whiteflag)
		{
			for (x = 0; x < AVI_FAKE_WIDTH; x++)
				*dest++ = (x > 10 && x < avconfig->video->width - 10) ? 0xff80 : 0x0080;
		}

		/* line 17/18 */
		else if ((y == 17 || y == 18) && line1718 != 0)
		{
			for (x = 0; x < avconfig->video->width; x++)
			{
				UINT16 pixel = 0x0080;
				if (x >= 20)
				{
					int bitnum = (x - 20) / 28;
					if (bitnum < 24)
					{
						int bithalf = (x - (bitnum * 28 + 20)) / 14;
						if ((line1718 << bitnum) & 0x800000) bithalf ^= 1;
						pixel = bithalf ? 0x0080 : 0xff80;
					}
				}
				*dest++ = pixel;
			}
		}

		/* anything else in VBI-land */
		else if (y < 22)
		{
			for (x = 0; x < avconfig->video->width; x++)
				*dest++ = 0x0080;
		}

		/* everything else */
		else
		{
			for (x = 0; x < avconfig->video->width; x++)
				*dest++ = framenum;
		}
	}

	return AVIERR_NONE;
}


/*-------------------------------------------------
    do_createav - create a new A/V file from an
    input AVI file and metadata
-------------------------------------------------*/

static int do_createav(int argc, char *argv[], int param)
{
	UINT32 fps_times_1million, width, height, interlaced, channels, rate, totalframes;
	UINT32 max_samples_per_frame, bytes_per_frame, firstframe, numframes;
	av_codec_compress_config avconfig = { 0 };
	const char *inputfile, *outputfile;
	bitmap_t *fullbitmap = NULL;
	const avi_movie_info *info;
	UINT8 *ldframedata = NULL;
	const chd_header *header;
	chd_file *chd = NULL;
	avi_file *avi = NULL;
	bitmap_t fakebitmap;
	double ratio = 1.0;
	char metadata[256];
	avi_error avierr;
	UINT32 framenum;
	chd_error err;
	int chnum;

	/* require 4-6 args total */
	if (argc < 4 || argc > 6)
		return usage();

	/* extract the first few parameters */
	inputfile = argv[2];
	outputfile = argv[3];
	firstframe = (argc > 4) ? atoi(argv[4]) : 0;
	numframes = (argc > 5) ? atoi(argv[5]) : 1000000;

	/* print some info */
	printf("Input file:   %s\n", inputfile);
	printf("Output file:  %s\n", outputfile);

	/* special AVI files */
	if (strcmp(inputfile, "2:2") == 0 || strcmp(inputfile, "3:2") == 0)
	{
		/* create a fake handle */
		avi = (strcmp(inputfile, "2:2") == 0) ? AVI_FAKE_FILE_22 : AVI_FAKE_FILE_32;

		/* fake the movie information */
		fps_times_1million = AVI_FAKE_FRAMERATE;
		width = AVI_FAKE_WIDTH;
		height = AVI_FAKE_HEIGHT;
		interlaced = TRUE;
		channels = 2;
		rate = AVI_FAKE_SAMPLERATE;
		totalframes = AVI_FAKE_FRAMES;
	}
	else
	{
		/* open the source file */
		avierr = avi_open(inputfile, &avi);
		if (avierr != AVIERR_NONE)
		{
			fprintf(stderr, "Error opening AVI file: %s\n", avi_error_string(avierr));
			err = CHDERR_INVALID_FILE;
			goto cleanup;
		}

		/* get the movie information */
		info = avi_get_movie_info(avi);
		fps_times_1million = (UINT64)info->video_timescale * 1000000 / info->video_sampletime;
		width = info->video_width;
		height = info->video_height;
		interlaced = ((fps_times_1million / 1000000) <= 30) && (height % 2 == 0) && (height > 288);
		channels = info->audio_channels;
		rate = info->audio_samplerate;
		totalframes = info->video_numsamples;
	}
	numframes = MIN(totalframes - firstframe, numframes);

	/* print some of it */
	printf("Use frames:   %d-%d\n", firstframe, firstframe + numframes - 1);
	printf("Frame rate:   %d.%06d\n", fps_times_1million / 1000000, fps_times_1million % 1000000);
	printf("Frame size:   %d x %d %s\n", width, height, interlaced ? "interlaced" : "non-interlaced");
	printf("Audio:        %d channels at %d Hz\n", channels, rate);
	printf("Total frames: %d (%02d:%02d:%02d)\n", totalframes,
			(UINT32)((UINT64)totalframes * 1000000 / fps_times_1million / 60 / 60),
			(UINT32)(((UINT64)totalframes * 1000000 / fps_times_1million / 60) % 60),
			(UINT32)(((UINT64)totalframes * 1000000 / fps_times_1million) % 60));

	/* adjust for interlacing */
	if (interlaced)
	{
		fps_times_1million *= 2;
		totalframes *= 2;
		height /= 2;
		firstframe *= 2;
		numframes *= 2;
	}

	/* allocate space for the frame data */
	if (height == 524/2 || height == 624/2)
	{
		ldframedata = (UINT8 *)malloc(numframes * VBI_PACKED_BYTES);
		if (ldframedata == NULL)
		{
			fprintf(stderr, "Out of memory allocating frame metadata\n");
			err = CHDERR_OUT_OF_MEMORY;
			goto cleanup;
		}
		memset(ldframedata, 0, numframes * VBI_PACKED_BYTES);
	}

	/* determine the number of bytes per frame */
	max_samples_per_frame = ((UINT64)rate * 1000000 + fps_times_1million - 1) / fps_times_1million;
	bytes_per_frame = 12 + channels * max_samples_per_frame * 2 + width * height * 2;

	/* allocate a video buffer */
	fullbitmap = bitmap_alloc(width, height * (interlaced ? 2 : 1), BITMAP_FORMAT_YUY16);
	if (fullbitmap == NULL)
	{
		fprintf(stderr, "Out of memory allocating temporary bitmap\n");
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}
	avconfig.video = &fakebitmap;

	/* allocate audio buffers */
	avconfig.channels = channels;
	for (chnum = 0; chnum < channels; chnum++)
	{
		avconfig.audio[chnum] = (INT16 *)malloc(max_samples_per_frame * 2);
		if (avconfig.audio[chnum] == NULL)
		{
			fprintf(stderr, "Out of memory allocating temporary audio buffer\n");
			err = CHDERR_OUT_OF_MEMORY;
			goto cleanup;
		}
	}

	/* create the new CHD */
	err = chd_create(outputfile, (UINT64)numframes * (UINT64)bytes_per_frame, bytes_per_frame, CHDCOMPRESSION_AV, NULL);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error creating CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* open the new CHD */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}
	header = chd_get_header(chd);

	/* write the metadata */
	sprintf(metadata, AV_METADATA_FORMAT, fps_times_1million / 1000000, fps_times_1million % 1000000, width, height, interlaced, channels, rate);
	err = chd_set_metadata(chd, AV_METADATA_TAG, 0, metadata, strlen(metadata) + 1, CHD_MDFLAGS_CHECKSUM);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error adding AV metadata: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* begin compressing */
	err = chd_compress_begin(chd);
	if (err != CHDERR_NONE)
		goto cleanup;

	/* loop over source hunks until we run out */
	for (framenum = 0; framenum < numframes; framenum++)
	{
		int effframe = firstframe + framenum;
		UINT32 first_sample;

		/* progress */
		progress(framenum == 0, "Compressing hunk %d/%d... (ratio=%d%%)  \r", framenum, header->totalhunks, (int)(100.0 * ratio));

		/* compute the number of samples in this frame */
		first_sample = ((UINT64)rate * (UINT64)effframe * (UINT64)1000000 + fps_times_1million - 1) / (UINT64)fps_times_1million;
		avconfig.samples = ((UINT64)rate * (UINT64)(effframe + 1) * (UINT64)1000000 + fps_times_1million - 1) / (UINT64)fps_times_1million - first_sample;

		/* read the frame into its proper format in the cache */
		if (IS_FAKE_AVI_FILE(avi))
			avierr = fake_avi_frame(avi, effframe, first_sample, fullbitmap, interlaced, &avconfig);
		else
			avierr = read_avi_frame(avi, effframe, first_sample, fullbitmap, interlaced, &avconfig);
		if (avierr != AVIERR_NONE)
		{
			fprintf(stderr, "Error reading frame %d from AVI file: %s\n", effframe, avi_error_string(avierr));
			err = CHDERR_COMPRESSION_ERROR;
		}

		/* update metadata for this frame */
		if (ldframedata != NULL)
		{
			/* parse the data and pack it */
			vbi_metadata vbi;
			vbi_parse_all((const UINT16 *)avconfig.video->base, avconfig.video->rowpixels, avconfig.video->width, 8, &vbi);
			vbi_metadata_pack(&ldframedata[framenum * VBI_PACKED_BYTES], framenum, &vbi);
		}

		/* configure the compressor for this frame */
		chd_codec_config(chd, AV_CODEC_COMPRESS_CONFIG, &avconfig);

		/* append the data */
		err = chd_compress_hunk(chd, NULL, &ratio);
		if (err != CHDERR_NONE)
			goto cleanup;
	}

	/* write the final metadata */
	if (ldframedata != NULL)
	{
		err = chd_set_metadata(chd, AV_LD_METADATA_TAG, 0, ldframedata, numframes * VBI_PACKED_BYTES, CHD_MDFLAGS_CHECKSUM);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error adding AVLD metadata: %s\n", chd_error_string(err));
			goto cleanup;
		}
	}

	/* finish compression */
	err = chd_compress_finish(chd, TRUE);
	if (err != CHDERR_NONE)
		goto cleanup;
	else
		progress(TRUE, "Compression complete ... final ratio = %d%%            \n", (int)(100.0 * ratio));

cleanup:
	/* close everything down */
	if (avi != NULL && !IS_FAKE_AVI_FILE(avi))
		avi_close(avi);
	if (chd != NULL)
		chd_close(chd);
	for (chnum = 0; chnum < ARRAY_LENGTH(avconfig.audio); chnum++)
		if (avconfig.audio[chnum] != NULL)
			free(avconfig.audio[chnum]);
	if (fullbitmap != NULL)
		bitmap_free(fullbitmap);
	if (ldframedata != NULL)
		free(ldframedata);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_createblankhd - create a new non-compressed
    hard disk image, with all hunks filled with 0s
-------------------------------------------------*/

static int do_createblankhd(int argc, char *argv[], int param)
{
	UINT32 cylinders, heads, sectors, sectorsize, hunksize, totalsectors, hunknum;
	const char *outputfile;
	chd_file *chd = NULL;
	UINT8 *cache = NULL;
	char metadata[256];
	chd_error err;

	/* require 6, 7, or 8 args total */
	if (argc != 6 && argc != 7 && argc != 8)
		return usage();

	/* extract the data */
	outputfile = argv[2];
	cylinders = atoi(argv[3]);
	heads = atoi(argv[4]);
	sectors = atoi(argv[5]);
	sectorsize = (argc >= 7) ? atoi(argv[6]) : IDE_SECTOR_SIZE;
	if (sectorsize == 0) sectorsize = IDE_SECTOR_SIZE;
	hunksize = (argc >= 8) ? atoi(argv[7]) : (sectorsize > 4096) ? sectorsize : ((4096 / sectorsize) * sectorsize);
	totalsectors = cylinders * heads * sectors;

	/* print some info */
	printf("Output file:  %s\n", outputfile);
	printf("Cylinders:    %d\n", cylinders);
	printf("Heads:        %d\n", heads);
	printf("Sectors:      %d\n", sectors);
	printf("Bytes/sector: %d\n", sectorsize);
	printf("Sectors/hunk: %d\n", hunksize / sectorsize);
	printf("Logical size: %s\n", big_int_string((UINT64)totalsectors * (UINT64)sectorsize));

	/* create the new hard drive */
	err = chd_create(outputfile, (UINT64)totalsectors * (UINT64)sectorsize, hunksize, CHDCOMPRESSION_NONE, NULL);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error creating CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* open the new hard drive */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* write the metadata */
	sprintf(metadata, HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, sectorsize);
	err = chd_set_metadata(chd, HARD_DISK_METADATA_TAG, 0, metadata, strlen(metadata) + 1, CHD_MDFLAGS_CHECKSUM);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error adding hard disk metadata: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* alloc and zero buffer*/
	cache = (UINT8 *)malloc(hunksize);
	if (cache == NULL)
	{
		fprintf(stderr, "Error allocating memory buffer\n");
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}
	memset(cache, 0, hunksize);

	/* Zero every hunk */
	for (hunknum = 0; hunknum < chd_get_header(chd)->totalhunks; hunknum++)
	{
		/* progress */
		progress(hunknum == 0, "Zeroing hunk %d/%d...  \r", hunknum, chd_get_header(chd)->totalhunks);

		/* write out the data */
		err = chd_write(chd, hunknum, cache);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error writing CHD file: %s\n", chd_error_string(err));
			goto cleanup;
		}
	}
	progress(TRUE, "Creation complete!                    \n");

cleanup:
	/* close everything down */
	if (cache != NULL)
		free(cache);
	if (chd != NULL)
		chd_close(chd);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_copydata - copy all hunks of data from one
    CHD file to another. The hunk sizes do not
    need to match. If the source is shorter than
    the destination, the source data will be
    padded with 0s.
-------------------------------------------------*/

static int do_copydata(int argc, char *argv[], int param)
{
	const char *inputfile, *outputfile;
	chd_file *outputchd = NULL;
	chd_file *inputchd = NULL;
	chd_error err;

	/* require 4 args total */
	if (argc != 4)
		return usage();

	/* extract the data */
	inputfile = argv[2];
	outputfile = argv[3];

	/* print some info */
	printf("Input file:  %s\n", inputfile);
	printf("Output file: %s\n", outputfile);

	/* open the src hard drive */
	err = chd_open(inputfile, CHD_OPEN_READ, NULL, &inputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening src CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* open the dest hard drive */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &outputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening dest CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* compress the source into the dest */
	err = chdman_compress_chd(outputchd, inputchd, 0);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression: %s\n", chd_error_string(err));

cleanup:
	/* close everything down */
	if (outputchd != NULL)
		chd_close(outputchd);
	if (inputchd != NULL)
		chd_close(inputchd);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_extract - extract a raw file from a
    CHD image
-------------------------------------------------*/

static int do_extract(int argc, char *argv[], int param)
{
	const char *inputfile, *outputfile;
	core_file *outfile = NULL;
	chd_file *infile = NULL;
	const chd_header *header;
	UINT64 bytesremaining;
	void *hunk = NULL;
	file_error filerr;
	chd_error err;
	int hunknum;

	/* require 4 args total */
	if (argc != 4)
		return usage();

	/* extract the data */
	inputfile = argv[2];
	outputfile = argv[3];

	/* print some info */
	printf("Input file:   %s\n", inputfile);
	printf("Output file:  %s\n", outputfile);

	/* get the header */
	err = chd_open(inputfile, CHD_OPEN_READ, NULL, &infile);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, chd_error_string(err));
		goto cleanup;
	}
	header = chd_get_header(infile);

	/* allocate memory to hold a hunk */
	hunk = malloc(header->hunkbytes);
	if (hunk == NULL)
	{
		fprintf(stderr, "Out of memory allocating hunk buffer!\n");
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}

	/* create the output file */
	filerr = core_fopen(outputfile, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &outfile);
	if (filerr != FILERR_NONE)
	{
		fprintf(stderr, "Error opening output file '%s'\n", outputfile);
		err = CHDERR_CANT_CREATE_FILE;
		goto cleanup;
	}

	/* loop over hunks, reading and writing */
	bytesremaining = header->logicalbytes;
	for (hunknum = 0; hunknum < header->totalhunks; hunknum++)
	{
		UINT32 byteswritten, bytes_to_write;

		/* progress */
		progress(hunknum == 0, "Extracting hunk %d/%d...  \r", hunknum, header->totalhunks);

		/* read the hunk into a buffer */
		err = chd_read(infile, hunknum, hunk);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error reading hunk %d from CHD file: %s\n", hunknum, chd_error_string(err));
			goto cleanup;
		}

		/* write the hunk to the file */
		bytes_to_write = MIN(bytesremaining, header->hunkbytes);
		core_fseek(outfile, (UINT64)hunknum * (UINT64)header->hunkbytes, SEEK_SET);
		byteswritten = core_fwrite(outfile, hunk, bytes_to_write);
		if (byteswritten != bytes_to_write)
		{
			fprintf(stderr, "Error writing hunk %d to output file: %s\n", hunknum, chd_error_string(CHDERR_WRITE_ERROR));
			err = CHDERR_WRITE_ERROR;
			goto cleanup;
		}
		bytesremaining -= byteswritten;
	}
	progress(TRUE, "Extraction complete!                    \n");

cleanup:
	/* clean up our mess */
	if (outfile != NULL)
		core_fclose(outfile);
	if (hunk != NULL)
		free(hunk);
	if (infile != NULL)
		chd_close(infile);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_extractcd - extract a CDRDAO .toc/.bin
    or CDRWIN .cue/.bin file from a CHD-CD image
-------------------------------------------------*/

static int do_extractcd(int argc, char *argv[], int param)
{
	const char *inputfile, *outputfile, *outputfile2;
	core_file *outfile2 = NULL;
	chd_file *inputchd = NULL;
	cdrom_file *cdrom = NULL;
	FILE *outfile = NULL;
	const cdrom_toc *toc;
	UINT64 out2offs;
	file_error filerr;
	chd_error err;
	int track, cuemode;

	/* require 5 args total */
	if (argc != 5)
		return usage();

	/* extract the data */
	inputfile = argv[2];
	outputfile = argv[3];
	outputfile2 = argv[4];

	/* print some info */
	printf("Input file:   %s\n", inputfile);
	printf("Output files:  %s and %s\n", outputfile, outputfile2);

	/* get the header */
	err = chd_open(inputfile, CHD_OPEN_READ, NULL, &inputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, chd_error_string(err));
		goto cleanup;
	}

	/* open the CD */
	cdrom = cdrom_open(inputchd);
	if (cdrom == NULL)
	{
		fprintf(stderr, "Error opening CHD-CD '%s'\n", inputfile);
		err = CHDERR_INVALID_FILE;
		goto cleanup;
	}

	/* check for CDRWIN format */
	cuemode = 0;
	if (strstr(outputfile, ".cue"))
	{
		cuemode = 1;
	}

	/* get the TOC data */
	toc = cdrom_get_toc(cdrom);

	/* create the output files */
	outfile = fopen(outputfile, "w");
	if (outfile == NULL)
	{
		fprintf(stderr, "Error opening output file '%s'\n", outputfile);
		err = CHDERR_CANT_CREATE_FILE;
		goto cleanup;
	}

	if (!cuemode)
	{
		fprintf(outfile, "CD_ROM\n\n\n");

		filerr = core_fopen(outputfile2, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &outfile2);
		if (filerr != FILERR_NONE)
		{
			fprintf(stderr, "Error opening output file '%s'\n", outputfile2);
			err = CHDERR_CANT_CREATE_FILE;
			goto cleanup;
		}
	}

	/* process away */
	out2offs = 0;
	for (track = 0; track < toc->numtrks; track++)
	{
		UINT32 m, s, f, frame, trackframes;
		char trackoutname[512];

		if (cuemode)
		{
			sprintf(trackoutname, "%s (track %02d).bin", outputfile2, track+1);

			filerr = core_fopen(trackoutname, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &outfile2);
			if (filerr != FILERR_NONE)
			{
				fprintf(stderr, "Error opening output file '%s'\n", outputfile2);
				err = CHDERR_CANT_CREATE_FILE;
				goto cleanup;
			}

			out2offs = 0;
		}

		progress(TRUE, "Extracting track %d...   \r", track+1);

		trackframes = toc->tracks[track].frames;

		if (cuemode)
		{
			char modestr[16];

			fprintf(outfile, "FILE \"%s\" BINARY\n", trackoutname);

			switch (toc->tracks[track].trktype)
			{
				case CD_TRACK_MODE1:
				case CD_TRACK_MODE1_RAW:
					sprintf(modestr, "MODE1/%04d", toc->tracks[track].datasize);
					break;

				case CD_TRACK_MODE2:
				case CD_TRACK_MODE2_FORM1:
				case CD_TRACK_MODE2_FORM2:
				case CD_TRACK_MODE2_FORM_MIX:
				case CD_TRACK_MODE2_RAW:
					sprintf(modestr, "MODE2/%04d", toc->tracks[track].datasize);
					break;

				case CD_TRACK_AUDIO:
					strcpy(modestr, "AUDIO");
					break;
			}

			fprintf(outfile, "  TRACK %02d %s\n", track+1, modestr);

			fprintf(outfile, "    INDEX 00 00:00:00\n");

			if (toc->tracks[track].pregap > 0)
			{
				f = toc->tracks[track].pregap;
				s = f / 75;
				f %= 75;
				m = s / 60;
				s %= 60;
				fprintf(outfile, "    INDEX 01 %02d:%02d:%02d\n", m, s, f);
			}

			if (toc->tracks[track].postgap > 0)
			{
				f = toc->tracks[track].postgap;
				s = f / 75;
				f %= 75;
				m = s / 60;
				s %= 60;
				fprintf(outfile, "  POSTGAP %02d:%02d:%02d\n", m, s, f);
			}
		}
		else
		{
			char modesubmode[64];

			fprintf(outfile, "// Track %d\n", track+1);

			/* write out the track type */
			if (toc->tracks[track].subtype != CD_SUB_NONE)
			{
				sprintf(modesubmode, "%s %s", cdrom_get_type_string(toc->tracks[track].trktype), cdrom_get_subtype_string(toc->tracks[track].subtype));
			}
			else
			{
				sprintf(modesubmode, "%s", cdrom_get_type_string(toc->tracks[track].trktype));
			}

			fprintf(outfile, "TRACK %s\n", modesubmode);

			/* write out the attributes */
			fprintf(outfile, "NO COPY\n");
			if (toc->tracks[track].trktype == CD_TRACK_AUDIO)
			{
				fprintf(outfile, "NO PRE_EMPHASIS\n");
				fprintf(outfile, "TWO_CHANNEL_AUDIO\n");
			}

			if (toc->tracks[track].pregap > 0)
			{
				f = toc->tracks[track].pregap;
				s = f / 75;
				f %= 75;
				m = s / 60;
				s %= 60;

				fprintf(outfile, "ZERO %s %02d:%02d:%02d\n", modesubmode, m, s, f);
			}

			/* convert to minutes/seconds/frames */
			f = trackframes;
			s = f / 75;
			f %= 75;
			m = s / 60;
			s %= 60;

			/* all tracks but the first one have a file offset */
			if (track > 0)
				fprintf(outfile, "DATAFILE \"%s\" #%d %02d:%02d:%02d // length in bytes: %d\n", outputfile2, (UINT32)out2offs, m, s, f, trackframes*(toc->tracks[track].datasize+toc->tracks[track].subsize));
			else
				fprintf(outfile, "DATAFILE \"%s\" %02d:%02d:%02d // length in bytes: %d\n", outputfile2, m, s, f, trackframes*(toc->tracks[track].datasize+toc->tracks[track].subsize));

			/* tracks with pregaps get a START marker too */
			if (toc->tracks[track].pregap > 0)
			{
				f = toc->tracks[track].pregap;
				s = f / 75;
				f %= 75;
				m = s / 60;
				s %= 60;

				fprintf(outfile, "START %02d:%02d:%02d\n", m, s, f);
			}

			fprintf(outfile, "\n\n");
		}

		/* now write the actual data */
		for (frame = 0; frame < trackframes; frame++)
		{
			UINT8 sector[CD_MAX_SECTOR_DATA + CD_MAX_SUBCODE_DATA];
			UINT32 byteswritten;

			progress(frame == 0, "Extracting track %d... %d/%d...   \r", track+1, frame, trackframes);

			/* read the raw data */
			cdrom_read_data(cdrom, cdrom_get_track_start(cdrom, track) + frame, sector, toc->tracks[track].trktype);

			/* write it out */
			core_fseek(outfile2, out2offs, SEEK_SET);
			byteswritten = core_fwrite(outfile2, sector, toc->tracks[track].datasize);
			if (byteswritten != toc->tracks[track].datasize)
			{
				fprintf(stderr, "Error writing frame %d to output file: %s\n", frame, chd_error_string(CHDERR_WRITE_ERROR));
				err = CHDERR_WRITE_ERROR;
				goto cleanup;
			}
			out2offs += toc->tracks[track].datasize;

			/* read the subcode data */
			if (toc->tracks[track].subtype != CD_SUB_NONE)
			{
				cdrom_read_subcode(cdrom, cdrom_get_track_start(cdrom, track) + frame, sector);

				/* write it out */
				core_fseek(outfile2, out2offs, SEEK_SET);
				byteswritten = core_fwrite(outfile2, sector, toc->tracks[track].subsize);
				if (byteswritten != toc->tracks[track].subsize)
				{
					fprintf(stderr, "Error writing frame %d to output file: %s\n", frame, chd_error_string(CHDERR_WRITE_ERROR));
					err = CHDERR_WRITE_ERROR;
					goto cleanup;
				}
				out2offs += toc->tracks[track].subsize;
			}
		}
		progress(TRUE, "Extracting track %d... complete         \n", track+1);

		if (cuemode)
		{
			core_fclose(outfile2);
			outfile2 = NULL;
		}
	}
	progress(TRUE, "Completed!\n");

cleanup:
	/* close everything down */
	if (outfile != NULL)
		fclose(outfile);
	if (outfile2 != NULL)
		core_fclose(outfile2);
	if (cdrom != NULL)
		cdrom_close(cdrom);
	if (inputchd != NULL)
		chd_close(inputchd);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_extractav - extract an AVI file from a
    CHD image
-------------------------------------------------*/

static int do_extractav(int argc, char *argv[], int param)
{
	int fps, fpsfrac, width, height, interlaced, channels, rate, totalframes;
	av_codec_decompress_config avconfig = { 0 };
	const char *inputfile, *outputfile;
	UINT32 firstframe, numframes;
	bitmap_t *fullbitmap = NULL;
	UINT32 framenum, numsamples;
	UINT32 fps_times_1million;
	const chd_header *header;
	chd_file *chd = NULL;
	avi_file *avi = NULL;
	bitmap_t fakebitmap;
	avi_movie_info info;
	char metadata[256];
	avi_error avierr;
	chd_error err;
	int chnum;

	/* require 4-6 args total */
	if (argc < 4 || argc > 6)
		return usage();

	/* extract the data */
	inputfile = argv[2];
	outputfile = argv[3];
	firstframe = (argc > 4) ? atoi(argv[4]) : 0;
	numframes = (argc > 5) ? atoi(argv[5]) : 1000000;

	/* print some info */
	printf("Input file:   %s\n", inputfile);
	printf("Output file:  %s\n", outputfile);

	/* get the header */
	err = chd_open(inputfile, CHD_OPEN_READ, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, chd_error_string(err));
		goto cleanup;
	}
	header = chd_get_header(chd);

	/* get the metadata */
	err = chd_get_metadata(chd, AV_METADATA_TAG, 0, metadata, sizeof(metadata), NULL, NULL, NULL);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error getting A/V metadata: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* extract the info */
	if (sscanf(metadata, AV_METADATA_FORMAT, &fps, &fpsfrac, &width, &height, &interlaced, &channels, &rate) != 7)
	{
		fprintf(stderr, "Improperly formatted metadata\n");
		err = CHDERR_INVALID_METADATA;
		goto cleanup;
	}
	fps_times_1million = fps * 1000000 + fpsfrac;
	totalframes = header->totalhunks;

	/* adjust for interlacing */
	if (interlaced)
	{
		fps_times_1million /= 2;
		height *= 2;
		firstframe *= 2;
		numframes *= 2;
	}
	numframes = MIN(totalframes - firstframe, numframes);

	/* allocate a video buffer */
	fullbitmap = bitmap_alloc(width, height, BITMAP_FORMAT_YUY16);
	if (fullbitmap ==  NULL)
	{
		fprintf(stderr, "Out of memory allocating temporary bitmap\n");
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}
	avconfig.video = &fakebitmap;

	/* allocate audio buffers */
	avconfig.maxsamples = ((UINT64)rate * 1000000 + fps_times_1million - 1) / fps_times_1million;
	avconfig.actsamples = &numsamples;
	for (chnum = 0; chnum < channels; chnum++)
	{
		avconfig.audio[chnum] = (INT16 *)malloc(avconfig.maxsamples * 2);
		if (avconfig.audio[chnum] == NULL)
		{
			fprintf(stderr, "Out of memory allocating temporary audio buffer\n");
			err = CHDERR_OUT_OF_MEMORY;
			goto cleanup;
		}
	}

	/* print some of it */
	printf("Use frames:   %d-%d\n", firstframe, firstframe + numframes - 1);
	printf("Frame rate:   %d.%06d\n", fps_times_1million / 1000000, fps_times_1million % 1000000);
	printf("Frame size:   %d x %d %s\n", width, height, interlaced ? "interlaced" : "non-interlaced");
	printf("Audio:        %d channels at %d Hz\n", channels, rate);
	printf("Total frames: %d (%02d:%02d:%02d)\n", totalframes,
			(UINT32)((UINT64)totalframes * 1000000 / fps_times_1million / 60 / 60),
			(UINT32)(((UINT64)totalframes * 1000000 / fps_times_1million / 60) % 60),
			(UINT32)(((UINT64)totalframes * 1000000 / fps_times_1million) % 60));

	/* build up the movie info */
	info.video_format = FORMAT_YUY2;
	info.video_timescale = fps_times_1million;
	info.video_sampletime = 1000000;
	info.video_width = width;
	info.video_height = height;
	info.video_depth = 16;
	info.audio_format = 0;
	info.audio_timescale = rate;
	info.audio_sampletime = 1;
	info.audio_channels = channels;
	info.audio_samplebits = 16;
	info.audio_samplerate = rate;

	/* create the output file */
	avierr = avi_create(outputfile, &info, &avi);
	if (avierr != AVIERR_NONE)
	{
		fprintf(stderr, "Error opening output file '%s': %s\n", outputfile, avi_error_string(avierr));
		err = CHDERR_CANT_CREATE_FILE;
		goto cleanup;
	}

	/* loop over hunks, reading and writing */
	for (framenum = 0; framenum < numframes; framenum++)
	{
		/* progress */
		progress(framenum == 0, "Extracting hunk %d/%d...  \r", framenum, numframes);

		/* set up the fake bitmap for this frame */
		bitmap_clone_existing(avconfig.video, fullbitmap);
		if (interlaced)
		{
			avconfig.video->base = BITMAP_ADDR16(avconfig.video, framenum % 2, 0);
			avconfig.video->rowpixels *= 2;
			avconfig.video->height /= 2;
		}

		/* configure the decompressor for this frame */
		chd_codec_config(chd, AV_CODEC_DECOMPRESS_CONFIG, &avconfig);

		/* read the hunk into the buffers */
		err = chd_read(chd, firstframe + framenum, NULL);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error reading hunk %d from CHD file: %s\n", firstframe + framenum, chd_error_string(err));
			goto cleanup;
		}

		/* write audio */
		for (chnum = 0; chnum < channels; chnum++)
		{
			avierr = avi_append_sound_samples(avi, chnum, avconfig.audio[chnum], numsamples, 0);
			if (avierr != AVIERR_NONE)
			{
				fprintf(stderr, "Error writing samples for hunk %d to AVI file: %s\n", firstframe + framenum, avi_error_string(avierr));
				goto cleanup;
			}
		}

		/* write video */
		if (!interlaced || (firstframe + framenum) % 2 == 1)
		{
			avierr = avi_append_video_frame_yuy16(avi, fullbitmap);
			if (avierr != AVIERR_NONE)
			{
				fprintf(stderr, "Error writing video for hunk %d to AVI file: %s\n", firstframe + framenum, avi_error_string(avierr));
				goto cleanup;
			}
		}
	}
	progress(TRUE, "Extraction complete!                    \n");

cleanup:
	/* clean up our mess */
	if (avi != NULL)
		avi_close(avi);
	for (chnum = 0; chnum < ARRAY_LENGTH(avconfig.audio); chnum++)
		if (avconfig.audio[chnum] != NULL)
			free(avconfig.audio[chnum]);
	if (fullbitmap != NULL)
		bitmap_free(fullbitmap);
	if (chd != NULL)
		chd_close(chd);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_verify - validate the MD5/SHA1 on a drive
    image
-------------------------------------------------*/

static int do_verify(int argc, char *argv[], int param)
{
	chd_verify_result verify;
	const char *inputfile;
	chd_file *chd = NULL;
	chd_header header;
	int fixed = FALSE;
	chd_error err;
	int i;

	/* require 3 args total */
	if (argc != 3)
		return usage();

	/* extract the data */
	inputfile = argv[2];

	/* print some info */
	printf("Input file:   %s\n", inputfile);

	/* open the CHD file */
	err = chd_open(inputfile, CHD_OPEN_READ, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}
	header = *chd_get_header(chd);

	/* verify the CHD data */
	err = chd_verify_begin(chd);
	if (err == CHDERR_NONE)
	{
		UINT32 hunknum;
		for (hunknum = 0; hunknum < header.totalhunks; hunknum++)
		{
			/* progress */
			progress(FALSE, "Verifying hunk %d/%d... \r", hunknum, header.totalhunks);

			/* verify the data */
			err = chd_verify_hunk(chd);
			if (err != CHDERR_NONE)
				break;
		}

		/* finish it */
		if (err == CHDERR_NONE)
			err = chd_verify_finish(chd, &verify);
	}

	/* handle errors */
	if (err != CHDERR_NONE)
	{
		if (err == CHDERR_CANT_VERIFY)
			fprintf(stderr, "Can't verify this type of image (probably writeable)\n");
		else
			fprintf(stderr, "\nError during verify: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* verify the MD5 */
	if (header.version <= 3)
	{
		if (memcmp(header.md5, verify.md5, sizeof(header.md5)) == 0)
			printf("MD5 verification successful!\n");
		else
		{
			fprintf(stderr, "Error: MD5 in header = ");
			for (i = 0; i < CHD_MD5_BYTES; i++)
				fprintf(stderr, "%02x", header.md5[i]);
			fprintf(stderr, "\n");
			fprintf(stderr, "          actual MD5 = ");
			for (i = 0; i < CHD_MD5_BYTES; i++)
				fprintf(stderr, "%02x", verify.md5[i]);
			fprintf(stderr, "\n");

			/* fix it */
			if (param)
			{
				memcpy(header.md5, verify.md5, sizeof(header.md5));
				fixed = TRUE;
			}
		}
	}

	/* verify the SHA1 */
	if (header.version >= 3)
	{
		if (memcmp(header.sha1, verify.sha1, sizeof(header.sha1)) == 0)
			printf("SHA1 verification successful!\n");
		else
		{
			fprintf(stderr, "Error: SHA1 in header = ");
			for (i = 0; i < CHD_SHA1_BYTES; i++)
				fprintf(stderr, "%02x", header.sha1[i]);
			fprintf(stderr, "\n");
			fprintf(stderr, "          actual SHA1 = ");
			for (i = 0; i < CHD_SHA1_BYTES; i++)
				fprintf(stderr, "%02x", verify.sha1[i]);
			fprintf(stderr, "\n");

			/* fix it */
			if (param)
			{
				memcpy(header.sha1, verify.sha1, sizeof(header.sha1));
				fixed = TRUE;
			}

			/* verify the raw SHA1 */
			if (header.version >= 4)
			{
				if (memcmp(header.rawsha1, verify.rawsha1, sizeof(header.rawsha1)) != 0)
				{
					fprintf(stderr, "Error: raw SHA1 in header = ");
					for (i = 0; i < CHD_SHA1_BYTES; i++)
						fprintf(stderr, "%02x", header.rawsha1[i]);
					fprintf(stderr, "\n");
					fprintf(stderr, "          actual raw SHA1 = ");
					for (i = 0; i < CHD_SHA1_BYTES; i++)
						fprintf(stderr, "%02x", verify.rawsha1[i]);
					fprintf(stderr, "\n");

					/* fix it */
					if (param)
					{
						memcpy(header.rawsha1, verify.rawsha1, sizeof(header.rawsha1));
						fixed = TRUE;
					}
				}
			}
		}
	}

	/* close the drive */
	chd_close(chd);
	chd = NULL;

	/* update the header */
	if (fixed)
	{
		err = chd_set_header(inputfile, &header);
		if (err != CHDERR_NONE)
			fprintf(stderr, "Error writing new header: %s\n", chd_error_string(err));
		else
			printf("Updated header successfully\n");
	}

cleanup:
	/* close everything down */
	if (chd != NULL)
		chd_close(chd);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_fixavdata - fix the AV metadata for an
    A/V file
-------------------------------------------------*/

static int do_fixavdata(int argc, char *argv[], int param)
{
	int fps, fpsfrac, width, height, interlaced, channels, rate;
	av_codec_decompress_config avconfig = { 0 };
	bitmap_t *fullbitmap = NULL;
	const char *inputfile;
	UINT8 *vbidata = NULL;
	int writeable = FALSE;
	chd_file *chd = NULL;
	bitmap_t fakebitmap;
	char metadata[256];
	chd_header header;
	UINT32 actlength;
	UINT32 framenum;
	chd_error err;
	int fixframes = 0;
	int fixes = 0;

	/* require 3 args total */
	if (argc != 3)
		return usage();

	/* extract the data */
	inputfile = argv[2];

	/* print some info */
	printf("Input file:   %s\n", inputfile);

	/* get the header */
	err = chd_open(inputfile, CHD_OPEN_READ, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, chd_error_string(err));
		goto cleanup;
	}
	header = *chd_get_header(chd);

	/* get the metadata */
	err = chd_get_metadata(chd, AV_METADATA_TAG, 0, metadata, sizeof(metadata), NULL, NULL, NULL);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error getting A/V metadata: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* extract the info */
	if (sscanf(metadata, AV_METADATA_FORMAT, &fps, &fpsfrac, &width, &height, &interlaced, &channels, &rate) != 7)
	{
		fprintf(stderr, "Improperly formatted metadata\n");
		err = CHDERR_INVALID_METADATA;
		goto cleanup;
	}

	/* allocate space for the frame data */
	if ((height != 524/2 && height != 624/2) || !interlaced)
	{
		fprintf(stderr, "This file does not need VBI metadata\n");
		err = CHDERR_INVALID_METADATA;
		goto cleanup;
	}

	/* allocate a video buffer */
	fullbitmap = bitmap_alloc(width, height, BITMAP_FORMAT_YUY16);
	if (fullbitmap ==  NULL)
	{
		fprintf(stderr, "Out of memory allocating temporary bitmap\n");
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}
	avconfig.video = &fakebitmap;

	/* allocate memory for VBI data */
	vbidata = (UINT8 *)malloc(header.totalhunks * VBI_PACKED_BYTES);
	if (vbidata == NULL)
	{
		fprintf(stderr, "Out of memory allocating VBI data\n");
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}

	/* read the metadata */
	err = chd_get_metadata(chd, AV_LD_METADATA_TAG, 0, vbidata, header.totalhunks * VBI_PACKED_BYTES, &actlength, NULL, NULL);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error getting VBI metadata: %s\n", chd_error_string(err));
		memset(vbidata, 0, header.totalhunks * VBI_PACKED_BYTES);
		fixes++;
	}
	if (actlength != header.totalhunks * VBI_PACKED_BYTES)
	{
		fprintf(stderr, "VBI metadata incorrect size\n");
		memset(vbidata, 0, header.totalhunks * VBI_PACKED_BYTES);
		fixes++;
	}

	/* loop over hunks, reading */
	for (framenum = 0; framenum < header.totalhunks; framenum++)
	{
		vbi_metadata origvbi;
		vbi_metadata vbi;
		UINT32 vbiframe;

		/* progress */
		progress(framenum == 0, "Processing hunk %d/%d...  \r", framenum, header.totalhunks);

		/* set up the fake bitmap for this frame */
		bitmap_clone_existing(avconfig.video, fullbitmap);

		/* configure the decompressor for this frame */
		chd_codec_config(chd, AV_CODEC_DECOMPRESS_CONFIG, &avconfig);

		/* read the hunk into the buffers */
		err = chd_read(chd, framenum, NULL);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error reading hunk %d from CHD file: %s\n", framenum, chd_error_string(err));
			goto cleanup;
		}

		/* unpack the current data for this frame */
		vbi_metadata_unpack(&origvbi, &vbiframe, &vbidata[framenum * VBI_PACKED_BYTES]);

		/* parse the video data */
		vbi_parse_all((const UINT16 *)avconfig.video->base, avconfig.video->rowpixels, avconfig.video->width, 8, &vbi);

		/* verify the data */
		if (vbiframe != 0 || origvbi.white != 0 || origvbi.line16 != 0 || origvbi.line17 != 0 || origvbi.line18 != 0 || origvbi.line1718 != 0)
		{
			int errors = 0;

			if (vbiframe != framenum)
			{
				fprintf(stderr, "%d:Frame mismatch in VBI data (%d, should be %d)\n", framenum, vbiframe, framenum);
				errors++;
			}
			if (vbi.white != origvbi.white)
			{
				fprintf(stderr, "%d:White flag mismatch in VBI data (%d, should be %d)\n", framenum, origvbi.white, vbi.white);
				errors++;
			}
			if (vbi.line16 != origvbi.line16)
			{
				fprintf(stderr, "%d:Line 16 mismatch in VBI data (%06X, should be %06X)\n", framenum, origvbi.line16, vbi.line16);
				errors++;
			}
			if (vbi.line17 != origvbi.line17)
			{
				fprintf(stderr, "%d:Line 17 mismatch in VBI data (%06X, should be %06X)\n", framenum, origvbi.line17, vbi.line17);
				errors++;
			}
			if (vbi.line18 != origvbi.line18)
			{
				fprintf(stderr, "%d:Line 18 mismatch in VBI data (%06X, should be %06X)\n", framenum, origvbi.line18, vbi.line18);
				errors++;
			}
			if (vbi.line1718 != origvbi.line1718)
			{
				fprintf(stderr, "%d:Line 17/18 mismatch in VBI data (%06X, should be %06X)\n", framenum, origvbi.line1718, vbi.line1718);
				errors++;
			}
			fixes += errors;
			fixframes += (errors != 0);
		}

		/* pack the new data */
		vbi_metadata_pack(&vbidata[framenum * VBI_PACKED_BYTES], framenum, &vbi);
	}
	progress(TRUE, "Processing complete!                                     \n");

	/* print final results */
	if (fixes == 0)
		printf("\nNo fixes required\n");
	else
		printf("\nFound %d errors on %d frames\n", fixes, fixframes);

	/* close the drive */
	chd_close(chd);
	chd = NULL;

	/* apply fixes */
	if (fixes > 0)
	{
		/* mark the CHD writeable */
		header.flags |= CHDFLAGS_IS_WRITEABLE;
		err = chd_set_header(inputfile, &header);
		if (err != CHDERR_NONE)
			fprintf(stderr, "Error writing new header: %s\n", chd_error_string(err));
		header.flags &= ~CHDFLAGS_IS_WRITEABLE;
		writeable = TRUE;

		/* open the file */
		err = chd_open(inputfile, CHD_OPEN_READWRITE, NULL, &chd);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, chd_error_string(err));
			goto cleanup;
		}

		/* write new metadata */
		err = chd_set_metadata(chd, AV_LD_METADATA_TAG, 0, vbidata, header.totalhunks * VBI_PACKED_BYTES, CHD_MDFLAGS_CHECKSUM);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error adding AVLD metadata: %s\n", chd_error_string(err));
			goto cleanup;
		}
		else
			printf("Updated metadata written successfully\n");

		/* allow cleanup code to close the file and revert the header */
	}

cleanup:
	/* clean up our mess */
	if (fullbitmap != NULL)
		bitmap_free(fullbitmap);
	if (vbidata != NULL)
		free(vbidata);
	if (chd != NULL)
		chd_close(chd);
	if (writeable)
		chd_set_header(inputfile, &header);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_info - dump the header information from
    a drive image
-------------------------------------------------*/

static int do_info(int argc, char *argv[], int param)
{
	const char *inputfile;
	chd_file *chd = NULL;
	UINT8 metadata[256];
	chd_header header;
	chd_error err;
	int i, j;

	/* require 3 args total */
	if (argc != 3)
		return usage();

	/* extract the data */
	inputfile = argv[2];

	/* print some info */
	printf("Input file:   %s\n", inputfile);

	/* get the header */
	err = chd_open(inputfile, CHD_OPEN_READ, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, chd_error_string(err));
		goto cleanup;
	}
	header = *chd_get_header(chd);

	/* print the info */
	printf("Header Size:  %d bytes\n", header.length);
	printf("File Version: %d\n", header.version);
	printf("Flags:        %s, %s\n",
			(header.flags & CHDFLAGS_HAS_PARENT) ? "HAS_PARENT" : "NO_PARENT",
			(header.flags & CHDFLAGS_IS_WRITEABLE) ? "WRITEABLE" : "READ_ONLY");
	printf("Compression:  %s\n", chd_get_codec_name(header.compression));
	printf("Hunk Size:    %d bytes\n", header.hunkbytes);
	printf("Total Hunks:  %d\n", header.totalhunks);
	printf("Logical size: %s bytes\n", big_int_string(header.logicalbytes));
	if (!(header.flags & CHDFLAGS_IS_WRITEABLE) && header.version <= 3)
		printf("MD5:          %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
				header.md5[0], header.md5[1], header.md5[2], header.md5[3],
				header.md5[4], header.md5[5], header.md5[6], header.md5[7],
				header.md5[8], header.md5[9], header.md5[10], header.md5[11],
				header.md5[12], header.md5[13], header.md5[14], header.md5[15]);
	if (!(header.flags & CHDFLAGS_IS_WRITEABLE) && header.version >= 3)
		printf("SHA1:         %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
				header.sha1[0], header.sha1[1], header.sha1[2], header.sha1[3],
				header.sha1[4], header.sha1[5], header.sha1[6], header.sha1[7],
				header.sha1[8], header.sha1[9], header.sha1[10], header.sha1[11],
				header.sha1[12], header.sha1[13], header.sha1[14], header.sha1[15],
				header.sha1[16], header.sha1[17], header.sha1[18], header.sha1[19]);
	if (!(header.flags & CHDFLAGS_IS_WRITEABLE) && header.version >= 4)
	{
		printf("Raw SHA1:     %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
				header.rawsha1[0], header.rawsha1[1], header.rawsha1[2], header.rawsha1[3],
				header.rawsha1[4], header.rawsha1[5], header.rawsha1[6], header.rawsha1[7],
				header.rawsha1[8], header.rawsha1[9], header.rawsha1[10], header.rawsha1[11],
				header.rawsha1[12], header.rawsha1[13], header.rawsha1[14], header.rawsha1[15],
				header.rawsha1[16], header.rawsha1[17], header.rawsha1[18], header.rawsha1[19]);
	}
	if (header.flags & CHDFLAGS_HAS_PARENT)
	{
		printf("Parent MD5:   %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
				header.parentmd5[0], header.parentmd5[1], header.parentmd5[2], header.parentmd5[3],
				header.parentmd5[4], header.parentmd5[5], header.parentmd5[6], header.parentmd5[7],
				header.parentmd5[8], header.parentmd5[9], header.parentmd5[10], header.parentmd5[11],
				header.parentmd5[12], header.parentmd5[13], header.parentmd5[14], header.parentmd5[15]);
		if (header.version >= 3)
			printf("Parent SHA1:  %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
					header.parentsha1[0], header.parentsha1[1], header.parentsha1[2], header.parentsha1[3],
					header.parentsha1[4], header.parentsha1[5], header.parentsha1[6], header.parentsha1[7],
					header.parentsha1[8], header.parentsha1[9], header.parentsha1[10], header.parentsha1[11],
					header.parentsha1[12], header.parentsha1[13], header.parentsha1[14], header.parentsha1[15],
					header.parentsha1[16], header.parentsha1[17], header.parentsha1[18], header.parentsha1[19]);
	}

	/* print out metadata */
	for (i = 0; ; i++)
	{
		UINT32 metatag, metasize;

		/* get the indexed metadata item; stop when we hit an error */
		err = chd_get_metadata(chd, CHDMETATAG_WILDCARD, i, metadata, sizeof(metadata), &metasize, &metatag, NULL);
		if (err != CHDERR_NONE)
			break;

		/* print either a string representation or a hex representation of the tag */
		if (isprint((metatag >> 24) & 0xff) && isprint((metatag >> 16) & 0xff) && isprint((metatag >> 8) & 0xff) && isprint(metatag & 0xff))
			printf("Metadata:     Tag='%c%c%c%c'    Length=%d\n", (metatag >> 24) & 0xff, (metatag >> 16) & 0xff, (metatag >> 8) & 0xff, metatag & 0xff, metasize);
		else
			printf("Metadata:     Tag=%08x  Length=%d\n", metatag, metasize);
		printf("              ");

		/* print up to 60 characters of metadata */
		metasize = MIN(60, metasize);
		for (j = 0; j < metasize; j++)
			printf("%c", isprint(metadata[j]) ? metadata[j] : '.');
		printf("\n");
	}

cleanup:
	/* close everything down */
	if (chd != NULL)
		chd_close(chd);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    handle_custom_chomp - custom chomp a file
-------------------------------------------------*/

#if ENABLE_CUSTOM_CHOMP
static chd_error handle_custom_chomp(const char *name, chd_file *chd, UINT32 *maxhunk)
{
	const chd_header *header = chd_get_header(chd);
	int sectors_per_hunk = (header->hunkbytes / IDE_SECTOR_SIZE);
	chd_error err = CHDERR_INVALID_DATA;
	UINT8 *temp = NULL;

	/* allocate memory to hold a hunk */
	temp = malloc(header->hunkbytes);
	if (temp == NULL)
	{
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}

	/* check for midway */
	if (strcmp(name, "midway") == 0)
	{
		UINT32 maxsector = 0;
		UINT32 numparts;
		chd_error err;
		int i;

		/* read sector 0 */
		err = chd_read(chd, 0, temp);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error reading hunk 0\n");
			goto cleanup;
		}

		/* look for the signature */
		if (temp[0] != 0x54 || temp[1] != 0x52 || temp[2] != 0x41 || temp[3] != 0x50)
			goto cleanup;

		/* determine the number of partitions */
		numparts = temp[4] | (temp[5] << 8) | (temp[6] << 16) | (temp[7] << 24);
		printf("%d partitions\n", numparts);

		/* get the partition information for each one and track the maximum referenced sector */
		for (i = 0; i < numparts; i++)
		{
			UINT32 pstart = temp[i*12 + 8] | (temp[i*12 + 9] << 8) | (temp[i*12 + 10] << 16) | (temp[i*12 + 11] << 24);
			UINT32 psize  = temp[i*12 + 12] | (temp[i*12 + 13] << 8) | (temp[i*12 + 14] << 16) | (temp[i*12 + 15] << 24);
			UINT32 pflags = temp[i*12 + 16] | (temp[i*12 + 17] << 8) | (temp[i*12 + 18] << 16) | (temp[i*12 + 19] << 24);
			printf("  %2d. %7d - %7d (%X)\n", i, pstart, pstart + psize - 1, pflags);
			if (i != 0 && pstart + psize > maxsector)
				maxsector = pstart + psize;
		}

		/* the maximum hunk is the one that contains the last sector */
		*maxhunk = (maxsector + sectors_per_hunk - 1) / sectors_per_hunk;
		printf("Maximum hunk: %d\n", *maxhunk);

		/* warn if there will be no effect */
		if (*maxhunk >= header->totalhunks)
		{
			printf("Warning: chomp will have no effect\n");
			*maxhunk = header->totalhunks;
		}
	}

	/* check for atari */
	if (strcmp(name, "atari") == 0)
	{
		UINT32 sectors[4];
		UINT8 *data;
		int i, maxdiff;
		chd_error err;

		/* read the second sector */
		err = chd_read(chd, 0x200 / header->hunkbytes, temp);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error reading sector 1\n");
			goto cleanup;
		}
		data = &temp[0x200 % header->hunkbytes];

		/* look for the signature */
		if (data[0] != 0x0d || data[1] != 0xf0 || data[2] != 0xed || data[3] != 0xfe)
			goto cleanup;

		/* loop over 4 partitions and compute the number of sectors in each */
		for (i = 0; i < 4; i++)
			sectors[i] = data[i*4+0x40] | (data[i*4+0x41] << 8) | (data[i*4+0x42] << 16) | (data[i*4+0x43] << 24);
		maxdiff = sectors[2] - sectors[1];
		if (sectors[3] - sectors[2] > maxdiff)
			maxdiff = sectors[3] - sectors[2];
		if (sectors[0] != 8)
			goto cleanup;

		/* the maximum hunk is the end of the fourth copy of the data */
		*maxhunk = (sectors[3] + maxdiff + sectors_per_hunk - 1) / sectors_per_hunk;
		printf("Maximum hunk: %d\n", *maxhunk);

		/* warn if there will be no effect */
		if (*maxhunk >= header->totalhunks)
		{
			fprintf(stderr, "Warning: chomp will have no effect\n");
			*maxhunk = header->totalhunks;
		}
	}

	/* if we fall through, there was no error */
	err = CHDERR_NONE;

cleanup:
	if (temp != NULL)
		free(temp);
	if (err == CHDERR_INVALID_DATA)
		fprintf(stderr, "Error: unable to identify file or compute chomping size.\n");
	return err;
}
#endif


/*-------------------------------------------------
    do_merge_update_chomp - merge a parent and its
    child together (also works for update & chomp)
-------------------------------------------------*/

static int do_merge_update_chomp(int argc, char *argv[], int param)
{
	const char *parentfile, *inputfile, *outputfile;
	const chd_header *inputheader;
	chd_file *parentchd = NULL;
	chd_file *outputchd = NULL;
	chd_file *inputchd = NULL;
	UINT32 maxhunk = ~0;
	chd_error err;

	/* require 4-5 args total */
	if (param == OPERATION_UPDATE && argc != 4)
		return usage();
	if ((param == OPERATION_MERGE || param == OPERATION_CHOMP) && argc != 5)
		return usage();

	/* extract the data */
	if (param == OPERATION_MERGE)
	{
		parentfile = argv[2];
		inputfile = argv[3];
		outputfile = argv[4];
	}
	else
	{
		parentfile = NULL;
		inputfile = argv[2];
		outputfile = argv[3];
		if (param == OPERATION_CHOMP)
			maxhunk = atoi(argv[4]);
	}

	/* print some info */
	if (parentfile != NULL)
	{
		printf("Parent file:  %s\n", parentfile);
		printf("Diff file:    %s\n", inputfile);
	}
	else
		printf("Input file:   %s\n", inputfile);
	printf("Output file:  %s\n", outputfile);
	if (param == OPERATION_CHOMP)
		printf("Maximum hunk: %d\n", maxhunk);

	/* open the parent CHD */
	if (parentfile != NULL)
	{
		err = chd_open(parentfile, CHD_OPEN_READ, NULL, &parentchd);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error opening CHD file '%s': %s\n", parentfile, chd_error_string(err));
			goto cleanup;
		}
	}

	/* open the diff CHD */
	err = chd_open(inputfile, CHD_OPEN_READ, parentchd, &inputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, chd_error_string(err));
		goto cleanup;
	}
	inputheader = chd_get_header(inputchd);

#if ENABLE_CUSTOM_CHOMP
	/* if we're chomping with a auto parameter, now is the time to figure it out */
	if (param == OPERATION_CHOMP && maxhunk == 0)
		if (handle_custom_chomp(argv[4], inputchd, &maxhunk) != CHDERR_NONE)
			return 1;
#endif

	/* create the new merged CHD */
	err = chd_create(outputfile, inputheader->logicalbytes, inputheader->hunkbytes, inputheader->compression, NULL);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error creating CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* open the new CHD */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &outputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* clone the metadata from the input file (which should have inherited from the parent) */
	err = chdman_clone_metadata(inputchd, outputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error cloning metadata: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* do the compression; our interface will route reads for us */
	err = chdman_compress_chd(outputchd, inputchd, (param == OPERATION_CHOMP) ? (maxhunk + 1) : 0);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression: %s\n", chd_error_string(err));

cleanup:
	/* close everything down */
	if (outputchd != NULL)
		chd_close(outputchd);
	if (inputchd != NULL)
		chd_close(inputchd);
	if (parentchd != NULL)
		chd_close(parentchd);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_diff - generate a difference between two
    CHD files
-------------------------------------------------*/

static int do_diff(int argc, char *argv[], int param)
{
	const char *parentfile = NULL, *inputfile = NULL, *outputfile = NULL;
	chd_file *parentchd = NULL;
	chd_file *outputchd = NULL;
	chd_file *inputchd = NULL;
	chd_error err;

	/* require 5 args total */
	if (argc != 5)
		return usage();

	/* extract the data */
	parentfile = argv[2];
	inputfile = argv[3];
	outputfile = argv[4];

	/* print some info */
	printf("Parent file:  %s\n", parentfile);
	printf("Input file:   %s\n", inputfile);
	printf("Diff file:    %s\n", outputfile);

	/* open the soon-to-be-parent CHD */
	err = chd_open(parentfile, CHD_OPEN_READ, NULL, &parentchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s': %s\n", parentfile, chd_error_string(err));
		goto cleanup;
	}

	/* open the input CHD */
	err = chd_open(inputfile, CHD_OPEN_READ, NULL, &inputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, chd_error_string(err));
		goto cleanup;
	}

	/* create the new CHD as a diff against the parent */
	err = chd_create(outputfile, 0, 0, chd_get_header(parentchd)->compression, parentchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error creating CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* open the new CHD */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, parentchd, &outputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* do the compression; our interface will route reads for us */
	err = chdman_compress_chd(outputchd, inputchd, 0);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression: %s\n", chd_error_string(err));

cleanup:
	/* close everything down */
	if (outputchd != NULL)
		chd_close(outputchd);
	if (inputchd != NULL)
		chd_close(inputchd);
	if (parentchd != NULL)
		chd_close(parentchd);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_setchs - change the CHS values on a hard
    disk image
-------------------------------------------------*/

static int do_setchs(int argc, char *argv[], int param)
{
	int oldcyls, oldhds, oldsecs, oldsecsize;
	UINT8 was_readonly = FALSE;
	UINT64 old_logicalbytes;
	const char *inoutfile;
	chd_file *chd = NULL;
	int cyls, hds, secs;
	char metadata[256];
	chd_header header;
	chd_error err;

	/* require 6 args total */
	if (argc != 6)
		return usage();

	/* extract the data */
	inoutfile = argv[2];
	cyls = atoi(argv[3]);
	hds = atoi(argv[4]);
	secs = atoi(argv[5]);

	/* print some info */
	printf("Input file:   %s\n", inoutfile);
	printf("Cylinders:    %d\n", cyls);
	printf("Heads:        %d\n", hds);
	printf("Sectors:      %d\n", secs);

	/* open the file read-only and get the header */
	err = chd_open(inoutfile, CHD_OPEN_READ, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s' read-only: %s\n", inoutfile, chd_error_string(err));
		goto cleanup;
	}
	header = *chd_get_header(chd);
	chd_close(chd);
	chd = NULL;

	/* if the drive is not writeable, note that, and make it so */
	if (!(header.flags & CHDFLAGS_IS_WRITEABLE))
	{
		was_readonly = TRUE;
		header.flags |= CHDFLAGS_IS_WRITEABLE;

		/* write the new header */
		err = chd_set_header(inoutfile, &header);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error making CHD file writeable: %s\n", chd_error_string(err));
			goto cleanup;
		}
	}

	/* open the file read/write */
	err = chd_open(inoutfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s' read/write: %s\n", inoutfile, chd_error_string(err));
		goto cleanup;
	}

	/* get the hard disk metadata */
	err = chd_get_metadata(chd, HARD_DISK_METADATA_TAG, 0, metadata, sizeof(metadata), NULL, NULL, NULL);
	if (err != CHDERR_NONE || sscanf(metadata, HARD_DISK_METADATA_FORMAT, &oldcyls, &oldhds, &oldsecs, &oldsecsize) != 4)
	{
		fprintf(stderr, "CHD file '%s' is not a hard disk!\n", inoutfile);
		err = CHDERR_INVALID_FILE;
		goto cleanup;
	}

	/* write our own */
	sprintf(metadata, HARD_DISK_METADATA_FORMAT, cyls, hds, secs, oldsecsize);
	err = chd_set_metadata(chd, HARD_DISK_METADATA_TAG, 0, metadata, strlen(metadata) + 1, CHD_MDFLAGS_CHECKSUM);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error writing new metadata to CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}

	/* get the header and compute the new logical size */
	header = *chd_get_header(chd);
	old_logicalbytes = header.logicalbytes;
	header.logicalbytes = (UINT64)cyls * (UINT64)hds * (UINT64)secs * (UINT64)oldsecsize;

	/* close the file */
	chd_close(chd);
	chd = NULL;

	/* restore the read-only state */
	if (was_readonly)
		header.flags &= ~CHDFLAGS_IS_WRITEABLE;

	/* set the new logical size */
	if (header.logicalbytes != old_logicalbytes || was_readonly)
	{
		err = chd_set_header(inoutfile, &header);
		if (err != CHDERR_NONE)
			fprintf(stderr, "Error writing new header to CHD file: %s\n", chd_error_string(err));
	}

	/* print a warning if the size is different */
	if (header.logicalbytes < old_logicalbytes)
		fprintf(stderr, "WARNING: new size is smaller; run chdman -update to reclaim empty space\n");
	else if (header.logicalbytes > old_logicalbytes)
		fprintf(stderr, "WARNING: new size is larger; run chdman -update to account for new empty space\n");

cleanup:
	if (chd != NULL)
		chd_close(chd);
	if (err != CHDERR_NONE && was_readonly)
	{
		header.flags &= ~CHDFLAGS_IS_WRITEABLE;
		chd_set_header(inoutfile, &header);
	}
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_addmeta - add metadata to a CHD from a
    file
-------------------------------------------------*/

static int do_addmeta(int argc, char *argv[], int param)
{
	const char *inoutfile, *srcfile, *tagstring;
	UINT8 was_readonly = FALSE;
	UINT8 *metadata = NULL;
	chd_file *chd = NULL;
	chd_header header;
	file_error filerr;
	UINT32 metalength;
	UINT32 metaindex;
	UINT32 metatag;
	chd_error err;

	/* require 5 or 6 args total */
	if (argc != 5 && argc != 6)
		return usage();

	/* extract the data */
	inoutfile = argv[2];
	tagstring = argv[3];
	if (argc == 5)
	{
		metaindex = 0;
		srcfile = argv[4];
	}
	else
	{
		metaindex = atoi(argv[4]);
		srcfile = argv[5];
	}

	/* verify the tag */
	if (strlen(tagstring) > 4)
	{
		fprintf(stderr, "Invalid tag '%s'; must be 4 characters or less\n", tagstring);
		return CHDERR_INVALID_PARAMETER;
	}
	metatag = ((tagstring[0] == 0) ? ' ' : tagstring[0]) << 24;
	metatag |= ((tagstring[1] == 0) ? ' ' : tagstring[1]) << 16;
	metatag |= ((tagstring[2] == 0) ? ' ' : tagstring[2]) << 8;
	metatag |= ((tagstring[3] == 0) ? ' ' : tagstring[3]) << 0;

	/* print some info */
	printf("Input file:   %s\n", inoutfile);
	printf("Tag:          '%c%c%c%c'\n", (metatag >> 24) & 0xff, (metatag >> 16) & 0xff, (metatag >> 8) & 0xff, metatag & 0xff);
	printf("Index:        %d\n", metaindex);
	printf("Source file:  %s\n", srcfile);

	/* open the file read-only and get the header */
	err = chd_open(inoutfile, CHD_OPEN_READ, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s' read-only: %s\n", inoutfile, chd_error_string(err));
		goto cleanup;
	}
	header = *chd_get_header(chd);
	chd_close(chd);
	chd = NULL;

	/* if the drive is not writeable, note that, and make it so */
	if (!(header.flags & CHDFLAGS_IS_WRITEABLE))
	{
		was_readonly = TRUE;
		header.flags |= CHDFLAGS_IS_WRITEABLE;

		/* write the new header */
		err = chd_set_header(inoutfile, &header);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error making CHD file writeable: %s\n", chd_error_string(err));
			goto cleanup;
		}
	}

	/* open the file read/write */
	err = chd_open(inoutfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s' read/write: %s\n", inoutfile, chd_error_string(err));
		goto cleanup;
	}

	/* attempt to open the source file */
	filerr = core_fload(srcfile, (void **)&metadata, &metalength);
	if (filerr != FILERR_NONE)
	{
		fprintf(stderr, "Error opening source file '%s'\n", srcfile);
		err = CHDERR_FILE_NOT_FOUND;
		goto cleanup;
	}

	/* if it's text, strip any trailing Ctrl-Z and CR/LF and add a trailing NULL */
	if (param)
	{
		metadata = (UINT8 *)realloc(metadata, metalength + 1);
		if (metadata == NULL)
		{
			fprintf(stderr, "Out of memory preparing metadata\n");
			err = CHDERR_OUT_OF_MEMORY;
			goto cleanup;
		}
		metadata[metalength++] = 0;
		while (metalength > 0 && (metadata[metalength - 2] == 0x0a || metadata[metalength - 2] == 0x0d || metadata[metalength - 2] == 0x1a))
			metadata[--metalength] = 0;
	}

	/* write the new metadata */
	err = chd_set_metadata(chd, metatag, metaindex, metadata, metalength, CHD_MDFLAGS_CHECKSUM);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error writing new metadata to CHD file: %s\n", chd_error_string(err));
		goto cleanup;
	}
	header = *chd_get_header(chd);

	/* close the file */
	chd_close(chd);
	chd = NULL;

	/* restore the read-only state */
	if (was_readonly)
	{
		header.flags &= ~CHDFLAGS_IS_WRITEABLE;
		err = chd_set_header(inoutfile, &header);
		if (err != CHDERR_NONE)
			fprintf(stderr, "Error writing new header to CHD file: %s\n", chd_error_string(err));
	}
	if (err == CHDERR_NONE)
		printf("Metadata added successfully as %s\n", param ? "text" : "binary");

cleanup:
	if (metadata != NULL)
		free(metadata);
	if (chd != NULL)
		chd_close(chd);
	if (err != CHDERR_NONE && was_readonly)
	{
		header.flags &= ~CHDFLAGS_IS_WRITEABLE;
		chd_set_header(inoutfile, &header);
	}
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    chdman_compress_file - compress a regular
    file via the compression interfaces
-------------------------------------------------*/

static chd_error chdman_compress_file(chd_file *chd, const char *rawfile, UINT32 offset)
{
	core_file *sourcefile;
	const chd_header *header;
	UINT64 sourceoffset = 0;
	UINT8 *cache = NULL;
	double ratio = 1.0;
	file_error filerr;
	chd_error err;
	int hunknum;

	/* open the raw file */
	filerr = core_fopen(rawfile, OPEN_FLAG_READ, &sourcefile);
	if (filerr != FILERR_NONE)
	{
		err = CHDERR_FILE_NOT_FOUND;
		goto cleanup;
	}

	/* get the header */
	header = chd_get_header(chd);
	cache = (UINT8 *)malloc(header->hunkbytes);
	if (cache == NULL)
	{
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}

	/* begin compressing */
	err = chd_compress_begin(chd);
	if (err != CHDERR_NONE)
		goto cleanup;

	/* loop over source hunks until we run out */
	for (hunknum = 0; hunknum < header->totalhunks; hunknum++)
	{
		UINT32 bytesread;

		/* progress */
		progress(hunknum == 0, "Compressing hunk %d/%d... (ratio=%d%%)  \r", hunknum, header->totalhunks, (int)(100.0 * ratio));

		/* read the data */
		core_fseek(sourcefile, sourceoffset + offset, SEEK_SET);
		bytesread = core_fread(sourcefile, cache, header->hunkbytes);
		if (bytesread < header->hunkbytes)
			memset(&cache[bytesread], 0, header->hunkbytes - bytesread);

		/* append the data */
		err = chd_compress_hunk(chd, cache, &ratio);
		if (err != CHDERR_NONE)
			goto cleanup;

		/* prepare for the next hunk */
		sourceoffset += header->hunkbytes;
	}

	/* finish compression */
	err = chd_compress_finish(chd, TRUE);
	if (err != CHDERR_NONE)
		goto cleanup;

	/* final progress update */
	progress(TRUE, "Compression complete ... final ratio = %d%%            \n", (int)(100.0 * ratio));

cleanup:
	if (sourcefile != NULL)
		core_fclose(sourcefile);
	if (cache != NULL)
		free(cache);
	return err;
}


/*-------------------------------------------------
    chdman_compress_chd - (re)compress a CHD file
    via the compression interfaces
-------------------------------------------------*/

static chd_error chdman_compress_chd(chd_file *chd, chd_file *source, UINT32 totalhunks)
{
	const chd_header *source_header;
	const chd_header *header;
	UINT8 *source_cache = NULL;
	UINT64 source_offset = 0;
	UINT32 source_bytes = 0;
	UINT8 *cache = NULL;
	double ratio = 1.0;
	chd_error err, verifyerr;
	int hunknum;

	/* get the header */
	header = chd_get_header(chd);
	cache = (UINT8 *)malloc(header->hunkbytes);
	if (cache == NULL)
	{
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}

	/* get the source CHD header */
	source_header = chd_get_header(source);
	source_cache = (UINT8 *)malloc(source_header->hunkbytes);
	if (source_cache == NULL)
	{
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}

	/* begin compressing */
	err = chd_compress_begin(chd);
	if (err != CHDERR_NONE)
		goto cleanup;

	/* also begin verifying the source driver */
	verifyerr = chd_verify_begin(source);

	/* a zero count means the natural number */
	if (totalhunks == 0)
		totalhunks = source_header->totalhunks;

	/* loop over source hunks until we run out */
	for (hunknum = 0; hunknum < totalhunks; hunknum++)
	{
		UINT32 bytesremaining = header->hunkbytes;
		UINT8 *dest = cache;

		/* progress */
		progress(hunknum == 0, "Compressing hunk %d/%d... (ratio=%d%%)  \r", hunknum, totalhunks, (int)(100.0 * ratio));

		/* read the data */
		while (bytesremaining > 0)
		{
			/* if we have data in the buffer, copy it */
			if (source_bytes > 0)
			{
				UINT32 bytestocopy = MIN(bytesremaining, source_bytes);
				memcpy(dest, &source_cache[source_header->hunkbytes - source_bytes], bytestocopy);
				dest += bytestocopy;
				source_bytes -= bytestocopy;
				bytesremaining -= bytestocopy;
			}

			/* otherwise, read in another hunk of the source */
			else
			{
				/* verify the next hunk */
				if (verifyerr == CHDERR_NONE)
					err = chd_verify_hunk(source);

				/* then read it (should be the same) */
				err = chd_read(source, source_offset / source_header->hunkbytes, source_cache);
				if (err != CHDERR_NONE)
					memset(source_cache, 0, source_header->hunkbytes);
				source_bytes = source_header->hunkbytes;
				source_offset += source_bytes;
			}
		}

		/* append the data */
		err = chd_compress_hunk(chd, cache, &ratio);
		if (err != CHDERR_NONE)
			goto cleanup;
	}

	/* if we read all the source data, verify the checksums */
	if (verifyerr == CHDERR_NONE && source_offset >= source_header->logicalbytes)
	{
		static const UINT8 empty_checksum[CHD_SHA1_BYTES] = { 0 };
		chd_verify_result verify;
		int i;

		/* get the final values */
		err = chd_verify_finish(source, &verify);

		/* check the MD5 */
		if (memcmp(source_header->md5, empty_checksum, CHD_MD5_BYTES) != 0)
		{
			if (memcmp(source_header->md5, verify.md5, CHD_MD5_BYTES) != 0)
			{
				progress(TRUE, "WARNING: expected input MD5 = ");
				for (i = 0; i < CHD_MD5_BYTES; i++)
					progress(TRUE, "%02x", source_header->md5[i]);
				progress(TRUE, "\n");

				progress(TRUE, "                 actual MD5 = ");
				for (i = 0; i < CHD_MD5_BYTES; i++)
					progress(TRUE, "%02x", verify.md5[i]);
				progress(TRUE, "\n");
			}
			else
				progress(TRUE, "Input MD5 verified                            \n");
		}

		/* check the SHA1 */
		if (memcmp(source_header->sha1, empty_checksum, CHD_SHA1_BYTES) != 0)
		{
			if (memcmp(source_header->sha1, verify.sha1, CHD_SHA1_BYTES) != 0)
			{
				progress(TRUE, "WARNING: expected input SHA1 = ");
				for (i = 0; i < CHD_SHA1_BYTES; i++)
					progress(TRUE, "%02x", source_header->sha1[i]);
				progress(TRUE, "\n");

				progress(TRUE, "                 actual SHA1 = ");
				for (i = 0; i < CHD_SHA1_BYTES; i++)
					progress(TRUE, "%02x", verify.sha1[i]);
				progress(TRUE, "\n");
			}
			else
				progress(TRUE, "Input SHA1 verified                            \n");
		}
	}

	/* finish compression */
	err = chd_compress_finish(chd, !(source_header->flags & CHDFLAGS_IS_WRITEABLE));
	if (err != CHDERR_NONE)
		goto cleanup;

	/* final progress update */
	progress(TRUE, "Compression complete ... final ratio = %d%%            \n", (int)(100.0 * ratio));

cleanup:
	if (source_cache != NULL)
		free(source_cache);
	if (cache != NULL)
		free(cache);
	return err;
}


/*-------------------------------------------------
    chdman_clone_metadata - clone the metadata from
    one CHD to a second
-------------------------------------------------*/

static chd_error chdman_clone_metadata(chd_file *source, chd_file *dest)
{
	const chd_header *header = chd_get_header(source);
	UINT8 metabuffer[MAX(4096, sizeof(cdrom_toc))];
	UINT32 metatag, metasize, metaindex;
	UINT8 metaflags;
	chd_error err;

	/* clone the metadata */
	for (metaindex = 0; ; metaindex++)
	{
		/* fetch the next piece of metadata */
		err = chd_get_metadata(source, CHDMETATAG_WILDCARD, metaindex, metabuffer, sizeof(metabuffer), &metasize, &metatag, &metaflags);
		if (err != CHDERR_NONE)
		{
			if (err == CHDERR_METADATA_NOT_FOUND)
				err = CHDERR_NONE;
			break;
		}

		/* promote certain bits of metadata to checksummed for older CHDs */
		if (header->version <= 3)
		{
			if (metatag == HARD_DISK_METADATA_TAG || metatag == CDROM_OLD_METADATA_TAG ||
			    metatag == CDROM_TRACK_METADATA_TAG || metatag == AV_METADATA_TAG ||
			    metatag == CDROM_TRACK_METADATA2_TAG || metatag == AV_LD_METADATA_TAG)
			{
				metaflags |= CHD_MDFLAGS_CHECKSUM;
			}

			/* convert old-style CD-ROM data to newer */
			if (metatag == CDROM_OLD_METADATA_TAG)
			{
				cdrom_toc *toc = (cdrom_toc *)metabuffer;
				err = cdrom_parse_metadata(source, toc);
				if (err == CHDERR_NONE)
					err = cdrom_write_metadata(dest, toc);
				if (err == CHDERR_NONE)
					continue;
			}
		}

		/* if that fit, just write it back from the temporary buffer */
		if (metasize <= sizeof(metabuffer))
		{
			/* write it to the target */
			err = chd_set_metadata(dest, metatag, CHD_METAINDEX_APPEND, metabuffer, metasize, metaflags);
			if (err != CHDERR_NONE)
				break;
		}

		/* otherwise, allocate a bigger temporary buffer */
		else
		{
			UINT8 *allocbuffer = (UINT8 *)malloc(metasize);
			if (allocbuffer == NULL)
			{
				err = CHDERR_OUT_OF_MEMORY;
				break;
			}

			/* re-read the whole thing */
			err = chd_get_metadata(source, CHDMETATAG_WILDCARD, metaindex, allocbuffer, metasize, &metasize, &metatag, &metaflags);
			if (err != CHDERR_NONE)
			{
				free(allocbuffer);
				break;
			}

			/* write it to the target */
			err = chd_set_metadata(dest, metatag, CHD_METAINDEX_APPEND, allocbuffer, metasize, metaflags);
			free(allocbuffer);
			if (err != CHDERR_NONE)
				break;
		}
	}
	return err;
}


/*-------------------------------------------------
    main - entry point
-------------------------------------------------*/

int CLIB_DECL main(int argc, char *argv[])
{
	static const struct
	{
		const char *	option;
		int (*callback)(int argc, char *argv[], int param);
		int param;
	} option_list[] =
	{
		{ "-createhd",		do_createhd, 0 },
		{ "-createuncomphd",	do_createhd_uncomp, 0 },
		{ "-createraw",		do_createraw, 0 },
		{ "-createcd",		do_createcd, 0 },
		{ "-createblankhd",	do_createblankhd, 0 },
		{ "-createav",		do_createav, 0 },
		{ "-copydata",		do_copydata, 0 },
		{ "-extract",		do_extract, 0 },
		{ "-extractcd",		do_extractcd, 0 },
		{ "-extractav",		do_extractav, 0 },
		{ "-verify",		do_verify, 0 },
		{ "-verifyfix",		do_verify, 1 },
		{ "-update",		do_merge_update_chomp, OPERATION_UPDATE },
		{ "-chomp",			do_merge_update_chomp, OPERATION_CHOMP },
		{ "-info",			do_info, 0 },
		{ "-merge",			do_merge_update_chomp, OPERATION_MERGE },
		{ "-diff",			do_diff, 0 },
		{ "-setchs",		do_setchs, 0 },
		{ "-fixavdata",		do_fixavdata, 0 },
		{ "-addmetatext",   do_addmeta, TRUE },
		{ "-addmetabin",    do_addmeta, FALSE },
	};
	extern const char build_version[];
	int i;

	/* print the header */
	printf("chdman - MAME Compressed Hunks of Data (CHD) manager %s\n", build_version);

	/* require at least 1 argument */
	if (argc < 2)
		return usage();

	/* handle the appropriate command */
	for (i = 0; i < ARRAY_LENGTH(option_list); i++)
		if (strcmp(argv[1], option_list[i].option) == 0)
			return (*option_list[i].callback)(argc, argv, option_list[i].param);

	return usage();
}
