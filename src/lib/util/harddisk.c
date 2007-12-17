/***************************************************************************

    Generic MAME hard disk implementation, with differencing files

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "harddisk.h"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _hard_disk_file
{
	chd_file *			chd;				/* CHD file */
	hard_disk_info 		info;				/* hard disk info */
	UINT32				hunksectors;		/* sectors per hunk */
	UINT32				cachehunk;			/* which hunk is cached */
	UINT8 *				cache;				/* cache of the current hunk */
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
	char metadata[256];
	chd_error err;

	/* punt if no CHD */
	if (chd == NULL)
		return NULL;

	/* read the hard disk metadata */
	err = chd_get_metadata(chd, HARD_DISK_METADATA_TAG, 0, metadata, sizeof(metadata), NULL, NULL);
	if (err != CHDERR_NONE)
		return NULL;

	/* parse the metadata */
	if (sscanf(metadata, HARD_DISK_METADATA_FORMAT, &cylinders, &heads, &sectors, &sectorbytes) != 4)
		return NULL;

	/* allocate memory for the hard disk file */
	file = malloc(sizeof(hard_disk_file));
	if (file == NULL)
		return NULL;

	/* fill in the data */
	file->chd = chd;
	file->info.cylinders = cylinders;
	file->info.heads = heads;
	file->info.sectors = sectors;
	file->info.sectorbytes = sectorbytes;
	file->hunksectors = chd_get_header(chd)->hunkbytes / file->info.sectorbytes;
	file->cachehunk = -1;

	/* allocate a cache */
	file->cache = malloc(chd_get_header(chd)->hunkbytes);
	if (file->cache == NULL)
	{
		free(file);
		return NULL;
	}

	return file;
}


/*-------------------------------------------------
    hard_disk_close - close a hard disk handle
-------------------------------------------------*/

void hard_disk_close(hard_disk_file *file)
{
	/* free the cache */
	if (file->cache != NULL)
		free(file->cache);
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

hard_disk_info *hard_disk_get_info(hard_disk_file *file)
{
	return &file->info;
}


/*-------------------------------------------------
    hard_disk_read - read sectors from a hard
    disk
-------------------------------------------------*/

UINT32 hard_disk_read(hard_disk_file *file, UINT32 lbasector, void *buffer)
{
	UINT32 hunknum = lbasector / file->hunksectors;
	UINT32 sectoroffs = lbasector % file->hunksectors;

	/* if we haven't cached this hunk, read it now */
	if (file->cachehunk != hunknum)
	{
		chd_error err = chd_read(file->chd, hunknum, file->cache);
		if (err != CHDERR_NONE)
			return 0;
		file->cachehunk = hunknum;
	}

	/* copy out the requested sector */
	memcpy(buffer, &file->cache[sectoroffs * file->info.sectorbytes], file->info.sectorbytes);
	return 1;
}


/*-------------------------------------------------
    hard_disk_write - write  sectors to a hard
    disk
-------------------------------------------------*/

UINT32 hard_disk_write(hard_disk_file *file, UINT32 lbasector, const void *buffer)
{
	UINT32 hunknum = lbasector / file->hunksectors;
	UINT32 sectoroffs = lbasector % file->hunksectors;
	chd_error err;

	/* if we haven't cached this hunk, read it now */
	if (file->cachehunk != hunknum)
	{
		err = chd_read(file->chd, hunknum, file->cache);
		if (err != CHDERR_NONE)
			return 0;
		file->cachehunk = hunknum;
	}

	/* copy in the requested data */
	memcpy(&file->cache[sectoroffs * file->info.sectorbytes], buffer, file->info.sectorbytes);

	/* write it back out */
	err = chd_write(file->chd, hunknum, file->cache);
	return (err == CHDERR_NONE) ? 1 : 0;
}
