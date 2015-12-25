// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    hardisk.c

    Generic MAME hard disk implementation, with differencing files

***************************************************************************/

#include <assert.h>

#include "harddisk.h"

#include <stdlib.h>


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct hard_disk_file
{
	chd_file *          chd;                /* CHD file */
	hard_disk_info      info;               /* hard disk info */
};



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    hard_disk_open - open a hard disk handle,
    given a chd_file
-------------------------------------------------*/

hard_disk_file *hard_disk_open(chd_file *chd)
{
	int cylinders, heads, sectors, sectorbytes;
	hard_disk_file *file;
	std::string metadata;
	chd_error err;

	/* punt if no CHD */
	if (chd == nullptr)
		return nullptr;

	/* read the hard disk metadata */
	err = chd->read_metadata(HARD_DISK_METADATA_TAG, 0, metadata);
	if (err != CHDERR_NONE)
		return nullptr;

	/* parse the metadata */
	if (sscanf(metadata.c_str(), HARD_DISK_METADATA_FORMAT, &cylinders, &heads, &sectors, &sectorbytes) != 4)
		return nullptr;

	/* allocate memory for the hard disk file */
	file = (hard_disk_file *)malloc(sizeof(hard_disk_file));
	if (file == nullptr)
		return nullptr;

	/* fill in the data */
	file->chd = chd;
	file->info.cylinders = cylinders;
	file->info.heads = heads;
	file->info.sectors = sectors;
	file->info.sectorbytes = sectorbytes;
	return file;
}


/*-------------------------------------------------
    hard_disk_close - close a hard disk handle
-------------------------------------------------*/

void hard_disk_close(hard_disk_file *file)
{
	free(file);
}


/*-------------------------------------------------
    hard_disk_get_chd - get a handle to a CHD
    from a hard disk
-------------------------------------------------*/

chd_file *hard_disk_get_chd(hard_disk_file *file)
{
	return file->chd;
}


/*-------------------------------------------------
    hard_disk_get_info - return information about
    a hard disk
-------------------------------------------------*/

/**
 * @fn  hard_disk_info *hard_disk_get_info(hard_disk_file *file)
 *
 * @brief   Hard disk get information.
 *
 * @param [in,out]  file    If non-null, the file.
 *
 * @return  null if it fails, else a hard_disk_info*.
 */

hard_disk_info *hard_disk_get_info(hard_disk_file *file)
{
	return &file->info;
}


/*-------------------------------------------------
    hard_disk_read - read sectors from a hard
    disk
-------------------------------------------------*/

/**
 * @fn  UINT32 hard_disk_read(hard_disk_file *file, UINT32 lbasector, void *buffer)
 *
 * @brief   Hard disk read.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   lbasector       The lbasector.
 * @param [in,out]  buffer  If non-null, the buffer.
 *
 * @return  An UINT32.
 */

UINT32 hard_disk_read(hard_disk_file *file, UINT32 lbasector, void *buffer)
{
	chd_error err = file->chd->read_units(lbasector, buffer);
	return (err == CHDERR_NONE);
}


/*-------------------------------------------------
    hard_disk_write - write  sectors to a hard
    disk
-------------------------------------------------*/

/**
 * @fn  UINT32 hard_disk_write(hard_disk_file *file, UINT32 lbasector, const void *buffer)
 *
 * @brief   Hard disk write.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   lbasector       The lbasector.
 * @param   buffer          The buffer.
 *
 * @return  An UINT32.
 */

UINT32 hard_disk_write(hard_disk_file *file, UINT32 lbasector, const void *buffer)
{
	chd_error err = file->chd->write_units(lbasector, buffer);
	return (err == CHDERR_NONE);
}
