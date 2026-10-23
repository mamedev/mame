// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 SID socket emulation

**********************************************************************/

#include "emu.h"
#include "sid.h"

#include "dualsid.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_SID_SLOT, c64_sid_slot_device, "c64_sid_slot", "C64 SID socket")
DEFINE_DEVICE_TYPE(C64_SID_6581, c64_sid_6581_device, "c64_sid_6581", "C64 MOS 6581 SID")
DEFINE_DEVICE_TYPE(C64_SID_8580, c64_sid_8580_device, "c64_sid_8580", "C64 MOS 8580 SID")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

device_c64_sid_interface::device_c64_sid_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "c64sid"),
	m_slot(dynamic_cast<c64_sid_slot_device *>(device.owner()))
{
}



//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

c64_sid_slot_device::c64_sid_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_SID_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_c64_sid_interface>(mconfig, *this),
	device_mixer_interface(mconfig, *this),
	m_read_potx(*this, 0xff),
	m_read_poty(*this, 0xff),
	m_card(nullptr)
{
}

void c64_sid_slot_device::device_start()
{
	m_card = get_card_device();

	if (!m_card)
		throw emu_fatalerror("%s: the SID socket must not be empty\n", tag());
}



//**************************************************************************
//  MOS 6581
//**************************************************************************

c64_sid_6581_device::c64_sid_6581_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c64_sid_6581_device(mconfig, C64_SID_6581, tag, owner, clock)
{
}

c64_sid_6581_device::c64_sid_6581_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_c64_sid_interface(mconfig, *this),
	m_sid(*this, "sid")
{
}

void c64_sid_6581_device::sid_config()
{
	m_sid->potx().set(*m_slot, FUNC(c64_sid_slot_device::potx_r));
	m_sid->poty().set(*m_slot, FUNC(c64_sid_slot_device::poty_r));
	m_sid->add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0, 0);
	m_sid->add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0, 1);
}

void c64_sid_6581_device::device_add_mconfig(machine_config &config)
{
	MOS6581(config, m_sid, DERIVED_CLOCK(1, 1));
	sid_config();
}

void c64_sid_6581_device::device_start()
{
}

uint8_t c64_sid_6581_device::read(offs_t offset)
{
	return m_sid->read(offset & 0x1f);
}

void c64_sid_6581_device::write(offs_t offset, uint8_t data)
{
	m_sid->write(offset & 0x1f, data);
}



//**************************************************************************
//  MOS 8580
//**************************************************************************

c64_sid_8580_device::c64_sid_8580_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c64_sid_6581_device(mconfig, C64_SID_8580, tag, owner, clock)
{
}

void c64_sid_8580_device::device_add_mconfig(machine_config &config)
{
	MOS8580(config, m_sid, DERIVED_CLOCK(1, 1));
	sid_config();
}



//**************************************************************************
//  CARDS
//**************************************************************************

void c64_sid_cards(device_slot_interface &device)
{
	device.option_add("6581", C64_SID_6581);
	device.option_add("8580", C64_SID_8580);
	device.option_add("dual6581", C64_SID_DUAL6581);
	device.option_add("dual8580", C64_SID_DUAL8580);
}
