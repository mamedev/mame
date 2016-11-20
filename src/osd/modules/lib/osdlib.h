// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  osdlib.h
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//  - Common low level routines
//  - Source files also provide the following from osdcore.h
//
//    - osd_ticks
//    - osd_sleep
//============================================================

#ifndef __OSDLIB__
#define __OSDLIB__

#include <string>
#include <type_traits>
#include <vector>
#include <memory>

/*-----------------------------------------------------------------------------
    osd_process_kill: kill the current process

    Parameters:

        None.

    Return value:

        None.
-----------------------------------------------------------------------------*/

void osd_process_kill(void);


/*-----------------------------------------------------------------------------
    osd_setenv: set environment variable

    Parameters:

        name  - name of environment variable
        value - value to write
        overwrite - overwrite if it exists

    Return value:

        0 on success
-----------------------------------------------------------------------------*/

int osd_setenv(const char *name, const char *value, int overwrite);


/*-----------------------------------------------------------------------------
    osd_get_clipboard_text: retrieves text from the clipboard

    Return value:

        the returned string needs to be free-ed!
-----------------------------------------------------------------------------*/

char *osd_get_clipboard_text(void);


/*-----------------------------------------------------------------------------
    dynamic_module: load functions from optional shared libraries

    Notes:

        - Supports Mac OS X, Unix and Windows (both desktop and Windows
          Store universal applications)
        - A symbol can be searched in a list of libraries (e.g. more
          revisions of a same library)
-----------------------------------------------------------------------------*/

namespace osd {
class dynamic_module
{
public:
	typedef std::unique_ptr<dynamic_module> ptr;

	static ptr open(std::vector<std::string> &&libraries);

	virtual ~dynamic_module() { };

	template <typename T>
	typename std::enable_if<std::is_pointer<T>::value, T>::type bind(char const *symbol)
	{
		return reinterpret_cast<T>(get_symbol_address(symbol));
	}

protected:
	typedef void (*generic_fptr_t)();

	virtual generic_fptr_t get_symbol_address(char const *symbol) = 0;
};

} // namespace osd

//=========================================================================================================================
// Dynamic API helpers. Useful in creating a singleton object that can be called with DYNAMIC_CALL macro for an entire API
// Usage for defining an API is below
//
// DYNAMIC_API_BEGIN(dxgi, "dxgi.dll")
//   DYNAMIC_API_FN(DWORD, WINAPI, CreateDXGIFactory1, REFIID, void**)
// DYNAMIC_API_END(dxgi)
//
// Calling then looks like: DYNAMIC_CALL(dxgi, CreateDXGIFactory1, p1, p2, etc)
//=========================================================================================================================

#if !defined(OSD_UWP)

#define DYNAMIC_API_BEGIN(apiname, ...) namespace osd { namespace dynamicapi { \
class apiname##_api { \
private: \
	osd::dynamic_module::ptr m_module = osd::dynamic_module::open( { __VA_ARGS__ } ); \
	static std::unique_ptr<apiname##_api> s_instance; \
	static std::once_flag s_once; \
public: \
	static apiname##_api &instance() { \
		std::call_once( apiname##_api::s_once, [](std::unique_ptr<apiname##_api> &inst) { \
			inst = std::make_unique<apiname##_api>(); }, std::ref(s_instance)); \
		return *apiname##_api::s_instance.get(); }

#define DYNAMIC_API_FN(ret, conv, apifunc, ...) ret(conv *m_##apifunc##_pfn)( __VA_ARGS__ ) = m_module->bind<ret(conv *)( __VA_ARGS__ )>(#apifunc);

#define DYNAMIC_API_END(apiname) }; \
std::once_flag apiname##_api::s_once; \
std::unique_ptr<apiname##_api> apiname##_api::s_instance = nullptr; }}

#define DYNAMIC_CALL(apiname, fname, ...) (*osd::dynamicapi::apiname##_api::instance().m_##fname##_pfn) ( __VA_ARGS__ )
#define DYNAMIC_API_TEST(apiname, fname) (osd::dynamicapi::apiname##_api::instance().m_##fname##_pfn != nullptr)

#else

#define DYNAMIC_API_BEGIN(apiname, ...)
#define DYNAMIC_API_FN(ret, conv, apifunc, ...)
#define DYNAMIC_API_END(apiname)

#define DYNAMIC_CALL(apiname, fname, ...) fname( __VA_ARGS__ )
#define DYNAMIC_API_TEST(apiname, fname) (true)

#endif

#endif  /* __OSDLIB__ */
