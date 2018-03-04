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

#include "sliderchangednotifier.h"

#define SLIDER_NOCHANGE     0x12345678

typedef std::function<int32_t(running_machine&, void*, int, std::string*, int32_t)> slider_update;

struct slider_state
{
	slider_update   update;             /* callback */
	void *          arg;                /* argument */
	int32_t           minval;             /* minimum value */
	int32_t           defval;             /* default value */
	int32_t           maxval;             /* maximum value */
	int32_t           incval;             /* increment value */
	int             id;
	std::string     description;        /* textual description */
};

#endif // __UI_SLIDER__
