/*
 * font_module.h
 *
 */

#ifndef FONT_MODULE_H_
#define FONT_MODULE_H_

#include "osdepend.h"
#include "modules/osdmodule.h"

//============================================================
//  CONSTANTS
//============================================================

#define OSD_FONT_PROVIDER   "uifontprovider"

class font_module
{
public:
	virtual ~font_module() { }
	virtual osd_font *font_alloc() = 0;
};


#endif /* FONT_MODULE_H_ */
