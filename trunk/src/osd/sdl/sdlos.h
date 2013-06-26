//============================================================
//
//  sdlos.h - SDLMAME OS dependent functions
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef __SDLOS__
#define __SDLOS__

/*-----------------------------------------------------------------------------
    osd_num_processors: return the number of processors

    Parameters:

        None.

    Return value:

        Number of processors
-----------------------------------------------------------------------------*/
int osd_get_num_processors(void);


/*-----------------------------------------------------------------------------
    osd_getenv: return pointer to environment variable

    Parameters:

        name  - name of environment variable

    Return value:

        pointer to value
-----------------------------------------------------------------------------*/
char *osd_getenv(const char *name);

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

#endif  /* __SDLOS__ */
