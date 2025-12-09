// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Commodore A2052

    Zorro-II RAM Expansion (0.5, 1 or 2 MB)

***************************************************************************/

#include "emu.h"
#include "a2052.h"

#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_A2052, bus::amiga::zorro::a2052_device, "amiga_a2052", "Commodore A2052 RAM Expansion Card")

namespace bus::amiga::zorro {

a2052_device::a2052_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_A2052, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_config(*this, "config"),
	m_ram_size(0)
{
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( a2052 )
	PORT_START("config")
	PORT_CONFNAME(0x03, 0x02, "A2052 Installed RAM")
	PORT_CONFSETTING(0x00, "512 KB")
	PORT_CONFSETTING(0x01, "1 MB")
	PORT_CONFSETTING(0x02, "2 MB")
INPUT_PORTS_END

ioport_constructor a2052_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a2052 );
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void a2052_device::device_start()
{
	// setup ram
	m_ram = make_unique_clear<uint16_t[]>(0x200000/2);

	// register for save states
	save_pointer(NAME(m_ram), 0x200000/2);
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void a2052_device::autoconfig_base_address(offs_t address)
{
	LOG("%s: autoconfig_base_address received: 0x%06x\n", shortname(), address);
	LOG("-> installing a2052\n");

	// stop responding to default autoconfig
	m_zorro->space().unmap_readwrite(0xe80000, 0xe8007f);

	// install access to the rom space
	m_zorro->space().install_ram(address, address + (m_ram_size << 20) - 1, m_ram.get());

	// we're done
	m_zorro->cfgout_w(0);
}

void a2052_device::cfgin_w(int state)
{
	LOG("%s: cfgin_w (%d)\n", shortname(), state);

	if (state == 0)
	{
		// setup autoconfig
		autoconfig_board_type(BOARD_TYPE_ZORRO2);

		// setup ram
		switch (m_config->read())
		{
		case 0:
			autoconfig_board_size(BOARD_SIZE_512K);
			m_ram_size = 0x080000 >> 20;
			break;
		case 1:
			autoconfig_board_size(BOARD_SIZE_1M);
			m_ram_size = 0x100000 >> 20;
			break;
		case 2:
			autoconfig_board_size(BOARD_SIZE_2M);
			m_ram_size = 0x200000 >> 20;
			break;
		}

		autoconfig_product(10);
		autoconfig_manufacturer(514);
		autoconfig_serial(0x00000000);

		autoconfig_link_into_memory(true);
		autoconfig_rom_vector_valid(false);
		autoconfig_multi_device(false);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true); // ?

		// install autoconfig handler
		m_zorro->space().install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
			write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
	}
}

} // namespace bus::amiga::zorro
