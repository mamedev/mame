// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pdynlib.c
 *
 */

#include "pdynlib.h"
#ifdef _WIN32
#include "windows.h"
#else
#include <dlfcn.h>
#endif

pdynlib::pdynlib(const pstring libname)
: m_isLoaded(false), m_lib(nullptr)
{
#ifdef _WIN32
	//fprintf(stderr, "win: loading <%s>\n", libname.cstr());
	if (libname != "")
		m_lib = LoadLibrary(libname.cstr());
	else
		m_lib = GetModuleHandle(nullptr);
	if (m_lib != nullptr)
		m_isLoaded = true;
	//else
	//  fprintf(stderr, "win: library <%s> not found!\n", libname.cstr());
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

pdynlib::pdynlib(const pstring path, const pstring libname)
: m_isLoaded(false), m_lib(nullptr)
{
	//  printf("win: loading <%s>\n", libname.cstr());
#ifdef _WIN32
	if (libname != "")
		m_lib = LoadLibrary(libname.cstr());
	else
		m_lib = GetModuleHandle(nullptr);
	if (m_lib != nullptr)
		m_isLoaded = true;
	else
	{
		//printf("win: library <%s> not found!\n", libname.cstr());
	}
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

pdynlib::~pdynlib()
{
	if (m_lib != nullptr)
	{
#ifdef _WIN32
#else
		dlclose(m_lib);
		//printf("Closed %s\n", dlerror());
#endif
	}
}

bool pdynlib::isLoaded() const
{
	return m_isLoaded;
}

void *pdynlib::getsym_p(const pstring name)
{
#ifdef _WIN32
	return (void *) GetProcAddress((HMODULE) m_lib, name.cstr());
#else
	return dlsym(m_lib, name.cstr());
#endif
}
