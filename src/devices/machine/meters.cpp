// license:BSD-3-Clause
// copyright-holders:James Wallace
/**********************************************************************

	Electromechanical Meter device

**********************************************************************/

#include "emu.h"
#include "meters.h"


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

DEFINE_DEVICE_TYPE(METERS, meters_device, "meters", "Electromechanical meters")

meters_device::meters_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, METERS, tag, owner, clock)
	, m_number_mtr(0)
{
	memset(m_meter_info, 0, sizeof(m_meter_info));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void meters_device::device_start()
{
	if (m_number_mtr > MAXMECHMETERS)
		m_number_mtr = MAXMECHMETERS;

	for (int i = 0; i < m_number_mtr; i++)
	{
		m_meter_info[i].reacttime = METERREACTTIME;
		m_meter_info[i].state     = 0;
		m_meter_info[i].count     = 0;
		m_meter_info[i].on        = 0;
		m_meter_info[i].meter_timer = timer_alloc(FUNC(meters_device::count_tick), this);

		save_item(NAME(m_meter_info[i].state), i);
		save_item(NAME(m_meter_info[i].count), i);
		save_item(NAME(m_meter_info[i].on), i);
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void meters_device::device_reset()
{
}

//-------------------------------------------------
//  count_tick - advance the meter's counter
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(meters_device::count_tick)
{
	m_meter_info[param].count++;
}


int meters_device::get_activity(int id)
{
	return m_meter_info[id].on;
}

int meters_device::update(int id, int state)
{
	if (id >= m_number_mtr)
		return 0;

	state = state ? 1 : 0;

	if (m_meter_info[id].state != state)
	{   // meter state is changing
		m_meter_info[id].state = state;

		if (state)
		{
			m_meter_info[id].on = true;
			m_meter_info[id].meter_timer->adjust(attotime::from_seconds(m_meter_info[id].reacttime), id);
		}
		else
		{
			m_meter_info[id].on = false;
			m_meter_info[id].meter_timer->adjust(attotime::never);
		}
	}

	return m_meter_info[id].on;
}
