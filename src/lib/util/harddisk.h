/***************************************************************************

    harddisk.h

    Generic MAME hard disk implementation, with differencing files

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __HARDDISK_H__
#define __HARDDISK_H__

#include "osdcore.h"
#include "chd.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _hard_disk_file hard_disk_file;

typedef struct _hard_disk_info hard_disk_info;
struct _hard_disk_info
{
	UINT32			cylinders;
	UINT32			heads;
	UINT32			sectors;
	UINT32			sectorbytes;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

hard_disk_file *hard_disk_open(chd_file *chd);
void hard_disk_close(hard_disk_file *file);

chd_file *hard_disk_get_chd(hard_disk_file *file);
hard_disk_info *hard_disk_get_info(hard_disk_file *file);

UINT32 hard_disk_read(hard_disk_file *file, UINT32 lbasector, void *buffer);
UINT32 hard_disk_write(hard_disk_file *file, UINT32 lbasector, const void *buffer);

#endif	/* __HARDDISK_H__ */
