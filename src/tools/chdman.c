/***************************************************************************

    CHD compression frontend

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "osdcore.h"
#include "corefile.h"
#include "chdcd.h"
#include "aviio.h"
#include "bitmap.h"
#include "md5.h"
#include "sha1.h"
#include <stdarg.h>
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



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static const char *error_strings[] =
{
	"no error",
	"no drive interface",
	"out of memory",
	"invalid file",
	"invalid parameter",
	"invalid data",
	"file not found",
	"requires parent",
	"file not writeable",
	"read error",
	"write error",
	"codec error",
	"invalid parent",
	"hunk out of range",
	"decompression error",
	"compression error",
	"can't create file",
	"can't verify file",
	"operation not supported",
	"can't find metadata",
	"invalid metadata size",
	"unsupported CHD version",
	"incomplete verify"
};

static clock_t lastprogress = 0;



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    put_bigendian_uint32 - write a UINT32 in big-endian order to memory
-------------------------------------------------*/

INLINE void put_bigendian_uint32(UINT8 *base, UINT32 value)
{
	base[0] = value >> 24;
	base[1] = value >> 16;
	base[2] = value >> 8;
	base[3] = value;
}


/*-------------------------------------------------
    print_big_int - 64-bit int printing with commas
-------------------------------------------------*/

void print_big_int(UINT64 intvalue, char *output)
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
    big_int_string - return a string for a big int
-------------------------------------------------*/

char *big_int_string(UINT64 intvalue)
{
	static char buffer[256];
	buffer[0] = 0;
	print_big_int(intvalue, buffer);
	return buffer;
}


/*-------------------------------------------------
    progress - generic progress callback
-------------------------------------------------*/

static void progress(int forceit, const char *fmt, ...)
{
	clock_t curtime = clock();
	va_list arg;

	/* skip if it hasn't been long enough */
	if (!forceit && curtime - lastprogress < CLOCKS_PER_SEC / 2)
		return;
	lastprogress = curtime;

	/* standard vfprintf stuff here */
	va_start(arg, fmt);
	vprintf(fmt, arg);
	fflush(stdout);
	va_end(arg);
}


/*-------------------------------------------------
    error_string - return an error sting
-------------------------------------------------*/

static const char *error_string(int err)
{
	static char temp_buffer[100];

	if (err < sizeof(error_strings) / sizeof(error_strings[0]))
		return error_strings[err];

	sprintf(temp_buffer, "unknown error %d", err);
	return temp_buffer;
}


/*-------------------------------------------------
    usage - generic usage error display
-------------------------------------------------*/

