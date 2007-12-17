/***************************************************************************

    jedutil.c

    JEDEC file utilities.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Binary file format:

    Offset
        0 = Total number of fuses (32 bits)
        4 = Raw fuse data, packed 8 bits at a time, LSB to MSB

****************************************************************************

    Known types:

    20-pin devices:
        PAL10H8     = QP20 QF0320
        PAL12H6     = QP20 QF0320
        PAL14H4     = QP20
        PAL16H2     = QP20
        PAL16C1     = QP20
        PAL10L8     = QP20 QF0320
        PAL12L6     = QP20
        PAL14L4     = QP20
        PAL16L2     = QP20

        15S8        = QP20 QF0448

        PLS153      = QP20 QF1842

        PAL16L8     = QP20 QF2048

        PAL16RA8    = QP20 QF2056

        PAL16V8R    = QP20 QF2194
        PALCE16V8   = QP20 QF2194
        GAL16V8A    = QP20 QF2194

        18CV8       = QP20 QF2696

    24-pin devices:
        GAL20V8A    = QP24 QF2706
        GAL22V10    = QP24 QF5892

    28-pin devices:
        PLS100      = QP28 QF1928

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "jedparse.h"



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static UINT8 *srcbuf;
static size_t srcbuflen;

static UINT8 *dstbuf;
static size_t dstbuflen;



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    read_source_file - read a raw source file
    into an allocated memory buffer
-------------------------------------------------*/

static int read_source_file(const char *srcfile)
{
	size_t bytes;
	FILE *file;

	/* open the source file */
	file = fopen(srcfile, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open source file '%s'!\n", srcfile);
		return 1;
	}

	/* allocate memory for the data */
	fseek(file, 0, SEEK_END);
	srcbuflen = ftell(file);
	fseek(file, 0, SEEK_SET);
	srcbuf = malloc(srcbuflen);
	if (!srcbuf)
	{
		fprintf(stderr, "Unable to allocate %d bytes for the source!\n", (int)srcbuflen);
		fclose(file);
		return 1;
	}

	/* read the data */
	bytes = fread(srcbuf, 1, srcbuflen, file);
	if (bytes != srcbuflen)
	{
		fprintf(stderr, "Error reading %d bytes from the source!\n", (int)srcbuflen);
		free(srcbuf);
		fclose(file);
		return 1;
	}

	/* close up shop */
	fclose(file);
	return 0;
}



/*-------------------------------------------------
    write_dest_file - write a memory buffer raw
    into a desintation file
-------------------------------------------------*/

static int write_dest_file(const char *dstfile)
{
	size_t bytes;
	FILE *file;

	/* open the source file */
	file = fopen(dstfile, "wb");
	if (!file)
	{
		fprintf(stderr, "Unable to open target file '%s'!\n", dstfile);
		return 1;
	}

	/* write the data */
	bytes = fwrite(dstbuf, 1, dstbuflen, file);
	if (bytes != dstbuflen)
	{
		fprintf(stderr, "Error writing %d bytes to the target!\n", (int)dstbuflen);
		fclose(file);
		return 1;
	}

	/* close up shop */
	fclose(file);
	return 0;
}



/*-------------------------------------------------
    main - primary entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	const char *srcfile, *dstfile;
	int src_is_jed, dst_is_jed;
	int numfuses = 0;
	jed_data jed;
	int len;
	int err;

	/* needs at least two arguments */
	if (argc < 3)
	{
		fprintf(stderr,
			"Usage:\n"
			"  jedutil <source.jed> <target.bin> [fuses] -- convert JED to binary form\n"
			"  jedutil <source.bin> <target.jed> -- convert binary to JED form\n"
		);
		return 0;
	}

	/* extract arguments */
	srcfile = argv[1];
	dstfile = argv[2];
	if (argc >= 4)
		numfuses = atoi(argv[3]);

	/* does the source end in '.jed'? */
	len = strlen(srcfile);
	src_is_jed = (srcfile[len - 4] == '.' &&
	             tolower(srcfile[len - 3]) == 'j' &&
	             tolower(srcfile[len - 2]) == 'e' &&
	             tolower(srcfile[len - 1]) == 'd');

	/* does the destination end in '.jed'? */
	len = strlen(dstfile);
	dst_is_jed = (dstfile[len - 4] == '.' &&
	             tolower(dstfile[len - 3]) == 'j' &&
	             tolower(dstfile[len - 2]) == 'e' &&
	             tolower(dstfile[len - 1]) == 'd');

	/* error if neither or both are .jed */
	if (!src_is_jed && !dst_is_jed)
	{
		fprintf(stderr, "At least one of the filenames must end in .jed!\n");
		return 1;
	}
	if (src_is_jed && dst_is_jed)
	{
		fprintf(stderr, "Both filenames cannot end in .jed!\n");
		return 1;
	}

	/* read the source file */
	err = read_source_file(srcfile);
	if (err != 0)
		return 1;

	/* if the source is JED, convert to binary */
	if (src_is_jed)
	{
		printf("Converting '%s' to binary form '%s'\n", srcfile, dstfile);

		/* read the JEDEC data */
		err = jed_parse(srcbuf, srcbuflen, &jed);
		switch (err)
		{
			case JEDERR_INVALID_DATA:	fprintf(stderr, "Fatal error: Invalid .JED file\n"); return 1;
			case JEDERR_BAD_XMIT_SUM:	fprintf(stderr, "Fatal error: Bad transmission checksum\n"); return 1;
			case JEDERR_BAD_FUSE_SUM:	fprintf(stderr, "Fatal error: Bad fusemap checksum\n"); return 1;
		}

		/* override the number of fuses */
		if (numfuses != 0)
			jed.numfuses = numfuses;

		/* print out data */
		printf("Source file read successfully\n");
		printf("  Total fuses = %d\n", jed.numfuses);

		/* generate the output */
		dstbuflen = jedbin_output(&jed, NULL, 0);
		dstbuf = malloc(dstbuflen);
		if (!dstbuf)
		{
			fprintf(stderr, "Unable to allocate %d bytes for the target buffer!\n", (int)dstbuflen);
			return 1;
		}
		dstbuflen = jedbin_output(&jed, dstbuf, dstbuflen);
	}

	/* if the source is binary, convert to JED */
	else
	{
		printf("Converting '%s' to JED form '%s'\n", srcfile, dstfile);

		/* read the binary data */
		err = jedbin_parse(srcbuf, srcbuflen, &jed);
		switch (err)
		{
			case JEDERR_INVALID_DATA:	fprintf(stderr, "Fatal error: Invalid binary JEDEC file\n"); return 1;
		}

		/* print out data */
		printf("Source file read successfully\n");
		printf("  Total fuses = %d\n", jed.numfuses);

		/* generate the output */
		dstbuflen = jed_output(&jed, NULL, 0);
		dstbuf = malloc(dstbuflen);
		if (!dstbuf)
		{
			fprintf(stderr, "Unable to allocate %d bytes for the target buffer!\n", (int)dstbuflen);
			return 1;
		}
		dstbuflen = jed_output(&jed, dstbuf, dstbuflen);
	}

	/* write the destination file */
	err = write_dest_file(dstfile);
	if (err != 0)
		return 1;

	printf("Target file written succesfully\n");
	return 0;
}
