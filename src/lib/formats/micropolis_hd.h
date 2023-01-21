// license:BSD-3-Clause
// copyright-holders:Eric Anderson
/***************************************************************************

Micropolis MFM HDD format

The format is similar to the Micropolis format for hard-sectored floppies.
The controller hardware uses a PLL to emulate hard sectors.

****************************************************************************/
#ifndef MAME_FORMATS_MICROPOLIS_HD_H
#define MAME_FORMATS_MICROPOLIS_HD_H

#pragma once

#include "mfm_hd.h"

class micropolis_mfmhd_format : public mfmhd_image_format_t
{
public:
	micropolis_mfmhd_format() { m_devtag = std::string("micropolis_mfmhd_format"); }
	std::error_condition load(chd_file *chdfile, uint16_t *trackimage, int tracksize, int cylinder, int head) override;
	std::error_condition save(chd_file *chdfile, uint16_t *trackimage, int tracksize, int cylinder, int head) override;

	bool save_param(mfmhd_param_t type) override;
	int get_default(mfmhd_param_t type) override { return -1; }

private:
	uint64_t chs_to_offset(int cylinder, int head, int sector);
};

extern const mfmhd_format_type MFMHD_MICROPOLIS_FORMAT;

#endif // MAME_FORMATS_MICROPOLIS_HD_H