static int usage(void)
{
	printf("usage: chdman -info input.chd\n");
	printf("   or: chdman -createraw inputhd.raw output.chd [inputoffs [hunksize]]\n");
	printf("   or: chdman -createhd inputhd.raw output.chd [inputoffs [cylinders heads sectors [sectorsize [hunksize]]]]\n");
	printf("   or: chdman -createblankhd output.chd cylinders heads sectors [sectorsize [hunksize]]\n");
	printf("   or: chdman -createcd input.toc output.chd\n");
	printf("   or: chdman -createav input.avi inputmeta.txt output.chd [firstframe [numframes]]\n");
	printf("   or: chdman -copydata input.chd output.chd\n");
	printf("   or: chdman -extract input.chd output.raw\n");
	printf("   or: chdman -extractcd input.chd output.toc output.bin\n");
	printf("   or: chdman -extractav input.chd output.avi outputmeta.txt [firstframe [numframes]]\n");
	printf("   or: chdman -verify input.chd\n");
	printf("   or: chdman -verifyfix input.chd\n");
	printf("   or: chdman -update input.chd output.chd\n");
	printf("   or: chdman -chomp inout.chd output.chd maxhunk\n");
	printf("   or: chdman -merge parent.chd diff.chd output.chd\n");
	printf("   or: chdman -diff parent.chd compare.chd diff.chd\n");
	printf("   or: chdman -setchs inout.chd cylinders heads sectors\n");
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
    do_createhd - create a new compressed hard
    disk image from a raw file
-------------------------------------------------*/

static int do_createhd(int argc, char *argv[], int param)
{
	UINT32 guess_cylinders = 0, guess_heads = 0, guess_sectors = 0, guess_sectorsize = 0;
	UINT32 cylinders, heads, sectors, sectorsize, hunksize, totalsectors, offset;
	const char *inputfile, *outputfile;
	chd_file *chd = NULL;
	char metadata[256];
	chd_error err;

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
		fprintf(stderr, "Error creating CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* open the new hard drive */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* write the metadata */
	sprintf(metadata, HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, sectorsize);
	err = chd_set_metadata(chd, HARD_DISK_METADATA_TAG, 0, metadata, strlen(metadata) + 1);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error adding hard disk metadata: %s\n", error_string(err));
		goto cleanup;
	}

	/* compress the hard drive */
	err = chdman_compress_file(chd, inputfile, offset);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression: %s\n", error_string(err));

cleanup:
	/* close everything down */
	if (chd != NULL)
		chd_close(chd);
	if (err != CHDERR_NONE)
		osd_rmfile(outputfile);
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
		fprintf(stderr, "Error creating CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* open the new CHD */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* compress the CHD */
	err = chdman_compress_file(chd, inputfile, offset);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression: %s\n", error_string(err));

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

	/* allocate a cache */
	cache = malloc(hunksize);
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
		fprintf(stderr, "Error reading input file: %s\n", error_string(err));
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
		fprintf(stderr, "Error creating CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* open the new CHD file */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* write the metadata */
	for (i = 0; i < toc.numtrks; i++)
	{
		char metadata[256];
		sprintf(metadata, CDROM_TRACK_METADATA_FORMAT, i + 1, cdrom_get_type_string(&toc.tracks[i]),
				cdrom_get_subtype_string(&toc.tracks[i]), toc.tracks[i].frames);

		err = chd_set_metadata(chd, CDROM_TRACK_METADATA_TAG, i, metadata, strlen(metadata) + 1);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error adding CD-ROM metadata: %s\n", error_string(err));
			goto cleanup;
		}
	}

	/* begin state for writing */
	err = chd_compress_begin(chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error compressing: %s\n", error_string(err));
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

		printf("Track %d/%d (%s:%d,%d frames,%d hunks,swap %d)\n", i+1, toc.numtrks, track_info.fname[i], track_info.offset[i], toc.tracks[i].frames, trackhunks, track_info.swap[i]);

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
				fprintf(stderr, "Error during compression: %s\n", error_string(err));
				goto cleanup;
			}
		}

		/* close the file */
		core_fclose(srcfile);
		srcfile = NULL;
	}

	/* cleanup */
	err = chd_compress_finish(chd);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression finalization: %s\n", error_string(err));
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

static avi_error read_avi_frame(avi_file *avi, UINT32 framenum, UINT8 *cache, bitmap_t *bitmap, int interlaced, UINT32 hunkbytes)
{
	const avi_movie_info *info = avi_get_movie_info(avi);
	int interlace_factor = interlaced ? 2 : 1;
	UINT32 first_sample, num_samples;
	avi_error avierr = AVIERR_NONE;
	int chnum, sampnum, x, y;
	UINT8 *dest = cache;
	INT16 *temp;

	/* compute the number of samples in this frame */
	first_sample = avi_first_sample_in_frame(avi, framenum / interlace_factor);
	num_samples = avi_first_sample_in_frame(avi, framenum / interlace_factor + 1) - first_sample;
	if (interlaced)
	{
		if (framenum % 2 == 0)
			num_samples = (num_samples + 1) / 2;
		else
		{
			first_sample += (num_samples + 1) / 2;
			num_samples -= (num_samples + 1) / 2;
		}
	}

	/* allocate a temporary buffer */
	temp = malloc(num_samples * 2);
	if (temp == NULL)
		return AVIERR_NO_MEMORY;

	/* update the header with the actual number of samples in the frame */
	dest[6] = num_samples >> 8;
	dest[7] = num_samples;
	dest += 12 + dest[4];

	/* loop over channels and read the samples */
	for (chnum = 0; chnum < info->audio_channels; chnum++)
	{
		/* read the sound samples */
		avierr = avi_read_sound_samples(avi, chnum, first_sample, num_samples, temp);
		if (avierr != AVIERR_NONE)
			goto cleanup;

		/* store them big endian at the destination */
		for (sampnum = 0; sampnum < num_samples; sampnum++)
		{
			INT16 sample = temp[sampnum];
			*dest++ = sample >> 8;
			*dest++ = sample;
		}
	}

	/* read the video data when we hit a new frame */
	if (framenum % interlace_factor == 0)
	{
		avierr = avi_read_video_frame_yuy16(avi, framenum / interlace_factor, bitmap);
		if (avierr != AVIERR_NONE)
			goto cleanup;
	}

	/* loop over the data and copy it to the cache */
	for (y = framenum % interlace_factor; y < info->video_height; y += interlace_factor)
	{
		UINT16 *source = (UINT16 *)bitmap->base + y * bitmap->rowpixels;

		for (x = 0; x < info->video_width; x++)
		{
			UINT16 pixel = *source++;
			*dest++ = pixel;
			*dest++ = pixel >> 8;
		}
	}

	/* fill the rest with 0 */
	while (dest < &cache[hunkbytes])
		*dest++ = 0;

cleanup:
	free(temp);
	return avierr;
}


/*-------------------------------------------------
    do_createav - create a new A/V file from an
    input AVI file and metadata
-------------------------------------------------*/

static int do_createav(int argc, char *argv[], int param)
{
	UINT32 fps_times_1million, width, height, interlaced, channels, rate, metabytes = 0, totalframes;
	UINT32 max_samples_per_frame, bytes_per_frame, firstframe, numframes;
	const char *inputfile, *metafile, *outputfile;
	bitmap_t videobitmap = { 0 };
	const avi_movie_info *info;
	const chd_header *header;
	char metadata[256];
	chd_file *chd = NULL;
	avi_file *avi = NULL;
	UINT8 *cache = NULL;
	double ratio = 1.0;
	FILE *meta = NULL;
	avi_error avierr;
	chd_error err;
	UINT32 framenum;

	/* require 5-7 args total */
	if (argc < 5 || argc > 7)
		return usage();

	/* extract the first few parameters */
	inputfile = argv[2];
	metafile = argv[3];
	outputfile = argv[4];
	firstframe = (argc > 5) ? atoi(argv[5]) : 0;
	numframes = (argc > 6) ? atoi(argv[6]) : 1000000;

	/* print some info */
	printf("Input file:   %s\n", inputfile);
	printf("Meta file:    %s\n", (metafile == NULL) ? "(none)" : metafile);
	printf("Output file:  %s\n", outputfile);

	/* open the meta file */
	if (metafile != NULL)
	{
		meta = fopen(metafile, "r");
		if (meta == NULL)
		{
			fprintf(stderr, "Error opening meta file\n");
			err = CHDERR_INVALID_FILE;
			goto cleanup;
		}
		if (fgets(metadata, sizeof(metadata), meta) == NULL || sscanf(metadata, "chdmeta %d\n", &metabytes) != 1)
		{
			fprintf(stderr, "Invalid data header in metafile\n");
			err = CHDERR_INVALID_FILE;
			goto cleanup;
		}
		if (metabytes > 255)
		{
			fprintf(stderr, "Metadata too large (255 bytes maximum)\n");
			err = CHDERR_INVALID_FILE;
			goto cleanup;
		}
	}

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
	numframes = MIN(totalframes - firstframe, numframes);

	/* allocate a video buffer */
	videobitmap.base = malloc(width * height * 2);
	if (videobitmap.base ==  NULL)
	{
		fprintf(stderr, "Out of memory allocating temporary bitmap\n");
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}
	videobitmap.format = BITMAP_FORMAT_YUY16;
	videobitmap.width = width;
	videobitmap.height = height;
	videobitmap.bpp = 16;
	videobitmap.rowpixels = width;

	/* print some of it */
	printf("Use frames:   %d-%d\n", firstframe, firstframe + numframes - 1);
	printf("Frame rate:   %d.%06d\n", fps_times_1million / 1000000, fps_times_1million % 1000000);
	printf("Frame size:   %d x %d %s\n", width, height, interlaced ? "interlaced" : "non-interlaced");
	printf("Audio:        %d channels at %d Hz\n", channels, rate);
	printf("Metadata:     %d bytes/frame\n", metabytes);
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

	/* determine the number of bytes per frame */
	max_samples_per_frame = ((UINT64)rate * 1000000 + fps_times_1million - 1) / fps_times_1million;
	bytes_per_frame = 12 + metabytes + channels * max_samples_per_frame * 2 + width * height * 2;

	/* create the new CHD */
	err = chd_create(outputfile, (UINT64)numframes * (UINT64)bytes_per_frame, bytes_per_frame, CHDCOMPRESSION_AV, NULL);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error creating CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* open the new CHD */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", error_string(err));
		goto cleanup;
	}
	header = chd_get_header(chd);

	/* write the metadata */
	sprintf(metadata, AV_METADATA_FORMAT, fps_times_1million / 1000000, fps_times_1million % 1000000, width, height, interlaced, channels, rate, metabytes);
	err = chd_set_metadata(chd, AV_METADATA_TAG, 0, metadata, strlen(metadata) + 1);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error adding AV metadata: %s\n", error_string(err));
		goto cleanup;
	}

	/* allocate a cache */
	cache = malloc(bytes_per_frame);
	if (cache == NULL)
	{
		fprintf(stderr, "Out of memory allocating temporary buffer\n");
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}

	/* fill in the basic values */
	cache[0] = 'c';
	cache[1] = 'h';
	cache[2] = 'a';
	cache[3] = 'v';
	cache[4] = metabytes;
	cache[5] = channels;
	cache[6] = max_samples_per_frame >> 8;
	cache[7] = max_samples_per_frame;
	cache[8] = width >> 8;
	cache[9] = width;
	cache[10] = (interlaced << 7) | (height >> 8);
	cache[11] = height;

	/* begin compressing */
	err = chd_compress_begin(chd);
	if (err != CHDERR_NONE)
		goto cleanup;

	/* loop over source hunks until we run out */
	for (framenum = 0; framenum < numframes; framenum++)
	{
		/* progress */
		progress(framenum == 0, "Compressing hunk %d/%d... (ratio=%d%%)  \r", framenum, header->totalhunks, (int)(100.0 * ratio));

		/* read the metadata */
		if (metabytes > 0)
		{
			memset(&cache[12], 0, metabytes);
			if (meta != NULL && fgets(metadata, sizeof(metadata), meta) != NULL)
			{
				int metaoffs, stroffs, length = strlen(metadata);

				for (metaoffs = stroffs = 0; metaoffs < metabytes && stroffs < length; metaoffs++, stroffs += 2)
				{
					int data;
					if (sscanf(&metadata[stroffs], "%02X", &data) != 1)
						break;
					cache[12 + metaoffs] = data;
				}
			}
		}

		/* read the frame into its proper format in the cache */
		avierr = read_avi_frame(avi, firstframe + framenum, cache, &videobitmap, interlaced, bytes_per_frame);
		if (avierr != AVIERR_NONE)
		{
			fprintf(stderr, "Error reading frame %d from AVI file: %s\n", firstframe + framenum, avi_error_string(avierr));
			err = CHDERR_COMPRESSION_ERROR;
		}

		/* append the data */
		err = chd_compress_hunk(chd, cache, &ratio);
		if (err != CHDERR_NONE)
			goto cleanup;
	}

	/* finish compression */
	err = chd_compress_finish(chd);
	if (err != CHDERR_NONE)
		goto cleanup;
	else
		progress(TRUE, "Compression complete ... final ratio = %d%%            \n", (int)(100.0 * ratio));

