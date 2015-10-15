/**
 * \file   premake.c
 * \brief  Program entry point.
 * \author Copyright (c) 2002-2013 Jason Perkins and the Premake project
 */

#include <stdlib.h>
#include <string.h>
#include "premake.h"
#include "version.h"

#if PLATFORM_MACOSX
#	include <CoreFoundation/CFBundle.h>
#endif


#define COPYRIGHT \
			"Copyright (c) 2014-2015 Branimir Karadžić, Neil Richardson,\n" \
			"Mike Popoloski, Drew Solomon, Ted de Munnik, Miodrag Milanović\n" \
			"Brett Vickers, Terry Hendrix II.\n" \
			"Copyright (C) 2002-2013 Jason Perkins and the Premake Project"
#define ERROR_MESSAGE  "%s\n"


static int process_arguments(lua_State* L, int argc, const char** argv);
static int process_option(lua_State* L, const char* arg);
static int load_builtin_scripts(lua_State* L);

int premake_locate(lua_State* L, const char* argv0);


/* A search path for script files */
static const char* scripts_path = NULL;


/* precompiled bytecode buffer; in bytecode.c */
extern const char* builtin_scripts[];


/* Built-in functions */
static const luaL_Reg path_functions[] = {
	{ "isabsolute",  path_isabsolute },
	{ NULL, NULL }
};

static const luaL_Reg os_functions[] = {
	{ "chdir",       os_chdir       },
	{ "copyfile",    os_copyfile    },
	{ "_is64bit",    os_is64bit     },
	{ "isdir",       os_isdir       },
	{ "getcwd",      os_getcwd      },
	{ "isfile",      os_isfile      },
	{ "matchdone",   os_matchdone   },
	{ "matchisfile", os_matchisfile },
	{ "matchname",   os_matchname   },
	{ "matchnext",   os_matchnext   },
	{ "matchstart",  os_matchstart  },
	{ "mkdir",       os_mkdir       },
	{ "pathsearch",  os_pathsearch  },
	{ "rmdir",       os_rmdir       },
	{ "stat",        os_stat        },
	{ "ticks",       os_ticks       },
	{ "uuid",        os_uuid        },
	{ NULL, NULL }
};

static const luaL_Reg string_functions[] = {
	{ "endswith",  string_endswith },
	{ NULL, NULL }
};


/**
 * Initialize the Premake Lua environment.
 */
int premake_init(lua_State* L)
{
	luaL_register(L, "path",   path_functions);
	luaL_register(L, "os",     os_functions);
	luaL_register(L, "string", string_functions);

	/* push the application metadata */
	lua_pushstring(L, LUA_COPYRIGHT);
	lua_setglobal(L, "_COPYRIGHT");

	lua_pushnumber(L, VERSION);
	lua_setglobal(L, "_GENIE_VERSION");

	lua_pushstring(L, VERSION_STR);
	lua_setglobal(L, "_GENIE_VERSION_STR");

	lua_pushstring(L, COPYRIGHT);
	lua_setglobal(L, "_PREMAKE_COPYRIGHT");

	/* set the OS platform variable */
	lua_pushstring(L, PLATFORM_STRING);
	lua_setglobal(L, "_OS");

	return OKAY;
}


int premake_execute(lua_State* L, int argc, const char** argv)
{
	/* Parse the command line arguments */
	int z = process_arguments(L, argc, argv);

	/* Run the built-in Premake scripts */
	if (z == OKAY)  z = load_builtin_scripts(L);

	return z;
}


/**
 * Locate the Premake executable, and push its full path to the Lua stack.
 * Based on:
 * http://sourceforge.net/tracker/index.php?func=detail&aid=3351583&group_id=71616&atid=531880
 * http://stackoverflow.com/questions/933850/how-to-find-the-location-of-the-executable-in-c
 * http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
 */
int premake_locate(lua_State* L, const char* argv0)
{
#if !defined(PATH_MAX)
#define PATH_MAX  (4096)
#endif

	char buffer[PATH_MAX];
	const char* path = NULL;

#if PLATFORM_WINDOWS
	DWORD len = GetModuleFileName(NULL, buffer, PATH_MAX);
	if (len > 0)
		path = buffer;
#endif

#if PLATFORM_MACOSX
	CFURLRef bundleURL = CFBundleCopyExecutableURL(CFBundleGetMainBundle());
	CFStringRef pathRef = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
	if (CFStringGetCString(pathRef, buffer, PATH_MAX - 1, kCFStringEncodingUTF8))
		path = buffer;
#endif

#if PLATFORM_LINUX
	int len = readlink("/proc/self/exe", buffer, PATH_MAX);
	if (len > 0)
		path = buffer;
#endif

#if PLATFORM_BSD
	int len = readlink("/proc/curproc/file", buffer, PATH_MAX);
	if (len < 0)
		len = readlink("/proc/curproc/exe", buffer, PATH_MAX);
	if (len > 0)
		path = buffer;
#endif

#if PLATFORM_SOLARIS
	int len = readlink("/proc/self/path/a.out", buffer, PATH_MAX);
	if (len > 0)
		path = buffer;
#endif

	/* As a fallback, search the PATH with argv[0] */
	if (!path)
	{
		lua_pushcfunction(L, os_pathsearch);
		lua_pushstring(L, argv0);
		lua_pushstring(L, getenv("PATH"));
		if (lua_pcall(L, 2, 1, 0) == OKAY && !lua_isnil(L, -1))
		{
			lua_pushstring(L, "/");
			lua_pushstring(L, argv0);
			lua_concat(L, 3);
			path = lua_tostring(L, -1);
		}
	}

	/* If all else fails, use argv[0] as-is and hope for the best */
	if (!path)
	{
		/* make it absolute, if needed */
		os_getcwd(L);
		lua_pushstring(L, "/");
		lua_pushstring(L, argv0);

		if (!path_isabsolute(L)) {
			lua_concat(L, 3);
		}
		else {
			lua_pop(L, 1);
		}

		path = lua_tostring(L, -1);
	}

	lua_pushstring(L, path);
	return 1;
}



