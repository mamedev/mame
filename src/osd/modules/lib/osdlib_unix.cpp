// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  osdlib_unix.cpp - OS specific low level code for POSIX-like systems
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// MAME headers
#include "osdcore.h"
#include "osdlib.h"

#ifdef SDLMAME_SDL3
#include <SDL3/SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <memory>
#include <string_view>

#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>


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

void osd_process_kill()
{
	kill(getpid(), SIGKILL);
}


//============================================================
//  osd_break_into_debugger
//============================================================

void osd_break_into_debugger(const char *message)
{
#if defined(__linux__)
	bool do_break = false;
	FILE *const f = std::fopen("/proc/self/status", "r");
	if (f)
	{
		using namespace std::literals;

		std::string_view const tag = "TracerPid:\t"sv;
		char buf[128];
		bool ignore = false;
		while (std::fgets(buf, std::size(buf), f))
		{
			// ignore excessively long lines
			auto const len = strnlen(buf, std::size(buf));
			bool const noeol = !len || ('\n' != buf[len - 1]);
			if (ignore || noeol)
			{
				ignore = noeol;
				continue;
			}

			if (!std::strncmp(buf, tag.data(), tag.length()))
			{
				long tpid;
				if ((std::sscanf(buf + tag.length(), "%ld", &tpid) == 1) && (0 != tpid))
					do_break = true;
				break;
			}
		}
		std::fclose(f);
	}
#elif defined(MAME_DEBUG)
	bool const do_break = true;
#else
	bool const do_break = false;
#endif
	if (do_break)
	{
		printf("MAME exception: %s\n", message);
		printf("Attempting to fall into debugger\n");
		kill(getpid(), SIGTRAP);
	}
	else
	{
		printf("Ignoring MAME exception: %s\n", message);
	}
}


//============================================================
//  osd_get_cache_line_size
//============================================================

std::pair<std::error_condition, unsigned> osd_get_cache_line_size() noexcept
{
#if defined(__linux__)
	FILE *const f = std::fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
	if (!f)
		return std::make_pair(std::error_condition(errno, std::generic_category()), 0U);

	unsigned result = 0;
	auto const cnt = std::fscanf(f, "%u", &result);
	std::fclose(f);
	if (1 == cnt)
		return std::make_pair(std::error_condition(), result);
	else
		return std::make_pair(std::errc::io_error, 0U);
#else // defined(__linux__)
	return std::make_pair(std::errc::not_supported, 0U);
#endif
}


#ifdef SDLMAME_ANDROID
std::string osd_get_clipboard_text() noexcept
{
	return std::string();
}

std::error_condition osd_set_clipboard_text(std::string_view text) noexcept
{
	return std::errc::io_error; // TODO: better error code?
}
#else
//============================================================
//  osd_get_clipboard_text
//============================================================

std::string osd_get_clipboard_text() noexcept
{
	// TODO: better error handling
	std::string result;

	if (SDL_HasClipboardText())
	{
		char *const temp = SDL_GetClipboardText();
		if (temp)
		{
			try
			{
				result.assign(temp);
			}
			catch (std::bad_alloc const &)
			{
			}
			SDL_free(temp);
		}
	}
	return result;
}


//============================================================
//  osd_set_clipboard_text
//============================================================

std::error_condition osd_set_clipboard_text(std::string_view text) noexcept
{
	try
	{
		std::string const clip(text); // need to do this to ensure there's a terminating NUL for SDL
		#ifdef SDLMAME_SDL3
		if (!SDL_SetClipboardText(clip.c_str()))
		#else
		if (0 > SDL_SetClipboardText(clip.c_str()))
		#endif
		{
			// SDL_GetError returns a message, can't really convert it to an error condition
			return std::errc::io_error; // TODO: better error code?
		}

		return std::error_condition();
	}
	catch (std::bad_alloc const &)
	{
		return std::errc::not_enough_memory;
	}
}
#endif

