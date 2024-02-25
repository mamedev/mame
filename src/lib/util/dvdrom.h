// license:BSD-3-Clause
// copyright-holders:Aaron Giles,R. Belmont
/***************************************************************************

    dvdrom.h

    Generic MAME dvd-rom implementation

***************************************************************************/
#ifndef MAME_LIB_UTIL_DVDROM_H
#define MAME_LIB_UTIL_DVDROM_H

#pragma once

#include "chd.h"
#include "ioprocs.h"
#include "osdcore.h"

class dvdrom_file {
public:
	dvdrom_file(chd_file *chd);
	dvdrom_file(std::string_view inputfile);
	~dvdrom_file();

	uint32_t get_sector_count() const { return sector_count; }

	/* core read access */
	std::error_condition read_data(uint32_t lbasector, void *buffer);

private:
	/** @brief  The chd, when there's one */
	chd_file *           chd;                /* CHD file */

	/** @brief  The raw file, when there isn't */
	util::random_read_write *   fhandle;    // file if not a CHD

	/** @brief  Size of the dvd image in sectors */
	uint32_t sector_count;
};

#endif // MAME_LIB_UTIL_DVDROM_H
