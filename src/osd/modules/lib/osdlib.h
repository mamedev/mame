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
    osd_dynamic_bind: load functions from optional shared libraries

    Notes:

        - Supports Mac OS X, Unix and Windows (both desktop and Windows
          Store universal applications)
        - A symbol can be searched in a list of libraries (e.g. more
          revisions of a same library)
-----------------------------------------------------------------------------*/

class osd_dynamic_bind_base
{
public:
	osd_dynamic_bind_base(const char *symbol, const std::vector<std::wstring> libraries);
	~osd_dynamic_bind_base();

protected:
	void *m_module;
	void *m_function;
};

template<typename TFunctionPtr>
class osd_dynamic_bind : private osd_dynamic_bind_base
{
public:
	// constructors which looks up the function
	osd_dynamic_bind(const char *symbol, const std::vector<std::wstring> libraries)
		: osd_dynamic_bind_base(symbol, libraries) { }

	// bool to test if the function is nullptr or not
	operator bool() const { return (m_function != nullptr); }

	// dereference to get the underlying pointer
	TFunctionPtr operator *() const { return reinterpret_cast<TFunctionPtr>(m_function); }
};

#endif  /* __OSDLIB__ */
