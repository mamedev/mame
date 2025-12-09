// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_MACHINE_MACSECONDS_H
#define MAME_MACHINE_MACSECONDS_H

#pragma once

class macseconds_interface
{
public:
	// construction/destruction
	macseconds_interface();
	virtual ~macseconds_interface();

	u32 get_local_seconds(system_time &systime);
	u32 get_seconds(int year, int month, int day, int day_of_week, int hour, int minute, int second);

	private : int m_is_dst;
	time_t m_gmt_offset;
};

#endif // MAME_MACHINE_MACSECONDS_H
