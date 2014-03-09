// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    MAME source code dependency generator

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <zlib.h>
#include "osdcore.h"
#include "astring.h"
#include "corefile.h"
#include "corestr.h"
#include "tagmap.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define HASH_SIZE       193
#define MAX_SOURCES  65536


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct include_path
{
	include_path *  next;
	astring         path;
};


struct exclude_path
{
	exclude_path *  next;
	astring         path;
	int             pathlen;
	UINT8           recursive;
};


struct list_entry
{
	list_entry *    next;
	astring         name;
};

struct librarylist_entry
{
	librarylist_entry *    next;
	list_entry *    sourcefiles;
	astring         name;
};

struct file_entry;

struct dependency
{
	dependency *    next;
	file_entry *    file;
};


struct file_entry
{
	astring         name;
	dependency *    deplist;
};

typedef tagmap_t<UINT8> dependency_map;



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static include_path *incpaths;
static exclude_path *excpaths;
static tagmap_t<file_entry *> file_map;
static librarylist_entry *librarylist;

static librarylist_entry *last_libraryitem;
static list_entry *last_sourceitem;

static tagmap_t<char *> include_map;


/***************************************************************************
    PROTOTYPES
***************************************************************************/

// core output functions
static int recurse_dir(astring &srcdir);
static file_entry &compute_dependencies(astring &srcfile);

// path helpers
static bool find_include_file(astring &srcincpath, const astring &srcfile, const astring &filename);

static bool check_file(astring &srcincpath)
{
	// see if we can open it
	core_file *testfile;
	if (core_fopen(srcincpath, OPEN_FLAG_READ, &testfile) == FILERR_NONE)
	{
		// close the file
		core_fclose(testfile);
		return true;
	}
	return false;
}
int include_mapping(const char *srcfile)
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
	while (srcptr < endptr)
	{
		char c = *srcptr++;
		// count newlines
		if (c == 13 || c == 10)
		{
			if (c == 13 && *srcptr == 10)
				srcptr++;
			continue;
		}
		// look for start of C comment
		if (c == '#' && *srcptr == '@')
		{
			srcptr++;
			//mapping
			char filename[256];
			filename[0] = 0;
			for (int pos = 0; srcptr < endptr && pos < ARRAY_LENGTH(filename) - 1 && (*srcptr!=','); pos++)
			{
				filename[pos] = *srcptr++;
				filename[pos+1] = 0;
			}
			srcptr++;           // skip comma
			char mapping[256];
			mapping[0] = 0;
			for (int pos = 0; srcptr < endptr && pos < ARRAY_LENGTH(mapping) - 1 && (*srcptr!=10) && (*srcptr!=13); pos++)
			{
				mapping[pos] = *srcptr++;
				mapping[pos+1] = 0;
			}
			include_map.add(filename,core_strdup(mapping));
			continue;
		}

		for (int pos = 0; srcptr < endptr && !isspace(*srcptr); pos++)
		{
			c = *srcptr++;
		}
	}

	osd_free(buffer);

	return 0;
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
			fprintf(stderr, "Importing drivers from '%s'\n", filename);
			parse_file(filename);
			continue;
		}
		if (c == '@')
		{
			// Used for makemak tool
			char drivname[256];
			drivname[0] = 0;
			for (int pos = 0; srcptr < endptr && pos < ARRAY_LENGTH(drivname) - 1 && !isspace(*srcptr); pos++)
			{
				drivname[pos] = *srcptr++;
				drivname[pos+1] = 0;
			}

			librarylist_entry *lentry = new librarylist_entry;
			lentry->name.cpy(drivname);
			lentry->next = NULL;
			lentry->sourcefiles = NULL;
			if (last_libraryitem!=NULL)
			{
				last_libraryitem->next = lentry;
			}
			last_libraryitem = lentry;
			last_sourceitem = NULL;

			if (librarylist==NULL)
			{
				librarylist = lentry;
			}

			continue;
		}

		srcptr--;
		// Used for makemak tool
		char drivname[256];
		drivname[0] = 0;
		for (int pos = 0; srcptr < endptr && pos < ARRAY_LENGTH(drivname) - 1 && !isspace(*srcptr); pos++)
		{
			drivname[pos] = *srcptr++;
			drivname[pos+1] = 0;
		}

		list_entry *lentry = new list_entry;
		lentry->name.cpy(drivname);
		lentry->next = NULL;
		if (last_sourceitem!=NULL)
		{
			last_sourceitem->next = lentry;
		}
		last_sourceitem = lentry;
		if (last_libraryitem->sourcefiles==NULL)
		{
			last_libraryitem->sourcefiles = lentry;
		}
	}

	osd_free(buffer);

	return 0;
}

