// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    makelist.c

    Create and sort the driver list.

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "corefile.h"
#include "cstrpool.h"


static dynamic_array<const char *> drivlist;
static dynamic_array<const char *> ignorelst;
static const_string_pool string_pool;


//-------------------------------------------------
//  driver_sort_callback - compare two items in
//  a string array
//-------------------------------------------------

int sort_callback(const void *elem1, const void *elem2)
{
	const char **item1 = (const char **)elem1;
	const char **item2 = (const char **)elem2;
	return strcmp(*item1, *item2);
}

//-------------------------------------------------
//  isignored - return info if item is in ignore
//  list or not
//-------------------------------------------------

bool isignored(const char *drivname)
{
	for (int i = 0; i < ignorelst.count(); i++)
		if (strcmp(ignorelst[i], drivname) == 0)
			return true;
	return false;
}

//-------------------------------------------------
//  parse_file - parse a single file, may be
//  called recursively
//-------------------------------------------------

int parse_file(const char *srcfile)
{
	// read source file
	dynamic_buffer buffer;
	file_error filerr = core_fload(srcfile, buffer);
	if (filerr != FILERR_NONE)
	{
		fprintf(stderr, "Unable to read source file '%s'\n", srcfile);
		return 1;
	}

	// rip through it to find all drivers
	char *srcptr = (char *)&buffer[0];
	char *endptr = srcptr + buffer.count();
	int linenum = 1;
	bool in_comment = false;
	while (srcptr < endptr)
	{
		char c = *srcptr++;

		// count newlines
		if (c == 13 || c == 10)
		{
			if (c == 13 && *srcptr == 10)
				srcptr++;
			linenum++;
			continue;
		}

		// skip any spaces
		if (isspace(c))
			continue;

		// look for end of C comment
		if (in_comment && c == '*' && *srcptr == '/')
		{
			srcptr++;
			in_comment = false;
			continue;
		}

		// skip anything else inside a C comment
		if (in_comment)
			continue;

		// look for start of C comment
		if (c == '/' && *srcptr == '*')
		{
			srcptr++;
			in_comment = true;
			continue;
		}

		// if we hit a C++ comment, scan to the end of line
		if (c == '/' && *srcptr == '/')
		{
			while (srcptr < endptr && *srcptr != 13 && *srcptr != 10)
				srcptr++;
			continue;
		}

		// look for an import directive
		if (c == '#')
		{
			char filename[256];
			filename[0] = 0;
			for (int pos = 0; srcptr < endptr && pos < ARRAY_LENGTH(filename) - 1 && !isspace(*srcptr); pos++)
			{
				filename[pos] = *srcptr++;
				filename[pos+1] = 0;
			}
			fprintf(stderr, "Importing drivers from '%s'\n", filename);
			parse_file(filename);
			continue;
		}
		if (c == '!')
		{
			char drivname[256];
			drivname[0] = 0;
			for (int pos = 0; srcptr < endptr && pos < ARRAY_LENGTH(drivname) - 1 && !isspace(*srcptr); pos++)
			{
				drivname[pos] = *srcptr++;
				drivname[pos+1] = 0;
			}
			fprintf(stderr, "Place driver '%s' to ignore list\n", drivname);
			ignorelst.append(string_pool.add(drivname));
			continue;
		}

		// otherwise treat as a driver name
		char drivname[32];
		drivname[0] = 0;
		srcptr--;
		for (int pos = 0; srcptr < endptr && pos < ARRAY_LENGTH(drivname) - 1 && !isspace(*srcptr); pos++)
		{
			drivname[pos] = *srcptr++;
			drivname[pos+1] = 0;
		}

		// verify the name as valid
		for (char *drivch = drivname; *drivch != 0; drivch++)
		{
			if ((*drivch >= 'a' && *drivch <= 'z') || (*drivch >= '0' && *drivch <= '9') || *drivch == '_')
				continue;
			fprintf(stderr, "%s:%d - Invalid character '%c' in driver \"%s\"\n", srcfile, linenum, *drivch, drivname);
			return 1;
		}

		// add it to the list
		if (!isignored(drivname))
			drivlist.append(string_pool.add(drivname));
	}

	return 0;
}


//-------------------------------------------------
//  main - primary entry point
//-------------------------------------------------

int main(int argc, char *argv[])
{
	// needs at least 1 argument
	if (argc < 2)
	{
		fprintf(stderr,
			"Usage:\n"
			"  makelist <source.lst>\n"
		);
		return 0;
	}

	// extract arguments
	const char *srcfile = argv[1];

	// parse the root file, exit early upon failure
	if (parse_file(srcfile))
		return 1;

	// output a count
	if (drivlist.count() == 0)
	{
		fprintf(stderr, "No drivers found\n");
		return 1;
	}
	fprintf(stderr, "%d drivers found\n", drivlist.count());

	// add a reference to the ___empty driver
	drivlist.append("___empty");

	// sort the list
	qsort(drivlist, drivlist.count(), sizeof(drivlist[0]), sort_callback);

	// start with a header
	printf("#include \"emu.h\"\n\n");
	printf("#include \"drivenum.h\"\n\n");

	// output the list of externs first
	for (int index = 0; index < drivlist.count(); index++)
		printf("GAME_EXTERN(%s);\n", drivlist[index]);
	printf("\n");

	// then output the array
	printf("const game_driver * const driver_list::s_drivers_sorted[%d] =\n", drivlist.count());
	printf("{\n");
	for (int index = 0; index < drivlist.count(); index++)
		printf("\t&GAME_NAME(%s)%s\n", drivlist[index], (index == drivlist.count() - 1) ? "" : ",");
	printf("};\n");
	printf("\n");

	// also output a global count
	printf("int driver_list::s_driver_count = %d;\n", drivlist.count());

	return 0;
}
