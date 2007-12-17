//============================================================
//
//  winutil.h - Win32 OSD core utility functions
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __WINUTIL__
#define __WINUTIL__

#include "osdcore.h"

// Shared code
file_error win_error_to_file_error(DWORD error);
osd_dir_entry_type win_attributes_to_entry_type(DWORD attributes);
BOOL win_is_gui_application(void);

#endif // __WINUTIL__
