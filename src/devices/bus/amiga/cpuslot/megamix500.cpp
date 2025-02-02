// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    3-State MegaMix 500

    External RAM expansion for the A500

    TODO: Passthrough

***************************************************************************/

#include "emu.h"
#include "megamix500.h"

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_CPUSLOT_MEGAMIX500, bus::amiga::cpuslot::megamix500_device, "amiga_megamix500", "3-State MegaMix 500")

namespace bus::amiga::cpuslot {

megamix500_device::megamix500_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_CPUSLOT_MEGAMIX500, tag, owner, clock),
	device_amiga_cpuslot_interface(mconfig, *this),
	m_config(*this, "config"),
	m_ram_size(0)
{
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( megamix500 )
	PORT_START("config")
	PORT_CONFNAME(0x03, 0x03, "Installed RAM")
	PORT_CONFSETTING(0x00, "Disabled")
	PORT_CONFSETTING(0x01, "2 MB")
	PORT_CONFSETTING(0x02, "4 MB")
	PORT_CONFSETTING(0x03, "8 MB")
INPUT_PORTS_END

ioport_constructor megamix500_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( megamix500 );
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void megamix500_device::device_start()
{
	// setup ram
	m_ram = make_unique_clear<uint16_t[]>(0x800000/2);

	// register for save states
	save_pointer(NAME(m_ram), 0x800000/2);
	save_item(NAME(m_ram_size));
	save_item(NAME(m_base_address));
}

void megamix500_device::rst_w(int state)
{
	if (state == 0)
	{
		// on reset, remove ram
		if (m_ram_size > 0)
		{
			LOG("unmapping ram, base = %06x\n", m_base_address);
			m_host->space().unmap_readwrite(m_base_address, m_base_address + (m_ram_size << 20) - 1);
		}

		m_ram_size = 0;
	}
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void megamix500_device::autoconfig_base_address(offs_t address)
{
	LOG("autoconfig_base_address received: 0x%06x\n", address);
	LOG("-> installing megamix500\n");

	m_base_address = address;

	m_host->space().unmap_readwrite(0xe80000, 0xe8007f);
	m_host->space().install_ram(address, address + (m_ram_size << 20) - 1, m_ram.get());

	m_host->cfgout_w(0);
}

void megamix500_device::cfgin_w(int state)
{
	LOG("cfgin_w: %d\n", state);

	uint8_t cfg = m_config->read();

	if (state == 0 && cfg)
	{
		// setup autoconfig
		autoconfig_board_type(BOARD_TYPE_ZORRO2);

		// setup ram
		switch (cfg)
		{
		case 1:
			autoconfig_board_size(BOARD_SIZE_2M);
			m_ram_size = 0x200000 >> 20;
			break;
		case 2:
			autoconfig_board_size(BOARD_SIZE_4M);
			m_ram_size = 0x400000 >> 20;
			break;
		case 3:
			autoconfig_board_size(BOARD_SIZE_8M);
			m_ram_size = 0x800000 >> 20;
			break;
		}

		autoconfig_product(2);
		autoconfig_manufacturer(512);
		autoconfig_serial(0x00000000);

		autoconfig_link_into_memory(true);
		autoconfig_rom_vector_valid(false);
		autoconfig_multi_device(false);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true); // ?

		// install autoconfig handler
		m_host->space().install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
			write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
	}
}

} // namespace bus::amiga::cpuslot
