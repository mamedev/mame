// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    harddisk.h

    Generic MAME hard disk implementation, with differencing files

***************************************************************************/

#ifndef MAME_LIB_UTIL_HARDDISK_H
#define MAME_LIB_UTIL_HARDDISK_H

#pragma once

#include "chd.h"
#include "utilfwd.h"

#include "osdcore.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct hard_disk_file;

struct hard_disk_info
{
	uint32_t          cylinders;
	uint32_t          heads;
	uint32_t          sectors;
	uint32_t          sectorbytes;
	uint32_t          fileoffset;       // offset in the file where the HDD image starts.  not valid for CHDs.
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

hard_disk_file *hard_disk_open(chd_file *chd);
hard_disk_file *hard_disk_open(util::core_file &corefile, uint32_t skipoffs);

void hard_disk_close(hard_disk_file *file);

chd_file *hard_disk_get_chd(hard_disk_file *file);
hard_disk_info *hard_disk_get_info(hard_disk_file *file);

uint32_t hard_disk_read(hard_disk_file *file, uint32_t lbasector, void *buffer);
uint32_t hard_disk_write(hard_disk_file *file, uint32_t lbasector, const void *buffer);

#endif // MAME_LIB_UTIL_HARDDISK_H
