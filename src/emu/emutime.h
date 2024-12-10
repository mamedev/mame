// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emutime.h

    System time utilities for MAME.

***************************************************************************/

#ifndef MAME_EMU_EMUTIME_H
#define MAME_EMU_EMUTIME_H

#pragma once

#include "timeconv.h"

#include <ctime>
#include <string_view>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> system_time

// system time description, both local and UTC
class system_time
{
public:
	system_time();
	explicit system_time(time_t t);
	void set(time_t t);

	bool customize(std::string_view str);

	int64_t                     time;       // number of seconds elapsed since midnight, January 1 1970 UTC
	util::arbitrary_datetime    local_time; // local time
	util::arbitrary_datetime    utc_time;   // UTC coordinated time
};

#endif // MAME_EMU_EMUTIME_H
