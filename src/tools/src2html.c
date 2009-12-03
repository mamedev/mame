/***************************************************************************

    MAME source code to HTML converter

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

****************************************************************************

    Template file format:

    <html header>
    <!--PATH--> = insert path
    <!--CONTENT--> = insert content
    <html footer>

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "osdcore.h"
#include "astring.h"
#include "corefile.h"


/***************************************************************************
    CONSTANTS & DEFINES
***************************************************************************/

#define PREPROC_CLASS		"preproc"
#define KEYWORD_CLASS		"keyword"
#define CUSTOM_CLASS		"custom"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

enum _file_type
{
	FILE_TYPE_INVALID,
	FILE_TYPE_C,
	FILE_TYPE_MAKE,
	FILE_TYPE_XML,
	FILE_TYPE_TEXT
};
typedef enum _file_type file_type;


typedef struct _ext_to_type ext_to_type;
struct _ext_to_type
{
	const char *	extension;
	file_type		type;
};


typedef struct _token_entry token_entry;
struct _token_entry
{
	const char *	token;
	const char *	color;
};


typedef struct _include_path include_path;
struct _include_path
{
	include_path *	next;
	const astring *	path;
};


typedef struct _list_entry list_entry;
struct _list_entry
{
	list_entry *	next;
	const astring *	name;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static include_path *incpaths;

static const ext_to_type extension_lookup[] =
{
	{ ".c", FILE_TYPE_C },
	{ ".h", FILE_TYPE_C },
	{ ".mak", FILE_TYPE_MAKE },
	{ "makefile", FILE_TYPE_MAKE },
	{ ".lay", FILE_TYPE_XML },
	{ ".txt", FILE_TYPE_TEXT },
};


static const token_entry dummy_token_table[] =
{
	{ NULL, KEYWORD_CLASS }
};


static const token_entry c_token_table[] =
{
	{ "#define", PREPROC_CLASS },
	{ "#elif", PREPROC_CLASS },
	{ "#else", PREPROC_CLASS },
	{ "#endif", PREPROC_CLASS },
	{ "#error", PREPROC_CLASS },
	{ "#if", PREPROC_CLASS },
	{ "#ifdef", PREPROC_CLASS },
	{ "#ifndef", PREPROC_CLASS },
	{ "#include", PREPROC_CLASS },
	{ "#line", PREPROC_CLASS },
	{ "#pragma", PREPROC_CLASS },
	{ "#undef", PREPROC_CLASS },

	{ "auto", KEYWORD_CLASS },
	{ "break", KEYWORD_CLASS },
	{ "case", KEYWORD_CLASS },
	{ "char", KEYWORD_CLASS },
	{ "const", KEYWORD_CLASS },
	{ "continue", KEYWORD_CLASS },
	{ "default", KEYWORD_CLASS },
	{ "do", KEYWORD_CLASS },
	{ "double", KEYWORD_CLASS },
	{ "else", KEYWORD_CLASS },
	{ "enum", KEYWORD_CLASS },
	{ "extern", KEYWORD_CLASS },
	{ "float", KEYWORD_CLASS },
	{ "for", KEYWORD_CLASS },
	{ "goto", KEYWORD_CLASS },
	{ "if", KEYWORD_CLASS },
	{ "int", KEYWORD_CLASS },
	{ "long", KEYWORD_CLASS },
	{ "register", KEYWORD_CLASS },
	{ "return", KEYWORD_CLASS },
	{ "short", KEYWORD_CLASS },
	{ "signed", KEYWORD_CLASS },
	{ "sizeof", KEYWORD_CLASS },
	{ "static", KEYWORD_CLASS },
	{ "struct", KEYWORD_CLASS },
	{ "switch", KEYWORD_CLASS },
	{ "typedef", KEYWORD_CLASS },
	{ "union", KEYWORD_CLASS },
	{ "unsigned", KEYWORD_CLASS },
	{ "void", KEYWORD_CLASS },
	{ "volatile", KEYWORD_CLASS },
	{ "while", KEYWORD_CLASS },

/*
    { "INLINE", CUSTOM_CLASS },
    { "INT8", CUSTOM_CLASS },
    { "UINT8", CUSTOM_CLASS },
    { "INT16", CUSTOM_CLASS },
    { "UINT16", CUSTOM_CLASS },
    { "INT32", CUSTOM_CLASS },
    { "UINT32", CUSTOM_CLASS },
    { "INT64", CUSTOM_CLASS },
    { "UINT64", CUSTOM_CLASS },
    { "ARRAY_LENGTH", CUSTOM_CLASS },
*/
	{ NULL, KEYWORD_CLASS }
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* core output functions */
static int recurse_dir(int srcrootlen, int dstrootlen, const astring *srcdir, const astring *dstdir, const astring *tempheader, const astring *tempfooter);
static int output_file(file_type type, int srcrootlen, int dstrootlen, const astring *srcfile, const astring *dstfile, int link_to_file, const astring *tempheader, const astring *tempfooter);

/* HTML helpers */
static core_file *create_file_and_output_header(const astring *filename, const astring *templatefile, const astring *path);
static void output_footer_and_close_file(core_file *file, const astring *templatefile, const astring *path);

/* path helpers */
static const astring *normalized_subpath(const astring *path, int start);
static void output_path_as_links(core_file *file, const astring *path, int end_is_directory, int link_to_file);
static astring *find_include_file(int srcrootlen, int dstrootlen, const astring *srcfile, const astring *dstfile, const astring *filename);



/***************************************************************************
    MAIN
***************************************************************************/

/*-------------------------------------------------
    main - main entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	astring *srcdir = NULL, *dstdir = NULL, *tempfilename = NULL, *tempheader = NULL, *tempfooter = NULL;
	int unadorned = 0;
	UINT32 bufsize;
	void *buffer;
	int result;
	int argnum;
	include_path **incpathhead = &incpaths;

	/* loop over arguments */
	for (argnum = 1; argnum < argc; argnum++)
	{
		char *arg = argv[argnum];

		/* include path? */
		if (arg[0] == '-' && arg[1] == 'I')
		{
			*incpathhead = (include_path *)malloc(sizeof(**incpathhead));
			if (*incpathhead != NULL)
			{
				(*incpathhead)->next = NULL;
				(*incpathhead)->path = astring_replacechr(astring_dupc(&arg[2]), '/', PATH_SEPARATOR[0]);
				incpathhead = &(*incpathhead)->next;
			}
		}

		/* other parameter */
		else if (arg[0] != '-' && unadorned == 0)
		{
			srcdir = astring_dupc(arg);
			unadorned++;
		}
		else if (arg[0] != '-' && unadorned == 1)
		{
			dstdir = astring_dupc(arg);
			unadorned++;
		}
		else if (arg[0] != '-' && unadorned == 2)
		{
			tempfilename = astring_dupc(arg);
			unadorned++;
		}
		else
			goto usage;
	}

	/* make sure we got 3 parameters */
	if (srcdir == NULL || dstdir == NULL || tempfilename == NULL)
		goto usage;

	/* read the template file into an astring */
	if (core_fload(astring_c(tempfilename), &buffer, &bufsize) == FILERR_NONE)
	{
		tempheader = astring_dupch((const char *)buffer, bufsize);
		free(buffer);
	}

	/* verify the template */
	if (tempheader == NULL)
	{
		fprintf(stderr, "Unable to read template file\n");
		return 1;
	}
	result = astring_findc(tempheader, 0, "<!--CONTENT-->");
	if (result == -1)
	{
		fprintf(stderr, "Template is missing a <!--CONTENT--> marker\n");
		return 1;
	}
	tempfooter = astring_substr(astring_dup(tempheader), result + 14, -1);
	tempheader = astring_substr(tempheader, 0, result);

	/* recurse over subdirectories */
	result = recurse_dir(astring_len(srcdir), astring_len(dstdir), srcdir, dstdir, tempheader, tempfooter);

	/* free source and destination directories */
	astring_free(srcdir);
	astring_free(dstdir);
	astring_free(tempfilename);
	astring_free(tempheader);
	astring_free(tempfooter);
	return result;

usage:
	fprintf(stderr, "Usage:\n%s <srcroot> <destroot> <template.html> [-Iincpath [-Iincpath [...]]]\n", argv[0]);
	return 1;
}



/***************************************************************************
    CORE OUTPUT FUNCTIONS
***************************************************************************/

static int compare_list_entries(const void *p1, const void *p2)
{
	const list_entry *entry1 = *(const list_entry **)p1;
	const list_entry *entry2 = *(const list_entry **)p2;
	return strcmp(astring_c(entry1->name), astring_c(entry2->name));
}


/*-------------------------------------------------
    recurse_dir - recurse through a directory
-------------------------------------------------*/

static int recurse_dir(int srcrootlen, int dstrootlen, const astring *srcdir, const astring *dstdir, const astring *tempheader, const astring *tempfooter)
{
	static const osd_dir_entry_type typelist[] = { ENTTYPE_DIR, ENTTYPE_FILE };
	const astring *srcdir_subpath;
	core_file *indexfile = NULL;
	astring *indexname;
	int result = 0;
	int entindex;

	/* extract a normalized subpath */
	srcdir_subpath = normalized_subpath(srcdir, srcrootlen + 1);
	if (srcdir_subpath == NULL)
		return 1;

	/* create an index file */
	indexname = astring_alloc();
	astring_printf(indexname, "%s%c%s", astring_c(dstdir), PATH_SEPARATOR[0], "index.html");
	indexfile = create_file_and_output_header(indexname, tempheader, srcdir_subpath);
	astring_free(indexname);

	/* output the directory navigation */
	core_fprintf(indexfile, "<h3>Viewing Directory: ");
	output_path_as_links(indexfile, srcdir_subpath, TRUE, FALSE);
	core_fprintf(indexfile, "</h3>");

	/* iterate first over directories, then over files */
	for (entindex = 0; entindex < ARRAY_LENGTH(typelist) && result == 0; entindex++)
	{
		osd_dir_entry_type entry_type = typelist[entindex];
		const osd_directory_entry *entry;
		list_entry **listarray = NULL;
		list_entry *list = NULL;
		list_entry *curlist;
		osd_directory *dir;
		int found = 0;

		/* open the directory and iterate through it */
		dir = osd_opendir(astring_c(srcdir));
		if (dir == NULL)
		{
			result = 1;
			goto error;
		}

		/* build up the list of files */
		while ((entry = osd_readdir(dir)) != NULL)
			if (entry->type == entry_type && entry->name[0] != '.')
			{
				list_entry *lentry = (list_entry *)malloc(sizeof(*lentry));
				lentry->name = astring_dupc(entry->name);
				lentry->next = list;
				list = lentry;
				found++;
			}

		/* close the directory */
		osd_closedir(dir);

		/* skip if nothing found */
		if (found == 0)
			continue;

		/* allocate memory for sorting */
		listarray = (list_entry **)malloc(sizeof(list_entry *) * found);
		found = 0;
		for (curlist = list; curlist != NULL; curlist = curlist->next)
			listarray[found++] = curlist;

		/* sort the list */
		qsort(listarray, found, sizeof(listarray[0]), compare_list_entries);

		/* rebuild the list */
		list = NULL;
		while (--found >= 0)
		{
			listarray[found]->next = list;
			list = listarray[found];
		}
		free(listarray);

		/* iterate through each file */
		for (curlist = list; curlist != NULL && result == 0; curlist = curlist->next)
		{
			astring *srcfile, *dstfile;

			/* add a header */
			if (curlist == list)
				core_fprintf(indexfile, "\t<h2>%s</h2>\n\t<ul>\n", (entry_type == ENTTYPE_DIR) ? "Directories" : "Files");

			/* build the source filename */
			srcfile = astring_alloc();
			astring_printf(srcfile, "%s%c%s", astring_c(srcdir), PATH_SEPARATOR[0], astring_c(curlist->name));

			/* if we have a file, output it */
			dstfile = astring_alloc();
			if (entry_type == ENTTYPE_FILE)
			{
				file_type type = FILE_TYPE_INVALID;
				int extnum;

				/* make sure we care, first */
				for (extnum = 0; extnum < ARRAY_LENGTH(extension_lookup); extnum++)
					if (core_filename_ends_with(astring_c(curlist->name), extension_lookup[extnum].extension))
					{
						type = extension_lookup[extnum].type;
						break;
					}

				/* if we got a valid file, process it */
				if (type != FILE_TYPE_INVALID)
				{
					astring_printf(dstfile, "%s%c%s.html", astring_c(dstdir), PATH_SEPARATOR[0], astring_c(curlist->name));
					if (indexfile != NULL)
						core_fprintf(indexfile, "\t<li><a href=\"%s.html\">%s</a></li>\n", astring_c(curlist->name), astring_c(curlist->name));
					result = output_file(type, srcrootlen, dstrootlen, srcfile, dstfile, astring_cmp(srcdir, dstdir) == 0, tempheader, tempfooter);
				}
			}

			/* if we have a directory, recurse */
			else
			{
				astring_printf(dstfile, "%s%c%s", astring_c(dstdir), PATH_SEPARATOR[0], astring_c(curlist->name));
				if (indexfile != NULL)
					core_fprintf(indexfile, "\t<li><a href=\"%s/index.html\">%s/</a></li>\n", astring_c(curlist->name), astring_c(curlist->name));
				result = recurse_dir(srcrootlen, dstrootlen, srcfile, dstfile, tempheader, tempfooter);
			}

			/* free memory for the names */
			astring_free(srcfile);
			astring_free(dstfile);
		}

		/* close the list if we found some stuff */
		if (list != NULL)
			core_fprintf(indexfile, "\t</ul>\n");

		/* free all the allocated entries */
		while (list != NULL)
		{
			list_entry *next = list->next;
			astring_free((astring *)list->name);
			free(list);
			list = next;
		}
	}

error:
	if (indexfile != NULL)
		output_footer_and_close_file(indexfile, tempfooter, srcdir_subpath);
	astring_free((astring *)srcdir_subpath);
	return result;
}


/*-------------------------------------------------
    output_file - output a file, converting to
    HTML
-------------------------------------------------*/

static int output_file(file_type type, int srcrootlen, int dstrootlen, const astring *srcfile, const astring *dstfile, int link_to_file, const astring *tempheader, const astring *tempfooter)
{
	const char *comment_start, *comment_end, *comment_inline, *token_chars;
	const char *comment_start_esc, *comment_end_esc, *comment_inline_esc;
	const token_entry *token_table;
	const astring *srcfile_subpath;
	char srcline[4096], *srcptr;
	int in_comment = FALSE;
	UINT8 is_token[256];
	int color_quotes;
	core_file *src;
	core_file *dst;
	int toknum;
	int linenum = 1;

	/* extract a normalized subpath */
	srcfile_subpath = normalized_subpath(srcfile, srcrootlen + 1);
	if (srcfile_subpath == NULL)
		return 1;

	fprintf(stderr, "Processing %s\n", astring_c(srcfile_subpath));

	/* set some defaults */
	color_quotes = FALSE;
	comment_start = comment_start_esc = "";
	comment_end = comment_end_esc = "";
	comment_inline = comment_inline_esc = "";
	token_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_#";
	token_table = dummy_token_table;

	/* based on the file type, set the comment info */
	switch (type)
	{
		case FILE_TYPE_C:
			color_quotes = TRUE;
			comment_start = comment_start_esc = "/*";
			comment_end = comment_end_esc = "*/";
			comment_inline = comment_inline_esc = "//";
			token_table = c_token_table;
			break;

		case FILE_TYPE_MAKE:
			color_quotes = TRUE;
			comment_inline = comment_inline_esc = "#";
			break;

		case FILE_TYPE_XML:
			color_quotes = TRUE;
			comment_start = "<!--";
			comment_start_esc = "&lt;!--";
			comment_end = "-->";
			comment_end_esc = "--&gt;";
			break;

		default:
		case FILE_TYPE_TEXT:
			break;
	}

	/* make the token lookup table */
	memset(is_token, 0, sizeof(is_token));
	for (toknum = 0; token_chars[toknum] != 0; toknum++)
		is_token[(UINT8)token_chars[toknum]] = TRUE;

	/* open the source file */
	if (core_fopen(astring_c(srcfile), OPEN_FLAG_READ, &src) != FILERR_NONE)
	{
		fprintf(stderr, "Unable to read file '%s'\n", astring_c(srcfile));
		return 1;
	}

	/* open the output file */
	dst = create_file_and_output_header(dstfile, tempheader, srcfile_subpath);
	if (dst == NULL)
	{
		fprintf(stderr, "Unable to write file '%s'\n", astring_c(dstfile));
		core_fclose(src);
		return 1;
	}

	/* output the directory navigation */
	core_fprintf(dst, "<h3>Viewing File: ");
	output_path_as_links(dst, srcfile_subpath, FALSE, link_to_file);
	core_fprintf(dst, "</h3>");

	/* start with some tags */
	core_fprintf(dst, "\t<pre class=\"source\">\n");

	/* iterate over lines in the source file */
	while (core_fgets(srcline, ARRAY_LENGTH(srcline), src) != NULL)
	{
		char dstline[4096], *dstptr = dstline;
		int in_inline_comment = FALSE;
		int last_token_was_include = FALSE;
		int last_was_token = FALSE;
		int quotes_are_linked = FALSE;
		char in_quotes = 0;
		int curcol = 0;

		/* start with the line number */
		dstptr += sprintf(dstptr, "<span class=\"linenum\">%5d</span>&nbsp;&nbsp;", linenum++);

		/* iterate over characters in the source line */
		for (srcptr = srcline; *srcptr != 0; )
		{
			UINT8 ch = *srcptr++;

			/* track whether or not we are within an extended (C-style) comment */
			if (!in_quotes && !in_inline_comment)
			{
				if (!in_comment && ch == comment_start[0] && strncmp(srcptr - 1, comment_start, strlen(comment_start)) == 0)
				{
					dstptr += sprintf(dstptr, "<span class=\"comment\">%s", comment_start_esc);
					curcol += strlen(comment_start);
					srcptr += strlen(comment_start) - 1;
					ch = 0;
					in_comment = TRUE;
				}
				else if (in_comment && ch == comment_end[0] && strncmp(srcptr - 1, comment_end, strlen(comment_end)) == 0)
				{
					dstptr += sprintf(dstptr, "%s</span>", comment_end_esc);
					curcol += strlen(comment_end);
					srcptr += strlen(comment_end) - 1;
					ch = 0;
					in_comment = FALSE;
				}
			}

			/* track whether or not we are within an inline (C++-style) comment */
			if (!in_quotes && !in_comment && !in_inline_comment && ch == comment_inline[0] && strncmp(srcptr - 1, comment_inline, strlen(comment_inline)) == 0)
			{
				dstptr += sprintf(dstptr, "<span class=\"comment\">%s", comment_inline_esc);
				curcol += strlen(comment_inline);
				srcptr += strlen(comment_inline) - 1;
				ch = 0;
				in_inline_comment = TRUE;
			}

			/* if this is the start of a new token, see if we want to color it */
			if (!in_quotes && !in_comment && !in_inline_comment && !last_was_token && is_token[ch])
			{
				const token_entry *curtoken;
				char *temp = srcptr;
				int toklength;

				/* find the end of the token */
				while (*temp != 0 && is_token[(UINT8)*temp])
					temp++;
				toklength = temp - (srcptr - 1);

				/* scan the token table */
				last_token_was_include = FALSE;
				for (curtoken = token_table; curtoken->token != NULL; curtoken++)
					if (strncmp(srcptr - 1, curtoken->token, toklength) == 0 && strlen(curtoken->token) == toklength)
					{
						dstptr += sprintf(dstptr, "<span class=\"%s\">%s</span>", curtoken->color, curtoken->token);
						curcol += strlen(curtoken->token);
						srcptr += strlen(curtoken->token) - 1;
						ch = 0;

						/* look for include tokens specially */
						if (type == FILE_TYPE_C && strcmp(curtoken->token, "#include") == 0)
							last_token_was_include = TRUE;
						break;
					}
			}
			last_was_token = is_token[ch];

			/* if we hit a tab, expand it */
			if (ch == 0x09)
			{
				/* compute how many spaces */
				int spaces = 4 - curcol % 4;
				while (spaces--)
				{
					*dstptr++ = ' ';
					curcol++;
				}
			}

			/* otherwise, copy the source character */
			else if (ch != 0x0a && ch != 0x0d && ch != 0)
			{
				/* track opening quotes */
				if (!in_comment && !in_inline_comment && !in_quotes && (ch == '"' || ch == '\''))
				{
					if (color_quotes)
						dstptr += sprintf(dstptr, "<span class=\"string\">%c", ch);
					else
						*dstptr++ = ch;
					in_quotes = ch;

					/* handle includes */
					if (last_token_was_include)
					{
						char *endquote = strchr(srcptr, ch);
						if (endquote != NULL)
						{
							astring *filename = astring_dupch(srcptr, endquote - srcptr);
							astring *target = find_include_file(srcrootlen, dstrootlen, srcfile, dstfile, filename);
							if (target != NULL)
							{
								dstptr += sprintf(dstptr, "<a href=\"%s\">", astring_c(target));
								quotes_are_linked = TRUE;
								astring_free(target);
							}
							astring_free(filename);
						}
					}
				}

				/* track closing quotes */
				else if (!in_comment && !in_inline_comment && in_quotes && ch == in_quotes && (type != FILE_TYPE_C || srcptr[-2] != '\\' || srcptr[-3] == '\\'))
				{
					if (quotes_are_linked)
						dstptr += sprintf(dstptr, "</a>");
					if (color_quotes)
						dstptr += sprintf(dstptr, "%c</span>", ch);
					else
						*dstptr++ = ch;
					in_quotes = 0;
					quotes_are_linked = FALSE;
				}

				/* else just output the current character */
				else if (ch == '&')
					dstptr += sprintf(dstptr, "&amp;");
				else if (ch == '<')
					dstptr += sprintf(dstptr, "&lt;");
				else if (ch == '>')
					dstptr += sprintf(dstptr, "&gt;");
				else
					*dstptr++ = ch;
				curcol++;
			}
		}

		/* finish inline comments */
		if (in_inline_comment)
		{
			dstptr += sprintf(dstptr, "</span>");
			in_inline_comment = FALSE;
		}

		/* append a break and move on */
		dstptr += sprintf(dstptr, "\n");
		core_fputs(dst, dstline);
	}

	/* close tags */
	core_fprintf(dst, "\t</pre>\n");

	/* close the file */
	output_footer_and_close_file(dst, tempfooter, srcfile_subpath);
	astring_free((astring *)srcfile_subpath);
	core_fclose(src);
	return 0;
}



/***************************************************************************
    HTML OUTPUT HELPERS
***************************************************************************/

/*-------------------------------------------------
    create_file_and_output_header - create a new
    HTML file with a standard header
-------------------------------------------------*/

static core_file *create_file_and_output_header(const astring *filename, const astring *templatefile, const astring *path)
{
	astring *modified;
	core_file *file;

	/* create the indexfile */
	if (core_fopen(astring_c(filename), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS | OPEN_FLAG_NO_BOM, &file) != FILERR_NONE)
		return NULL;

	/* print a header */
	modified = astring_dup(templatefile);
	astring_replacec(modified, 0, "<!--PATH-->", astring_c(path));
	core_fwrite(file, astring_c(modified), astring_len(modified));

	/* return the file */
	astring_free(modified);
	return file;
}


/*-------------------------------------------------
    output_footer_and_close_file - write a
    standard footer to an HTML file and close it
-------------------------------------------------*/

static void output_footer_and_close_file(core_file *file, const astring *templatefile, const astring *path)
{
	astring *modified;

	modified = astring_dup(templatefile);
	astring_replacec(modified, 0, "<!--PATH-->", astring_c(path));
	core_fwrite(file, astring_c(modified), astring_len(modified));
	astring_free(modified);
	core_fclose(file);
}



/***************************************************************************
    HTML OUTPUT HELPERS
***************************************************************************/

/*-------------------------------------------------
    normalized_subpath - normalize a path to
    forward slashes and extract a subpath
-------------------------------------------------*/

static const astring *normalized_subpath(const astring *path, int start)
{
	astring *result = astring_dupsubstr(path, start, -1);
	if (result != NULL)
		astring_replacechr(result, PATH_SEPARATOR[0], '/');
	return result;
}


/*-------------------------------------------------
    output_path_as_links - output a path as a
    series of links
-------------------------------------------------*/

static void output_path_as_links(core_file *file, const astring *path, int end_is_directory, int link_to_file)
{
	astring *substr = astring_alloc();
	int srcdepth, curdepth, depth;
	int slashindex, lastslash;

	/* first count how deep we are */
	srcdepth = 0;
	for (slashindex = astring_chr(path, 0, '/'); slashindex != -1; slashindex = astring_chr(path, slashindex + 1, '/'))
		srcdepth++;
	if (end_is_directory)
		srcdepth++;

	/* output a link to the root */
	core_fprintf(file, "<a href=\"");
	for (depth = 0; depth < srcdepth; depth++)
		core_fprintf(file, "../");
	core_fprintf(file, "index.html\">&lt;root&gt;</a>/");

	/* now output links to each path up the chain */
	curdepth = 0;
	lastslash = 0;
	for (slashindex = astring_chr(path, lastslash, '/'); slashindex != -1; slashindex = astring_chr(path, lastslash, '/'))
	{
		astring_cpysubstr(substr, path, lastslash, slashindex - lastslash);

		curdepth++;
		core_fprintf(file, "<a href=\"");
		for (depth = curdepth; depth < srcdepth; depth++)
			core_fprintf(file, "../");
		core_fprintf(file, "index.html\">%s</a>/", astring_c(substr));

		lastslash = slashindex + 1;
	}

	/* and a final link to the current directory */
	astring_cpysubstr(substr, path, lastslash, -1);
	if (end_is_directory)
		core_fprintf(file, "<a href=\"index.html\">%s</a>", astring_c(substr));
	else if (link_to_file)
		core_fprintf(file, "<a href=\"%s\">%s</a>", astring_c(substr), astring_c(substr));
	else
		core_fprintf(file, "<a href=\"%s.html\">%s</a>", astring_c(substr), astring_c(substr));

	astring_free(substr);
}


/*-------------------------------------------------
    find_include_file - find an include file
-------------------------------------------------*/

static astring *find_include_file(int srcrootlen, int dstrootlen, const astring *srcfile, const astring *dstfile, const astring *filename)
{
	include_path *curpath;

	/* iterate over include paths and find the file */
	for (curpath = incpaths; curpath != NULL; curpath = curpath->next)
	{
		astring *srcincpath = astring_cat(astring_dupsubstr(srcfile, 0, srcrootlen + 1), curpath->path);
		core_file *testfile;
		int lastsepindex = 0;
		int sepindex;

		/* a '.' include path is specially treated */
		if (astring_cmpc(curpath->path, ".") == 0)
			astring_cpysubstr(srcincpath, srcfile, 0, astring_rchr(srcfile, 0, PATH_SEPARATOR[0]));

		/* append the filename piecemeal to account for directories */
		while ((sepindex = astring_chr(filename, lastsepindex, '/')) != -1)
		{
			astring *pathpart = astring_dupsubstr(filename, lastsepindex, sepindex - lastsepindex);

			/* handle .. by removing a chunk from the incpath */
			if (astring_cmpc(pathpart, "..") == 0)
			{
				sepindex = astring_rchr(srcincpath, 0, PATH_SEPARATOR[0]);
				if (sepindex != -1)
					astring_substr(srcincpath, 0, sepindex);
			}

			/* otherwise, append a path separator and the pathpart */
			else
				astring_cat(astring_catc(srcincpath, PATH_SEPARATOR), pathpart);

			/* advance past the previous index */
			lastsepindex = sepindex + 1;

			/* free the path part we extracted */
			astring_free(pathpart);
		}

		/* now append the filename */
		astring_catsubstr(astring_catc(srcincpath, PATH_SEPARATOR), filename, lastsepindex, -1);

		/* see if we can open it */
		if (core_fopen(astring_c(srcincpath), OPEN_FLAG_READ, &testfile) == FILERR_NONE)
		{
			astring *tempfile = astring_alloc();
			astring *tempinc = astring_alloc();

			/* close the file */
			core_fclose(testfile);

			/* find the longest matching directory substring between the include and source file */
			lastsepindex = 0;
			while ((sepindex = astring_chr(srcincpath, lastsepindex, PATH_SEPARATOR[0])) != -1)
			{
				/* get substrings up to the current directory */
				astring_cpysubstr(tempfile, srcfile, 0, sepindex);
				astring_cpysubstr(tempinc, srcincpath, 0, sepindex);

				/* if we don't match, stop */
				if (astring_cmp(tempfile, tempinc) != 0)
					break;
				lastsepindex = sepindex + 1;
			}

			/* chop off the common parts of the paths */
			astring_cpysubstr(tempfile, srcfile, lastsepindex, -1);
			astring_replacechr(astring_substr(srcincpath, lastsepindex, -1), PATH_SEPARATOR[0], '/');

			/* for each directory left in the filename, we need to prepend a "../" */
			while ((sepindex = astring_chr(tempfile, 0, PATH_SEPARATOR[0])) != -1)
			{
				astring_substr(tempfile, sepindex + 1, -1);
				astring_insc(srcincpath, 0, "../");
			}
			astring_catc(srcincpath, ".html");

			/* free the strings and return the include path */
			astring_free(tempfile);
			astring_free(tempinc);
			return srcincpath;
		}

		/* free our include path */
		astring_free(srcincpath);
	}
	return NULL;
}