cleanup:
	/* close everything down */
	if (avi != NULL)
		avi_close(avi);
	if (chd != NULL)
		chd_close(chd);
	if (meta != NULL)
		fclose(meta);
	if (cache != NULL)
		free(cache);
	if (videobitmap.base != NULL)
		free(videobitmap.base);
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
		fprintf(stderr, "Error creating CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* open the new hard drive */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* write the metadata */
	sprintf(metadata, HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, sectorsize);
	err = chd_set_metadata(chd, HARD_DISK_METADATA_TAG, 0, metadata, strlen(metadata) + 1);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error adding hard disk metadata: %s\n", error_string(err));
		goto cleanup;
	}

	/* alloc and zero buffer*/
	cache = malloc(hunksize);
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
			fprintf(stderr, "Error writing CHD file: %s\n", error_string(err));
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
		fprintf(stderr, "Error opening src CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* open the dest hard drive */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &outputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening dest CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* compress the source into the dest */
	err = chdman_compress_chd(outputchd, inputchd, 0);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression: %s\n", error_string(err));

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
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, error_string(err));
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
			fprintf(stderr, "Error reading hunk %d from CHD file: %s\n", hunknum, error_string(err));
			goto cleanup;
		}

		/* write the hunk to the file */
 		bytes_to_write = MIN(bytesremaining, header->hunkbytes);
		core_fseek(outfile, (UINT64)hunknum * (UINT64)header->hunkbytes, SEEK_SET);
		byteswritten = core_fwrite(outfile, hunk, bytes_to_write);
		if (byteswritten != bytes_to_write)
		{
			fprintf(stderr, "Error writing hunk %d to output file: %s\n", hunknum, error_string(CHDERR_WRITE_ERROR));
			err = CHDERR_WRITE_ERROR;
			goto cleanup;
		}
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
    file from a CHD-CD image
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
	int track;

	/* require 5 args total */
	if (argc != 5)
		return usage();

	/* extract the data */
	inputfile = argv[2];
	outputfile = argv[3];
	outputfile2 = argv[4];

	/* print some info */
	printf("Input file:   %s\n", inputfile);
	printf("Output files:  %s (toc) and %s (bin)\n", outputfile, outputfile2);

	/* get the header */
	err = chd_open(inputfile, CHD_OPEN_READ, NULL, &inputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, error_string(err));
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
	fprintf(outfile, "CD_ROM\n\n\n");

	filerr = core_fopen(outputfile2, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &outfile2);
	if (filerr != FILERR_NONE)
	{
		fprintf(stderr, "Error opening output file '%s'\n", outputfile2);
		err = CHDERR_CANT_CREATE_FILE;
		goto cleanup;
	}

	/* process away */
	out2offs = 0;
	for (track = 0; track < toc->numtrks; track++)
	{
		UINT32 m, s, f, frame, trackframes;

		progress(TRUE, "Extracting track %d...   \r", track+1);

		fprintf(outfile, "// Track %d\n", track+1);

		/* write out the track type */
		if (toc->tracks[track].subtype != CD_SUB_NONE)
			fprintf(outfile, "TRACK %s %s\n", cdrom_get_type_string(&toc->tracks[track]), cdrom_get_subtype_string(&toc->tracks[track]));
		else
			fprintf(outfile, "TRACK %s\n", cdrom_get_type_string(&toc->tracks[track]));

		/* write out the attributes */
		fprintf(outfile, "NO COPY\n");
		if (toc->tracks[track].trktype == CD_TRACK_AUDIO)
		{
			fprintf(outfile, "NO PRE_EMPHASIS\n");
			fprintf(outfile, "TWO_CHANNEL_AUDIO\n");

			/* the first audio track on a mixed-track disc always has a 2 second pad */
			if (track == 1)
			{
				if (toc->tracks[track].subtype != CD_SUB_NONE)
					fprintf(outfile, "ZERO AUDIO %s 00:02:00\n", cdrom_get_subtype_string(&toc->tracks[track]));
				else
					fprintf(outfile, "ZERO AUDIO 00:02:00\n");
			}
		}

		/* convert to minutes/seconds/frames */
		trackframes = toc->tracks[track].frames;
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

		if ((toc->tracks[track].trktype == CD_TRACK_AUDIO) && (track == 1))
			fprintf(outfile, "START 00:02:00\n");

		fprintf(outfile, "\n\n");

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
				fprintf(stderr, "Error writing frame %d to output file: %s\n", frame, error_string(CHDERR_WRITE_ERROR));
				err = CHDERR_WRITE_ERROR;
				goto cleanup;
			}
			out2offs += toc->tracks[track].datasize;

			/* read the subcode data */
			cdrom_read_subcode(cdrom, cdrom_get_track_start(cdrom, track) + frame, sector);

			/* write it out */
			core_fseek(outfile2, out2offs, SEEK_SET);
			byteswritten = core_fwrite(outfile2, sector, toc->tracks[track].subsize);
			if (byteswritten != toc->tracks[track].subsize)
			{
				fprintf(stderr, "Error writing frame %d to output file: %s\n", frame, error_string(CHDERR_WRITE_ERROR));
				err = CHDERR_WRITE_ERROR;
				goto cleanup;
			}
			out2offs += toc->tracks[track].subsize;
		}
		progress(TRUE, "Extracting track %d... complete         \n", track+1);
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
    write_avi_frame - write an AVI frame
