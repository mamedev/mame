// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    hardisk.c

    Generic MAME hard disk implementation, with differencing files

***************************************************************************/

#include "harddisk.h"

#include "chd.h"
#include "ioprocs.h"

#include "osdcore.h"

#include <cstdlib>
#include <tuple>


/*-------------------------------------------------
    constructor - open a hard disk handle,
    given a chd_file
-------------------------------------------------*/

hard_disk_file::hard_disk_file(chd_file *_chd)
{
	chd = _chd;
	fhandle = nullptr;
	fileoffset = 0;

	std::string metadata;
	std::error_condition err;

	/* punt if no CHD */
	if (chd == nullptr)
		throw nullptr;

	/* read the hard disk metadata */
	err = _chd->read_metadata(HARD_DISK_METADATA_TAG, 0, metadata);
	if (err)
		throw nullptr;

	/* parse the metadata */
	if (sscanf(metadata.c_str(), HARD_DISK_METADATA_FORMAT, &hdinfo.cylinders, &hdinfo.heads, &hdinfo.sectors, &hdinfo.sectorbytes) != 4)
		throw nullptr;
}

hard_disk_file::hard_disk_file(util::random_read_write &corefile, uint32_t skipoffs)
{
	// bail if getting the file length fails
	std::uint64_t length;
	if (corefile.length(length))
		throw nullptr;

	chd = nullptr;
	fhandle = &corefile;
	hdinfo.sectorbytes = 512;
	hdinfo.cylinders = 0;
	hdinfo.heads = 0;
	hdinfo.sectors = 0;
	fileoffset = skipoffs;

	// attempt to guess geometry in case this is an ATA situation
	for (uint32_t totalsectors = (length - skipoffs) / hdinfo.sectorbytes; ; totalsectors++)
		for (uint32_t cursectors = 63; cursectors > 1; cursectors--)
			if (totalsectors % cursectors == 0)
			{
				uint32_t totalheads = totalsectors / cursectors;
				for (uint32_t curheads = 16; curheads > 1; curheads--)
					if (totalheads % curheads == 0)
					{
						hdinfo.cylinders = totalheads / curheads;
						hdinfo.heads = curheads;
						hdinfo.sectors = cursectors;
						osd_printf_verbose("Guessed CHS of %d/%d/%d\n", hdinfo.cylinders, hdinfo.heads, hdinfo.sectors);
						return;
					}
			}
}


/*-------------------------------------------------
    destructor - close a hard disk handle
-------------------------------------------------*/

hard_disk_file::~hard_disk_file()
{
	if (fhandle)
		fhandle->flush();
}


/*-------------------------------------------------
    read - read sectors from a hard disk
-------------------------------------------------*/

/**
 * @fn  bool read(uint32_t lbasector, void *buffer)
 *
 * @brief   Hard disk read.
 *
 * @param   lbasector       The sector number (Linear Block Address) to read.
 * @param   buffer          The buffer where the hard disk data will be placed.
 *
 * @return  True if the operation succeeded
 */

bool hard_disk_file::read(uint32_t lbasector, void *buffer)
{
	if (chd)
	{
		std::error_condition err = chd->read_units(lbasector, buffer);
		return !err;
	}
	else
	{
		size_t actual = 0;
		std::error_condition err = fhandle->seek(fileoffset + (lbasector * hdinfo.sectorbytes), SEEK_SET);
		if (!err)
			std::tie(err, actual) = util::read(*fhandle, buffer, hdinfo.sectorbytes);
		return !err && (actual == hdinfo.sectorbytes);
	}
}


/*-------------------------------------------------
    write - write  sectors to a hard disk
-------------------------------------------------*/

/**
 * @fn  bool write(uint32_t lbasector, const void *buffer)
 *
 * @brief   Hard disk write.
 *
 * @param   lbasector       The sector number (Linear Block Address) to write.
 * @param   buffer          The buffer containing the data to write.
 *
 * @return  True if the operation succeeded
 */

bool hard_disk_file::write(uint32_t lbasector, const void *buffer)
{
	if (chd)
	{
		std::error_condition err = chd->write_units(lbasector, buffer);
		return !err;
	}
	else
	{
		size_t actual = 0;
		std::error_condition err = fhandle->seek(fileoffset + (lbasector * hdinfo.sectorbytes), SEEK_SET);
		if (!err)
			std::tie(err, actual) = util::write(*fhandle, buffer, hdinfo.sectorbytes);
		return !err;
	}
}


/*-------------------------------------------------
    set_block_size - sets the block size
    for a non-CHD-backed hard disk (a bare file).
-------------------------------------------------*/

/**
 * @fn  bool set_block_size(uint32_t blocksize)
 *
 * @brief   Hard disk set block size (works only for non-CHD-files)
 *
 * @param   blocksize       The block size of this hard disk, in bytes.
 *
 * @return  true on success, false on failure.  Failure means a CHD is in use and the CHD
 *          block size does not match the passed in size.  If a CHD is in use and the block
 *          sizes match, success is returned).
 */

bool hard_disk_file::set_block_size(uint32_t blocksize)
{
	if (chd)
	{
		// if the CHD block size matches our block size, we're OK.
		if (chd->unit_bytes() == blocksize)
		{
			return true;
		}

		// indicate failure, since we can't change the block size of a CHD.
		return false;
	}
	else
	{
		hdinfo.sectorbytes = blocksize;
		return true;
	}
}


std::error_condition hard_disk_file::get_inquiry_data(std::vector<uint8_t> &data) const
{
	if(chd)
		return chd->read_metadata(HARD_DISK_IDENT_METADATA_TAG, 0, data);
	else
		return std::error_condition(chd_file::error::METADATA_NOT_FOUND);
}

std::error_condition hard_disk_file::get_cis_data(std::vector<uint8_t> &data) const
{
	if(chd)
		return chd->read_metadata(PCMCIA_CIS_METADATA_TAG, 0, data);
	else
		return std::error_condition(chd_file::error::METADATA_NOT_FOUND);
}

std::error_condition hard_disk_file::get_disk_key_data(std::vector<uint8_t> &data) const
{
	if(chd)
		return chd->read_metadata(HARD_DISK_KEY_METADATA_TAG, 0, data);
	else
		return std::error_condition(chd_file::error::METADATA_NOT_FOUND);
}
