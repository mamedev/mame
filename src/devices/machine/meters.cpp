// license:BSD-3-Clause
// copyright-holders:James Wallace
///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Electro mechanical meters                                             //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

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
	if ( m_number_mtr > MAXMECHMETERS )
		m_number_mtr = MAXMECHMETERS;

	for ( int i = 0; i < m_number_mtr; i++ )
	{
		m_meter_info[i].reacttime = METERREACTTIME;
		m_meter_info[i].state     = 0;
		m_meter_info[i].count     = 0;
		m_meter_info[i].on        = 0;
		m_meter_info[i].meter_timer = timer_alloc(i);
		m_meter_info[i].meter_timer->reset();

		//save_item(NAME(m_meter_info[i].reacttime), i); //enable if void ReactTime(int id, int32_t cycles) gets used
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
//  device_timer - device-specific timer events
//-------------------------------------------------

void meters_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id >= m_number_mtr)
			assert_always(false, "Unknown id in meters_device::device_timer");

	m_meter_info[param].count++;
}


///////////////////////////////////////////////////////////////////////////

int meters_device::GetNumberMeters(void)  // currently unused
{
	return m_number_mtr;
}

///////////////////////////////////////////////////////////////////////////

void meters_device::Setcount(int id, int32_t count) // currently unused
{
	if ( id >= m_number_mtr )
		return;

	m_meter_info[id].count = count;
}

///////////////////////////////////////////////////////////////////////////

int32_t meters_device::Getcount(int id) // currently unused
{
	int32_t result = 0;

	if ( id < m_number_mtr )
		result = m_meter_info[id].count;

	return result;
}

///////////////////////////////////////////////////////////////////////////

void meters_device::ReactTime(int id, int32_t cycles) // currently unused
{
	if ( id >= m_number_mtr )
		return;

	m_meter_info[id].reacttime = cycles;
}

///////////////////////////////////////////////////////////////////////////

int meters_device::GetActivity(int id)
{
	return m_meter_info[id].on;
}

///////////////////////////////////////////////////////////////////////////

int meters_device::update(int id, int state)
{
	int res = 0;

	if ( id >= m_number_mtr )
		return res;

	state = state?1:0;

	if ( m_meter_info[id].state != state )
	{   // meter state is changing
		m_meter_info[id].state = state;

		if ( state )
		{
			m_meter_info[id].on =1;
			m_meter_info[id].meter_timer->adjust(attotime::from_seconds(m_meter_info[id].reacttime), id);
		}
		else
		{
			m_meter_info[id].on =0;
			m_meter_info[id].meter_timer->adjust(attotime::never, id);
		}
	}
	return m_meter_info[id].on;
}
