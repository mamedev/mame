// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 8706 Speech Glue Logic ASIC emulation

**********************************************************************/

#include "emu.h"
#include "mos8706.h"

#include <algorithm>
#include <iterator>

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MOS8706, mos8706_device, "mos8706", "MOS 8706")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos8706_device - constructor
//-------------------------------------------------

mos8706_device::mos8706_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MOS8706, tag, owner, clock)
	, m_write_command(*this)
	, m_write_di(*this)
	, m_write_irq(*this)
	, m_command(0)
	, m_control(0)
	, m_head(0)
	, m_count(0)
	, m_bit(0)
	, m_eos(1)
	, m_apd(1)
	, m_phi2(0)
	, m_dtrd(1)
	, m_irq(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos8706_device::device_start()
{
	save_item(NAME(m_command));
	save_item(NAME(m_control));
	save_item(NAME(m_fifo));
	save_item(NAME(m_head));
	save_item(NAME(m_count));
	save_item(NAME(m_bit));
	save_item(NAME(m_eos));
	save_item(NAME(m_apd));
	save_item(NAME(m_phi2));
	save_item(NAME(m_dtrd));
	save_item(NAME(m_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos8706_device::device_reset()
{
	m_command = 0;
	m_control = 0;
	m_head = m_count = m_bit = 0;
	m_eos = m_apd = m_dtrd = 1;
	m_phi2 = 0;
	std::fill(std::begin(m_fifo), std::end(m_fifo), 0);
	m_write_di(1);
	update_irq();
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t mos8706_device::read(offs_t offset)
{
	switch (offset & 3)
	{
	case 0:
		return m_command;
	case 1:
		return m_control | (m_eos << 6) | ((m_count < std::size(m_fifo)) ? 0x80 : 0);
	default:
		return 0xff;
	}
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void mos8706_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 3)
	{
	case 0:
		if (!BIT(m_command, 7) && BIT(data, 7))
			m_write_command(data & 15);
		m_command = data;
		break;
	case 1:
		m_control = data & 3;
		update_irq();
		break;
	case 2:
		if (m_count < std::size(m_fifo))
		{
			m_fifo[(m_head + m_count) % std::size(m_fifo)] = data;
			++m_count;
			update_irq();
		}
		break;
	}
}

void mos8706_device::update_irq()
{
	bool const irq = (BIT(m_control, 0) && !m_eos) || (BIT(m_control, 1) && m_count < std::size(m_fifo));
	if (irq != m_irq)
	{
		m_irq = irq;
		m_write_irq(irq);
	}
}

void mos8706_device::eos_w(int state)
{
	m_eos = state;
	update_irq();
}

void mos8706_device::apd_w(int state)
{
	if (state && !m_apd)
		m_head = m_count = m_bit = 0;
	m_apd = state;
	update_irq();
}

void mos8706_device::dtrd_w(int state)
{
	m_dtrd = state;
}

void mos8706_device::phi2_w(int state)
{
	if (state && !m_phi2 && !m_dtrd)
	{
		m_write_di(m_count ? BIT(m_fifo[m_head], m_bit) : 1);
		if (m_count && ++m_bit == 8)
		{
			m_bit = 0;
			m_head = (m_head + 1) % std::size(m_fifo);
			--m_count;
			update_irq();
		}
	}
	m_phi2 = state;
}
