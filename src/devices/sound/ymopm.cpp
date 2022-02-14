// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ymopm.h"


//*********************************************************
//  YM2151 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM2151, ym2151_device, "ym2151", "YM2151 OPM")

//-------------------------------------------------
//  ym2151_device - constructor
//-------------------------------------------------

ym2151_device::ym2151_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym2151>(mconfig, tag, owner, clock, YM2151),
	m_reset_state(1)
{
}


//-------------------------------------------------
//  write/register_w/data_w - write access
//-------------------------------------------------

void ym2151_device::write(offs_t offset, u8 data)
{
	if (m_reset_state == 0)
		return;
	parent::write(offset, data);
}

void ym2151_device::address_w(u8 data)
{
	if (m_reset_state == 0)
		return;
	parent::address_w(data);
}

void ym2151_device::data_w(u8 data)
{
	if (m_reset_state == 0)
		return;
	parent::data_w(data);
}


//-------------------------------------------------
//  reset_w - reset line, active LOW
//-------------------------------------------------

WRITE_LINE_MEMBER(ym2151_device::reset_w)
{
	if (state != m_reset_state)
	{
		m_stream->update();
		m_reset_state = state;
		if (state != 0)
			m_chip.reset();
	}
}



//*********************************************************
//  YM2164 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM2164, ym2164_device, "ym2164", "YM2164 OPP")

//-------------------------------------------------
//  ym2164_device - constructor
//-------------------------------------------------

ym2164_device::ym2164_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym2164>(mconfig, tag, owner, clock, YM2164)
{
}
