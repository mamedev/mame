
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#include <memoryapi.h>
#else
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#endif

// MAME headers
#include "osdlib.h"
#include "osdcomm.h"
#include "osdcore.h"
#include "strconv.h"

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
#if defined(_WIN32)
   char *buf;
   int result;

   if (!overwrite)
   {
      if (osd_getenv(name) != NULL)
         return 0;
   }
   buf = (char *) malloc(strlen(name)+strlen(value)+2);
   sprintf(buf, "%s=%s", name, value);
   result = putenv(buf);

   /* will be referenced by environment
    * Therefore it is not freed here
    */

   return result;
#else
   return setenv(name, value, overwrite);
#endif
}

//============================================================
//  osd_process_kill
//============================================================

void osd_process_kill(void)
{
#ifndef _WIN32
    kill(getpid(), SIGKILL);
#else
    TerminateProcess(GetCurrentProcess(), -1);
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
#if defined(_WIN32)
   return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
#else
	return (void *)mmap(0, size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
#endif
}

//============================================================
//  osd_free_executable
//
//  frees memory allocated with osd_alloc_executable
//============================================================

void osd_free_executable(void *ptr, size_t size)
{
#if defined(_WIN32)
   VirtualFree(ptr, 0, MEM_RELEASE);
#else
	munmap(ptr, size);
#endif
}

//============================================================
//  osd_break_into_debugger
//============================================================

void osd_break_into_debugger(const char *message)
{
	#ifdef MAME_DEBUG
	printf("MAME exception: %s\n", message);
	printf("Attempting to fall into debugger\n");
	kill(getpid(), SIGTRAP);
	#else
	printf("Ignoring MAME exception: %s\n", message);
	#endif
}

//============================================================
//  osd_get_clipboard_text
//============================================================

std::string osd_get_clipboard_text(void)
{
	return NULL;
}


//============================================================
//  osd_getpid
//============================================================

int osd_getpid(void)
{
#if defined(_WIN32)
	return GetCurrentProcessId();
#else
	return getpid();
#endif
}

//============================================================
//  osd_dynamic_bind
//============================================================
#if defined(_WIN32)
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
// for classic desktop applications
#define load_library(filename) LoadLibrary(filename)
#else
// for Windows Store universal applications
#define load_library(filename) LoadPackagedLibrary(filename, 0)
#endif
#endif
namespace osd {

namespace {

#if defined(_WIN32)
class dynamic_module_win32_impl : public dynamic_module
{
public:
	dynamic_module_win32_impl(std::vector<std::string> &&libraries) : m_libraries(std::move(libraries))
	{
	}

	virtual ~dynamic_module_win32_impl() override
	{
		if (m_module)
			FreeLibrary(m_module);
	}

protected:
	virtual generic_fptr_t get_symbol_address(char const *symbol) override
	{
		/*
		 * given a list of libraries, if a first symbol is successfully loaded from
		 * one of them, all additional symbols will be loaded from the same library
		 */
		if (m_module)
			return reinterpret_cast<generic_fptr_t>(GetProcAddress(m_module, symbol));

		for (auto const &library : m_libraries)
		{
			osd::text::tstring const tempstr = osd::text::to_tstring(library);
			HMODULE const module = load_library(tempstr.c_str());

			if (module)
			{
				auto const function = reinterpret_cast<generic_fptr_t>(GetProcAddress(module, symbol));

				if (function)
				{
					m_module = module;
					return function;
				}
				else
				{
					FreeLibrary(module);
				}
			}
		}

		return nullptr;
	}

private:
	std::vector<std::string> m_libraries;
	HMODULE                  m_module = nullptr;
};
#else
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
#endif
} // anonymous namespace

bool invalidate_instruction_cache(void const *start, std::size_t size)
{
#if defined(_WIN32)
	return FlushInstructionCache(GetCurrentProcess(), start, size) != 0;
#else
#if !defined(SDLMAME_EMSCRIPTEN) && !defined(TARGET_OS_IPHONE)
	char const *const begin(reinterpret_cast<char const *>(start));
	char const *const end(begin + size);
	__builtin___clear_cache(const_cast<char *>(begin), const_cast<char *>(end));
#endif
	return true;
#endif
}

void *virtual_memory_allocation::do_alloc(std::initializer_list<std::size_t> blocks, unsigned intent, std::size_t &size, std::size_t &page_size)
{
#if defined(_WIN32)
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	SIZE_T s(0);
	for (std::size_t b : blocks)
		s += (b + info.dwPageSize - 1) / info.dwPageSize;
	s *= info.dwPageSize;
	if (!s)
		return nullptr;
	LPVOID const result(VirtualAlloc(nullptr, s, MEM_COMMIT, PAGE_NOACCESS));
	if (result)
	{
		size = s;
		page_size = info.dwPageSize;
	}
	return result;
#else
	long const p(sysconf(_SC_PAGE_SIZE));
	if (0 >= p)
		return nullptr;
	std::size_t s(0);
	for (std::size_t b : blocks)
		s += (b + p - 1) / p;
	s *= p;
	if (!s)
		return nullptr;
#if defined(SDLMAME_BSD) || defined(SDLMAME_MACOSX) || defined(SDLMAME_EMSCRIPTEN)
	int const fd(-1);
#else
	// TODO: portable applications are supposed to use -1 for anonymous mappings - detect whatever requires 0 specifically
	int const fd(0);
#endif
	void *const result(mmap(nullptr, s, PROT_NONE, MAP_ANON | MAP_SHARED, fd, 0));
	if (result == (void *)-1)
		return nullptr;
	size = s;
	page_size = p;
	return result;
#endif
}

void virtual_memory_allocation::do_free(void *start, std::size_t size)
{
#if defined(_WIN32)
	VirtualFree(start, 0, MEM_RELEASE);
#else
	munmap(reinterpret_cast<char *>(start), size);
#endif
}

bool virtual_memory_allocation::do_set_access(void *start, std::size_t size, unsigned access)
{
#if defined(_WIN32)
	DWORD p, o;
	if (access & EXECUTE)
		p = (access & WRITE) ? PAGE_EXECUTE_READWRITE : (access & READ) ? PAGE_EXECUTE_READ : PAGE_EXECUTE;
	else
		p = (access & WRITE) ? PAGE_READWRITE : (access & READ) ? PAGE_READONLY : PAGE_NOACCESS;
	return VirtualProtect(start, size, p, &o) != 0;
#else
	int prot((NONE == access) ? PROT_NONE : 0);
	if (access & READ)
		prot |= PROT_READ;
	if (access & WRITE)
		prot |= PROT_WRITE;
	if (access & EXECUTE)
		prot |= PROT_EXEC;
	return mprotect(reinterpret_cast<char *>(start), size, prot) == 0;
#endif
}

dynamic_module::ptr dynamic_module::open(std::vector<std::string> &&names)
{
#if defined(_WIN32)
	return std::make_unique<dynamic_module_win32_impl>(std::move(names));
#else
	return std::make_unique<dynamic_module_posix_impl>(std::move(names));
#endif
}

} // namespace osd