-------------------------------------------------*/

static avi_error write_avi_frame(avi_file *avi, UINT32 framenum, const UINT8 *buffer, bitmap_t *bitmap)
{
	const avi_movie_info *info = avi_get_movie_info(avi);
	UINT32 channels, samples, width, height;
	avi_error avierr = AVIERR_NONE;
	int chnum, sampnum, x, y;
	int interlace_factor;
	INT16 *temp;

	/* extract core data */
	channels = buffer[5];
	samples = (buffer[6] << 8) | buffer[7];
	width = (buffer[8] << 8) | buffer[9];
	height = (buffer[10] << 8) | buffer[11];
	interlace_factor = (height & 0x8000) ? 2 : 1;
	height &= 0x7fff;
	height *= interlace_factor;
	buffer += 12 + buffer[4];

	/* make sure it makes sense */
	if (width != info->video_width || height != info->video_height)
		return AVIERR_INVALID_DATA;

	/* allocate a temporary buffer */
	temp = malloc(samples * 2);
	if (temp == NULL)
		return AVIERR_NO_MEMORY;

	/* loop over audio channels */
	for (chnum = 0; chnum < channels; chnum++)
	{
		/* extract samples */
		for (sampnum = 0; sampnum < samples; sampnum++)
		{
			INT16 sample = *buffer++ << 8;
			temp[sampnum] = sample | *buffer++;
		}

		/* write the samples */
		avierr = avi_append_sound_samples(avi, chnum, temp, samples);
		if (avierr != AVIERR_NONE)
			goto cleanup;
	}

	/* loop over the data and copy it to the bitmap */
	for (y = framenum % interlace_factor; y < height; y += interlace_factor)
	{
		UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;

		for (x = 0; x < width; x++)
		{
			UINT16 pixel = *buffer++;
			*dest++ = pixel | *buffer++ << 8;
		}
	}

	/* write the video data */
	if (interlace_factor == 1 || framenum % 2 == 1)
	{
		avierr = avi_append_video_frame_yuy16(avi, bitmap);
		if (avierr != AVIERR_NONE)
			goto cleanup;
	}

cleanup:
	free(temp);
	return avierr;
}


