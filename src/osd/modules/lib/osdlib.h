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

/*-----------------------------------------------------------------------------
    osd_num_processors: return the number of processors

    Parameters:

        None.

    Return value:

        Number of processors
-----------------------------------------------------------------------------*/
int osd_get_num_processors(void);

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

#endif  /* __OSDLIB__ */
