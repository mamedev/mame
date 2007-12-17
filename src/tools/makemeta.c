/***************************************************************************

    makemeta.c

    Laserdisc metadata generator.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Metadata format (12 bytes/field):

      Offset   Description
      ------   ------------------------------------------
         0   = version (currently 2)
         1   = internal flags:
                bit 0 = previous field is the same frame
                bit 1 = next field is the same frame
         2   = white flag
        3-5  = line 16 Philips code
        6-8  = line 17 Philips code
        9-11 = line 18 Philips code

***************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "aviio.h"
#include "bitmap.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define INSERT_FRAME_CODE		0xff000000
#define INSERT_FRAME_CODE_INC	0xff000001
#define INVALID_CODE			0xffffffff



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _pattern_data pattern_data;
struct _pattern_data
{
	pattern_data *next;
	UINT32 line16, line17, line18;
	int white;
};



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    parse_line - parse a Philips code from a
    line of video data
-------------------------------------------------*/

static int parse_line(bitmap_t *bitmap, int line, int expected_bits, UINT8 *result)
{
	const UINT16 *source = BITMAP_ADDR16(bitmap, line, 0);
	int x, edges = 0, bits = 0;
	int minwidth = 1000;
	UINT8 bit[720];
	int edge[720];
	int error = 0;

	/* clamp expected bits */
	expected_bits *= 2;
	if (expected_bits > ARRAY_LENGTH(edge) - 1)
		expected_bits = ARRAY_LENGTH(edge) - 1;

	/* find the edges in the line */
	for (x = 1; x < bitmap->width && edges < ARRAY_LENGTH(edge); x++)
		if (source[x] >= 0xc000 && source[x - 1] < 0xc000)
			edge[edges++] = x;
		else if (source[x] <= 0x4000 && source[x - 1] > 0x4000)
			edge[edges++] = x;

	/* find the minimum width */
	for (x = 1; x < edges; x++)
	{
		int width = edge[x] - edge[x - 1];
		if (width > 3 && width < minwidth)
			minwidth = edge[x] - edge[x - 1];
	}

	/* now generate the bits */
	for (x = 1; x < edges; x++)
	{
		int width = edge[x] - edge[x - 1];
		if (width > 3)
		{
			int count = (width > 3 * minwidth / 2) ? 2 : 1;
			while (count--)
				bit[bits++] = (source[edge[x - 1]] >= 0x8000) ? 1 : 0;
		}
	}

	/* look for improperly paired bits in the sequence */
	if (bits < expected_bits)
	{
		/* look for two bits in a row of the same type on an even boundary */
		for (x = 0; x < bits; x += 2)
			if (bit[x] == bit[x + 1])
				break;

		/* if we got something wrong, assume we're missing an opening 0 bit */
		if (x < bits)
		{
			memmove(&bit[1], &bit[0], bits);
			bit[0] = 0;
			bits++;
		}
	}

	/* trailing bits are 0 */
	while (bits < expected_bits)
		bit[bits++] = 0;

	/* output */
	for (x = 0; x < MIN(bits, expected_bits); x += 2)
	{
		static const UINT8 trans[4] = { 0x80, 1, 0, 0xff };
		result[x/2] = trans[(bit[x] << 1) | bit[x + 1]];
		if (result[x/2] > 1)
			error++;
	}
	return error ? -(bits / 2) : (bits / 2);
}


/*-------------------------------------------------
    output_meta - output a line of metadata
-------------------------------------------------*/

static void output_meta(UINT8 flags, UINT8 white, UINT32 line12, UINT32 line13, UINT32 line14, UINT32 framenum, UINT32 chapternum)
{
	/* start with the raw metadata, followed by a comment */
	printf("02%02X%02X%06X%06X%06X   ; ",
		flags, white, line12, line13, line14);

	/* separate comments for leadin/leadout */
	if (line13 == 0x88ffff)
		printf("leadin\n");
	else if (line13 == 0x80eeee)
		printf("leadout\n");

	/* otherwise, display the frame and chapter, and indicate white flag/stop code */
	else
	{
		printf("frame %05x ch %02x", framenum, chapternum);
		if (white)
			printf(" (white)");
		if (line12 == 0x82cfff)
			printf(" (stop)");
		printf("\n");
	}
}


/*-------------------------------------------------
    generate_from_avi - generate the data from
    an AVI file
-------------------------------------------------*/

