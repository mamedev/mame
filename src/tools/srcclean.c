/***************************************************************************

    srcclean.c

    Basic source code cleanear.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "corestr.h"
#include "osdcore.h"


/***************************************************************************
    CONSTANTS & DEFINES
***************************************************************************/

#define MAX_FILE_SIZE	(10 * 1024 * 1024)



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static UINT8 original[MAX_FILE_SIZE];
static UINT8 modified[MAX_FILE_SIZE];



/***************************************************************************
    MAIN
***************************************************************************/

int main(int argc, char *argv[])
{
	int removed_tabs = 0, removed_spaces = 0, fixed_mac_style = 0, fixed_nix_style = 0;
	int src = 0, dst = 0, in_c_comment = FALSE, in_cpp_comment = FALSE;
	int hichars = 0;
	int is_c_file;
	const char *ext;
	FILE *file;
	int bytes;

	/* print usage info */
	if (argc != 2)
	{
		printf("Usage:\nsrcclean <file>\n");
		return 0;
 	}

	/* read the file */
	file = fopen(argv[1], "rb");
	if (file == NULL)
	{
		fprintf(stderr, "Can't open %s\n", argv[1]);
		return 1;
	}
	bytes = fread(original, 1, MAX_FILE_SIZE, file);
	fclose(file);

	/* determine if we are a C file */
	ext = strrchr(argv[1], '.');
	is_c_file = (ext && (core_stricmp(ext, ".c") == 0 || core_stricmp(ext, ".h") == 0 || core_stricmp(ext, ".cpp") == 0));

	/* rip through it */
	for (src = 0; src < bytes; )
	{
		UINT8 ch = original[src++];

		/* check for invalid upper-ASCII chars */
		if (ch != 13 && ch != 10 && ch != 9 && (ch > 127 || ch < 32))
		{
			ch = '?';
			hichars++;
		}

		/* track whether or not we are within a C-style comment */
		if (is_c_file && !in_cpp_comment)
		{
			if (!in_c_comment && ch == '/' && original[src] == '*')
				in_c_comment = TRUE;
			else if (in_c_comment && ch == '*' && original[src] == '/')
				in_c_comment = FALSE;
		}

		/* track whether or not we are within a C++-style comment */
		if (is_c_file && !in_c_comment && ch == '/' && original[src] == '/')
			in_cpp_comment = TRUE;

		/* if we hit a LF without a CR, back up and act like we hit a CR */
		if (ch == 0x0a)
		{
			src--;
			ch = 0x0d;
			fixed_nix_style = 1;
		}

		/* if we hit a CR, clean up from there */
		if (ch == 0x0d)
		{
			/* remove all extra spaces/tabs at the end */
			while (dst > 0 && (modified[dst-1] == ' ' || modified[dst-1] == 0x09))
			{
				removed_spaces++;
				dst--;
			}

			/* insert a proper CR/LF */
			modified[dst++] = 0x0d;
			modified[dst++] = 0x0a;

			/* skip over any LF in the source file */
			if (original[src] == 0x0a)
				src++;
			else
				fixed_mac_style = 1;

			/* we are no longer in a C++-style comment */
			in_cpp_comment = FALSE;
		}

		/* if we hit a tab within a comment, expand it */
		else if (ch == 0x09 && (in_c_comment || in_cpp_comment))
		{
			int temp, col;

			/* scan backwards to find the start of line */
			for (temp = dst; temp >= 0; temp--)
				if (modified[temp] == 0x0a)
					break;

			/* scan forwards to compute the current column */
			for (temp++, col = 0; temp < dst; temp++)
				if (modified[temp] == 0x09)
					col += 4 - col % 4;
				else
					col++;

			/* compute how many spaces */
			col = 4 - col % 4;
			while (col--) modified[dst++] = ' ';
			removed_tabs++;
		}

		/* otherwise, copy the source character */
		else
			modified[dst++] = ch;
	}

	/* if we didn't find an end of comment, we screwed up */
	if (in_c_comment)
	{
		printf("Error: unmatched C-style comment (%s)!\n", argv[1]);
		return 1;
	}

	/* if the result == original, skip it */
	if (dst != bytes || memcmp(original, modified, bytes))
	{
		/* explain what we did */
		printf("Cleaned up %s:", argv[1]);
		if (removed_spaces) printf(" removed %d spaces", removed_spaces);
		if (removed_tabs) printf(" removed %d tabs", removed_tabs);
		if (hichars) printf(" fixed %d high-ASCII chars", hichars);
		if (fixed_nix_style) printf(" fixed *nix-style line-ends");
		if (fixed_mac_style) printf(" fixed Mac-style line-ends");
		printf("\n");

		/* write the file */
		file = fopen(argv[1], "wb");
		fwrite(modified, 1, dst, file);
		fclose(file);
	}

	return 0;
}
