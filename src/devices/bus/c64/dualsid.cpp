// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 dual SID socket adapter emulation

    The second SID shares the socket's chip select and is selected
    by A5 ($D420) or A8 ($D500) taken from a flying lead.

**********************************************************************/

#include "emu.h"
#include "dualsid.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_SID_DUAL6581, c64_sid_dual6581_device, "c64_sid_dual6581", "C64 dual MOS 6581 SID adapter")
DEFINE_DEVICE_TYPE(C64_SID_DUAL8580, c64_sid_dual8580_device, "c64_sid_dual8580", "C64 dual MOS 8580 SID adapter")



//-------------------------------------------------
//  INPUT_PORTS( c64_sid_dual )
//-------------------------------------------------

static INPUT_PORTS_START( c64_sid_dual )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "SID 2 address" )
	PORT_CONFSETTING( 0x00, "$D420 (A5)" )
	PORT_CONFSETTING( 0x01, "$D500 (A8)" )
	PORT_CONFNAME( 0x02, 0x02, "Audio output" )
	PORT_CONFSETTING( 0x00, "Mono" )
	PORT_CONFSETTING( 0x02, "Stereo" )
INPUT_PORTS_END

ioport_constructor c64_sid_dual6581_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_sid_dual );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

c64_sid_dual6581_device::c64_sid_dual6581_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c64_sid_dual6581_device(mconfig, C64_SID_DUAL6581, tag, owner, clock)
{
}

c64_sid_dual6581_device::c64_sid_dual6581_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_c64_sid_interface(mconfig, *this),
	m_sid1(*this, "sid1"),
	m_sid2(*this, "sid2"),
	m_config(*this, "CONFIG"),
	m_a8(true)
{
}

c64_sid_dual8580_device::c64_sid_dual8580_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c64_sid_dual6581_device(mconfig, C64_SID_DUAL8580, tag, owner, clock)
{
}

void c64_sid_dual6581_device::sid_config(mos6581_device &sid)
{
	sid.add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0, 0);
	sid.add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0, 1);
}

void c64_sid_dual6581_device::device_add_mconfig(machine_config &config)
{
	MOS6581(config, m_sid1, DERIVED_CLOCK(1, 1));
	m_sid1->potx().set(*m_slot, FUNC(c64_sid_slot_device::potx_r));
	m_sid1->poty().set(*m_slot, FUNC(c64_sid_slot_device::poty_r));
	sid_config(*m_sid1);

	MOS6581(config, m_sid2, DERIVED_CLOCK(1, 1));
	sid_config(*m_sid2);
}

void c64_sid_dual8580_device::device_add_mconfig(machine_config &config)
{
	MOS8580(config, m_sid1, DERIVED_CLOCK(1, 1));
	m_sid1->potx().set(*m_slot, FUNC(c64_sid_slot_device::potx_r));
	m_sid1->poty().set(*m_slot, FUNC(c64_sid_slot_device::poty_r));
	sid_config(*m_sid1);

	MOS8580(config, m_sid2, DERIVED_CLOCK(1, 1));
	sid_config(*m_sid2);
}

void c64_sid_dual6581_device::device_start()
{
	save_item(NAME(m_a8));
}

void c64_sid_dual6581_device::device_reset()
{
	uint8_t const config = m_config->read();
	bool const stereo = BIT(config, 1);
	float const own = stereo ? 1.0 : 0.5;
	float const cross = stereo ? 0.0 : 0.5;

	m_a8 = BIT(config, 0);

	m_sid1->set_route_gain(0, m_slot, 0, own);
	m_sid1->set_route_gain(0, m_slot, 1, cross);
	m_sid2->set_route_gain(0, m_slot, 0, cross);
	m_sid2->set_route_gain(0, m_slot, 1, own);
}

uint8_t c64_sid_dual6581_device::read(offs_t offset)
{
	return sid2_selected(offset) ? m_sid2->read(offset & 0x1f) : m_sid1->read(offset & 0x1f);
}

void c64_sid_dual6581_device::write(offs_t offset, uint8_t data)
{
	if (sid2_selected(offset))
		m_sid2->write(offset & 0x1f, data);
	else
		m_sid1->write(offset & 0x1f, data);
}