int parse_for_drivers(const char *srcfile)
{
	// read source file
	core_file *file = NULL;

	file_error filerr = core_fopen(srcfile, OPEN_FLAG_READ, &file);
	if (filerr != FILERR_NONE)
	{
		fprintf(stderr, "Unable to read source file '%s'\n", srcfile);
		return 1;
	}
	// loop over lines in the file
	char buffer[4096];
	while (core_fgets(buffer, ARRAY_LENGTH(buffer), file) != NULL)
	{
		astring line;

		// rip through it to find all drivers
		char *srcptr = (char *)buffer;
		char *endptr = srcptr + strlen(buffer);
		bool in_comment = false;
		while (srcptr < endptr)
		{
			char c = *srcptr++;

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

			srcptr--;
			for (int pos = 0; srcptr < endptr && !isspace(*srcptr); pos++)
			{
				line.cat(*srcptr++);
			}
		}

		if ((line.find(0,"GAME(")==0) || (line.find(0,"GAMEL(")==0) ||
			(line.find(0,"COMP(")==0) || (line.find(0,"CONS(")==0) ||
			(line.find(0,"SYST(")==0))
		{
			int p1 = line.find(0,",");
			if (p1<0) continue;
			int p2 = line.find(p1+1,",");
			if (p2<0) continue;

			printf("%s\n",line.substr(p1+1,p2-p1-1).cstr());
		}
	}
	core_fclose(file);
	return 0;
}

/***************************************************************************
    MAIN
***************************************************************************/

/*-------------------------------------------------
    main - main entry point
-------------------------------------------------*/

void usage(const char *argv0)
{
	fprintf(stderr, "Usage:\n%s <source.lst> <srcroot> [-Iincpath [-Iincpath [...]]]\n", argv0);
	exit(1);
}