//============================================================
//  osd_getpid
//============================================================

int osd_getpid() noexcept
{
	return getpid();
}


namespace osd {

namespace {

class dynamic_module_posix_impl : public dynamic_module
{
public:
	dynamic_module_posix_impl(std::vector<std::string> &&libraries) : m_libraries(std::move(libraries))
	{
	}

	virtual ~dynamic_module_posix_impl() override
	{
		if (m_module)
			dlclose(m_module);
	}

protected:
	virtual generic_fptr_t get_symbol_address(char const *symbol) override
	{
		/*
		 * given a list of libraries, if a first symbol is successfully loaded from
		 * one of them, all additional symbols will be loaded from the same library
		 */
		if (m_module)
			return reinterpret_cast<generic_fptr_t>(dlsym(m_module, symbol));

		for (auto const &library : m_libraries)
		{
			void *const module = dlopen(library.c_str(), RTLD_LAZY);

			if (module != nullptr)
			{
				generic_fptr_t const function = reinterpret_cast<generic_fptr_t>(dlsym(module, symbol));

				if (function)
				{
					m_module = module;
					return function;
				}
				else
				{
					dlclose(module);
				}
			}
		}

		return nullptr;
	}

private:
	std::vector<std::string> m_libraries;
	void *                   m_module = nullptr;
};

} // anonymous namespace


bool invalidate_instruction_cache(void const *start, std::size_t size) noexcept
{
#if !defined(SDLMAME_EMSCRIPTEN)
	char const *const begin(reinterpret_cast<char const *>(start));
	char const *const end(begin + size);
	__builtin___clear_cache(const_cast<char *>(begin), const_cast<char *>(end));
#endif
	return true;
}


void *virtual_memory_allocation::do_alloc(std::initializer_list<std::size_t> blocks, unsigned intent, std::size_t &size, std::size_t &page_size) noexcept
{
	long const p(sysconf(_SC_PAGE_SIZE));
	if (0 >= p)
		return nullptr;
	std::size_t s(0);
	for (std::size_t b : blocks)
		s += (b + p - 1) / p;
	s *= p;
	if (!s)
		return nullptr;
#if defined __NetBSD__
	int req((NONE == intent) ? PROT_NONE : 0);
	if (intent & READ)
		req |= PROT_READ;
	if (intent & WRITE)
		req |= PROT_WRITE;
	if (intent & EXECUTE)
		req |= PROT_EXEC;
	int const prot(PROT_MPROTECT(req));
#else
	int const prot(PROT_NONE);
#endif
#if defined(SDLMAME_BSD) || defined(SDLMAME_MACOSX) || defined(SDLMAME_EMSCRIPTEN)
	int const fd(-1);
#else
	// TODO: portable applications are supposed to use -1 for anonymous mappings - detect whatever requires 0 specifically
	int const fd(0);
#endif
	void *const result(mmap(nullptr, s, prot, MAP_ANON | MAP_SHARED, fd, 0));
	if (result == (void *)-1)
		return nullptr;
	size = s;
	page_size = p;
	return result;
}

void virtual_memory_allocation::do_free(void *start, std::size_t size) noexcept
{
	munmap(reinterpret_cast<char *>(start), size);
}

bool virtual_memory_allocation::do_set_access(void *start, std::size_t size, unsigned access) noexcept
{
	int prot((NONE == access) ? PROT_NONE : 0);
	if (access & READ)
		prot |= PROT_READ;
	if (access & WRITE)
		prot |= PROT_WRITE;
	if (access & EXECUTE)
		prot |= PROT_EXEC;
	return mprotect(reinterpret_cast<char *>(start), size, prot) == 0;
}


dynamic_module::ptr dynamic_module::open(std::vector<std::string> &&names)
{
	return std::make_unique<dynamic_module_posix_impl>(std::move(names));
}

} // namespace osd