/*-------------------------------------------------
    do_extractav - extract an AVI file from a
    CHD image
-------------------------------------------------*/

static int do_extractav(int argc, char *argv[], int param)
{
	int fps, fpsfrac, width, height, interlaced, channels, rate, metabytes, totalframes;
	const char *inputfile, *metafile, *outputfile;
	int firstframe, numframes;
	bitmap_t videobitmap = { 0 };
	UINT32 fps_times_1million;
	const chd_header *header;
	chd_file *chd = NULL;
	avi_file *avi = NULL;
	avi_movie_info info;
	char metadata[256];
	void *hunk = NULL;
	FILE *meta = NULL;
	avi_error avierr;
	chd_error err;
	int framenum;

	/* require 5-7 args total */
	if (argc < 5 || argc > 7)
		return usage();

	/* extract the data */
	inputfile = argv[2];
	outputfile = argv[3];
	metafile = argv[4];
	firstframe = (argc > 5) ? atoi(argv[5]) : 0;
	numframes = (argc > 6) ? atoi(argv[6]) : 1000000;

	/* print some info */
	printf("Input file:   %s\n", inputfile);
	printf("Output file:  %s\n", outputfile);
	printf("Meta file:    %s\n", (metafile == NULL) ? "(none)" : metafile);

	/* get the header */
	err = chd_open(inputfile, CHD_OPEN_READ, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, error_string(err));
		goto cleanup;
	}
	header = chd_get_header(chd);

	/* get the metadata */
	err = chd_get_metadata(chd, AV_METADATA_TAG, 0, metadata, sizeof(metadata), NULL, NULL);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error getting A/V metadata: %s\n", error_string(err));
		goto cleanup;
	}

	/* extract the info */
	if (sscanf(metadata, AV_METADATA_FORMAT, &fps, &fpsfrac, &width, &height, &interlaced, &channels, &rate, &metabytes) != 8)
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
	}
	numframes = MIN(totalframes - firstframe, numframes);

	/* allocate a video buffer */
	videobitmap.base = malloc(width * height * 2);
	if (videobitmap.base ==  NULL)
	{
		fprintf(stderr, "Out of memory allocating temporary bitmap\n");
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}
	videobitmap.format = BITMAP_FORMAT_YUY16;
	videobitmap.width = width;
	videobitmap.height = height;
	videobitmap.bpp = 16;
	videobitmap.rowpixels = width;

	/* print some of it */
	printf("Use frames:   %d-%d\n", firstframe, firstframe + numframes - 1);
	printf("Frame rate:   %d.%06d\n", fps_times_1million / 1000000, fps_times_1million % 1000000);
	printf("Frame size:   %d x %d %s\n", width, height, interlaced ? "interlaced" : "non-interlaced");
	printf("Audio:        %d channels at %d Hz\n", channels, rate);
	printf("Metadata:     %d bytes/frame\n", metabytes);
	printf("Total frames: %d (%02d:%02d:%02d)\n", totalframes,
			(UINT32)((UINT64)totalframes * 1000000 / fps_times_1million / 60 / 60),
			(UINT32)(((UINT64)totalframes * 1000000 / fps_times_1million / 60) % 60),
			(UINT32)(((UINT64)totalframes * 1000000 / fps_times_1million) % 60));

	if (metabytes > 0 && metafile == NULL)
		fprintf(stderr, "Warning: per-frame metadata included but not extracted\n");

	/* allocate memory to hold a hunk */
	hunk = malloc(header->hunkbytes);
	if (hunk == NULL)
	{
		fprintf(stderr, "Out of memory allocating hunk buffer!\n");
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}

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

	/* create the metadata file */
	if (metafile != NULL && metabytes > 0)
	{
		meta = fopen(metafile, "w");
		if (meta == NULL)
		{
			fprintf(stderr, "Error opening meta file '%s': %s\n", metafile, avi_error_string(avierr));
			err = CHDERR_CANT_CREATE_FILE;
			goto cleanup;
		}
		fprintf(meta, "chdmeta %d\n", metabytes);
	}

	/* loop over hunks, reading and writing */
	for (framenum = 0; framenum < numframes; framenum++)
	{
		/* progress */
		progress(framenum == 0, "Extracting hunk %d/%d...  \r", framenum, numframes);

		/* read the hunk into a buffer */
		err = chd_read(chd, firstframe + framenum, hunk);
		if (err != CHDERR_NONE)
		{
			fprintf(stderr, "Error reading hunk %d from CHD file: %s\n", firstframe + framenum, error_string(err));
			goto cleanup;
		}

		/* write the metadata */
		if (meta != NULL)
		{
			int metaoffs;
			for (metaoffs = 0; metaoffs < metabytes; metaoffs++)
				fprintf(meta, "%02X", ((UINT8 *)hunk)[12 + metaoffs]);
			fprintf(meta, "\n");
		}

		/* write the hunk to the file */
		avierr = write_avi_frame(avi, framenum, hunk, &videobitmap);
		if (avierr != AVIERR_NONE)
		{
			fprintf(stderr, "Error writing AVI frame: %s\n", avi_error_string(avierr));
			err = CHDERR_DECOMPRESSION_ERROR;
			goto cleanup;
		}
	}
	progress(TRUE, "Extraction complete!                    \n");