int main(int argc, char *argv[])
{
	include_path **incpathhead = &incpaths;
	exclude_path **excpathhead = &excpaths;
	astring srcdir;
	int unadorned = 0;

	librarylist = NULL;
	last_libraryitem = NULL;
	last_sourceitem = NULL;


	// extract arguments
	const char *srcfile = argv[1];
	if (parse_file(srcfile))
		return 1;

	// loop over arguments
	for (int argnum = 2; argnum < argc; argnum++)
	{
		char *arg = argv[argnum];

		// include path?
		if (arg[0] == '-' && arg[1] == 'I')
		{
			*incpathhead = new include_path;
			(*incpathhead)->next = NULL;
			(*incpathhead)->path.cpy(&arg[2]).replacechr('/', PATH_SEPARATOR[0]);
			incpathhead = &(*incpathhead)->next;
		}

		// exclude path?
		else if (arg[0] == '-' && arg[1] == 'X')
		{
			*excpathhead = new exclude_path;
			(*excpathhead)->next = NULL;
			(*excpathhead)->path.cpy(&arg[2]).replacechr(PATH_SEPARATOR[0], '/');
			(*excpathhead)->recursive = ((*excpathhead)->path.replace((*excpathhead)->path.len() - 4, "/...", "") != 0);
			(*excpathhead)->pathlen = (*excpathhead)->path.len();
			excpathhead = &(*excpathhead)->next;
		}

		// ignore -include which is used by sdlmame to include sdlprefix.h before all other includes
		else if (strcmp(arg,"-include") == 0)
		{
			argnum++;
		}

		// other parameter
		else if (arg[0] != '-' && unadorned == 0)
		{
			srcdir.cpy(arg).replacechr('/', PATH_SEPARATOR[0]);
			unadorned++;
		}
		else
			usage(argv[0]);
	}

	// generate list of drivers
	if (srcdir.len() == 0)
	{
		for (librarylist_entry *lib = librarylist; lib != NULL; lib = lib->next)
		{
			for (list_entry *src = lib->sourcefiles; src != NULL; src = src->next)
			{
				printf("// Drivers from %s.c\n",src->name.cstr());
				astring srcfile;
				// build the source filename
				srcfile.printf("%s%c%s.c", "src", PATH_SEPARATOR[0], src->name.cstr());
				parse_for_drivers(srcfile);

				astring srcfile_inc;
				// build the source filename
				srcfile_inc.printf("%s%c%s.inc", "src", PATH_SEPARATOR[0], src->name.cstr());
				if(check_file(srcfile_inc))
					parse_for_drivers(srcfile_inc);
			}
		}
		return 0;
	}
	else
	{
		include_mapping("src/emu/cpu/cpu.mak");
		include_mapping("src/emu/video/video.mak");
		include_mapping("src/emu/sound/sound.mak");
		include_mapping("src/emu/machine/machine.mak");
		include_mapping("src/emu/bus/bus.mak");
		if (librarylist!=NULL)
		{
			printf("OBJDIRS += \\\n");
			printf("\t$(OBJ)/target \\\n");
			printf("\t$(OBJ)/mame/audio \\\n");
			printf("\t$(OBJ)/mame/drivers \\\n");
			printf("\t$(OBJ)/mame/layout \\\n");
			printf("\t$(OBJ)/mame/machine \\\n");
			printf("\t$(OBJ)/mame/video \\\n");
			printf("\t$(OBJ)/mess/audio \\\n");
			printf("\t$(OBJ)/mess/drivers \\\n");
			printf("\t$(OBJ)/mess/layout \\\n");
			printf("\t$(OBJ)/mess/machine \\\n");
			printf("\t$(OBJ)/mess/video \\\n");
			printf("\n\n");
			printf("DRVLIBS += \\\n");

			for (librarylist_entry *lib = librarylist; lib != NULL; lib = lib->next)
			{
				printf("\t$(OBJ)/target/%s.a \\\n",lib->name.cstr());
			}
			printf("\n");
		}

		// recurse over subdirectories
		return recurse_dir(srcdir);
	}
}



/***************************************************************************
    CORE OUTPUT FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    recurse_dependencies - recurse through the
    dependencies found, adding the mto the tagmap
    unless we already exist in the map
-------------------------------------------------*/

static void recurse_dependencies(file_entry &file, dependency_map &map)
{
	// skip if we're in an exclude path
	int filelen = file.name.len();
	for (exclude_path *exclude = excpaths; exclude != NULL; exclude = exclude->next)
		if (exclude->pathlen < filelen && strncmp(file.name, exclude->path, exclude->pathlen) == 0)
			if (exclude->recursive || file.name.chr(exclude->pathlen + 1, '/') == -1)
				return;

	// attempt to add; if we get an error, we're already present
	if (map.add(file.name, 0) != TMERR_NONE)
		return;

	// recurse the list from there
	for (dependency *dep = file.deplist; dep != NULL; dep = dep->next)
		recurse_dependencies(*dep->file, map);
}


/*-------------------------------------------------
    recurse_dir - recurse through a directory
-------------------------------------------------*/

