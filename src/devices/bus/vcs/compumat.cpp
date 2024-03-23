// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 Atari 2600 cart Spectravideo Compumate (Cart + keyboard!)

 This is tricky to implement and it is only a skeleton ATM.
 The device needs to interface with both the TIA and the RIOT.

***************************************************************************/


#include "emu.h"
#include "compumat.h"

DEFINE_DEVICE_TYPE(A26_ROM_COMPUMATE, a26_rom_cm_device, "a2600_cm", "Atari 2600 ROM Cart Compumate")


a26_rom_cm_device::a26_rom_cm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_base_device(mconfig, A26_ROM_COMPUMATE, tag, owner, clock)
	, m_bank(*this, "bank")
{
}

//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void a26_rom_cm_device::device_reset()
{
	m_bank->set_entry(3);
}


static INPUT_PORTS_START( keyboard )
INPUT_PORTS_END


ioport_constructor a26_rom_cm_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}


void a26_rom_cm_device::install_memory_handlers(address_space *space)
{
	m_bank->configure_entries(0, 4, get_rom_base(), 0x1000);

	space->install_read_bank(0x1000, 0x1fff, m_bank);
}

