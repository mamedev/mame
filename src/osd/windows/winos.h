//============================================================
//
//  winos.h - Win32 OS specific low level code
//
//============================================================

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