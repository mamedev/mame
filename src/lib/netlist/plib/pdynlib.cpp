// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * dynlib.c
 *
 */

#include "pdynlib.h"

#ifdef WIN32
#include "windows.h"
#include "palloc.h"

namespace plib {
CHAR *astring_from_utf8(const char *utf8string)
{
	WCHAR *wstring;
	int char_count;
	CHAR *result;

	// convert MAME string (UTF-8) to UTF-16
	char_count = MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, nullptr, 0);
	wstring = (WCHAR *)alloca(char_count * sizeof(*wstring));
	MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, wstring, char_count);

	// convert UTF-16 to "ANSI code page" string
	char_count = WideCharToMultiByte(CP_ACP, 0, wstring, -1, nullptr, 0, nullptr, nullptr);
	result = palloc_array<CHAR>(char_count);
	if (result != nullptr)
		WideCharToMultiByte(CP_ACP, 0, wstring, -1, result, char_count, nullptr, nullptr);

	return result;
}

WCHAR *wstring_from_utf8(const char *utf8string)
{
	int char_count;
	WCHAR *result;

	// convert MAME string (UTF-8) to UTF-16
	char_count = MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, nullptr, 0);
	result = palloc_array<WCHAR>(char_count);
	if (result != nullptr)
		MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, result, char_count);

	return result;
}
}

#ifdef UNICODE
#define tstring_from_utf8   plib::wstring_from_utf8
#else // !UNICODE
#define tstring_from_utf8   plib::astring_from_utf8
#endif // UNICODE

#else
#include <dlfcn.h>
#endif

namespace plib {
dynlib::dynlib(const pstring libname)
: m_isLoaded(false), m_lib(nullptr)
{
#ifdef WIN32
	//fprintf(stderr, "win: loading <%s>\n", libname.cstr());
	TCHAR *buffer = tstring_from_utf8(libname.cstr());
	if (libname != "")
		m_lib = LoadLibrary(buffer);
	else
		m_lib = GetModuleHandle(nullptr);
	if (m_lib != nullptr)
		m_isLoaded = true;
	//else
	//  fprintf(stderr, "win: library <%s> not found!\n", libname.cstr());
	pfree_array(buffer);
#else
	//printf("loading <%s>\n", libname.cstr());
	if (libname != "")
		m_lib = dlopen(libname.cstr(), RTLD_LAZY);
	else
		m_lib = dlopen(nullptr, RTLD_LAZY);
	if (m_lib != nullptr)
		m_isLoaded = true;
	//else
	//  printf("library <%s> not found: %s\n", libname.cstr(), dlerror());
#endif
	}

dynlib::dynlib(const pstring path, const pstring libname)
: m_isLoaded(false), m_lib(nullptr)
{
	//  printf("win: loading <%s>\n", libname.cstr());
#ifdef WIN32
	TCHAR *buffer = tstring_from_utf8(libname.cstr());
	if (libname != "")
		m_lib = LoadLibrary(buffer);
	else
		m_lib = GetModuleHandle(nullptr);
	if (m_lib != nullptr)
		m_isLoaded = true;
	else
	{
		//printf("win: library <%s> not found!\n", libname.cstr());
	}
	pfree_array(buffer);
#else
	//printf("loading <%s>\n", libname.cstr());
	if (libname != "")
		m_lib = dlopen(libname.cstr(), RTLD_LAZY);
	else
		m_lib = dlopen(nullptr, RTLD_LAZY);
	if (m_lib != nullptr)
		m_isLoaded = true;
	else
	{
		//printf("library <%s> not found!\n", libname.cstr());
	}
#endif
}

dynlib::~dynlib()
{
	if (m_lib != nullptr)
	{
#ifdef WIN32
#else
		dlclose(m_lib);
		//printf("Closed %s\n", dlerror());
#endif
	}
}

bool dynlib::isLoaded() const
{
	return m_isLoaded;
}

void *dynlib::getsym_p(const pstring name)
{
#ifdef WIN32
	return (void *) GetProcAddress((HMODULE) m_lib, name.cstr());
#else
	return dlsym(m_lib, name.cstr());
#endif
}

}
