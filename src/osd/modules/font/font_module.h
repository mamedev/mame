// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * font_module.h
 *
 */

#ifndef MAME_OSD_MODULES_FONT_FONTMODULE_H
#define MAME_OSD_MODULES_FONT_FONTMODULE_H

#include "osdepend.h"
#include "modules/osdmodule.h"

#include <string>
#include <vector>


//============================================================
//  CONSTANTS
//============================================================

#define OSD_FONT_PROVIDER   "uifontprovider"

class font_module
{
public:
	virtual ~font_module() { }

	/** attempt to allocate a font instance */
	virtual osd_font::ptr font_alloc() = 0;

	/** attempt to list available font families */
	virtual bool get_font_families(std::string const &font_path, std::vector<std::pair<std::string, std::string> > &result) = 0;
};


#endif // MAME_OSD_MODULES_FONT_FONTMODULE_H