/**
 * Process the command line arguments, splitting them into options, the
 * target action, and any arguments to that action. The results are pushed
 * into the session for later use. I could have done this in the scripts,
 * but I need the value of the /scripts option to find them.
 * \returns OKAY if successful.
 */
int process_arguments(lua_State* L, int argc, const char** argv)
{
	int i;

	/* Create empty lists for Options and Args */
	lua_newtable(L);
	lua_newtable(L);

	for (i = 1; i < argc; ++i)
	{
		/* Options start with '/' or '--'. The first argument that isn't an option
		 * is the action. Anything after that is an argument to the action */
		if (argv[i][0] == '/')
		{
			process_option(L, argv[i] + 1);
		}
		else if (argv[i][0] == '-' && argv[i][1] == '-')
		{
			process_option(L, argv[i] + 2);
		}
		else
		{
			/* not an option, is the action */
			lua_pushstring(L, argv[i++]);
			lua_setglobal(L, "_ACTION");

			/* everything else is an argument */
			while (i < argc)
			{
				lua_pushstring(L, argv[i++]);
				lua_rawseti(L, -2, lua_rawlen(L, -2) + 1);
			}
		}
	}

	/* push the Options and Args lists */
	lua_setglobal(L, "_ARGS");
	lua_setglobal(L, "_OPTIONS");
	return OKAY;
}



/**
 * Parse an individual command-line option.
 * \returns OKAY if successful.
 */
int process_option(lua_State* L, const char* arg)
{
	char key[512];
	const char* value;

	/* If a value is specified, split the option into a key/value pair */
	char* ptr = strchr(arg, '=');
	if (ptr)
	{
		int len = ptr - arg;
		if (len > 511) len = 511;
		strncpy(key, arg, len);
		key[len] = '\0';
		value = ptr + 1;
	}
	else
	{
		strcpy(key, arg);
		value = "";
	}

	/* Store it in the Options table, which is already on the stack */
	lua_pushstring(L, value);
	lua_setfield(L, -3, key);

	/* The /scripts option gets picked up here to find the built-in scripts */
	if (strcmp(key, "scripts") == 0 && strlen(value) > 0)
	{
		scripts_path = value;
	}

	return OKAY;
}



#if !defined(NDEBUG)
/**
 * When running in debug mode, the scripts are loaded from the disk. The path to
 * the scripts must be provided via either the /scripts command line option or
 * the PREMAKE_PATH environment variable.
 */
int load_builtin_scripts(lua_State* L)
{
	const char* filename;

	/* call os.pathsearch() to locate _premake_main.lua */
	lua_pushcfunction(L, os_pathsearch);
	lua_pushstring(L, "_premake_main.lua");
	lua_pushstring(L, scripts_path);
	lua_pushstring(L, getenv("PREMAKE_PATH"));
	lua_call(L, 3, 1);

	if (lua_isnil(L, -1))
	{
		printf(ERROR_MESSAGE,
			"Unable to find _premake_main.lua; use /scripts option when in debug mode!\n"
			"Please refer to the documentation (or build in release mode instead)."
		);
		return !OKAY;
	}

	/* run the bootstrapping script */
	scripts_path = lua_tostring(L, -1);
	filename = lua_pushfstring(L, "%s/_premake_main.lua", scripts_path);
	if (luaL_dofile(L, filename))
	{
		printf(ERROR_MESSAGE, lua_tostring(L, -1));
		return !OKAY;
	}

	/* in debug mode, show full traceback on all errors */
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");

	/* hand off control to the scripts */
	lua_getglobal(L, "_premake_main");
	lua_pushstring(L, scripts_path);
	if (lua_pcall(L, 1, 1, -3) != OKAY)
	{
		printf(ERROR_MESSAGE, lua_tostring(L, -1));
		return !OKAY;
	}
	else
	{
		return (int)lua_tonumber(L, -1);
	}
}
#endif


#if defined(NDEBUG)
/**
 * When running in release mode, the scripts are loaded from a static data
 * buffer, where they were stored by a preprocess. To update these embedded
 * scripts, run `premake4 embed` then rebuild.
 */
int load_builtin_scripts(lua_State* L)
{
	int i;
	for (i = 0; builtin_scripts[i]; ++i)
	{
		if (luaL_dostring(L, builtin_scripts[i]) != OKAY)
		{
			printf(ERROR_MESSAGE, lua_tostring(L, -1));
			return !OKAY;
		}
	}

	/* set error handler to get tracebacks in built-in scripts  */
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	lua_remove(L, -2);
	int ehpos = lua_gettop(L);

	/* hand off control to the scripts */
	lua_getglobal(L, "_premake_main");
	if (lua_pcall(L, 0, 1, ehpos) != OKAY)
	{
		printf(ERROR_MESSAGE, lua_tostring(L, -1));
		return !OKAY;
	}
	else
	{
		return (int)lua_tonumber(L, -1);
	}
}
#endif