static int generate_from_avi(const char *aviname)
{
	UINT32 line12 = 0, line13 = 0, line14 = 0, framenum = 0, chapternum = 0;
	const avi_movie_info *info;
	bitmap_t *bitmap;
	avi_error avierr;
	avi_file *avi;
	int white = 0;
	int frame;

	/* open the file */
	avierr = avi_open(aviname, &avi);
	if (avierr != AVIERR_NONE)
	{
		fprintf(stderr, "Error opening AVI file: %s\n", avi_error_string(avierr));
		return 1;
	}

	/* extract movie info */
	info = avi_get_movie_info(avi);
	fprintf(stderr, "%dx%d - %d frames total\n", info->video_width, info->video_height, info->video_numsamples);
	if (info->video_height != 39)
	{
		fprintf(stderr, "Unknown VANC capture format: expected 39 rows\n");
		return 1;
	}

	/* allocate a bitmap to hold it */
	bitmap = bitmap_alloc(info->video_width, info->video_height, BITMAP_FORMAT_YUY16);
	if (bitmap == NULL)
	{
		fprintf(stderr, "Out of memory allocating %dx%d bitmap\n", info->video_width, info->video_height);
		return 1;
	}

	/* loop over frames */
	for (frame = 0; frame < info->video_numsamples; frame++)
	{
		int field;
		UINT8 bits[24];

		/* read the frame */
		avierr = avi_read_video_frame_yuy16(avi, frame, bitmap);
		if (avierr != AVIERR_NONE)
		{
			fprintf(stderr, "Error reading AVI frame %d: %s\n", frame, avi_error_string(avierr));
			break;
		}

		/* loop over two fields */
		for (field = 0; field < 2; field++)
		{
			int prevwhite = white;
			int i;

			/* line 7 contains the white flag */
			white = 0;
			if (*BITMAP_ADDR16(bitmap, 20 * field + 7, bitmap->width / 2) > 0x8000)
				white = 1;

			/* output metadata for *previous* field */
			if (frame > 0 || field > 0)
			{
				int flags = 0;

				if (!prevwhite) flags |= 0x01;
				if (!white) flags |= 0x02;
				output_meta(flags, prevwhite, line12, line13, line14, framenum, chapternum);
			}

			/* line 12 contains stop code and other interesting bits */
			line12 = 0;
			if (parse_line(bitmap, 20 * field + 12, 24, bits) == 24)
				for (i = 0; i < 24; i++)
					line12 = (line12 << 1) | bits[i];

			/* line 13 and 14 contain frame/chapter/lead in/out encodings */
			line13 = 0;
			if (parse_line(bitmap, 20 * field + 13, 24, bits) == 24)
				for (i = 0; i < 24; i++)
					line13 = (line13 << 1) | bits[i];

			line14 = 0;
			if (parse_line(bitmap, 20 * field + 14, 24, bits) == 24)
				for (i = 0; i < 24; i++)
					line14 = (line14 << 1) | bits[i];

			/* the two lines must match */
//          if (line13 != 0 && line14 != 0 && line13 != line14)
//              line13 = line14 = 0;

			/* is this a frame number? */
			if ((line13 & 0xf00000) == 0xf00000)
				framenum = line13 & 0x7ffff;
			if ((line13 & 0xf00fff) == 0x800ddd)
				chapternum = (line13 >> 12) & 0x7f;
		}
	}

	/* output metadata for *previous* field */
	{
		int flags = 0;

		if (!white) flags |= 0x01;
		output_meta(flags, white, line12, line13, line14, framenum, chapternum);
	}

	bitmap_free(bitmap);
	return 0;
}


/*-------------------------------------------------
    parse_philips_code - parse a single Philips
    code from a string, stopping at the given
    end characters
-------------------------------------------------*/

static UINT32 parse_philips_code(char **argptr, const char *endchars)
{
	char *arg = *argptr;
	UINT32 value = 0;

	/* look for special chars first */
	if (*arg == '+')
	{
		*argptr = arg + 1;
		return INSERT_FRAME_CODE_INC;
	}
	else if (*arg == '@')
	{
		*argptr = arg + 1;
		return INSERT_FRAME_CODE;
	}

	/* parse the rest as hex digits */
	for ( ; *arg != 0 && strchr(endchars, *arg) == NULL; arg++)
	{
		if (*arg >= '0' && *arg <= '9')
			value = (value << 4) + (*arg - '0');
		else if (*arg >= 'a' && *arg <= 'f')
			value = (value << 4) + 10 + (*arg - 'a');
		else if (*arg >= 'A' && *arg <= 'F')
			value = (value << 4) + 10 + (*arg - 'A');
		else
			return INVALID_CODE;
	}

	/* if we're too big, we're invalid */
	if (value > 0xffffff)
		return INVALID_CODE;

	*argptr = arg;
	return value;
}


