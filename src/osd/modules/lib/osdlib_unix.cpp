// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <signal.h>
#include <dlfcn.h>

// MAME headers
#include "osdcore.h"
#include "osdlib.h"
#include "strconv.h"

#include <SDL2/SDL.h>

//============================================================
//  osd_getenv
//============================================================

const char *osd_getenv(const char *name)
{
	return getenv(name);
}

//============================================================
//  osd_setenv
//============================================================

int osd_setenv(const char *name, const char *value, int overwrite)
{
	return setenv(name, value, overwrite);
}

//============================================================
//  osd_process_kill
//============================================================

void osd_process_kill(void)
{
	kill(getpid(), SIGKILL);
}

//============================================================
//  osd_malloc
//============================================================

void *osd_malloc(size_t size)
{
#ifndef MALLOC_DEBUG
	return malloc(size);
#else
#error "MALLOC_DEBUG not yet supported"
#endif
}


//============================================================
//  osd_malloc_array
//============================================================

void *osd_malloc_array(size_t size)
{
#ifndef MALLOC_DEBUG
	return malloc(size);
#else
#error "MALLOC_DEBUG not yet supported"
#endif
}


//============================================================
//  osd_free
//============================================================

void osd_free(void *ptr)
{
#ifndef MALLOC_DEBUG
	free(ptr);
#else
#error "MALLOC_DEBUG not yet supported"
#endif
}

//============================================================
//  osd_alloc_executable
//
//  allocates "size" bytes of executable memory.  this must take
//  things like NX support into account.
//============================================================

void *osd_alloc_executable(size_t size)
{
#if defined(SDLMAME_BSD) || defined(SDLMAME_MACOSX)
	return (void *)mmap(0, size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
#elif defined(SDLMAME_UNIX)
	return (void *)mmap(0, size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, 0, 0);
#endif
}

//============================================================
//  osd_free_executable
//
//  frees memory allocated with osd_alloc_executable
//============================================================

void osd_free_executable(void *ptr, size_t size)
{
#ifdef SDLMAME_SOLARIS
	munmap((char *)ptr, size);
#else
	munmap(ptr, size);
#endif
}

//============================================================
//  osd_break_into_debugger
//============================================================

void osd_break_into_debugger(const char *message)
{
	//#ifdef MAME_DEBUG
	#if 1
	printf("MAME exception: %s\n", message);
	printf("Attempting to fall into debugger\n");
	kill(getpid(), SIGTRAP);
	#else
	printf("Ignoring MAME exception: %s\n", message);
	#endif
}

#ifdef SDLMAME_ANDROID
char *osd_get_clipboard_text(void)
{
	return nullptr;
}
#else
//============================================================
//  osd_get_clipboard_text
//============================================================

char *osd_get_clipboard_text(void)
{
	char *result = nullptr;

	if (SDL_HasClipboardText())
	{
		char *temp = SDL_GetClipboardText();
		result = (char *) osd_malloc_array(strlen(temp) + 1);
		strcpy(result, temp);
		SDL_free(temp);
	}
	return result;
}

#endif

//============================================================
//  osd_dynamic_bind
//============================================================

osd_dynamic_bind_base::osd_dynamic_bind_base(const char *symbol, const std::vector<std::wstring> libraries)
	: m_module(nullptr), m_function(nullptr)
{
	for (int i = 0; i < libraries.size(); i++)
	{
		char *utf8_name = utf8_from_wstring(libraries[i].c_str());
	
		void *module = dlopen(utf8_name, RTLD_LAZY);

		osd_free(utf8_name);

		if (module != nullptr)
		{
			m_function = reinterpret_cast<void *>(dlsym(module, symbol));

			if (m_function != nullptr)
			{
				m_module = reinterpret_cast<void *>(module);		
				break;
			}
			else
			{
				dlclose(module);
			}
		}
	}
}

osd_dynamic_bind_base::~osd_dynamic_bind_base()
{
	if (m_module != nullptr)
		dlclose(module);
}
