// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************************************

 Sega AI Expansion slot emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "segaai_exp.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


DEFINE_DEVICE_TYPE(SEGAAI_EXP_SLOT, segaai_exp_slot_device, "segaai_exp_slot", "Sega AI Expansion Slot")


device_segaai_exp_interface::device_segaai_exp_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "segaai_exp")
{
}


device_segaai_exp_interface::~device_segaai_exp_interface()
{
}


segaai_exp_slot_device::segaai_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEGAAI_EXP_SLOT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
{
}


segaai_exp_slot_device::~segaai_exp_slot_device()
{
}


void segaai_exp_slot_device::device_start()
{
	m_exp = dynamic_cast<device_segaai_exp_interface *>(get_card_device());
}


u8 segaai_exp_slot_device::read_lo(offs_t offset)
{
	if (m_exp)
	{
		return m_exp->read_lo(offset);
	}
	else
	{
		return 0xff;
	}
}


void segaai_exp_slot_device::write_lo(offs_t offset, u8 data)
{
	if (m_exp)
	{
		m_exp->write_lo(offset, data);
	}
}


u8 segaai_exp_slot_device::read_hi(offs_t offset)
{
	if (m_exp)
	{
		return m_exp->read_hi(offset);
	}
	else
	{
		return 0xff;
	}
}


void segaai_exp_slot_device::write_hi(offs_t offset, u8 data)
{
	if (m_exp)
	{
		m_exp->write_hi(offset, data);
	}
}


u8 segaai_exp_slot_device::read_io(offs_t offset)
{
	if (m_exp)
	{   
		return m_exp->read_io(offset);
	}
	else
	{   
		return 0xff;
	}
}


void segaai_exp_slot_device::write_io(offs_t offset, u8 data)
{
	if (m_exp)
	{
		m_exp->write_io(offset, data);
	}
}


// slot interfaces
#include "soundbox.h"

void segaai_exp(device_slot_interface &device)
{
	device.option_add("soundbox",  SEGAAI_SOUNDBOX);
}