/*-------------------------------------------------
    parse_pattern - parse a pattern into a series
    of pattern_data structs
-------------------------------------------------*/

pattern_data *parse_pattern(char *arg, int *countptr)
{
	pattern_data *head = NULL;
	pattern_data **tailptr = &head;
	int count = 0;

	/* first parse the count */
	for ( ; *arg != 0 && *arg != '*'; arg++)
	{
		if (!isdigit(*arg))
			return NULL;
		count = count * 10 + (*arg - '0');
	}
	if (*arg == 0)
		return NULL;
	arg++;
	*countptr = count;

	/* loop until we hit the end */
	while (*arg != 0)
	{
		pattern_data *pat;

		/* allocate a new structure */
		pat = malloc(sizeof(*pat));
		if (pat == NULL)
			return NULL;
		memset(pat, 0, sizeof(*pat));

		/* bang at the beginning means white flag */
		if (*arg == '!')
		{
			arg++;
			pat->white = 1;
		}

		/* parse line16 until we hit a period or comma */
		pat->line16 = parse_philips_code(&arg, ".,");
		if (pat->line16 == INVALID_CODE)
			return NULL;
		if (*arg != '.' && *arg != ',' && *arg != 0)
			return NULL;
		if (*arg == '.')
			arg++;

		/* parse line17 until we hit a period */
		pat->line17 = parse_philips_code(&arg, ".,");
		if (pat->line17 == INVALID_CODE)
			return NULL;
		if (*arg != '.' && *arg != ',' && *arg != 0)
			return NULL;
		if (*arg == '.')
			arg++;

		/* parse line18 until we hit a comma */
		pat->line18 = parse_philips_code(&arg, ",");
		if (pat->line18 == INVALID_CODE)
			return NULL;
		if (*arg != ',' && *arg != 0)
			return NULL;
		if (*arg == ',')
			arg++;

		/* append to the end */
		*tailptr = pat;
		tailptr = &pat->next;
	}

	return head;
}


/*-------------------------------------------------
    generate_from_pattern - generate metadata
    from a pattern
-------------------------------------------------*/

static int generate_from_pattern(pattern_data *pattern, int patcount)
{
	pattern_data *curpat = pattern;
	int framenum = 0, bcdframenum = 0;

	/* loop until we exceed the pattern frame count */
	while (1)
	{
		UINT32 line16, line17, line18;
		int flags = 0;

		/* handle special codes for line 16 */
		line16 = curpat->line16;
		if (line16 == INSERT_FRAME_CODE_INC)
		{
			framenum++;
			bcdframenum = (((framenum / 10000) % 10) << 16) | (((framenum / 1000) % 10) << 12) | (((framenum / 100) % 10) << 8) | (((framenum / 10) % 10) << 4) | (framenum % 10);
		}
		if (line16 == INSERT_FRAME_CODE || line16 == INSERT_FRAME_CODE_INC)
			line16 = 0xf80000 | bcdframenum;

		/* handle special codes for line 17 */
		line17 = curpat->line17;
		if (line17 == INSERT_FRAME_CODE_INC)
		{
			framenum++;
			bcdframenum = (((framenum / 10000) % 10) << 16) | (((framenum / 1000) % 10) << 12) | (((framenum / 100) % 10) << 8) | (((framenum / 10) % 10) << 4) | (framenum % 10);
		}
		if (line17 == INSERT_FRAME_CODE || line17 == INSERT_FRAME_CODE_INC)
			line17 = 0xf80000 | bcdframenum;

		/* handle special codes for line 18 */
		line18 = curpat->line18;
		if (line18 == INSERT_FRAME_CODE_INC)
		{
			framenum++;
			bcdframenum = (((framenum / 10000) % 10) << 16) | (((framenum / 1000) % 10) << 12) | (((framenum / 100) % 10) << 8) | (((framenum / 10) % 10) << 4) | (framenum % 10);
		}
		if (line18 == INSERT_FRAME_CODE || line18 == INSERT_FRAME_CODE_INC)
			line18 = 0xf80000 | bcdframenum;

		/* bail if we passed the end */
		if (framenum > patcount)
			return 0;

		/* if we don't have a white flag, the previous frame must match us */
		if (!curpat->white) flags |= 0x01;

		/* advance to the next pattern piece */
		curpat = curpat->next;
		if (curpat == NULL)
			curpat = pattern;

		/* if the new field doesn't have a white flag, it must match our current frame */
		if (!curpat->white) flags |= 0x02;

		/* output the result */
		output_meta(flags, curpat->white, line16, line17, line18, bcdframenum, 0);
	}

	return 0;
}


