// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/

#include "emu.h"
#include "machine/mb14241.h"


//-------------------------------------------------
//  mb14241_device - constructor
//-------------------------------------------------


const device_type MB14241 = &device_creator<mb14241_device>;

mb14241_device::mb14241_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MB14241, "MB14241 Data Shifter", tag, owner, clock, "mb14241", __FILE__), m_shift_data(0), m_shift_count(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mb14241_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb14241_device::device_start()
{
	save_item(NAME(m_shift_data));
	save_item(NAME(m_shift_count));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb14241_device::device_reset()
{
	m_shift_data = 0;
	m_shift_count = 0;
}


/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

void mb14241_device::shift_count_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_shift_count = ~data & 0x07;
}

void mb14241_device::shift_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_shift_data = (m_shift_data >> 8) | ((uint16_t)data << 7);
}

uint8_t mb14241_device::shift_result_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_shift_data >> m_shift_count;
}