cleanup:
	/* clean up our mess */
	if (avi != NULL)
		avi_close(avi);
	if (hunk != NULL)
		free(hunk);
	if (videobitmap.base != NULL)
		free(videobitmap.base);
	if (chd != NULL)
		chd_close(chd);
	if (meta != NULL)
		fclose(meta);
	if (err != CHDERR_NONE)
	{
		osd_rmfile(outputfile);
		osd_rmfile(metafile);
	}
	return (err != CHDERR_NONE);
}


/*-------------------------------------------------
    do_verify - validate the MD5/SHA1 on a drive
    image
-------------------------------------------------*/

static int do_verify(int argc, char *argv[], int param)
{
	UINT8 actualmd5[CHD_MD5_BYTES], actualsha1[CHD_SHA1_BYTES];
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
		fprintf(stderr, "Error opening CHD file: %s\n", error_string(err));
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
			err = chd_verify_finish(chd, actualmd5, actualsha1);
	}

	/* handle errors */
	if (err != CHDERR_NONE)
	{
		if (err == CHDERR_CANT_VERIFY)
			fprintf(stderr, "Can't verify this type of image (probably writeable)\n");
		else
			fprintf(stderr, "\nError during verify: %s\n", error_string(err));
		goto cleanup;
	}

	/* verify the MD5 */
	if (memcmp(header.md5, actualmd5, sizeof(header.md5)) == 0)
		printf("MD5 verification successful!\n");
	else
	{
		fprintf(stderr, "Error: MD5 in header = ");
		for (i = 0; i < CHD_MD5_BYTES; i++)
			fprintf(stderr, "%02x", header.md5[i]);
		fprintf(stderr, "\n");
		fprintf(stderr, "          actual MD5 = ");
		for (i = 0; i < CHD_MD5_BYTES; i++)
			fprintf(stderr, "%02x", actualmd5[i]);
		fprintf(stderr, "\n");

		/* fix it */
		if (param)
		{
			memcpy(header.md5, actualmd5, sizeof(header.md5));
			fixed = TRUE;
		}
	}

	/* verify the SHA1 */
	if (header.version >= 3)
	{
		if (memcmp(header.sha1, actualsha1, sizeof(header.sha1)) == 0)
			printf("SHA1 verification successful!\n");
		else
		{
			fprintf(stderr, "Error: SHA1 in header = ");
			for (i = 0; i < CHD_SHA1_BYTES; i++)
				fprintf(stderr, "%02x", header.sha1[i]);
			fprintf(stderr, "\n");
			fprintf(stderr, "          actual SHA1 = ");
			for (i = 0; i < CHD_SHA1_BYTES; i++)
				fprintf(stderr, "%02x", actualsha1[i]);
			fprintf(stderr, "\n");

			/* fix it */
			if (param)
			{
				memcpy(header.sha1, actualsha1, sizeof(header.sha1));
				fixed = TRUE;
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
			fprintf(stderr, "Error writing new header: %s\n", error_string(err));
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
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, error_string(err));
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
	if (!(header.flags & CHDFLAGS_IS_WRITEABLE))
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
		err = chd_get_metadata(chd, CHDMETATAG_WILDCARD, i, metadata, sizeof(metadata), &metasize, &metatag);
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
			fprintf(stderr, "Error opening CHD file '%s': %s\n", parentfile, error_string(err));
			goto cleanup;
		}
	}

	/* open the diff CHD */
	err = chd_open(inputfile, CHD_OPEN_READ, parentchd, &inputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, error_string(err));
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
		fprintf(stderr, "Error creating CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* open the new CHD */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, NULL, &outputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* clone the metadata from the input file (which should have inherited from the parent) */
	err = chd_clone_metadata(inputchd, outputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error cloning metadata: %s\n", error_string(err));
		goto cleanup;
	}

	/* do the compression; our interface will route reads for us */
	err = chdman_compress_chd(outputchd, inputchd, (param == OPERATION_CHOMP) ? (maxhunk + 1) : 0);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression: %s\n", error_string(err));

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
		fprintf(stderr, "Error opening CHD file '%s': %s\n", parentfile, error_string(err));
		goto cleanup;
	}

	/* open the input CHD */
	err = chd_open(inputfile, CHD_OPEN_READ, NULL, &inputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s': %s\n", inputfile, error_string(err));
		goto cleanup;
	}

	/* create the new CHD as a diff against the parent */
	err = chd_create(outputfile, 0, 0, chd_get_header(parentchd)->compression, parentchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error creating CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* open the new CHD */
	err = chd_open(outputfile, CHD_OPEN_READWRITE, parentchd, &outputchd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", error_string(err));
		goto cleanup;
	}

	/* do the compression; our interface will route reads for us */
	err = chdman_compress_chd(outputchd, inputchd, 0);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error during compression: %s\n", error_string(err));

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
		fprintf(stderr, "Error opening CHD file '%s' read-only: %s\n", inoutfile, error_string(err));
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
			fprintf(stderr, "Error making CHD file writeable: %s\n", error_string(err));
			goto cleanup;
		}
	}

	/* open the file read/write */
	err = chd_open(inoutfile, CHD_OPEN_READWRITE, NULL, &chd);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file '%s' read/write: %s\n", inoutfile, error_string(err));
		goto cleanup;
	}

	/* get the hard disk metadata */
	err = chd_get_metadata(chd, HARD_DISK_METADATA_TAG, 0, metadata, sizeof(metadata), NULL, NULL);
	if (err != CHDERR_NONE || sscanf(metadata, HARD_DISK_METADATA_FORMAT, &oldcyls, &oldhds, &oldsecs, &oldsecsize) != 4)
	{
		fprintf(stderr, "CHD file '%s' is not a hard disk!\n", inoutfile);
		err = CHDERR_INVALID_FILE;
		goto cleanup;
	}

	/* write our own */
	sprintf(metadata, HARD_DISK_METADATA_FORMAT, cyls, hds, secs, oldsecsize);
	err = chd_set_metadata(chd, HARD_DISK_METADATA_TAG, 0, metadata, strlen(metadata) + 1);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error writing new metadata to CHD file: %s\n", error_string(err));
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
			fprintf(stderr, "Error writing new header to CHD file: %s\n", error_string(err));
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
	cache = malloc(header->hunkbytes);
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
	err = chd_compress_finish(chd);
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
	cache = malloc(header->hunkbytes);
	if (cache == NULL)
	{
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}

	/* get the source CHD header */
	source_header = chd_get_header(source);
	source_cache = malloc(source_header->hunkbytes);
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
		UINT8 md5[CHD_MD5_BYTES];
		UINT8 sha1[CHD_SHA1_BYTES];
		int i;

		/* get the final values */
		err = chd_verify_finish(source, md5, sha1);

		/* check the MD5 */
		if (memcmp(source_header->md5, empty_checksum, CHD_MD5_BYTES) != 0)
		{
			if (memcmp(source_header->md5, md5, CHD_MD5_BYTES) != 0)
			{
				progress(TRUE, "WARNING: expected input MD5 = ");
				for (i = 0; i < CHD_MD5_BYTES; i++)
					progress(TRUE, "%02x", source_header->md5[i]);
				progress(TRUE, "\n");

				progress(TRUE, "                 actual MD5 = ");
				for (i = 0; i < CHD_MD5_BYTES; i++)
					progress(TRUE, "%02x", md5[i]);
				progress(TRUE, "\n");
			}
			else
				progress(TRUE, "Input MD5 verified                            \n");
		}

		/* check the SHA1 */
		if (memcmp(source_header->sha1, empty_checksum, CHD_SHA1_BYTES) != 0)
		{
			if (memcmp(source_header->sha1, sha1, CHD_SHA1_BYTES) != 0)
			{
				progress(TRUE, "WARNING: expected input SHA1 = ");
				for (i = 0; i < CHD_SHA1_BYTES; i++)
					progress(TRUE, "%02x", source_header->sha1[i]);
				progress(TRUE, "\n");

				progress(TRUE, "                 actual SHA1 = ");
				for (i = 0; i < CHD_SHA1_BYTES; i++)
					progress(TRUE, "%02x", sha1[i]);
				progress(TRUE, "\n");
			}
			else
				progress(TRUE, "Input SHA1 verified                            \n");
		}
	}

	/* finish compression */
	err = chd_compress_finish(chd);
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
    main - entry point
-------------------------------------------------*/

int CLIB_DECL main(int argc, char **argv)
{
	static const struct
	{
		const char *	option;
		int (*callback)(int argc, char *argv[], int param);
		int param;
	} option_list[] =
	{
		{ "-createhd",		do_createhd, 0 },
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
		{ "-setchs",		do_setchs, 0 }
	};
	extern char build_version[];
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