/*-------------------------------------------------
    usage - display program usage
-------------------------------------------------*/

static int usage(void)
{
	fprintf(stderr, "Usage: \n");
	fprintf(stderr, "  makemeta [avifile.avi] [<option> [<option> [...]]]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -leadin <count> -- prepend <count> fields of leadin codes\n");
	fprintf(stderr, "  -leadout <count> -- append <count> fields of leadout codes\n");
	fprintf(stderr, "  -pattern <frames>*<pat0>[,<pat1>[,...]] -- repeat Philips code pattern\n");
	fprintf(stderr, "      in place of AVI. <pat0>, <pat1>, etc. are of the form: \n");
	fprintf(stderr, "      [!]A[.B[.C]][,<pattern>] where A,B,C are either hexadecimal\n");
	fprintf(stderr, "      values one of these special codes:\n");
	fprintf(stderr, "          ! -- at the start of a pattern sets the white flag\n");
	fprintf(stderr, "          + -- increments frame number and inserts frame code\n");
	fprintf(stderr, "          @ -- inserts repeat of most recent frame code\n");
	fprintf(stderr, "      non-present values are assumed to be 0\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Examples:\n");
	fprintf(stderr, "  makemeta -pattern 54000*!8da01e.+.@,8da01e,!8da01e.+.@,8da01e,8da01e\n");
	fprintf(stderr, "    generates the following pattern of Philips codes:\n");
	fprintf(stderr, "        8da01e f80001 f80001 (white)\n");
	fprintf(stderr, "        8da01e 000000 000000\n");
	fprintf(stderr, "        8da01e f80002 f80002 (white)\n");
	fprintf(stderr, "        8da01e 000000 000000\n");
	fprintf(stderr, "        8da01e 000000 000000\n");
	fprintf(stderr, "        <repeat until frame is 54000>\n");
	return 1;
}


/*-------------------------------------------------
    main - main entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	int leadin = 0, leadout = 0;
	const char *avifile = NULL;
	pattern_data *pattern = NULL;
	int patcount = 0;
	int arg;
	int i;

	/* iterate over arguments */
	for (arg = 1; arg < argc; arg++)
	{
		/* assume anything without a - is a filename */
		if (argv[arg][0] != '-')
		{
			if (avifile != NULL || pattern != NULL)
				return usage();
			avifile = argv[arg];
		}

		/* look for options */
		else if (strcmp(argv[arg], "-leadin") == 0)
		{
			if (++arg >= argc)
				return usage();
			leadin = atoi(argv[arg]);
		}
		else if (strcmp(argv[arg], "-leadout") == 0)
		{
			if (++arg >= argc)
				return usage();
			leadout = atoi(argv[arg]);
		}
		else if (strcmp(argv[arg], "-pattern") == 0)
		{
			if (avifile != NULL || pattern != NULL)
				return usage();
			if (++arg >= argc)
				return usage();
			pattern = parse_pattern(argv[arg], &patcount);
			if (pattern == NULL)
				return usage();
		}
	}

	/* must have an AVI file or a pattern */
	if (avifile == NULL && pattern == NULL)
		return usage();

	/* output header and leadin */
	printf("chdmeta 12\n");
	for (i = 0; i < leadin; i++)
	{
		int flags = (i == 0) ? 0x02 : (i == leadin - 1) ? 0x01 : 0x03;
		printf("02%02X%02X00000088FFFF88FFFF\n", flags, (i % 2));
	}

	/* if we got a file, output it */
	if (avifile != NULL)
		generate_from_avi(avifile);
	else if (pattern != NULL)
		generate_from_pattern(pattern, patcount);

	/* output leadout */
	for (i = 0; i < leadout; i++)
	{
		int flags = (i == 0) ? 0x02 : (i == leadout - 1) ? 0x01 : 0x03;
		printf("02%02X%02X00000080EEEE80EEEE\n", flags, (i % 2));
	}

	return 0;
}
