// license:BSD-3-Clause
// copyright-holders:Aaron Giles,R. Belmont
/***************************************************************************

    dvdrom.c

    Generic MAME DVD-ROM utilties - build IDE and SCSI DVD-ROMs on top of this

***************************************************************************/

#include "dvdrom.h"

#include "corestr.h"
#include "osdfile.h"
#include "strformat.h"

#include <cassert>
#include <cstdlib>



/**
 * @fn  constructor
 *
 * @brief   Open a dvdrom for a file.
 *
 * @param   inputfile   The inputfile.
 */

dvdrom_file::dvdrom_file(std::string_view inputfile)
{
	fhandle = nullptr;
}

/*-------------------------------------------------
    constructor - "open" a DVD-ROM file from an
    already-opened CHD file
-------------------------------------------------*/

/**
 * @fn  dvdrom_file *dvdrom_open(chd_file *chd)
 *
 * @brief   Queries if a given dvdrom open.
 *
 * @param [in,out]  chd If non-null, the chd.
 *
 * @return  null if it fails, else a dvdrom_file*.
 */

dvdrom_file::dvdrom_file(chd_file *_chd)
{
	chd = _chd;

	/* validate the CHD information */
	if (chd->hunk_bytes() != 2048)
		throw nullptr;
	if (chd->unit_bytes() != 2048)
		throw nullptr;

	/* check it's actually a DVD-ROM */
	if (std::error_condition err = chd->check_is_dvd())
		throw err;

	sector_count = chd->unit_count();
}


/*-------------------------------------------------
    destructor - "close" a DVD-ROM file
-------------------------------------------------*/

dvdrom_file::~dvdrom_file()
{
}



/***************************************************************************
    CORE READ ACCESS
***************************************************************************/


/*-------------------------------------------------
    dvdrom_read_data - read one 2048 bytes sector
    from a DVD-ROM
-------------------------------------------------*/

/**
 * @fn  bool read_data(uint32_t lbasector, void *buffer)
 *
 * @brief   Dvdrom read data.
 *
 * @param   lbasector       The lbasector.
 * @param   buffer          The buffer.
 *
 * @return  Success status.
 */

std::error_condition dvdrom_file::read_data(uint32_t lbasector, void *buffer)
{
	if (lbasector >= sector_count)
		return std::error_condition(chd_file::error::HUNK_OUT_OF_RANGE);

	return chd->read_hunk(lbasector, buffer);
}
