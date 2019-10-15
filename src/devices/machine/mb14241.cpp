// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    MB14241 shifter IC emulation

****************************************************************************/

#include "emu.h"
#include "machine/mb14241.h"


//-------------------------------------------------
//  mb14241_device - constructor
//-------------------------------------------------


DEFINE_DEVICE_TYPE(MB14241, mb14241_device, "mb14241", "MB14241 Data Shifter")

mb14241_device::mb14241_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MB14241, tag, owner, clock), m_shift_data(0), m_shift_count(0)
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

void mb14241_device::shift_count_w(u8 data)
{
	m_shift_count = ~data & 0x07;
}

void mb14241_device::shift_data_w(u8 data)
{
	m_shift_data = (m_shift_data >> 8) | (u16(data) << 7);
}

u8 mb14241_device::shift_result_r()
{
	return u8((m_shift_data >> m_shift_count) & 0x00ff);
}
