/***************************************************************************

    makedev.c

    Create and sort the driver list.

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

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "corefile.h"


#define MAX_DEVICES 65536

static const char *devlist[MAX_DEVICES];
static int devcount;


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
//  parse_file - parse a single file, may be
//  called recursively
//-------------------------------------------------

int parse_file(const char *srcfile)
{
	// read source file
	void *buffer;
	UINT32 length;
	file_error filerr = core_fload(srcfile, &buffer, &length);
	if (filerr != FILERR_NONE)
	{
		fprintf(stderr, "Unable to read source file '%s'\n", srcfile);
		return 1;
	}

	// rip through it to find all drivers
	char *srcptr = (char *)buffer;
	char *endptr = srcptr + length;
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
			fprintf(stderr, "Importing devices from '%s'\n", filename);
			parse_file(filename);
			continue;
		}

		// otherwise treat as a device name
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
			if ((*drivch >= 'A' && *drivch <= 'Z') || (*drivch >= '0' && *drivch <= '9') || *drivch == '_')
				continue;
			fprintf(stderr, "%s:%d - Invalid character '%c' in device type \"%s\"\n", srcfile, linenum, *drivch, drivname);
			return 1;
		}

		// add it to the list
		char *name = (char *)malloc(strlen(drivname) + 1);
		strcpy(name, drivname);
		devlist[devcount++] = name;
	}

	osd_free(buffer);

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
			"  makedev <source.lst>\n"
		);
		return 0;
	}

	// extract arguments
	const char *srcfile = argv[1];

	// parse the root file, exit early upon failure
	devcount = 0;
	if (parse_file(srcfile))
		return 1;

	// output a count
	if (devcount == 0)
	{
		fprintf(stderr, "No devices found\n");
	} else {
		fprintf(stderr, "%d devices found\n", devcount);
	}

	// sort the list
	qsort(devlist, devcount, sizeof(*devlist), sort_callback);

	// start with a header
	printf("#include \"emu.h\"\n\n");

	// output the list of externs first
	for (int index = 0; index < devcount; index++)
		printf("extern const device_type %s;\n", devlist[index]);
	printf("\n");

	// then output the array
	printf("const device_type * s_devices_sorted[] =\n");
	printf("{\n");
	for (int index = 0; index < devcount; index++)
		printf("\t&%s%s\n", devlist[index], (index == devcount - 1) ? "" : ",");
	printf("};\n");
	printf("\n");

	// also output a global count
	printf("int m_device_count = %d;\n", devcount);

	return 0;
}
