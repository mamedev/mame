// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Regression test report generator

****************************************************************************/

#include "corefile.h"
#include "corestr.h"
#include "ioprocsstream.h"
#include "path.h"
#include "png.h"
#include "strformat.h"

#include "osdcomm.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale>
#include <new>


/***************************************************************************
    CONSTANTS & DEFINES
***************************************************************************/

#define MAX_COMPARES            16
#define BITMAP_SPACE            4

enum
{
	STATUS_NOT_PRESENT = 0,
	STATUS_SUCCESS,
	STATUS_SUCCESS_DIFFERENT,
	STATUS_MISSING_FILES,
	STATUS_EXCEPTION,
	STATUS_FATAL_ERROR,
	STATUS_FAILED_VALIDITY,
	STATUS_OTHER,
	STATUS_COUNT
};

enum
{
	BUCKET_UNKNOWN = 0,
	BUCKET_IMPROVED,
	BUCKET_REGRESSED,
	BUCKET_CHANGED,
	BUCKET_MULTI_ERROR,
	BUCKET_CONSISTENT_ERROR,
	BUCKET_GOOD,
	BUCKET_GOOD_BUT_CHANGED,
	BUCKET_GOOD_BUT_CHANGED_SCREENSHOTS,
	BUCKET_COUNT
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct summary_file
{
	summary_file *  next;
	char            name[20];
	char            source[100];
	uint8_t         status[MAX_COMPARES];
	uint8_t         matchbitmap[MAX_COMPARES];
	std::string     text[MAX_COMPARES];
};


struct summary_list
{
	summary_list *  next;
	summary_file *  files;
	char *          dir;
	char            version[40];
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static summary_file *filehash[128][128];
static summary_list lists[MAX_COMPARES];
static int list_count;

static const char *const bucket_name[] =
{
	"Unknown",
	"Games That Have Improved",
	"Games That Have Regressed",
	"Games With Changed Screenshots",
	"Games With Multiple Errors",
	"Games With Consistent Errors",
	"Games That Are Consistently Good",
	"Games That Regressed But Improved",
	"Games With Changed Screenshots",
};

static const int bucket_output_order[] =
{
	BUCKET_REGRESSED,
	BUCKET_IMPROVED,
	BUCKET_CHANGED,
	BUCKET_GOOD_BUT_CHANGED_SCREENSHOTS,
	BUCKET_GOOD_BUT_CHANGED,
	BUCKET_MULTI_ERROR,
	BUCKET_CONSISTENT_ERROR
};

static const char *const status_text[] =
{
	"",
	"Success",
	"Changed",
	"Missing Files",
	"Exception",
	"Fatal Error",
	"Failed Validity Check",
	"Other Unknown Error"
};

static const char *const status_color[] =
{
	"",
	"background:#00A000",
	"background:#E0E000",
	"background:#8000C0",
	"background:#C00000",
	"background:#C00000",
	"background:#C06000",
	"background:#C00000",
	"background:#C00000",
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* summary parsing */
static int read_summary_log(const char *filename, int index);
static summary_file *parse_driver_tag(char *linestart, int index);
static summary_file *get_file(const char *filename);
static int CLIB_DECL compare_file(const void *file0ptr, const void *file1ptr);
static summary_file *sort_file_list(void);

/* HTML helpers */
static void output_header(std::ostream &file, std::string_view templatefile, const std::string &title);
static void output_footer(std::ostream &file, std::string_view templatefile, const std::string &title);

/* report generators */
static void output_report(std::string_view dirname, std::string_view tempheader, std::string_view tempfooter, summary_file *filelist);
static int compare_screenshots(summary_file *curfile);
static int generate_png_diff(const summary_file *curfile, std::string_view destdir, std::string_view destname);
static void create_linked_file(std::string_view dirname, const summary_file *curfile, const summary_file *prevfile, const summary_file *nextfile, std::string_view pngfile, std::string_view tempheader, std::string_view tempfooter);
static void append_driver_list_table(const char *header, std::string_view dirname, std::ostream &indexfile, const summary_file *listhead, std::string_view tempheader, std::string_view tempfooter);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    trim_string - trim leading/trailing spaces
    from a string
-------------------------------------------------*/

static inline char *trim_string(char *string)
{
	int length;

	/* trim leading spaces */
	while (*string != 0 && isspace((uint8_t)*string))
		string++;

	/* trim trailing spaces */
	length = strlen(string);
	while (length > 0 && isspace((uint8_t)string[length - 1]))
		string[--length] = 0;

	return string;
}


/*-------------------------------------------------
    get_unique_index - get the unique bitmap
    index for a given entry
-------------------------------------------------*/

static inline int get_unique_index(const summary_file *curfile, int index)
{
	int listnum, curindex = 0;

	/* if we're invalid, just return that */
	if (curfile->matchbitmap[index] == 0xff)
		return -1;

	/* count unique elements up to us */
	for (listnum = 0; listnum < curfile->matchbitmap[index]; listnum++)
		if (curfile->matchbitmap[listnum] == listnum)
			curindex++;
	return curindex;
}



/***************************************************************************
    MAIN
***************************************************************************/

/*-------------------------------------------------
    main - main entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	size_t bufsize;
	void *buffer;
	int listnum;
	int result;

	/* first argument is the directory */
	if (argc < 4)
	{
		fprintf(stderr, "Usage:\nregrep <template> <outputdir> <summary1> [<summary2> [<summary3> ...]]\n");
		return 1;
	}
	std::string tempfilename(argv[1]);
	std::string dirname(argv[2]);
	list_count = argc - 3;

	/* read the template file into an astring */
	std::string tempheader;
	if (!util::core_file::load(tempfilename, &buffer, bufsize))
	{
		tempheader.assign((const char *)buffer, bufsize);
		free(buffer);
	}

	/* verify the template */
	if (tempheader.length() == 0)
	{
		fprintf(stderr, "Unable to read template file\n");
		return 1;
	}
	result = tempheader.find("<!--CONTENT-->");
	if (result == -1)
	{
		fprintf(stderr, "Template is missing a <!--CONTENT--> marker\n");
		return 1;
	}
	std::string tempfooter(tempheader);
	tempfooter = tempfooter.substr(result + 14);
	tempfooter = tempheader.substr(0, result);

	/* loop over arguments and read the files */
	for (listnum = 0; listnum < list_count; listnum++)
	{
		result = read_summary_log(argv[listnum + 3], listnum);
		if (result != 0)
			return result;
	}

	/* output the summary */
	output_report(dirname, tempheader, tempfooter, sort_file_list());
	return 0;
}



/***************************************************************************
    SUMMARY PARSING
***************************************************************************/

/*-------------------------------------------------
    get_file - lookup a driver name in the hash
    table and return a pointer to it; if none
    found, allocate a new entry
-------------------------------------------------*/

static summary_file *get_file(const char *filename)
{
	summary_file *file = nullptr;

	/* use the first two characters as a lookup */
	for (file = filehash[filename[0] & 0x7f][filename[1] & 0x7f]; file != nullptr; file = file->next)
		if (strcmp(filename, file->name) == 0)
			return file;

	/* didn't find one -- allocate */
	file = new (std::nothrow) summary_file;
	if (file == nullptr)
		return nullptr;
	file->next = nullptr;
	std::fill(std::begin(file->name), std::end(file->name), '\0');
	std::fill(std::begin(file->source), std::end(file->source), '\0');
	std::fill(std::begin(file->status), std::end(file->status), 0);
	std::fill(std::begin(file->matchbitmap), std::end(file->matchbitmap), 0);

	/* set the name so we find it in the future */
	strcpy(file->name, filename);

	/* add to the head of the list */
	file->next = filehash[filename[0] & 0x7f][filename[1] & 0x7f];
	filehash[filename[0] & 0x7f][filename[1] & 0x7f] = file;
	return file;
}


/*-------------------------------------------------
    read_summary_log - read a summary.log file
    and build entries for its data
-------------------------------------------------*/

static int read_summary_log(const char *filename, int index)
{
	summary_file *curfile = nullptr;
	char linebuffer[1024];
	char *linestart;
	int drivers = 0;
	FILE *file;

	/* open the logfile */
	file = fopen(filename, "r");
	if (file == nullptr)
	{
		fprintf(stderr, "Error: file '%s' not found\n", filename);
		return 1;
	}

	/* parse it */
	while (fgets(linebuffer, sizeof(linebuffer), file) != nullptr)
	{
		/* trim the leading/trailing spaces */
		linestart = trim_string(linebuffer);

		/* is this one of our specials? */
		if (strncmp(linestart, "@@@@@", 5) == 0)
		{
			/* advance past the signature */
			linestart += 5;

			/* look for the driver= tag */
			if (strncmp(linestart, "driver=", 7) == 0)
			{
				curfile = parse_driver_tag(linestart + 7, index);
				if (curfile == nullptr)
					goto error;
				drivers++;
			}

			/* look for the source= tag */
			else if (strncmp(linestart, "source=", 7) == 0)
			{
				/* error if no driver yet */
				if (curfile == nullptr)
				{
					fprintf(stderr, "Unexpected @@@@@source= tag\n");
					goto error;
				}

				/* copy the string */
				strcpy(curfile->source, trim_string(linestart + 7));
			}

			/* look for the dir= tag */
			else if (strncmp(linestart, "dir=", 4) == 0)
			{
				char *dirname = trim_string(linestart + 4);

				/* allocate a copy of the string */
				lists[index].dir = (char *)malloc(strlen(dirname) + 1);
				if (lists[index].dir == nullptr)
					goto error;
				strcpy(lists[index].dir, dirname);
				fprintf(stderr, "Directory %s\n", lists[index].dir);
			}
		}

		/* if not, consider other options */
		else if (curfile != nullptr)
		{
			int foundchars = 0;
			char *curptr;

			/* look for the pngcrc= tag */
			if (strncmp(linestart, "pngcrc: ", 7) == 0)
			{
			}

			/* otherwise, accumulate the text */
			else
			{
				/* find the end of the line and normalize it with a CR */
				for (curptr = linestart; *curptr != 0 && *curptr != '\n' && *curptr != '\r'; curptr++)
					if (!isspace((uint8_t)*curptr))
						foundchars = 1;
				*curptr++ = '\n';
				*curptr = 0;

				/* ignore blank lines */
				if (!foundchars)
					continue;

				/* append our text */
				curfile->text[index].append(linestart);
			}
		}

		/* look for the MAME header */
		else if (strncmp(linestart, "MAME v", 6) == 0)
		{
			char *start = linestart + 6;
			char *end;

			/* find the end */
			for (end = start; !isspace((uint8_t)*end); end++) ;
			*end = 0;
			strcpy(lists[index].version, start);
			fprintf(stderr, "Parsing results from version %s\n", lists[index].version);
		}
	}

	fclose(file);
	fprintf(stderr, "Parsed %d drivers\n", drivers);
	return 0;

error:
	fclose(file);
	return 1;
}


/*-------------------------------------------------
    parse_driver_tag - parse the status info
    from a driver tag
-------------------------------------------------*/

static summary_file *parse_driver_tag(char *linestart, int index)
{
	summary_file *curfile;
	char *colon;

	/* find the colon separating name from status */
	colon = strchr(linestart, ':');
	if (colon == nullptr)
	{
		fprintf(stderr, "Unexpected text after @@@@@driver=\n");
		return nullptr;
	}

	/* NULL terminate at the colon and look up the file */
	*colon = 0;
	curfile = get_file(trim_string(linestart));
	if (curfile == nullptr)
	{
		fprintf(stderr, "Unable to allocate memory for driver\n");
		return nullptr;
	}

	/* clear out any old status for this file */
	curfile->status[index] = STATUS_NOT_PRESENT;
	curfile->text[index].clear();

	/* strip leading/trailing spaces from the status */
	colon = trim_string(colon + 1);

	/* convert status into statistics */
	if (strcmp(colon, "Success") == 0)
		curfile->status[index] = STATUS_SUCCESS;
	else if (strcmp(colon, "Missing files") == 0)
		curfile->status[index] = STATUS_MISSING_FILES;
	else if (strcmp(colon, "Exception") == 0)
		curfile->status[index] = STATUS_EXCEPTION;
	else if (strcmp(colon, "Fatal error") == 0)
		curfile->status[index] = STATUS_FATAL_ERROR;
	else if (strcmp(colon, "Failed validity check") == 0)
		curfile->status[index] = STATUS_FAILED_VALIDITY;
	else
		curfile->status[index] = STATUS_OTHER;

	return curfile;
}


/*-------------------------------------------------
    compare_file - compare two files, sorting
    first by source filename, then by driver name
-------------------------------------------------*/

static int CLIB_DECL compare_file(const void *file0ptr, const void *file1ptr)
{
	summary_file *file0 = *(summary_file **)file0ptr;
	summary_file *file1 = *(summary_file **)file1ptr;
	int result = strcmp(file0->source, file1->source);
	if (result == 0)
		result = strcmp(file0->name, file1->name);
	return result;
}


/*-------------------------------------------------
    sort_file_list - convert the hashed lists
    into a single, sorted list
-------------------------------------------------*/

static summary_file *sort_file_list()
{
	summary_file *listhead, **tailptr, *curfile, **filearray;
	int numfiles, filenum;
	int c0, c1;

	/* count the total number of files */
	numfiles = 0;
	for (c0 = 0; c0 < 128; c0++)
		for (c1 = 0; c1 < 128; c1++)
			for (curfile = filehash[c0][c1]; curfile != nullptr; curfile = curfile->next)
				numfiles++;

	/* allocate an array of files */
	filearray = (summary_file **)malloc(numfiles * sizeof(*filearray));
	if (filearray == nullptr)
	{
		fprintf(stderr, "Out of memory!\n");
		return nullptr;
	}

	/* populate the array */
	numfiles = 0;
	for (c0 = 0; c0 < 128; c0++)
		for (c1 = 0; c1 < 128; c1++)
			for (curfile = filehash[c0][c1]; curfile != nullptr; curfile = curfile->next)
				filearray[numfiles++] = curfile;

	/* sort the array */
	qsort(filearray, numfiles, sizeof(filearray[0]), compare_file);

	/* now regenerate a single list */
	listhead = nullptr;
	tailptr = &listhead;
	for (filenum = 0; filenum < numfiles; filenum++)
	{
		*tailptr = filearray[filenum];
		tailptr = &(*tailptr)->next;
	}
	*tailptr = nullptr;
	free(filearray);

	return listhead;
}



/***************************************************************************
    HTML OUTPUT HELPERS
***************************************************************************/

/*-------------------------------------------------
    output_header - create a new HTML file with
    a standard header
-------------------------------------------------*/

static void output_header(std::ostream &file, std::string_view templatefile, const std::string &title)
{
	/* print a header */
	std::string modified(templatefile);
	strreplace(modified, "<!--TITLE-->", title);
	file << modified;
}


/*-------------------------------------------------
    output_footer - write a standard footer to an
    HTML file
-------------------------------------------------*/

static void output_footer(std::ostream &file, std::string_view templatefile, const std::string &title)
{
	std::string modified(templatefile);
	strreplace(modified, "<!--TITLE-->", title);
	file << modified;
}



/***************************************************************************
    REPORT GENERATORS
***************************************************************************/

/*-------------------------------------------------
    output_report - generate the summary
    report HTML files
-------------------------------------------------*/

static void output_report(std::string_view dirname, std::string_view tempheader, std::string_view tempfooter, summary_file *filelist)
{
	summary_file *buckethead[BUCKET_COUNT], **buckettailptr[BUCKET_COUNT];

	/* initialize the lists */
	for (int bucknum = 0; bucknum < BUCKET_COUNT; bucknum++)
	{
		buckethead[bucknum] = nullptr;
		buckettailptr[bucknum] = &buckethead[bucknum];
	}

	/* compute the total number of files */
	int total = 0;
	for (summary_file *curfile = filelist; curfile; curfile = curfile->next)
		total++;

	/* first bucketize the games */
	int count = 0;
	for (summary_file *curfile = filelist; curfile; curfile = curfile->next)
	{
		int statcount[STATUS_COUNT] = { 0 };
		int bucket = BUCKET_UNKNOWN;

		/* print status */
		if (++count % 100 == 0)
			fprintf(stderr, "Processing file %d/%d\n", count, total);

		/* find the first valid entry */
		int first_valid = 0;
		while (curfile->status[first_valid] == STATUS_NOT_PRESENT)
			first_valid++;

		/* do we need to output anything? */
		int unique_codes = 0;
		for (int listnum = first_valid; listnum < list_count; listnum++)
		{
			if (statcount[curfile->status[listnum]]++ == 0)
				unique_codes++;
		}

		/* were we consistent? */
		if (unique_codes == 1)
		{
			if (curfile->status[first_valid] == STATUS_SUCCESS)
			{
				/* consistently OK */
				bucket = compare_screenshots(curfile);
			}
			else
			{
				/* must have been consistently erroring */
				bucket = BUCKET_CONSISTENT_ERROR;
			}
		}
		else
		{
			/* OK, we're not consistent; could be a number of things */
			if (curfile->status[first_valid] == STATUS_SUCCESS && curfile->status[list_count - 1] == STATUS_SUCCESS)
			{
				/* OK at the start and end but not in the middle */
				bucket = BUCKET_GOOD_BUT_CHANGED;
			}
			else if (curfile->status[first_valid] == STATUS_SUCCESS)
			{
				/* went from good to bad */
				bucket = BUCKET_REGRESSED;
			}
			else if (curfile->status[list_count - 1] == STATUS_SUCCESS)
			{
				/* went from bad to good */
				bucket = BUCKET_IMPROVED;
			}
			else
			{
				/* must have had multiple errors */
				bucket = BUCKET_MULTI_ERROR;
			}
		}

		/* add us to the appropriate list */
		*buckettailptr[bucket] = curfile;
		buckettailptr[bucket] = &curfile->next;
	}

	/* terminate all the lists */
	for (int bucknum = 0; bucknum < BUCKET_COUNT; bucknum++)
		*buckettailptr[bucknum] = nullptr;

	/* create the file */
	util::core_file::ptr indexfile;
	{
		const std::string tempname = util::path_concat(dirname, "index.html");
		if (util::core_file::open(tempname, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, indexfile))
		{
			fprintf(stderr, "Error creating file '%s'\n", tempname.c_str());
			return;
		}
	}
	util::owritestream str(*indexfile, util::owritestream::UTF_8, false);
	str.imbue(std::locale::classic());

	/* output header */
	const std::string title("MAME Regressions");
	output_header(str, tempheader, title);

	/* iterate over buckets and output them */
	for (int bucknum = 0; bucknum < std::size(bucket_output_order); bucknum++)
	{
		int curbucket = bucket_output_order[bucknum];

		if (buckethead[curbucket] != nullptr)
		{
			fprintf(stderr, "Outputting bucket: %s\n", bucket_name[curbucket]);
			append_driver_list_table(bucket_name[curbucket], dirname, str, buckethead[curbucket], tempheader, tempfooter);
		}
	}

	/* output footer */
	output_footer(str, tempfooter, title);
}


/*-------------------------------------------------
    compare_screenshots - compare the screenshots
    for all the games in a file
-------------------------------------------------*/

static int compare_screenshots(summary_file *curfile)
{
	bitmap_argb32 bitmaps[MAX_COMPARES];
	int unique[MAX_COMPARES]{};
	int numunique = 0;

	/* iterate over all files and load their bitmaps */
	for (int listnum = 0; listnum < list_count; listnum++)
		if (curfile->status[listnum] == STATUS_SUCCESS)
		{
			std::string fullname;
			std::error_condition filerr;
			util::core_file::ptr file;

			/* get the filename for the image */
			fullname = util::path_concat(lists[listnum].dir, "snap", curfile->name, "final.png");

			/* open the file */
			filerr = util::core_file::open(fullname, OPEN_FLAG_READ, file);

			/* if that failed, look in the old location */
			if (filerr)
			{
				/* get the filename for the image */
				fullname = util::path_concat(lists[listnum].dir, "snap", util::string_format("_%s.png", curfile->name));

				/* open the file */
				filerr = util::core_file::open(fullname, OPEN_FLAG_READ, file);
			}

			/* if that worked, load the file */
			if (!filerr)
			{
				util::png_read_bitmap(*file, bitmaps[listnum]);
				file.reset();
			}
		}

	/* now find all the different bitmap types */
	int listnum;
	for (listnum = 0; listnum < list_count; listnum++)
	{
		curfile->matchbitmap[listnum] = 0xff;
		if (bitmaps[listnum].valid())
		{
			bitmap_argb32 &this_bitmap = bitmaps[listnum];

			/* compare against all unique bitmaps */
			int compnum;
			for (compnum = 0; compnum < numunique; compnum++)
			{
				/* if the sizes are different, we differ; otherwise start off assuming we are the same */
				bitmap_argb32 &base_bitmap = bitmaps[unique[compnum]];
				bool bitmaps_differ = (this_bitmap.width() != base_bitmap.width() || this_bitmap.height() != base_bitmap.height());

				/* compare scanline by scanline */
				for (int y = 0; y < this_bitmap.height() && !bitmaps_differ; y++)
				{
					uint32_t const *base = &base_bitmap.pix(y);
					uint32_t const *curr = &this_bitmap.pix(y);

					/* scan the scanline */
					int x;
					for (x = 0; x < this_bitmap.width(); x++)
						if (*base++ != *curr++)
							break;
					bitmaps_differ = (x != this_bitmap.width());
				}

				/* if we matched, remember which listnum index we matched, and stop */
				if (!bitmaps_differ)
				{
					curfile->matchbitmap[listnum] = unique[compnum];
					break;
				}

				/* if different from the first unique entry, adjust the status */
				if (bitmaps_differ && compnum == 0)
					curfile->status[listnum] = STATUS_SUCCESS_DIFFERENT;
			}

			/* if we're unique, add ourselves to the list */
			if (compnum >= numunique)
			{
				unique[numunique++] = listnum;
				curfile->matchbitmap[listnum] = listnum;
				continue;
			}
		}
	}

	/* if all screenshots matched, we're good */
	if (numunique == 1)
		return BUCKET_GOOD;

	/* if the last screenshot matched the first unique one, we're good but changed */
	if (curfile->matchbitmap[listnum - 1] == unique[0])
		return BUCKET_GOOD_BUT_CHANGED_SCREENSHOTS;

	/* otherwise we're just changed */
	return BUCKET_CHANGED;
}


/*-------------------------------------------------
    generate_png_diff - create a new PNG file
    that shows multiple differing PNGs side by
    side with a third set of differences
-------------------------------------------------*/

static int generate_png_diff(const summary_file *curfile, std::string_view destdir, std::string_view destname)
{
	bitmap_argb32 bitmaps[MAX_COMPARES];
	std::string srcimgname;
	std::string dstfilename;
	bitmap_argb32 finalbitmap;
	int width, height, maxwidth;
	int bitmapcount = 0;
	util::core_file::ptr file;
	std::error_condition filerr;
	int error = -1;
	int starty;

	/* generate the common source filename */
	dstfilename = util::path_concat(destdir, destname);
	srcimgname = util::path_concat("snap", curfile->name, "final.png");

	/* open and load all unique bitmaps */
	for (int listnum = 0; listnum < list_count; listnum++)
		if (curfile->matchbitmap[listnum] == listnum)
		{
			std::string tempname = util::path_concat(lists[listnum].dir, srcimgname);

			/* open the source image */
			filerr = util::core_file::open(tempname, OPEN_FLAG_READ, file);
			if (filerr)
				goto error;

			/* load the source image */
			filerr = util::png_read_bitmap(*file, bitmaps[bitmapcount++]);
			file.reset();
			if (filerr)
				goto error;
		}

	/* if there's only one unique bitmap, skip it */
	if (bitmapcount <= 1)
		goto error;

	/* determine the size of the final bitmap */
	height = width = 0;
	maxwidth = bitmaps[0].width();
	for (int bmnum = 1; bmnum < bitmapcount; bmnum++)
	{
		int curwidth;

		/* determine the maximal width */
		maxwidth = std::max(maxwidth, bitmaps[bmnum].width());
		curwidth = bitmaps[0].width() + BITMAP_SPACE + maxwidth + BITMAP_SPACE + maxwidth;
		width = std::max(width, curwidth);

		/* add to the height */
		height += std::max(bitmaps[0].height(), bitmaps[bmnum].height());
		if (bmnum != 1)
			height += BITMAP_SPACE;
	}

	/* allocate the final bitmap */
	finalbitmap.allocate(width, height);

	/* now copy and compare each set of bitmaps */
	starty = 0;
	for (int bmnum = 1; bmnum < bitmapcount; bmnum++)
	{
		bitmap_argb32 const &bitmap1 = bitmaps[0];
		bitmap_argb32 const &bitmap2 = bitmaps[bmnum];
		int curheight = std::max(bitmap1.height(), bitmap2.height());

		/* iterate over rows in these bitmaps */
		for (int y = 0; y < curheight; y++)
		{
			uint32_t const *src1 = (y < bitmap1.height()) ? &bitmap1.pix(y) : nullptr;
			uint32_t const *src2 = (y < bitmap2.height()) ? &bitmap2.pix(y) : nullptr;
			uint32_t *dst1 = &finalbitmap.pix(starty + y, 0);
			uint32_t *dst2 = &finalbitmap.pix(starty + y, bitmap1.width() + BITMAP_SPACE);
			uint32_t *dstdiff = &finalbitmap.pix(starty + y, bitmap1.width() + BITMAP_SPACE + maxwidth + BITMAP_SPACE);

			/* now iterate over columns */
			for (int x = 0; x < maxwidth; x++)
			{
				int pix1 = -1, pix2 = -2;

				if (src1 != nullptr && x < bitmap1.width())
					pix1 = dst1[x] = src1[x];
				if (src2 != nullptr && x < bitmap2.width())
					pix2 = dst2[x] = src2[x];
				dstdiff[x] = (pix1 != pix2) ? 0xffffffff : 0xff000000;
			}
		}

		/* update the starting Y position */
		starty += BITMAP_SPACE + std::max(bitmap1.height(), bitmap2.height());
	}

	/* write the final PNG */
	filerr = util::core_file::open(dstfilename, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, file);
	if (filerr)
		goto error;
	filerr = util::png_write_bitmap(*file, nullptr, finalbitmap, 0, nullptr);
	file.reset();
	if (filerr)
		goto error;

	/* if we get here, we are error free */
	error = 0;

error:
	if (error)
		osd_file::remove(dstfilename);
	return error;
}


/*-------------------------------------------------
    create_linked_file - create a comparison
    file between differing versions
-------------------------------------------------*/

static void create_linked_file(std::string_view dirname, const summary_file *curfile, const summary_file *prevfile, const summary_file *nextfile, std::string_view pngfile, std::string_view tempheader, std::string_view tempfooter)
{
	/* create the file */
	util::core_file::ptr linkfile;
	{
		const std::string filename = util::string_format("%s.html", curfile->name);
		const std::string linkname = util::path_concat(dirname, filename);
		if (util::core_file::open(linkname, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, linkfile))
		{
			fprintf(stderr, "Error creating file '%s'\n", filename.c_str());
			return;
		}
	}
	util::owritestream str(*linkfile, util::owritestream::UTF_8, false);
	str.imbue(std::locale::classic());

	/* output header */
	const std::string title = util::string_format("%s Regressions (%s)", curfile->name, curfile->source);
	output_header(str, tempheader, title);

	/* link to the previous/next entries */
	str << "\t<p>\n";
	str << "\t<table width=\"100%\">\n";
	str << "\t\t<td align=\"left\" width=\"40%\" style=\"border:none\">";
	if (prevfile)
		util::stream_format(str, "<a href=\"%s.html\"><< %s (%s)</a>", prevfile->name, prevfile->name, prevfile->source);
	str << "</td>\n";
	str << "\t\t<td align=\"center\" width=\"20%\" style=\"border:none\"><a href=\"index.html\">Home</a></td>\n";
	str << "\t\t<td align=\"right\" width=\"40%\" style=\"border:none\">";
	if (nextfile)
		util::stream_format(str, "<a href=\"%s.html\">%s (%s) >></a>", nextfile->name, nextfile->name, nextfile->source);
	str << "</td>\n";
	str << "\t</table>\n";
	str << "\t</p>\n";

	/* output data for each one */
	for (int listnum = 0; listnum < list_count; listnum++)
	{
		int imageindex = -1;

		/* generate the HTML */
		util::stream_format(str, "\n\t<h2>%s</h2>\n", lists[listnum].version);
		str << "\t<p>\n";
		util::stream_format(str, "\t<b>Status:</b> %s\n", status_text[curfile->status[listnum]]);
		if (!pngfile.empty())
			imageindex = get_unique_index(curfile, listnum);
		if (imageindex != -1)
			util::stream_format(str, " [%d]", imageindex);
		str << "\t</p>\n";
		if (curfile->text[listnum].length() != 0)
		{
			str << "\t<p>\n";
			str << "\t<b>Errors:</b>\n";
			util::stream_format(str, "\t<pre>%s</pre>\n", curfile->text[listnum]);
			str << "\t</p>\n";
		}
	}

	/* output link to the image */
	if (!pngfile.empty())
	{
		str << "\n\t<h2>Screenshot Comparisons</h2>\n";
		str << "\t<p>\n";
		util::stream_format(str, "\t<img src=\"%s\" />\n", pngfile);
		str << "\t</p>\n";
	}

	/* output footer */
	output_footer(str, tempfooter, title);
}


/*-------------------------------------------------
    append_driver_list_table - append a table
    of drivers from a list to an HTML file
-------------------------------------------------*/

static void append_driver_list_table(const char *header, std::string_view dirname, std::ostream &indexfile, const summary_file *listhead, std::string_view tempheader, std::string_view tempfooter)
{
	int width = 100 / (2 + list_count);

	/* output a header */
	util::stream_format(indexfile, "\t<h2>%s</h2>\n", header);

	/* start the table */
	indexfile << "\t<p><table width=\"90%\">\n";
	util::stream_format(indexfile, "\t\t<tr>\n\t\t\t<th width=\"%d%%\">Source</th><th width=\"%d%%\">Driver</th>", width, width);
	for (int listnum = 0; listnum < list_count; listnum++)
		util::stream_format(indexfile, "<th width=\"%d%%\">%s</th>", width, lists[listnum].version);
	indexfile << "\n\t\t</tr>\n";

	/* if nothing, print a default message */
	if (listhead == nullptr)
	{
		indexfile << "\t\t<tr>\n\t\t\t";
		util::stream_format(indexfile, "<td colspan=\"%d\" align=\"center\">(No regressions detected)</td>", list_count + 2);
		indexfile << "\n\t\t</tr>\n";
	}

	/* iterate over files */
	for (const summary_file *prevfile = nullptr, *curfile = listhead; curfile; prevfile = curfile, curfile = curfile->next)
	{
		int rowspan = 0, uniqueshots = 0;

		/* if this is the first entry in this source file, count how many rows we need to span */
		if (!prevfile || strcmp(prevfile->source, curfile->source) != 0)
		{
			const summary_file *cur;
			for (cur = curfile; cur; cur = cur->next)
			{
				if (strcmp(cur->source, curfile->source) == 0)
					rowspan++;
				else
					break;
			}
		}

		/* create screenshots if necessary */
		std::string pngdiffname;
		for (int listnum = 0; listnum < list_count; listnum++)
		{
			if (curfile->matchbitmap[listnum] == listnum)
				uniqueshots++;
		}
		if (uniqueshots > 1)
		{
			pngdiffname = util::string_format("compare_%s.png", curfile->name);
			if (generate_png_diff(curfile, dirname, pngdiffname) != 0)
				pngdiffname.clear();
		}

		/* create a linked file */
		create_linked_file(dirname, curfile, prevfile, curfile->next, pngdiffname, tempheader, tempfooter);

		/* create a row */
		indexfile << "\t\t<tr>\n\t\t\t";
		if (rowspan > 0)
			util::stream_format(indexfile, "<td rowspan=\"%d\">%s</td>", rowspan, curfile->source);
		util::stream_format(indexfile, "<td><a href=\"%s.html\">%s</a></td>", curfile->name, curfile->name);
		for (int listnum = 0; listnum < list_count; listnum++)
		{
			int unique_index = -1;

			if (!pngdiffname.empty())
				unique_index = get_unique_index(curfile, listnum);
			if (unique_index != -1)
				util::stream_format(indexfile, "<td><span style=\"%s\">&nbsp;&nbsp;&nbsp;</span> %s [<a href=\"%s\" target=\"blank\">%d</a>]</td>", status_color[curfile->status[listnum]], status_text[curfile->status[listnum]], pngdiffname, unique_index);
			else
				util::stream_format(indexfile, "<td><span style=\"%s\">&nbsp;&nbsp;&nbsp;</span> %s</td>", status_color[curfile->status[listnum]], status_text[curfile->status[listnum]]);
		}
		indexfile << "\n\t\t</tr>\n";

		/* also print the name and source file */
		printf("%s %s\n", curfile->name, curfile->source);
	}

	/* end of table */
	indexfile << "</table></p>\n";
}