static int recurse_dir(astring &srcdir)
{
	int result = 0;

	// iterate through each file
	for (librarylist_entry *lib = librarylist; lib != NULL; lib = lib->next)
	{
		for (list_entry *src = lib->sourcefiles; src != NULL; src = src->next)
		{
			astring srcfile;

			// build the source filename
			srcfile.printf("%s%s.c", srcdir.cstr(), src->name.cstr());

			dependency_map depend_map;

			// find dependencies
			file_entry &file = compute_dependencies(srcfile);
			recurse_dependencies(file, depend_map);

			for (dependency_map::entry_t *entry = depend_map.first(); entry != NULL; entry = depend_map.next(entry))
			{
				astring t(entry->tag());
				if (core_filename_ends_with(t, ".h"))
				{
					char *foundfile = include_map.find(t);
					if (foundfile != NULL) {
						printf("%s\n", foundfile);
						// we add things just once when needed
						include_map.remove(t);
					}
				}
			}
		}
	}


	// iterate through each file
	for (librarylist_entry *lib = librarylist; lib != NULL; lib = lib->next)
	{
		// convert the target from source to object (makes assumptions about rules)
		astring target("$(OBJ)/target/",lib->name.cstr());
		target.cat(".a");
		printf("\n%s : \\\n", target.cstr());

		for (list_entry *src = lib->sourcefiles; src != NULL; src = src->next)
		{
			astring srcfile;

			// build the source filename
			srcfile.printf("%s%s.c", srcdir.cstr(), src->name.cstr());
			dependency_map depend_map;

			// find dependencies
			file_entry &file = compute_dependencies(srcfile);
			recurse_dependencies(file, depend_map);

			// iterate over the hashed dependencies and output them as well
			for (dependency_map::entry_t *entry = depend_map.first(); entry != NULL; entry = depend_map.next(entry))
			{
				astring t(entry->tag());
				t.replace(0, "src/", "$(OBJ)/");
				t.replace(0, ".c", ".o");
				if (core_filename_ends_with(t, ".o"))
				{
					printf("\t%s \\\n", t.cstr());
				}
			}
		}
		printf("\n");
		for (list_entry *src = lib->sourcefiles; src != NULL; src = src->next)
		{
			astring srcfile;

			// build the source filename
			srcfile.printf("%s%s.c", srcdir.cstr(), src->name.cstr());
			dependency_map depend_map;

			// find dependencies
			file_entry &file = compute_dependencies(srcfile);
			recurse_dependencies(file, depend_map);
			for (dependency_map::entry_t *entry = depend_map.first(); entry != NULL; entry = depend_map.next(entry))
			{
				astring t(entry->tag());
				if (core_filename_ends_with(t, ".lay"))
				{
					astring target2(file.name);
					target2.replace(0, "src/", "$(OBJ)/");
					target2.replace(0, ".c", ".o");

					t.replace(0, "src/", "$(OBJ)/");
					t.replace(0, ".lay", ".lh");

					printf("%s: %s\n", target2.cstr(), t.cstr());
				}
				if (core_filename_ends_with(t, ".inc"))
				{
					astring target2(file.name);
					target2.replace(0, "src/", "$(OBJ)/");
					target2.replace(0, ".c", ".o");

					printf("%s: %s\n", target2.cstr(), t.cstr());
				}
			}
		}
	}
	return result;
}

/*-------------------------------------------------
    output_file - output a file, converting to
    HTML
-------------------------------------------------*/

