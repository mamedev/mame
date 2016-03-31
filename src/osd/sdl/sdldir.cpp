// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdldir.c - SDL core directory access functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifdef SDLMAME_WIN32
#include "../windows/windir.cpp"
#else

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifdef SDLMAME_LINUX
#define __USE_LARGEFILE64
#endif
#ifndef SDLMAME_BSD
#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#define _XOPEN_SOURCE 500
#endif

#include <cctype>
#include <cstdlib>
#include <utility>

//#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef __USE_BSD
#define __USE_BSD   // to get DT_xxx on Linux
#endif
#undef _POSIX_C_SOURCE  // to get DT_xxx on OS X
#include <dirent.h>

#include "osdcore.h"
#include "modules/lib/osdlib.h"

#if defined(SDLMAME_WIN32)
#define PATHSEPCH '\\'
#define INVPATHSEPCH '/'
#else
#define PATHSEPCH '/'
#define INVPATHSEPCH '\\'
#endif

#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_HAIKU) || defined(SDLMAME_EMSCRIPTEN) || defined(SDLMAME_ANDROID)
typedef struct dirent sdl_dirent;
typedef struct stat sdl_stat;
#define sdl_readdir readdir
#define sdl_stat_fn stat
#else
typedef struct dirent64 sdl_dirent;
typedef struct stat64 sdl_stat;
#define sdl_readdir readdir64
#define sdl_stat_fn stat64
#endif

#define HAS_DT_XXX defined(SDLMAME_LINUX) || defined(SDLMAME_BSD) || defined(SDLMAME_DARWIN)

struct osd_directory
{
	osd_directory_entry ent;
	sdl_dirent *data;
	DIR *fd;
	char *path;
};

static char *build_full_path(const char *path, const char *file)
{
	char *ret = (char *) osd_malloc_array(strlen(path)+strlen(file)+2);
	char *p = ret;

	strcpy(p, path);
	p += strlen(path);
	*p++ = PATHSEPCH;
	strcpy(p, file);
	return ret;
}


#if HAS_DT_XXX
static osd_dir_entry_type get_attributes_enttype(int attributes, char *path)
{
	switch ( attributes )
	{
		case DT_DIR:
			return ENTTYPE_DIR;

		case DT_REG:
			return ENTTYPE_FILE;

		case DT_LNK:
		{
			struct stat s;

			if ( stat(path, &s) != 0 )
				return ENTTYPE_OTHER;
			else
				return S_ISDIR(s.st_mode) ? ENTTYPE_DIR : ENTTYPE_FILE;
		}

		default:
			return ENTTYPE_OTHER;
	}
}
#else

static osd_dir_entry_type get_attributes_stat(const char *file)
{
	sdl_stat st;
	if(sdl_stat_fn(file, &st))
		return (osd_dir_entry_type) 0;

	if (S_ISDIR(st.st_mode))
		return ENTTYPE_DIR;
	else
		return ENTTYPE_FILE;
}
#endif

static UINT64 osd_get_file_size(const char *file)
{
	sdl_stat st;
	if(sdl_stat_fn(file, &st))
		return 0;
	return st.st_size;
}

//============================================================
//  osd_opendir
//============================================================

osd_directory *osd_opendir(const char *dirname)
{
	osd_directory *dir = NULL;
	char *tmpstr, *envstr;
	int i, j;

	dir = (osd_directory *) osd_malloc(sizeof(osd_directory));
	if (dir)
	{
		memset(dir, 0, sizeof(osd_directory));
		dir->fd = NULL;
	}

	tmpstr = (char *) osd_malloc_array(strlen(dirname)+1);
	strcpy(tmpstr, dirname);

	if (tmpstr[0] == '$')
	{
		envstr = (char *) osd_malloc_array(strlen(tmpstr)+1);

		strcpy(envstr, tmpstr);

		i = 0;
		while (envstr[i] != PATHSEPCH && envstr[i] != INVPATHSEPCH && envstr[i] != 0 && envstr[i] != '.')
		{
			i++;
		}

		envstr[i] = '\0';

		const char *envval = osd_getenv(&envstr[1]);
		if (envval != NULL)
		{
			j = strlen(envval) + strlen(tmpstr) + 1;
			osd_free(tmpstr);
			tmpstr = (char *) osd_malloc_array(j);

			// start with the value of $HOME
			strcpy(tmpstr, envval);
			// replace the null with a path separator again
			envstr[i] = PATHSEPCH;
			// append it
			strcat(tmpstr, &envstr[i]);
		}
		else
			fprintf(stderr, "Warning: osd_opendir environment variable %s not found.\n", envstr);
		osd_free(envstr);
	}

	dir->fd = opendir(tmpstr);
	dir->path = tmpstr;

	if (dir && (dir->fd == NULL))
	{
		osd_free(dir->path);
		osd_free(dir);
		dir = NULL;
	}

	return dir;
}


//============================================================
//  osd_readdir
//============================================================

const osd_directory_entry *osd_readdir(osd_directory *dir)
{
	char *temp;
	dir->data = sdl_readdir(dir->fd);

	if (dir->data == NULL)
		return NULL;

	dir->ent.name = dir->data->d_name;
	temp = build_full_path(dir->path, dir->data->d_name);
	#if HAS_DT_XXX
	dir->ent.type = get_attributes_enttype(dir->data->d_type, temp);
	#else
	dir->ent.type = get_attributes_stat(temp);
	#endif
	dir->ent.size = osd_get_file_size(temp);
	osd_free(temp);
	return &dir->ent;
}


//============================================================
//  osd_closedir
//============================================================

void osd_closedir(osd_directory *dir)
{
	if (dir->fd != NULL)
		closedir(dir->fd);
	osd_free(dir->path);
	osd_free(dir);
}


//============================================================
//  osd_subst_env
//============================================================

void osd_subst_env(std::string &dst, std::string const &src)
{
	std::string result, var;
	auto start = src.begin();

	// a leading tilde expands as $HOME
	if ((src.end() != start) && ('~' == *start))
	{
		char const *const home = std::getenv("HOME");
		if (home)
		{
			++start;
			if ((src.end() == start) || (PATHSEPCH == *start))
				result.append(home);
			else
				result.push_back('~');
		}
	}

	while (src.end() != start)
	{
		// find $ marking start of environment variable or end of string
		auto it = start;
		while ((src.end() != it) && ('$' != *it)) ++it;
		if (start != it) result.append(start, it);
		start = it;

		if (src.end() != start)
		{
			start = ++it;
			if ((src.end() != start) && ('{' == *start))
			{
				start = ++it;
				for (++it; (src.end() != it) && ('}' != *it); ++it) { }
				if (src.end() == it)
				{
					result.append("${").append(start, it);
					start = it;
				}
				else
				{
					var.assign(start, it);
					start = ++it;
					const char *const exp = std::getenv(var.c_str());
					if (exp)
						result.append(exp);
					else
						fprintf(stderr, "Warning: osd_subst_env variable %s not found.\n", var.c_str());
				}
			}
			else if ((src.end() != start) && (('_' == *start) || std::isalnum(*start)))
			{
				for (++it; (src.end() != it) && (('_' == *it) || std::isalnum(*it)); ++it) { }
				var.assign(start, it);
				start = it;
				const char *const exp = std::getenv(var.c_str());
				if (exp)
					result.append(exp);
				else
					fprintf(stderr, "Warning: osd_subst_env variable %s not found.\n", var.c_str());
			}
			else
			{
				result.push_back('$');
			}
		}
	}

	dst = std::move(result);
}

#endif
