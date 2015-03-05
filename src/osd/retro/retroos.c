#include <stdlib.h>
#include <unistd.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/time.h>
#endif

#include <time.h>
#include <sys/stat.h>

// MAME headers
#include "osdcore.h"


//============================================================
//  osd_get_clipboard_text
//============================================================

char *osd_get_clipboard_text(void)
{
	return NULL;
}
