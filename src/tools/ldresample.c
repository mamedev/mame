/***************************************************************************

    ldresample.c

    Laserdisc audio synchronizer and resampler.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "bitmap.h"
#include "chd.h"
#include "vbiparse.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* size of window where we scan ahead to find maximum; this should be large enough to
   catch peaks of even slow waves */
#define MAXIMUM_WINDOW_SIZE		40

/* number of standard deviations away from silence that we consider a real signal */
#define SIGNAL_DEVIATIONS		100

/* number of standard deviations away from silence that we consider the start of a signal */
#define SIGNAL_START_DEVIATIONS	5

/* number of consecutive entries of signal before we consider that we found it */
#define MINIMUM_SIGNAL_COUNT	20



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _movie_info movie_info;
struct _movie_info
{
	double		framerate;
	int			iframerate;
	int			numfields;
	int			width;
	int			height;
	int			samplerate;
	int			channels;
	int			interlaced;
	bitmap_t *	bitmap;
	INT16 *		lsound;
	INT16 *		rsound;
	UINT32		samples;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    field_to_sample_number - given a field number
    compute the absolute sample number for the
    first sample of that field
-------------------------------------------------*/

INLINE UINT32 field_to_sample_number(const movie_info *info, UINT32 field)
{
	return ((UINT64)info->samplerate * (UINT64)field * (UINT64)1000000 + info->iframerate - 1) / (UINT64)info->iframerate;
}


/*-------------------------------------------------
    sample_number_to_field - given a sample number
    compute the field where it is located and
    the offset within the field
-------------------------------------------------*/

INLINE UINT32 sample_number_to_field(const movie_info *info, UINT32 samplenum, UINT32 *offset)
{
	UINT32 guess = ((UINT64)samplenum * (UINT64)info->iframerate + ((UINT64)info->samplerate * (UINT64)1000000 - 1)) / ((UINT64)info->samplerate * (UINT64)1000000);
	while (1)
	{
		UINT32 fieldstart = field_to_sample_number(info, guess);
		UINT32 fieldend = field_to_sample_number(info, guess + 1);
		if (samplenum >= fieldstart && samplenum < fieldend)
		{
			*offset = samplenum - fieldstart;
			return guess;
		}
		else if (samplenum < fieldstart)
			guess--;
		else
			guess++;
	}
}



/***************************************************************************
    CHD HANDLING
***************************************************************************/

/*-------------------------------------------------
    chd_allocate_buffers - allocate buffers for
    CHD I/O
-------------------------------------------------*/

static int chd_allocate_buffers(movie_info *info)
{
	/* allocate a bitmap */
	info->bitmap = bitmap_alloc(info->width, info->height, BITMAP_FORMAT_YUY16);
	if (info->bitmap == NULL)
	{
		fprintf(stderr, "Out of memory creating %dx%d bitmap\n", info->width, info->height);
		return FALSE;
	}

	/* allocate sound buffers */
	info->lsound = (INT16 *)malloc(info->samplerate * sizeof(*info->lsound));
	info->rsound = (INT16 *)malloc(info->samplerate * sizeof(*info->rsound));
	if (info->lsound == NULL || info->rsound == NULL)
	{
		fprintf(stderr, "Out of memory allocating sound buffers of %d bytes\n", (INT32)(info->samplerate * sizeof(*info->rsound)));
		return FALSE;
	}
	return TRUE;
}


/*-------------------------------------------------
    chd_free_buffers - release buffers for
    CHD I/O
-------------------------------------------------*/

static void chd_free_buffers(movie_info *info)
{
	if (info->bitmap != NULL)
		free(info->bitmap);
	if (info->lsound != NULL)
		free(info->lsound);
	if (info->rsound != NULL)
		free(info->rsound);
}


/*-------------------------------------------------
    open_chd - open a CHD file and return
    information about it
-------------------------------------------------*/

static chd_file *open_chd(const char *filename, movie_info *info)
{
	int fps, fpsfrac, width, height, interlaced, channels, rate;
	char metadata[256];
	chd_error chderr;
	chd_file *chd;

	/* open the file */
	chderr = chd_open(filename, CHD_OPEN_READ, NULL, &chd);
	if (chderr != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening CHD file: %s\n", chd_error_string(chderr));
		return NULL;
	}

	/* get the metadata */
	chderr = chd_get_metadata(chd, AV_METADATA_TAG, 0, metadata, sizeof(metadata), NULL, NULL, NULL);
	if (chderr != CHDERR_NONE)
	{
		fprintf(stderr, "Error getting A/V metadata: %s\n", chd_error_string(chderr));
		chd_close(chd);
		return NULL;
	}

	/* extract the info */
	if (sscanf(metadata, AV_METADATA_FORMAT, &fps, &fpsfrac, &width, &height, &interlaced, &channels, &rate) != 7)
	{
		fprintf(stderr, "Improperly formatted metadata\n");
		chd_close(chd);
		return NULL;
	}

	/* extract movie info */
	info->iframerate = fps * 1000000 + fpsfrac;
	info->framerate = info->iframerate / 1000000.0;
	info->numfields = chd_get_header(chd)->totalhunks;
	info->width = width;
	info->height = height;
	info->interlaced = interlaced;
	info->samplerate = rate;
	info->channels = channels;

	/* allocate buffers */
	if (!chd_allocate_buffers(info))
		return NULL;

	return chd;
}


/*-------------------------------------------------
    create_chd - create a new CHD file
-------------------------------------------------*/

static chd_file *create_chd(const char *filename, chd_file *source, const movie_info *info)
{
	const chd_header *srcheader = chd_get_header(source);
	chd_error chderr;
	chd_file *chd;

	/* create the file */
	chderr = chd_create(filename, srcheader->logicalbytes, srcheader->hunkbytes, CHDCOMPRESSION_AV, NULL);
	if (chderr != CHDERR_NONE)
	{
		fprintf(stderr, "Error creating new CHD file: %s\n", chd_error_string(chderr));
		return NULL;
	}

	/* open the file */
	chderr = chd_open(filename, CHD_OPEN_READWRITE, NULL, &chd);
	if (chderr != CHDERR_NONE)
	{
		fprintf(stderr, "Error opening new CHD file: %s\n", chd_error_string(chderr));
		return NULL;
	}

	/* clone the metadata */
	chderr = chd_clone_metadata(source, chd);
	if (chderr != CHDERR_NONE)
	{
		fprintf(stderr, "Error cloning metadata: %s\n", chd_error_string(chderr));
		chd_close(chd);
		return NULL;
	}

	/* begin compressing */
	chderr = chd_compress_begin(chd);
	if (chderr != CHDERR_NONE)
	{
		fprintf(stderr, "Error beginning compression: %s\n", chd_error_string(chderr));
		return NULL;
	}

	return chd;
}


/*-------------------------------------------------
    read_chd - read a field from a CHD file
-------------------------------------------------*/

static int read_chd(chd_file *file, UINT32 field, movie_info *info, UINT32 soundoffs)
{
	av_codec_decompress_config avconfig = { 0 };
	chd_error chderr;

	/* configure the codec */
	avconfig.video = info->bitmap;
	avconfig.maxsamples = 48000;
	avconfig.actsamples = &info->samples;
	avconfig.audio[0] = info->lsound + soundoffs;
	avconfig.audio[1] = info->rsound + soundoffs;

	/* configure the decompressor for this field */
	chd_codec_config(file, AV_CODEC_DECOMPRESS_CONFIG, &avconfig);

	/* read the field */
	chderr = chd_read(file, field, NULL);
	if (chderr != CHDERR_NONE)
		return FALSE;

	return TRUE;
}


/*-------------------------------------------------
    write_chd - write a field to a CHD file
-------------------------------------------------*/

static int write_chd(chd_file *file, UINT32 field, movie_info *info)
{
	av_codec_compress_config avconfig = { 0 };
	chd_error chderr;

	/* configure the codec */
	avconfig.video = info->bitmap;
	avconfig.channels = 2;
	avconfig.samples = info->samples;
	avconfig.audio[0] = info->lsound;
	avconfig.audio[1] = info->rsound;

	/* configure the decompressor for this field */
	chd_codec_config(file, AV_CODEC_COMPRESS_CONFIG, &avconfig);

	/* read the field */
	chderr = chd_compress_hunk(file, NULL, NULL);
	if (chderr != CHDERR_NONE)
		return FALSE;

	return TRUE;
}


/*-------------------------------------------------
    create_close_chd - close a CHD file
-------------------------------------------------*/

static void create_close_chd(chd_file *file)
{
	chd_error err;

	err = chd_compress_finish(file, TRUE);
	if (err != CHDERR_NONE)
		fprintf(stderr, "Error finishing compression: %s\n", chd_error_string(err));

	chd_close(file);
}


/*-------------------------------------------------
    close_chd - close a CHD file
-------------------------------------------------*/

static void close_chd(chd_file *file, movie_info *info)
{
	if (info != NULL)
		chd_free_buffers(info);
	chd_close(file);
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    find_edge_near_field - given a field number,
    load +/- 1/2 second on either side and find
    an audio edge
-------------------------------------------------*/

static int find_edge_near_field(chd_file *srcfile, UINT32 fieldnum, movie_info *info, int report_best_field, INT32 *delta)
{
	int fields_to_read = info->iframerate / 1000000;
	UINT32 firstlavg = 0, firstravg = 0;
	UINT32 firstldev = 0, firstrdev = 0;
	UINT32 lcount = 0, rcount = 0;
	UINT32 targetsoundstart = 0;
	UINT32 firstfieldend = 0;
	INT32 firstfield, curfield;
	UINT32 fieldstart[100];
	UINT32 soundend = 0;
	UINT32 sampnum;

	/* clear the sound buffers */
	memset(info->lsound, 0, info->samplerate * sizeof(*info->lsound));
	memset(info->rsound, 0, info->samplerate * sizeof(*info->rsound));

	/* read 1 second around the target area */
	firstfield = fieldnum - (fields_to_read / 2);
	for (curfield = 0; curfield < fields_to_read; curfield++)
	{
		/* remember the start of each field */
		fieldstart[curfield] = soundend;

		/* remember the sound offset where the initial fieldnum is */
		if (firstfield + curfield == fieldnum)
			targetsoundstart = soundend;

		/* read the frame and samples */
		if (firstfield + curfield >= 0)
		{
			read_chd(srcfile, firstfield + curfield, info, soundend);
			soundend += info->samples;

			/* also remember the offset at the end of the first field */
			if (firstfieldend == 0)
				firstfieldend = soundend;
		}
	}

	/* compute absolute deltas across the samples */
	for (sampnum = 0; sampnum < soundend; sampnum++)
	{
		info->lsound[sampnum] = labs(info->lsound[sampnum + 1] - info->lsound[sampnum]);
		info->rsound[sampnum] = labs(info->rsound[sampnum + 1] - info->rsound[sampnum]);
	}

	/* for each sample in the collection, find the highest deltas over the
       next few samples, and take the nth highest value (to remove outliers) */
	for (sampnum = 0; sampnum < soundend - MAXIMUM_WINDOW_SIZE; sampnum++)
	{
		UINT32 lmax = 0, rmax = 0;
		UINT32 scannum;

		/* scan forward over the maximum window */
		for (scannum = 0; scannum < MAXIMUM_WINDOW_SIZE; scannum++)
		{
			if (info->lsound[sampnum + scannum] > lmax)
				lmax = info->lsound[sampnum + scannum];
			if (info->rsound[sampnum + scannum] > rmax)
				rmax = info->rsound[sampnum + scannum];
		}

		/* replace this sample with the maximum value found */
		info->lsound[sampnum] = lmax;
		info->rsound[sampnum] = rmax;
	}

	/* now compute the average over the first field, which is assumed to be silence */
	for (sampnum = 0; sampnum < firstfieldend; sampnum++)
	{
		firstlavg += info->lsound[sampnum];
		firstravg += info->rsound[sampnum];
	}
	firstlavg /= firstfieldend;
	firstravg /= firstfieldend;

	/* then compute the standard deviation over the first field */
	for (sampnum = 0; sampnum < firstfieldend; sampnum++)
	{
		firstldev += (info->lsound[sampnum] - firstlavg) * (info->lsound[sampnum] - firstlavg);
		firstrdev += (info->rsound[sampnum] - firstravg) * (info->rsound[sampnum] - firstravg);
	}
	firstldev = sqrt((double)firstldev / firstfieldend);
	firstrdev = sqrt((double)firstrdev / firstfieldend);

	/* scan forward through the samples, counting consecutive samples more than
       SIGNAL_DEVIATIONS standard deviations away from silence */
	for (sampnum = 0; sampnum < soundend; sampnum++)
	{
		/* left speaker */
		if (info->lsound[sampnum] > firstlavg + SIGNAL_DEVIATIONS * firstldev)
			lcount++;
		else
			lcount = 0;

		/* right speaker */
		if (info->rsound[sampnum] > firstravg + SIGNAL_DEVIATIONS * firstrdev)
			rcount++;
		else
			rcount = 0;

		/* stop if we find enough */
		if (lcount > MINIMUM_SIGNAL_COUNT || rcount > MINIMUM_SIGNAL_COUNT)
			break;
	}

	/* if we didn't find any, return failure */
	if (sampnum >= soundend)
	{
		if (!report_best_field)
			printf("Field %5d: Unable to find edge\n", fieldnum);
		return FALSE;
	}

	/* scan backwards to find the start of the signal */
	for ( ; sampnum > 0; sampnum--)
		if (info->lsound[sampnum - 1] < firstlavg + SIGNAL_START_DEVIATIONS * firstldev ||
			info->rsound[sampnum - 1] < firstravg + SIGNAL_START_DEVIATIONS * firstrdev)
			break;

	/* if we're to report the best field, figure out which field we are in */
	if (report_best_field)
	{
		for (curfield = 0; curfield < fields_to_read - 1; curfield++)
			if (sampnum < fieldstart[curfield + 1])
				break;
		printf("Field %5d: Edge found at offset %d (frame %.1f)\n", firstfield + curfield, sampnum - fieldstart[curfield], (double)(firstfield + curfield) * 0.5);
	}

	/* otherwise, compute the delta from the provided field number */
	else
	{
		printf("Field %5d: Edge at offset %d from expected (found at %d, expected %d)\n", fieldnum, sampnum - targetsoundstart, sampnum, targetsoundstart);
		*delta = sampnum - targetsoundstart;
	}
	return TRUE;
}


/*-------------------------------------------------
    usage - display program usage
-------------------------------------------------*/

static int usage(void)
{
	fprintf(stderr, "Usage: \n");
	fprintf(stderr, "  ldresample source.chd\n");
	fprintf(stderr, "  ldresample source.chd output.chd offset [slope]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Where offset and slope make a linear equation f(x) which\n");
	fprintf(stderr, "describes the sample offset from the source as a function\n");
	fprintf(stderr, "of field number.\n");
	return 1;
}


/*-------------------------------------------------
    main - main entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	movie_info info = { 0 };
	const char *srcfilename;
	const char *dstfilename;
	double offset, slope;
	chd_file *srcfile;
	chd_file *dstfile;

	/* verify arguments */
	if (argc < 2)
		return usage();
	srcfilename = argv[1];
	dstfilename = (argc < 3) ? NULL : argv[2];
	offset = (argc < 4) ? 0.0 : atof(argv[3]);
	slope = (argc < 5) ? 1.0 : atof(argv[4]);

	/* print basic information */
	printf("Input file: %s\n", srcfilename);
	if (dstfilename != NULL)
	{
		printf("Output file: %s\n", dstfilename);
		printf("Offset: %f\n", offset);
		printf("Slope: %f\n", slope);
	}

	/* open the source file */
	srcfile = open_chd(srcfilename, &info);
	if (srcfile == NULL)
	{
		fprintf(stderr, "Unable to open file '%s'\n", srcfilename);
		return 1;
	}

	/* output some basics */
	printf("Video dimensions: %dx%d\n", info.width, info.height);
	printf("Video frame rate: %.2fHz\n", info.framerate);
	printf("Sample rate: %dHz\n", info.samplerate);
	printf("Total fields: %d\n", info.numfields);

	/* if we don't have a destination file, scan for edges */
	if (dstfilename == NULL)
	{
		UINT32 fieldnum;
		INT32 delta;

		for (fieldnum = 60; fieldnum < info.numfields - 60; fieldnum += 30)
		{
			fprintf(stderr, "Field %5d\r", fieldnum);
			find_edge_near_field(srcfile, fieldnum, &info, TRUE, &delta);
		}
	}

	/* otherwise, resample the source to the destination */
	else
	{
		INT64 ioffset = (INT64)(offset * 65536.0 * 256.0);
		INT64 islope = (INT64)(slope * 65536.0 * 256.0);
		UINT32 fieldnum;

		/* open the destination file */
		dstfile = create_chd(dstfilename, srcfile, &info);
		if (dstfile == NULL)
		{
			fprintf(stderr, "Unable to create file '%s'\n", dstfilename);
			return 1;
		}

		/* loop over all the fields in the source file */
		for (fieldnum = 0; fieldnum < info.numfields; fieldnum++)
		{
			UINT32 srcbegin = field_to_sample_number(&info, fieldnum);
			UINT32 srcend = field_to_sample_number(&info, fieldnum + 1);
			INT64 dstbegin = ((INT64)srcbegin << 24) + ioffset + islope * fieldnum;
			INT64 dstend = ((INT64)srcend << 24) + ioffset + islope * (fieldnum + 1);
			UINT32 dstbeginoffset, dstendoffset, dstoffset;
			INT32 dstbeginfield, dstendfield, dstfield;
			INT64 dstpos, dststep;
			UINT32 srcoffset;

			/* update progress (this ain't fast!) */
			if (fieldnum % 10 == 0)
				fprintf(stderr, "Field %d\r", fieldnum);

			/* determine the first and last fields needed to cover this range of samples */
			if (dstbegin >= 0)
				dstbeginfield = sample_number_to_field(&info, dstbegin >> 24, &dstbeginoffset);
			else
			{
				dstbeginfield = -1 - sample_number_to_field(&info, -dstbegin >> 24, &dstbeginoffset);
				dstbeginoffset = (field_to_sample_number(&info, -dstbeginfield) - field_to_sample_number(&info, -dstbeginfield - 1)) - dstbeginoffset;
			}
			if (dstend >= 0)
				dstendfield = sample_number_to_field(&info, dstend >> 24, &dstendoffset);
			else
			{
				dstendfield = -1 - -sample_number_to_field(&info, -dstend >> 24, &dstendoffset);
				dstendoffset = (field_to_sample_number(&info, -dstendfield) - field_to_sample_number(&info, -dstendfield - 1)) - dstendoffset;
			}
/*
printf("%5d: start=%10d (%5d.%03d) end=%10d (%5d.%03d)\n",
        fieldnum,
        (INT32)(dstbegin >> 24), dstbeginfield, dstbeginoffset,
        (INT32)(dstend >> 24), dstendfield, dstendoffset);
*/
			/* read all samples required into the end of the sound buffers */
			dstoffset = srcend - srcbegin;
			for (dstfield = dstbeginfield; dstfield <= dstendfield; dstfield++)
			{
				if (dstfield >= 0)
					read_chd(srcfile, dstfield, &info, dstoffset);
				else
				{
					info.samples = field_to_sample_number(&info, -dstfield) - field_to_sample_number(&info, -dstfield - 1);
					memset(&info.lsound[dstoffset], 0, info.samples * sizeof(info.lsound[0]));
					memset(&info.rsound[dstoffset], 0, info.samples * sizeof(info.rsound[0]));
				}
				dstoffset += info.samples;
			}

			/* resample the destination samples to the source */
			dstoffset = srcend - srcbegin;
			dstpos = dstbegin;
			dststep = (dstend - dstbegin) / (INT64)(srcend - srcbegin);
			for (srcoffset = 0; srcoffset < srcend - srcbegin; srcoffset++)
			{
				info.lsound[srcoffset] = info.lsound[dstoffset + dstbeginoffset + (dstpos >> 24) - (dstbegin >> 24)];
				info.rsound[srcoffset] = info.rsound[dstoffset + dstbeginoffset + (dstpos >> 24) - (dstbegin >> 24)];
				dstpos += dststep;
			}

			/* read the original frame, pointing the sound buffer past where we've calculated */
			read_chd(srcfile, fieldnum, &info, srcend - srcbegin);

			/* write it to the destination */
			write_chd(dstfile, fieldnum, &info);
		}

		/* close the destination file */
		create_close_chd(dstfile);
	}

	/* close the source file */
	close_chd(srcfile, &info);

	return 0;
}
