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
-----------------------------------------------------------------------------*/
std::string osd_get_clipboard_text(void);

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

//=========================================================================================================
// Dynamic API helpers. Useful in creating a class members that expose dynamically bound API functions.
//
// OSD_DYNAMIC_API(dxgi, "dxgi.dll")
// DYNAMIC_API_FN(dxgi, DWORD, WINAPI, CreateDXGIFactory1, REFIID, void**)
//
// Calling then looks like: DYNAMIC_CALL(CreateDXGIFactory1, p1, p2, etc)
//=========================================================================================================

#if !defined(OSD_UWP)

#define OSD_DYNAMIC_API(apiname, ...) osd::dynamic_module::ptr m_##apiname##module = osd::dynamic_module::open( { __VA_ARGS__ } )
#define OSD_DYNAMIC_API_FN(apiname, ret, conv, fname, ...) ret(conv *m_##fname##_pfn)( __VA_ARGS__ ) = m_##apiname##module->bind<ret(conv *)( __VA_ARGS__ )>(#fname)
#define OSD_DYNAMIC_CALL(fname, ...) (*m_##fname##_pfn) ( __VA_ARGS__ )
#define OSD_DYNAMIC_API_TEST(fname) (m_##fname##_pfn != nullptr)

#else

#define OSD_DYNAMIC_API(apiname, ...)
#define OSD_DYNAMIC_API_FN(apiname, ret, conv, fname, ...)
#define OSD_DYNAMIC_CALL(fname, ...) fname( __VA_ARGS__ )
#define OSD_DYNAMIC_API_TEST(fname) (true)

#endif

#endif  /* __OSDLIB__ */
