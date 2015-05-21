// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 Atari 2600 cart Spectravideo Compumate (Cart + keyboard!)

 This is tricky to implement and it is only a skeleton ATM.
 The device needs to interface with both the TIA and the RIOT.

***************************************************************************/


#include "emu.h"
#include "compumat.h"

const device_type A26_ROM_COMPUMATE = &device_creator<a26_rom_cm_device>;


a26_rom_cm_device::a26_rom_cm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
						: a26_rom_f6_device(mconfig, A26_ROM_COMPUMATE, "Atari 2600 ROM Cart Compumate", tag, owner, clock, "a2600_cm", __FILE__)
{
}

//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void a26_rom_cm_device::device_start()
{
	save_item(NAME(m_base_bank));
}

void a26_rom_cm_device::device_reset()
{
	m_base_bank = 3;
}


static INPUT_PORTS_START( keyboard )
INPUT_PORTS_END


ioport_constructor a26_rom_cm_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}



READ8_MEMBER(a26_rom_cm_device::read_rom)
{
	return m_rom[offset + (m_base_bank * 0x1000)];
}
