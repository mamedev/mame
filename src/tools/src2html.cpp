// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    MAME source code to HTML converter

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
#include <assert.h>
#include "osdcore.h"
#include "corestr.h"
#include "corefile.h"


/***************************************************************************
    CONSTANTS & DEFINES
***************************************************************************/

#define PREPROC_CLASS       "preproc"
#define KEYWORD_CLASS       "keyword"
#define CUSTOM_CLASS        "custom"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

enum file_type
{
	FILE_TYPE_INVALID,
	FILE_TYPE_C,
	FILE_TYPE_MAKE,
	FILE_TYPE_XML,
	FILE_TYPE_TEXT
};


struct ext_to_type
{
	const char *    extension;
	file_type       type;
};


struct token_entry
{
	const char *    token;
	const char *    color;
};


struct include_path
{
	include_path *  next;
	std::string     path;
};


struct list_entry
{
	list_entry *    next;
	std::string     name;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static include_path *incpaths;

static const ext_to_type extension_lookup[] =
{
	{ ".c", FILE_TYPE_C },
	{ ".h", FILE_TYPE_C },
	{ ".lst", FILE_TYPE_C },
	{ ".mak", FILE_TYPE_MAKE },
	{ "makefile", FILE_TYPE_MAKE },
	{ ".lay", FILE_TYPE_XML },
	{ ".xml", FILE_TYPE_XML },
	{ ".txt", FILE_TYPE_TEXT },
};


static const token_entry dummy_token_table[] =
{
	{ nullptr, KEYWORD_CLASS }
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
	{ "bool", KEYWORD_CLASS },
	{ "break", KEYWORD_CLASS },
	{ "case", KEYWORD_CLASS },
	{ "catch", KEYWORD_CLASS },
	{ "char", KEYWORD_CLASS },
	{ "class", KEYWORD_CLASS },
	{ "const", KEYWORD_CLASS },
	{ "const_cast", KEYWORD_CLASS },
	{ "continue", KEYWORD_CLASS },
	{ "default", KEYWORD_CLASS },
	{ "delete", KEYWORD_CLASS },
	{ "do", KEYWORD_CLASS },
	{ "double", KEYWORD_CLASS },
	{ "dynamic_cast", KEYWORD_CLASS },
	{ "else", KEYWORD_CLASS },
	{ "enum", KEYWORD_CLASS },
	{ "explicit", KEYWORD_CLASS },
	{ "export", KEYWORD_CLASS },
	{ "extern", KEYWORD_CLASS },
	{ "false", KEYWORD_CLASS },
	{ "float", KEYWORD_CLASS },
	{ "for", KEYWORD_CLASS },
	{ "friend", KEYWORD_CLASS },
	{ "goto", KEYWORD_CLASS },
	{ "if", KEYWORD_CLASS },
	{ "inline", KEYWORD_CLASS },
	{ "int", KEYWORD_CLASS },
	{ "long", KEYWORD_CLASS },
	{ "mutable", KEYWORD_CLASS },
	{ "namespace", KEYWORD_CLASS },
	{ "new", KEYWORD_CLASS },
	{ "operator", KEYWORD_CLASS },
	{ "private", KEYWORD_CLASS },
	{ "protected", KEYWORD_CLASS },
	{ "public", KEYWORD_CLASS },
	{ "register", KEYWORD_CLASS },
	{ "reinterpret_cast", KEYWORD_CLASS },
	{ "return", KEYWORD_CLASS },
	{ "short", KEYWORD_CLASS },
	{ "signed", KEYWORD_CLASS },
	{ "sizeof", KEYWORD_CLASS },
	{ "static", KEYWORD_CLASS },
	{ "static_cast", KEYWORD_CLASS },
	{ "struct", KEYWORD_CLASS },
	{ "switch", KEYWORD_CLASS },
	{ "template", KEYWORD_CLASS },
	{ "this", KEYWORD_CLASS },
	{ "throw", KEYWORD_CLASS },
	{ "true", KEYWORD_CLASS },
	{ "try", KEYWORD_CLASS },
	{ "typedef", KEYWORD_CLASS },
	{ "typeid", KEYWORD_CLASS },
	{ "typename", KEYWORD_CLASS },
	{ "union", KEYWORD_CLASS },
	{ "unsigned", KEYWORD_CLASS },
	{ "virtual", KEYWORD_CLASS },
	{ "void", KEYWORD_CLASS },
	{ "volatile", KEYWORD_CLASS },
	{ "wchar_t", KEYWORD_CLASS },
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
	{ nullptr, KEYWORD_CLASS }
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

// core output functions
static int recurse_dir(int srcrootlen, int dstrootlen, std::string &srcdir, std::string &dstdir, std::string &tempheader, std::string &tempfooter);
static int output_file(file_type type, int srcrootlen, int dstrootlen, std::string &srcfile, std::string &dstfile, bool link_to_file, std::string &tempheader, std::string &tempfooter);

// HTML helpers
static core_file *create_file_and_output_header(std::string &filename, std::string &templatefile, std::string &path);
static void output_footer_and_close_file(core_file *file, std::string &templatefile, std::string &path);

// path helpers
static std::string &normalized_subpath(std::string &dest, std::string &path, int start);
static void output_path_as_links(core_file *file, std::string &path, bool end_is_directory, bool link_to_file);
static bool find_include_file(std::string &srcincpath, int srcrootlen, int dstrootlen, std::string &srcfile, std::string &dstfile, std::string &filename);



/***************************************************************************
    MAIN
***************************************************************************/

/*-------------------------------------------------
    main - main entry point
-------------------------------------------------*/

void usage(const char *argv0)
{
	fprintf(stderr, "Usage:\n%s <srcroot> <destroot> <template.html> [-Iincpath [-Iincpath [...]]]\n", argv0);
	exit(1);
}

int main(int argc, char *argv[])
{
	// loop over arguments
	include_path **incpathhead = &incpaths;
	std::string srcdir, dstdir, tempfilename, tempheader, tempfooter;
	int unadorned = 0;
	for (int argnum = 1; argnum < argc; argnum++)
	{
		char *arg = argv[argnum];

		// include path?
		if (arg[0] == '-' && arg[1] == 'I')
		{
			*incpathhead = new include_path;
			if (*incpathhead != nullptr)
			{
				(*incpathhead)->next = nullptr;
				strreplacechr((*incpathhead)->path.assign(&arg[2]), '/', PATH_SEPARATOR[0]);
				incpathhead = &(*incpathhead)->next;
			}
		}

		// other parameter
		else if (arg[0] != '-' && unadorned == 0)
		{
			srcdir.assign(arg);
			unadorned++;
		}
		else if (arg[0] != '-' && unadorned == 1)
		{
			dstdir.assign(arg);
			unadorned++;
		}
		else if (arg[0] != '-' && unadorned == 2)
		{
			tempfilename.assign(arg);
			unadorned++;
		}
		else
			usage(argv[0]);
	}

	// make sure we got 3 parameters
	if (srcdir.length() == 0 || dstdir.length() == 0 || tempfilename.length() == 0)
		usage(argv[0]);

	// read the template file into an std::string
	UINT32 bufsize;
	void *buffer;
	if (core_fload(tempfilename.c_str(), &buffer, &bufsize) == FILERR_NONE)
	{
		tempheader.assign((const char *)buffer, bufsize);
		osd_free(buffer);
	}

	// verify the template
	if (tempheader.length() == 0)
	{
		fprintf(stderr, "Unable to read template file\n");
		return 1;
	}
	int result = tempheader.find("<!--CONTENT-->");
	if (result == -1)
	{
		fprintf(stderr, "Template is missing a <!--CONTENT--> marker\n");
		return 1;
	}
	tempfooter.assign(tempheader).substr(result + 14, -1);
	tempheader.substr(0, result);

	// recurse over subdirectories
	return recurse_dir(srcdir.length(), dstdir.length(), srcdir, dstdir, tempheader, tempfooter);
}



/***************************************************************************
    CORE OUTPUT FUNCTIONS
***************************************************************************/

static int compare_list_entries(const void *p1, const void *p2)
{
	const list_entry *entry1 = *(const list_entry **)p1;
	const list_entry *entry2 = *(const list_entry **)p2;
	return entry1->name.compare(entry2->name);
}


/*-------------------------------------------------
    recurse_dir - recurse through a directory
-------------------------------------------------*/

static int recurse_dir(int srcrootlen, int dstrootlen, std::string &srcdir, std::string &dstdir, std::string &tempheader, std::string &tempfooter)
{
	static const osd_dir_entry_type typelist[] = { ENTTYPE_DIR, ENTTYPE_FILE };

	// extract a normalized subpath
	std::string srcdir_subpath;
	normalized_subpath(srcdir_subpath, srcdir, srcrootlen + 1);

	// create an index file
	std::string indexname;
	strprintf(indexname,"%s%c%s", dstdir.c_str(), PATH_SEPARATOR[0], "index.html");
	core_file *indexfile = create_file_and_output_header(indexname, tempheader, srcdir_subpath);

	// output the directory navigation
	core_fprintf(indexfile, "<h3>Viewing Directory: ");
	output_path_as_links(indexfile, srcdir_subpath, true, false);
	core_fprintf(indexfile, "</h3>");

	// iterate first over directories, then over files
	int result = 0;
	for (int entindex = 0; entindex < ARRAY_LENGTH(typelist) && result == 0; entindex++)
	{
		osd_dir_entry_type entry_type = typelist[entindex];

		// open the directory and iterate through it
		osd_directory *dir = osd_opendir(srcdir.c_str());
		if (dir == nullptr)
		{
			result = 1;
			break;
		}

		// build up the list of files
		const osd_directory_entry *entry;
		int found = 0;
		list_entry *list = nullptr;
		while ((entry = osd_readdir(dir)) != nullptr)
			if (entry->type == entry_type && entry->name[0] != '.')
			{
				auto lentry = new list_entry;
				lentry->name.assign(entry->name);
				lentry->next = list;
				list = lentry;
				found++;
			}

		// close the directory
		osd_closedir(dir);

		// skip if nothing found
		if (found == 0)
			continue;

		// allocate memory for sorting
		auto listarray = new list_entry *[found];
		found = 0;
		for (list_entry *curlist = list; curlist != nullptr; curlist = curlist->next)
			listarray[found++] = curlist;

		// sort the list
		qsort(listarray, found, sizeof(listarray[0]), compare_list_entries);

		// rebuild the list
		list = nullptr;
		while (--found >= 0)
		{
			listarray[found]->next = list;
			list = listarray[found];
		}
		delete[] listarray;

		// iterate through each file
		for (list_entry *curlist = list; curlist != nullptr && result == 0; curlist = curlist->next)
		{
			// add a header
			if (curlist == list)
				core_fprintf(indexfile, "\t<h2>%s</h2>\n\t<ul>\n", (entry_type == ENTTYPE_DIR) ? "Directories" : "Files");

			// build the source filename
			std::string srcfile;
			strprintf(srcfile, "%s%c%s", srcdir.c_str(), PATH_SEPARATOR[0], curlist->name.c_str());

			// if we have a file, output it
			std::string dstfile;
			if (entry_type == ENTTYPE_FILE)
			{
				// make sure we care, first
				file_type type = FILE_TYPE_INVALID;
				for (int extnum = 0; extnum < ARRAY_LENGTH(extension_lookup); extnum++)
					if (core_filename_ends_with(curlist->name.c_str(), extension_lookup[extnum].extension))
					{
						type = extension_lookup[extnum].type;
						break;
					}

				// if we got a valid file, process it
				if (type != FILE_TYPE_INVALID)
				{
					strprintf(dstfile, "%s%c%s.html", dstdir.c_str(), PATH_SEPARATOR[0], curlist->name.c_str());
					if (indexfile != nullptr)
						core_fprintf(indexfile, "\t<li><a href=\"%s.html\">%s</a></li>\n", curlist->name.c_str(), curlist->name.c_str());
					result = output_file(type, srcrootlen, dstrootlen, srcfile, dstfile, srcdir.compare(dstdir) == 0, tempheader, tempfooter);
				}
			}

			// if we have a directory, recurse
			else
			{
				strprintf(dstfile, "%s%c%s", dstdir.c_str(), PATH_SEPARATOR[0], curlist->name.c_str());
				if (indexfile != nullptr)
					core_fprintf(indexfile, "\t<li><a href=\"%s/index.html\">%s/</a></li>\n", curlist->name.c_str(), curlist->name.c_str());
				result = recurse_dir(srcrootlen, dstrootlen, srcfile, dstfile, tempheader, tempfooter);
			}
		}

		// close the list if we found some stuff
		if (list != nullptr)
			core_fprintf(indexfile, "\t</ul>\n");

		// free all the allocated entries
		while (list != nullptr)
		{
			list_entry *next = list->next;
			delete list;
			list = next;
		}
	}

	if (indexfile != nullptr)
		output_footer_and_close_file(indexfile, tempfooter, srcdir_subpath);
	return result;
}


/*-------------------------------------------------
    output_file - output a file, converting to
    HTML
-------------------------------------------------*/

static int output_file(file_type type, int srcrootlen, int dstrootlen, std::string &srcfile, std::string &dstfile, bool link_to_file, std::string &tempheader, std::string &tempfooter)
{
	// extract a normalized subpath
	std::string srcfile_subpath;
	normalized_subpath(srcfile_subpath, srcfile, srcrootlen + 1);

	fprintf(stderr, "Processing %s\n", srcfile_subpath.c_str());

	// set some defaults
	bool color_quotes = false;
	const char *comment_start = "";
	const char *comment_start_esc = "";
	const char *comment_end = "";
	const char *comment_end_esc = "";
	const char *comment_inline = "";
	const char *comment_inline_esc = "";
	const char *token_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_#";
	const token_entry *token_table = dummy_token_table;

	// based on the file type, set the comment info
	switch (type)
	{
		case FILE_TYPE_C:
			color_quotes = true;
			comment_start = comment_start_esc = "/*";
			comment_end = comment_end_esc = "*/";
			comment_inline = comment_inline_esc = "//";
			token_table = c_token_table;
			break;

		case FILE_TYPE_MAKE:
			color_quotes = true;
			comment_inline = comment_inline_esc = "#";
			break;

		case FILE_TYPE_XML:
			color_quotes = true;
			comment_start = "<!--";
			comment_start_esc = "&lt;!--";
			comment_end = "-->";
			comment_end_esc = "--&gt;";
			break;

		default:
		case FILE_TYPE_TEXT:
			break;
	}

	// make the token lookup table
	bool is_token[256];
	memset(is_token, 0, sizeof(is_token));
	for (int toknum = 0; token_chars[toknum] != 0; toknum++)
		is_token[(UINT8)token_chars[toknum]] = true;

	// open the source file
	core_file *src;
	if (core_fopen(srcfile.c_str(), OPEN_FLAG_READ, &src) != FILERR_NONE)
	{
		fprintf(stderr, "Unable to read file '%s'\n", srcfile.c_str());
		return 1;
	}

	// open the output file
	core_file *dst = create_file_and_output_header(dstfile, tempheader, srcfile_subpath);
	if (dst == nullptr)
	{
		fprintf(stderr, "Unable to write file '%s'\n", dstfile.c_str());
		core_fclose(src);
		return 1;
	}

	// output the directory navigation
	core_fprintf(dst, "<h3>Viewing File: ");
	output_path_as_links(dst, srcfile_subpath, false, link_to_file);
	core_fprintf(dst, "</h3>");

	// start with some tags
	core_fprintf(dst, "\t<pre class=\"source\">\n");

	// iterate over lines in the source file
	int linenum = 1;
	bool in_comment = false;
	char srcline[4096];
	while (core_fgets(srcline, ARRAY_LENGTH(srcline), src) != nullptr)
	{
		// start with the line number
		std::string dstline;
		strcatprintf(dstline, "<span class=\"linenum\">%5d</span>&nbsp;&nbsp;", linenum++);

		// iterate over characters in the source line
		bool escape = false;
		bool in_quotes = false;
		bool in_inline_comment = false;
		bool last_token_was_include = false;
		bool last_was_token = false;
		bool quotes_are_linked = false;
		UINT8 curquote = 0;
		int curcol = 0;
		for (char *srcptr = srcline; *srcptr != 0; )
		{
			UINT8 ch = *srcptr++;

			// track whether or not we are within an extended (C-style) comment
			if (!in_quotes && !in_inline_comment)
			{
				if (!in_comment && ch == comment_start[0] && strncmp(srcptr - 1, comment_start, strlen(comment_start)) == 0)
				{
					strcatprintf(dstline, "<span class=\"comment\">%s", comment_start_esc);
					curcol += strlen(comment_start);
					srcptr += strlen(comment_start) - 1;
					ch = 0;
					in_comment = true;
				}
				else if (in_comment && ch == comment_end[0] && strncmp(srcptr - 1, comment_end, strlen(comment_end)) == 0)
				{
					strcatprintf(dstline, "%s</span>", comment_end_esc);
					curcol += strlen(comment_end);
					srcptr += strlen(comment_end) - 1;
					ch = 0;
					in_comment = false;
				}
			}

			// track whether or not we are within an inline (C++-style) comment
			if (!in_quotes && !in_comment && !in_inline_comment && ch == comment_inline[0] && strncmp(srcptr - 1, comment_inline, strlen(comment_inline)) == 0)
			{
				strcatprintf(dstline, "<span class=\"comment\">%s", comment_inline_esc);
				curcol += strlen(comment_inline);
				srcptr += strlen(comment_inline) - 1;
				ch = 0;
				in_inline_comment = true;
			}

			// if this is the start of a new token, see if we want to color it
			if (!in_quotes && !in_comment && !in_inline_comment && !last_was_token && is_token[ch])
			{
				const token_entry *curtoken;
				char *temp = srcptr;
				int toklength;

				// find the end of the token
				while (*temp != 0 && is_token[(UINT8)*temp])
					temp++;
				toklength = temp - (srcptr - 1);

				// scan the token table
				last_token_was_include = false;
				for (curtoken = token_table; curtoken->token != nullptr; curtoken++)
					if (strncmp(srcptr - 1, curtoken->token, toklength) == 0 && strlen(curtoken->token) == toklength)
					{
						strcatprintf(dstline, "<span class=\"%s\">%s</span>", curtoken->color, curtoken->token);
						curcol += strlen(curtoken->token);
						srcptr += strlen(curtoken->token) - 1;
						ch = 0;

						// look for include tokens specially
						if (type == FILE_TYPE_C && strcmp(curtoken->token, "#include") == 0)
							last_token_was_include = true;
						break;
					}
			}
			last_was_token = is_token[ch];

			// if we hit a tab, expand it
			if (ch == 0x09)
			{
				// compute how many spaces
				int spaces = 4 - curcol % 4;
				while (spaces--)
				{
					dstline.push_back(' ');
					curcol++;
				}
			}

			// otherwise, copy the source character
			else if (ch != 0x0a && ch != 0x0d && ch != 0)
			{
				// track opening quotes
				if (!in_comment && !in_inline_comment && !in_quotes && (ch == '"' || ch == '\''))
				{
					if (color_quotes)
						strcatprintf(dstline, "<span class=\"string\">%c", ch);
					else
						dstline.push_back(ch);
					in_quotes = true;
					curquote = ch;

					// handle includes
					if (last_token_was_include)
					{
						char *endquote = strchr(srcptr, ch);
						if (endquote != nullptr)
						{
							std::string filename(srcptr, endquote - srcptr);
							std::string target;
							if (find_include_file(target, srcrootlen, dstrootlen, srcfile, dstfile, filename))
							{
								strcatprintf(dstline, "<a href=\"%s\">", target.c_str());
								quotes_are_linked = true;
							}
						}
					}
				}

				// track closing quotes
				else if (!in_comment && !in_inline_comment && in_quotes && (ch == curquote) && !escape)
				{
					if (quotes_are_linked)
						dstline.append("</a>");
					if (color_quotes)
						strcatprintf(dstline, "%c</span>", ch);
					else
						dstline.push_back(ch);
					in_quotes = false;
					curquote = 0;
					quotes_are_linked = false;
				}

				// else just output the current character
				else if (ch == '&')
					dstline.append("&amp;");
				else if (ch == '<')
					dstline.append("&lt;");
				else if (ch == '>')
					dstline.append("&gt;");
				else
					dstline.push_back(ch);
				curcol++;
			}

			// Update escape state
			if (in_quotes)
				escape = (ch == '\\' && type == FILE_TYPE_C) ? !escape : false;
		}

		// finish inline comments
		if (in_inline_comment)
		{
			dstline.append("</span>");
			in_inline_comment = false;
		}

		// append a break and move on
		dstline.append("\n");
		core_fputs(dst, dstline.c_str());
	}

	// close tags
	core_fprintf(dst, "\t</pre>\n");

	// close the file
	output_footer_and_close_file(dst, tempfooter, srcfile_subpath);
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

static core_file *create_file_and_output_header(std::string &filename, std::string &templatefile, std::string &path)
{
	// create the indexfile
	core_file *file;
	if (core_fopen(filename.c_str(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS | OPEN_FLAG_NO_BOM, &file) != FILERR_NONE)
		return nullptr;

	// print a header
	std::string modified(templatefile);
	strreplace(modified, "<!--PATH-->", path.c_str());
	core_fwrite(file, modified.c_str(), modified.length());

	// return the file
	return file;
}


/*-------------------------------------------------
    output_footer_and_close_file - write a
    standard footer to an HTML file and close it
-------------------------------------------------*/

static void output_footer_and_close_file(core_file *file, std::string &templatefile, std::string &path)
{
	std::string modified(templatefile);
	strreplace(modified, "<!--PATH-->", path.c_str());
	core_fwrite(file, modified.c_str(), modified.length());
	core_fclose(file);
}



/***************************************************************************
    HTML OUTPUT HELPERS
***************************************************************************/

/*-------------------------------------------------
    normalized_subpath - normalize a path to
    forward slashes and extract a subpath
-------------------------------------------------*/

static std::string &normalized_subpath(std::string &dest, std::string &path, int start)
{
	strreplacechr(dest.assign(path.substr(start, -1)),PATH_SEPARATOR[0], '/');
	return dest;
}


/*-------------------------------------------------
    output_path_as_links - output a path as a
    series of links
-------------------------------------------------*/

static void output_path_as_links(core_file *file, std::string &path, bool end_is_directory, bool link_to_file)
{
	// first count how deep we are
	int srcdepth = 0;
	for (int slashindex = path.find_first_of('/'); slashindex != -1; slashindex = path.find_first_of('/', slashindex + 1))
		srcdepth++;
	if (end_is_directory)
		srcdepth++;

	// output a link to the root
	core_fprintf(file, "<a href=\"");
	for (int depth = 0; depth < srcdepth; depth++)
		core_fprintf(file, "../");
	core_fprintf(file, "index.html\">&lt;root&gt;</a>/");

	// now output links to each path up the chain
	int curdepth = 0;
	int lastslash = 0;
	for (int slashindex = path.find_first_of('/', lastslash); slashindex != -1; slashindex = path.find_first_of('/', lastslash))
	{
		std::string substr(path, lastslash, slashindex - lastslash);

		curdepth++;
		core_fprintf(file, "<a href=\"");
		for (int depth = curdepth; depth < srcdepth; depth++)
			core_fprintf(file, "../");
		core_fprintf(file, "index.html\">%s</a>/", substr.c_str());

		lastslash = slashindex + 1;
	}

	// and a final link to the current directory
	std::string substr(path, lastslash, -1);
	if (end_is_directory)
		core_fprintf(file, "<a href=\"index.html\">%s</a>", substr.c_str());
	else if (link_to_file)
		core_fprintf(file, "<a href=\"%s\">%s</a>", substr.c_str(), substr.c_str());
	else
		core_fprintf(file, "<a href=\"%s.html\">%s</a>", substr.c_str(), substr.c_str());
}


/*-------------------------------------------------
    find_include_file - find an include file
-------------------------------------------------*/

static bool find_include_file(std::string &srcincpath, int srcrootlen, int dstrootlen, std::string &srcfile, std::string &dstfile, std::string &filename)
{
	// iterate over include paths and find the file
	for (include_path *curpath = incpaths; curpath != nullptr; curpath = curpath->next)
	{
		// a '.' include path is specially treated
		if (curpath->path.compare(".") == 0)
			srcincpath.assign(srcfile.substr(0, srcfile.find_last_of(PATH_SEPARATOR[0])));
		else
			srcincpath.assign(srcfile.substr(0, srcrootlen + 1)).append(curpath->path);

		// append the filename piecemeal to account for directories
		int lastsepindex = 0;
		int sepindex;
		while ((sepindex = filename.find_first_of('/', lastsepindex)) != -1)
		{
			// handle .. by removing a chunk from the incpath
			std::string pathpart(filename, lastsepindex, sepindex - lastsepindex);
			if (pathpart.compare("..")==0)
			{
				sepindex = srcincpath.find_last_of(PATH_SEPARATOR[0]);
				if (sepindex != -1)
					srcincpath.substr(0, sepindex);
			}

			// otherwise, append a path separator and the pathpart
			else
				srcincpath.append(PATH_SEPARATOR).append(pathpart);

			// advance past the previous index
			lastsepindex = sepindex + 1;
		}

		// now append the filename
		srcincpath.append(PATH_SEPARATOR).append(filename.substr(lastsepindex, -1));

		// see if we can open it
		core_file *testfile;
		if (core_fopen(srcincpath.c_str(), OPEN_FLAG_READ, &testfile) == FILERR_NONE)
		{
			// close the file
			core_fclose(testfile);

			// find the longest matching directory substring between the include and source file
			lastsepindex = 0;
			while ((sepindex = srcincpath.find_first_of(PATH_SEPARATOR[0], lastsepindex)) != -1)
			{
				// get substrings up to the current directory
				std::string tempfile(srcfile, 0, sepindex);
				std::string tempinc(srcincpath, 0, sepindex);

				// if we don't match, stop
				if (tempfile.compare(tempinc)!=0)
					break;
				lastsepindex = sepindex + 1;
			}

			// chop off the common parts of the paths
			std::string tempfile(srcfile, lastsepindex, -1);
			srcincpath = srcincpath.substr(lastsepindex, -1);
			strreplacechr(srcincpath, PATH_SEPARATOR[0], '/');

			// for each directory left in the filename, we need to prepend a "../"
			while ((sepindex = tempfile.find_first_of(PATH_SEPARATOR[0])) != -1)
			{
				tempfile.substr(sepindex + 1, -1);
				srcincpath.insert(0, "../");
			}
			srcincpath.append(".html");

			// free the strings and return the include path
			return true;
		}
	}
	return false;
}
