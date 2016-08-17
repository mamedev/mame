// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods

/***************************************************************************

    ui/slider.h

    Internal data representation for an adjustment slider.

***************************************************************************/

#pragma once

#ifndef __UI_SLIDER__
#define __UI_SLIDER__

#include <functional>

#include "emu.h"
#include "sliderchangednotifier.h"

#define SLIDER_NOCHANGE     0x12345678

typedef std::function<INT32(running_machine&, void*, int, std::string*, INT32)> slider_update;

struct slider_state
{
	slider_state *  next;               /* pointer to next slider */
	slider_update   update;             /* callback */
	void *          arg;                /* argument */
	INT32           minval;             /* minimum value */
	INT32           defval;             /* default value */
	INT32           maxval;             /* maximum value */
	INT32           incval;             /* increment value */
	int             id;
	char            description[1];     /* textual description */
};

#endif // __UI_SLIDER__
