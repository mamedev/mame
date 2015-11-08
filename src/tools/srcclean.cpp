// license:BSD-3-Clause
// copyright-holders:Aaron Giles, smf
/***************************************************************************

    srcclean.c

    Basic source code cleanear.

****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "corestr.h"
#include "osdcore.h"


/***************************************************************************
    CONSTANTS & DEFINES
***************************************************************************/

#define MAX_FILE_SIZE   (32 * 1024 * 1024)



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static UINT8 original[MAX_FILE_SIZE];
static UINT8 modified[MAX_FILE_SIZE];


static int le_convert(char *buffer, int size)
{
	char *pos;
	char *end = buffer + size;

	/* brute force */
	*end = 0;
	pos = strchr(buffer, 0x0d);
	while (pos != NULL)
	{
		memmove(pos, pos+1,end - pos + 1);
		size--;
		buffer = pos + 1;
		pos = strchr(buffer, 0x0d);
	}
	return size;
}

/***************************************************************************
    MAIN
***************************************************************************/

int main(int argc, char *argv[])
{
	bool unix_le = false;
	int removed_tabs = 0;
	int added_tabs = 0;
	int removed_spaces = 0;
	int removed_continuations = 0;
	int fixed_dos_style = 0;
	int fixed_mac_style = 0;
	int fixed_nix_style = 0;
	int added_newline = 0;
	int removed_newlines = 0;
	int src = 0;
	int dst = 0;
	bool in_multiline_comment = false;
	bool in_singleline_comment = false;
	int indent_multiline_comment = 0;
	int in_c_string = FALSE;
	int hichars = 0;
	bool is_c_file;
	bool is_xml_file;
	const char *ext;
	FILE *file;
	int bytes;
	int col = 0;
	int escape = 0;
	int consume = 0;
	const int tab_size = 4;
	bool arg_found = true;
	bool dry_run = false;

	while (arg_found && argc > 1) {
		if (strcmp(argv[1], "-u") == 0)
		{
			unix_le = true;
			argc--;
			argv++;
		}
		else if (strcmp(argv[1], "-d") == 0)
		{
			dry_run = true;
			argc--;
			argv++;
		}
		else
			arg_found = false;

	}

	/* print usage info */
	if (argc < 2)
	{
		printf("Usage:\nsrcclean [-u] [-d] <file>\n");
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

	/* check whether we have dos line endings and are in unix mode */
	if (unix_le && (strchr((char *) original, 0x0d) != NULL))
		fixed_dos_style = 1;

	/* determine if we are a C file */
	ext = strrchr(argv[1], '.');
	is_c_file = (ext && (core_stricmp(ext, ".c") == 0 || core_stricmp(ext, ".h") == 0 || core_stricmp(ext, ".cpp") == 0 || core_stricmp(ext, ".inc") == 0 || core_stricmp(ext, ".lst") == 0));
	is_xml_file = (ext && core_stricmp(ext, ".xml") == 0);

	/* rip through it */
	for (src = 0; src < bytes; )
	{
		UINT8 ch = original[src++];

		if (consume == 0)
		{
			/* C-specific handling */
			if (is_c_file)
			{
				/* check for string/char literals */
				if ((ch == '"' || ch == '\'') && !in_multiline_comment && !in_singleline_comment )
				{
					if (ch == in_c_string && !escape)
						in_c_string = 0;
					else if (!in_c_string)
						in_c_string = ch;
				}

				/* Update escape state */
				if (in_c_string)
					escape = (ch == '\\') ? !escape : 0;

				if (!in_c_string && !in_singleline_comment)
				{
					/* track whether or not we are within a C-style comment */
					if (!in_multiline_comment && ch == '/' && original[src] == '*')
					{
						in_multiline_comment = true;
						if (col > 0 && modified[dst-1] == 0x09)
						{
							indent_multiline_comment = col;
						}
						else
						{
							indent_multiline_comment = 0;
						}
						consume = 2;
					}
					else if (in_multiline_comment && ch == '*' && original[src] == '/')
					{
						in_multiline_comment = false;
						indent_multiline_comment = 0;
						consume = 2;
					}

					/* track whether or not we are within a C++-style comment */
					else if (!in_multiline_comment && ch == '/' && original[src] == '/')
					{
						in_singleline_comment = true;
						consume = 2;
					}
				}
			}

			if (is_xml_file)
			{
				/* track whether or not we are within a XML comment */
				if (!in_multiline_comment && ch == '<' && original[src] == '!' && original[src+1] == '-' && original[src+2] == '-')
				{
					in_multiline_comment = true;
					if (col > 0 && modified[dst-1] == 0x09)
					{
						indent_multiline_comment = col;
					}
					else
					{
						indent_multiline_comment = 0;
					}
					consume = 4;
				}
				else if (in_multiline_comment && ch == '-' && original[src] == '-' && original[src+1] == '>')
				{
					in_multiline_comment = false;
					indent_multiline_comment = 0;
					consume = 3;
				}
			}
		}

		if (consume != 0)
		{
			modified[dst++] = ch;
			col++;
			consume--;
		}

		/* if we hit a CR or LF, clean up from there */
		else if (ch == 0x0d || ch == 0x0a)
		{
			while (true)
			{
				/* remove all extra spaces/tabs at the end */
				if (dst > 0 && (modified[dst-1] == ' ' || modified[dst-1] == 0x09))
				{
					removed_spaces++;
					dst--;
				}
				/* remove extraneous line continuation followed by a blank line */
				else if (is_c_file && !in_multiline_comment && dst > 2 && modified[dst-3] == '\\' && modified[dst-2] == 0x0d && modified[dst-1]==0x0a)
				{
					removed_continuations++;
					dst -= 3;
				}
				/* remove blank lines following an opening brace */
				else if (is_c_file && !in_multiline_comment && dst > 2 && modified[dst-3] == '{' && modified[dst-2] == 0x0d && modified[dst-1]==0x0a)
				{
					removed_newlines++;
					dst -= 2;
				}
				else
				{
					break;
				}
			}

			/* insert a proper CR/LF */
			modified[dst++] = 0x0d;
			modified[dst++] = 0x0a;
			col = 0;

			/* skip over any LF in the source file */
			if (ch == 0x0d && original[src] == 0x0a)
				src++;
			else if (ch == 0x0a)
				fixed_nix_style = 1;
			else
				fixed_mac_style = 1;

			/* we are no longer in a C++-style comment */
			in_singleline_comment = false;

			if (in_c_string && modified[dst-3] != '\\')
			{
				printf("Error: unterminated string literal: %x (%s)\n", src, argv[1]);
				return 1;
			}
		}

		/* if we hit a tab... */
		else if (ch == 0x09)
		{
			int spaces = tab_size - (col % tab_size);

			/* convert tabs to spaces, if not used for indenting */
			if ((in_multiline_comment && col >= indent_multiline_comment) || (col != 0 && modified[dst-1] != 0x09))
			{
				while (spaces > 0)
				{
					modified[dst++] = ' ';
					col++;
					spaces--;
				}

				removed_tabs++;
			}
			else
			{
				modified[dst++] = ch;
				col += spaces;
			}
		}

		/* if we hit a space... */
		else if (ch == 0x20)
		{
			int spaces = 1;

			while (original[src] == 0x20)
			{
				spaces++;
				src++;
			}

			/* Remove invisible spaces */
			if (original[src] == 0x09)
			{
				int realign = (col + spaces) % tab_size;
				removed_spaces += realign;
				spaces -= realign;
			}

			/* convert spaces to tabs, if used for indenting */
			while (spaces > 0 && (!in_multiline_comment || col < indent_multiline_comment) && (col == 0 || modified[dst-1] == 0x09) && !in_c_string)
			{
				modified[dst++] = 0x09;
				spaces -= tab_size;
				col += tab_size;
				added_tabs++;
			}

			while (spaces > 0)
			{
				modified[dst++] = ' ';
				col++;
				spaces--;
			}
		}

		/* otherwise, copy the source character */
		else
		{
			/* check for invalid upper-ASCII chars, but only for non-xml files (swlists might contain UTF-8 chars) */
			if (!is_xml_file && (ch < 32 || ch > 127))
			{
				ch = '?';
				hichars++;
			}

			modified[dst++] = ch;
			col++;
		}
	}

	/* if we didn't find an end of comment, we screwed up */
	if (in_multiline_comment)
	{
		printf("Error: unmatched multi-line comment (%s)!\n", argv[1]);
		return 1;
	}

	if (is_c_file)
	{
		if (modified[dst - 1] != 0x0a)
		{
			modified[dst++] = 0x0d;
			modified[dst++] = 0x0a;
			added_newline = 1;
		}
		else
		{
			while (dst >= 4 && modified[dst - 4] == 0x0d && modified[dst - 3] == 0x0a)
			{
				dst -= 2;
				removed_newlines++;
			}
		}
	}

	/* convert to unix_le if requested */

	if (unix_le)
		dst = le_convert((char *) modified, dst);

	/* if the result == original, skip it */
	if (dst != bytes || memcmp(original, modified, bytes))
	{
		/* explain what we did */
		printf("Cleaned up %s:", argv[1]);
		if (added_newline) printf(" added newline at end of file");
		if (removed_newlines) printf(" removed %d newline(s)", removed_newlines);
		if (removed_spaces) printf(" removed %d space(s)", removed_spaces);
		if (removed_continuations) printf(" removed %d continuation(s)", removed_continuations);
		if (removed_tabs) printf(" removed %d tab(s)", removed_tabs);
		if (added_tabs) printf(" added %d tab(s)", added_tabs);
		if (hichars) printf(" fixed %d high-ASCII char(s)", hichars);
		if (fixed_nix_style && !unix_le) printf(" fixed *nix-style line-ends");
		if (fixed_mac_style) printf(" fixed Mac-style line-ends");
		if (fixed_dos_style) printf(" fixed Dos-style line-ends");
		printf("\n");

		if (!dry_run)
		{
			/* write the file */
			file = fopen(argv[1], "wb");
			fwrite(modified, 1, dst, file);
			fclose(file);
		}
	}

	return 0;
}
