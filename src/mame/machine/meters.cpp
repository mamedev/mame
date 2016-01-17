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

const device_type METERS = &device_creator<meters_device>;

meters_device::meters_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, METERS, "Electro mechanical meters", tag, owner, clock, "meters", __FILE__),
	m_number_mtr(0)
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
		
		//save_item(NAME(m_meter_info[i].reacttime), i); //enable if void ReactTime(int id, INT32 cycles) gets used
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
			assert_always(FALSE, "Unknown id in meters_device::device_timer");
	
	m_meter_info[param].count++;
}


///////////////////////////////////////////////////////////////////////////

int meters_device::GetNumberMeters(void)  // currently unused
{
	return m_number_mtr;
}

///////////////////////////////////////////////////////////////////////////

void meters_device::Setcount(int id, INT32 count) // currently unused
{
	if ( id >= m_number_mtr )
		return;

	m_meter_info[id].count = count;
}

///////////////////////////////////////////////////////////////////////////

INT32 meters_device::Getcount(int id) // currently unused
{
	INT32 result = 0;

	if ( id < m_number_mtr )
		result = m_meter_info[id].count;

	return result;
}

///////////////////////////////////////////////////////////////////////////

void meters_device::ReactTime(int id, INT32 cycles) // currently unused
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
