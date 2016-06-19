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
//    - osd_malloc
//    - osd_malloc_array
//    - osd_free
//============================================================

#ifndef __OSDLIB__
#define __OSDLIB__

#include <string>
#include <type_traits>
#include <vector>

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

        the returned string needs to be osd_free()-ed!
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

#endif  /* __OSDLIB__ */
