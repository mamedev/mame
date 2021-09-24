// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    hardisk.c

    Generic MAME hard disk implementation, with differencing files

***************************************************************************/

#include "harddisk.h"

#include "corefile.h"

#include <cstdlib>


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct hard_disk_file
{
	chd_file *          chd;                /* CHD file */
	util::core_file     *fhandle;           /* core_file if not a CHD */
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
	std::error_condition err;

	/* punt if no CHD */
	if (chd == nullptr)
		return nullptr;

	/* read the hard disk metadata */
	err = chd->read_metadata(HARD_DISK_METADATA_TAG, 0, metadata);
	if (err)
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
	file->fhandle = nullptr;
	file->info.cylinders = cylinders;
	file->info.heads = heads;
	file->info.sectors = sectors;
	file->info.sectorbytes = sectorbytes;
	file->info.fileoffset = 0;
	return file;
}

hard_disk_file *hard_disk_open(util::core_file &corefile, uint32_t skipoffs)
{
	hard_disk_file *file;

	/* allocate memory for the hard disk file */
	file = (hard_disk_file *)malloc(sizeof(hard_disk_file));
	if (file == nullptr)
		return nullptr;

	file->chd = nullptr;
	file->fhandle = &corefile;
	file->info.sectorbytes = 512;
	file->info.cylinders = 0;
	file->info.heads = 0;
	file->info.sectors = 0;
	file->info.fileoffset = skipoffs;

	// attempt to guess geometry in case this is an ATA situation
	for (uint32_t totalsectors = (corefile.size() - skipoffs) / file->info.sectorbytes; ; totalsectors++)
		for (uint32_t cursectors = 63; cursectors > 1; cursectors--)
			if (totalsectors % cursectors == 0)
			{
				uint32_t totalheads = totalsectors / cursectors;
				for (uint32_t curheads = 16; curheads > 1; curheads--)
					if (totalheads % curheads == 0)
					{
						file->info.cylinders = totalheads / curheads;
						file->info.heads = curheads;
						file->info.sectors = cursectors;
						osd_printf_verbose("Guessed CHS of %d/%d/%d\n", file->info.cylinders, file->info.heads, file->info.sectors);
						return file;
					}
			}

	return file;
}


/*-------------------------------------------------
    hard_disk_close - close a hard disk handle
-------------------------------------------------*/

void hard_disk_close(hard_disk_file *file)
{
	if (file->fhandle)
	{
		file->fhandle->flush();
	}

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
 * @param [in,out]  file    The hard disk file object to operate on.
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
 * @fn  uint32_t hard_disk_read(hard_disk_file *file, uint32_t lbasector, void *buffer)
 *
 * @brief   Hard disk read.
 *
 * @param [in,out]  file    The hard disk file object to operate on.
 * @param   lbasector       The sector number (Linear Block Address) to read.
 * @param   buffer          The buffer where the hard disk data will be placed.
 *
 * @return  An uint32_t.
 */

uint32_t hard_disk_read(hard_disk_file *file, uint32_t lbasector, void *buffer)
{
	if (file->chd)
	{
		std::error_condition err = file->chd->read_units(lbasector, buffer);
		return !err;
	}
	else
	{
		uint32_t actual = 0;
		file->fhandle->seek(file->info.fileoffset + (lbasector * file->info.sectorbytes), SEEK_SET);
		actual = file->fhandle->read(buffer, file->info.sectorbytes);
		return (actual == file->info.sectorbytes);
	}
}


/*-------------------------------------------------
    hard_disk_write - write  sectors to a hard
    disk
-------------------------------------------------*/

/**
 * @fn  uint32_t hard_disk_write(hard_disk_file *file, uint32_t lbasector, const void *buffer)
 *
 * @brief   Hard disk write.
 *
 * @param [in,out]  file    The hard disk file object to operate on.
 * @param   lbasector       The sector number (Linear Block Address) to write.
 * @param   buffer          The buffer containing the data to write.
 *
 * @return  An uint32_t.
 */

uint32_t hard_disk_write(hard_disk_file *file, uint32_t lbasector, const void *buffer)
{
	if (file->chd)
	{
		std::error_condition err = file->chd->write_units(lbasector, buffer);
		return !err;
	}
	else
	{
		uint32_t actual = 0;
		file->fhandle->seek(file->info.fileoffset + (lbasector * file->info.sectorbytes), SEEK_SET);
		actual = file->fhandle->write(buffer, file->info.sectorbytes);
		return (actual == file->info.sectorbytes);
	}
}

/*-------------------------------------------------
    hard_disk_set_block_size - sets the block size
    for a non-CHD-backed hard disk (a bare file).
-------------------------------------------------*/

/**
 * @fn  bool hard_disk_set_block_size(hard_disk_file *file, uint32_t blocksize)
 *
 * @brief   Hard disk set block size (works only for non-CHD-files)
 *
 * @param [in,out]  file    The hard_disk_file object to operate on.
 * @param   blocksize       The block size of this hard disk, in bytes.
 *
 * @return  true on success, false on failure.  Failure means a CHD is in use and the CHD
 *          block size does not match the passed in size.  If a CHD is in use and the block
 *          sizes match, success is returned).
 */

bool hard_disk_set_block_size(hard_disk_file *file, uint32_t blocksize)
{
	if (file->chd)
	{
		// if the CHD block size matches our block size, we're OK.
		if (file->chd->unit_bytes() == blocksize)
		{
			return true;
		}

		// indicate failure, since we can't change the block size of a CHD.
		return false;
	}
	else
	{
		file->info.sectorbytes = blocksize;
	}
	return true;
}