static file_entry &compute_dependencies(astring &srcfile)
{
	// see if we already have an entry
	astring normalfile(srcfile);
	normalfile.replacechr(PATH_SEPARATOR[0], '/');
	file_entry *foundfile = file_map.find(normalfile);
	if (foundfile != NULL)
		return *foundfile;

	// create a new header entry
	file_entry &file = *new file_entry;
	file.deplist = NULL;
	file.name = normalfile;
	file_map.add(file.name, &file);

	// read the source file
	UINT32 filelength;
	char *filedata;
	if (core_fload(srcfile, (void **)&filedata, &filelength) != FILERR_NONE)
	{
		fprintf(stderr, "Unable to read file '%s'\n", srcfile.cstr());
		return file;
	}

	astring audiofile = astring(srcfile);
	audiofile.replace("drivers","audio");
	if (check_file(audiofile))
	{
		dependency *dep = new dependency;
		dep->next = file.deplist;
		file.deplist = dep;
		dep->file = &compute_dependencies(audiofile);
	}

	astring machinefile = astring(srcfile);
	machinefile.replace("drivers","machine");
	if (check_file(machinefile))
	{
		dependency *dep = new dependency;
		dep->next = file.deplist;
		file.deplist = dep;
		dep->file = &compute_dependencies(machinefile);
	}

	astring videofile = astring(srcfile);
	videofile.replace("drivers","video");
	if (check_file(videofile))
	{
		dependency *dep = new dependency;
		dep->next = file.deplist;
		file.deplist = dep;
		dep->file = &compute_dependencies(videofile);
	}


	// find the #include directives in this file
	for (int index = 0; index < filelength; index++)
		if (filedata[index] == '#' && strncmp(&filedata[index + 1], "include", 7) == 0)
		{
			// first make sure we're not commented or quoted
			bool just_continue = false;
			for (int scan = index; scan > 2 && filedata[scan] != 13 && filedata[scan] != 10; scan--)
				if ((filedata[scan] == '/' && filedata[scan - 1] == '/') || filedata[scan] == '"')
				{
					just_continue = true;
					break;
				}
			if (just_continue)
				continue;

			// scan forward to find the quotes or bracket
			index += 7;
			int scan;
			for (scan = index; scan < filelength && filedata[scan] != '<' && filedata[scan] != '"' && filedata[scan] != 13 && filedata[scan] != 10; scan++) ;

			// ignore if not found or if it's bracketed
			if (scan >= filelength || filedata[scan] != '"')
				continue;
			int start = ++scan;

			// find the closing quote
			while (scan < filelength && filedata[scan] != '"')
				scan++;
			if (scan >= filelength)
				continue;

			// find the include file
			astring filename(&filedata[start], scan - start);
			astring target;

			filename.replace(".lh",".lay");

			// create a new dependency
			if (find_include_file(target, srcfile, filename))
			{
				dependency *dep = new dependency;
				dep->next = file.deplist;
				file.deplist = dep;
				dep->file = &compute_dependencies(target);
			}
			// create a new dependency
		}

	osd_free(filedata);
	return file;
}



/***************************************************************************
    HELPERS
***************************************************************************/

/*-------------------------------------------------
    find_include_file - find an include file
-------------------------------------------------*/

static bool find_include_file(astring &srcincpath, const astring &srcfile, const astring &filename)
{
	// iterate over include paths and find the file
	for (include_path *curpath = incpaths; curpath != NULL; curpath = curpath->next)
	{
		// a '.' include path is specially treated
		if (curpath->path == ".")
			srcincpath.cpysubstr(srcfile, 0, srcfile.rchr(0, PATH_SEPARATOR[0]));
		else
			srcincpath.cpy(curpath->path);

		// append the filename piecemeal to account for directories
		int lastsepindex = 0;
		int sepindex;
		while ((sepindex = filename.chr(lastsepindex, '/')) != -1)
		{
			astring pathpart(filename, lastsepindex, sepindex - lastsepindex);

			// handle .. by removing a chunk from the incpath
			if (pathpart == "..")
			{
				int sepindex_part = srcincpath.rchr(0, PATH_SEPARATOR[0]);
				if (sepindex_part != -1)
					srcincpath.substr(0, sepindex_part);
			}

			// otherwise, append a path separator and the pathpart
			else
				srcincpath.cat(PATH_SEPARATOR).cat(pathpart);

			// advance past the previous index
			lastsepindex = sepindex + 1;
		}

		// now append the filename
		srcincpath.cat(PATH_SEPARATOR).catsubstr(filename, lastsepindex, -1);

		// see if we can open it
		core_file *testfile;
		if (core_fopen(srcincpath, OPEN_FLAG_READ, &testfile) == FILERR_NONE)
		{
			// close the file
			core_fclose(testfile);
			return true;
		}
	}
	return false;
}
