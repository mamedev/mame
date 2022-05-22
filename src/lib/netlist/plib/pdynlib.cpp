// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "pdynlib.h"

#ifdef _WIN32
#include "windows.h"
#else
#include <dlfcn.h>
#endif

#include <type_traits>

namespace plib
{
	using winapi_string = std::conditional<compile_info::unicode::value,
		pwstring, pu8string>::type;

dynlib::dynlib(const pstring &libname)
: m_lib(nullptr)
{
#ifdef _WIN32
	//fprintf(stderr, "win: loading <%s>\n", libname.c_str());
	if (!libname.empty())
		m_lib = LoadLibrary(winapi_string(putf8string(libname)).c_str());
	else
		m_lib = GetModuleHandle(nullptr);
#elif defined(__EMSCRIPTEN__)
	//no-op
#else
	//printf("loading <%s>\n", libname.c_str());
	if (!libname.empty())
		m_lib = dlopen(putf8string(libname).c_str(), RTLD_LAZY);
	else
		m_lib = dlopen(nullptr, RTLD_LAZY);
#endif
	if (m_lib != nullptr)
		set_loaded(true);
	//else
	//  printf("library <%s> not found: %s\n", libname.c_str(), dlerror());
	}

dynlib::dynlib([[maybe_unused]] const pstring &path, const pstring &libname)
: m_lib(nullptr)
{
	// FIXME: implement path search
	//  printf("win: loading <%s>\n", libname.c_str());
#ifdef _WIN32
	if (!libname.empty())
		m_lib = LoadLibrary(winapi_string(putf8string(libname)).c_str());
	else
		m_lib = GetModuleHandle(nullptr);
#elif defined(__EMSCRIPTEN__)
	//no-op
#else
	//printf("loading <%s>\n", libname.c_str());
	if (!libname.empty())
		m_lib = dlopen(putf8string(libname).c_str(), RTLD_LAZY);
	else
		m_lib = dlopen(nullptr, RTLD_LAZY);
#endif
	if (m_lib != nullptr)
		set_loaded(true);
	else
	{
		//printf("library <%s> not found!\n", libname.c_str());
	}
}

dynlib::~dynlib()
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

void *dynlib::getsym_p(const pstring &name) const noexcept
{
#ifdef _WIN32
	return (void *) GetProcAddress((HMODULE) m_lib, putf8string(name).c_str());
#else
	return dlsym(m_lib, putf8string(name).c_str());
#endif
}

} // namespace plib
