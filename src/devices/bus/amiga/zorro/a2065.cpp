// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Commodore A2065

    Zorro-II Ethernet Network Interface

***************************************************************************/

#include "emu.h"
#include "a2065.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(A2065, a2065_device, "a2065", "CBM A2065 Ethernet Card")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2065_device::device_add_mconfig(machine_config &config)
{
	AM7990(config, m_lance);
	m_lance->intr_out().set(FUNC(a2065_device::lance_irq_w));
	m_lance->dma_in().set(FUNC(a2065_device::lance_ram_r));
	m_lance->dma_out().set(FUNC(a2065_device::lance_ram_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2065_device - constructor
//-------------------------------------------------

a2065_device::a2065_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A2065, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_lance(*this, "lance")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2065_device::device_start()
{
	// setup ram
	m_ram = std::make_unique<uint16_t[]>(0x4000);
	memset(m_ram.get(), 0xff, 0x4000 * sizeof(uint16_t));

	// register for save states
	save_pointer(NAME(m_ram), 0x4000);

	set_zorro_device();
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void a2065_device::autoconfig_base_address(offs_t address)
{
	LOG("%s('%s'): autoconfig_base_address received: 0x%06x\n", shortname(), basetag(), address);
	LOG("-> installing a2065\n");

	// stop responding to default autoconfig
	m_slot->space().unmap_readwrite(0xe80000, 0xe8007f);

	// install autoconfig handler to new location
	m_slot->space().install_readwrite_handler(address, address + 0x7f,
			read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
			write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);

	// install access to lance registers
	m_slot->space().install_readwrite_handler(address + 0x4000, address + 0x4003,
			read16_delegate(*m_lance, FUNC(am7990_device::regs_r)),
			write16_delegate(*m_lance, FUNC(am7990_device::regs_w)), 0xffff);

	// install access to onboard ram (32k)
	m_slot->space().install_readwrite_handler(address + 0x8000, address + 0x8000 + 0x7fff,
			read16_delegate(*this, FUNC(a2065_device::host_ram_r)),
			write16_delegate(*this, FUNC(a2065_device::host_ram_w)), 0xffff);

	// we're done
	m_slot->cfgout_w(0);
}

WRITE_LINE_MEMBER( a2065_device::cfgin_w )
{
	LOG("%s('%s'): configin_w (%d)\n", shortname(), basetag(), state);

	if (state == 0)
	{
		// setup autoconfig
		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		autoconfig_board_size(BOARD_SIZE_64K);

		autoconfig_product(0x70);
		autoconfig_manufacturer(0x0202);
		autoconfig_serial(0x00123456); // last 3 bytes = last 3 bytes of mac address

		autoconfig_link_into_memory(false);
		autoconfig_rom_vector_valid(false);
		autoconfig_multi_device(false);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true); // ?

		// install autoconfig handler
		m_slot->space().install_readwrite_handler(0xe80000, 0xe8007f,
				read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
				write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
	}
}

READ16_MEMBER( a2065_device::host_ram_r )
{
	// logerror("host read offset %04x\n", offset);
	return m_ram[offset & 0x3fff];
}

WRITE16_MEMBER( a2065_device::host_ram_w )
{
	// logerror("host write %04x = %04x\n", offset, data);
	COMBINE_DATA(&m_ram[offset]);
}

READ16_MEMBER( a2065_device::lance_ram_r )
{
	offset = (offset >> 1) & 0x3fff;
	// logerror("lance read offset %04x\n", offset);
	return m_ram[offset];
}

WRITE16_MEMBER( a2065_device::lance_ram_w )
{
	offset = (offset >> 1) & 0x3fff;
	// logerror("lance write %04x = %04x\n", offset, data);
	COMBINE_DATA(&m_ram[offset]);
}

WRITE_LINE_MEMBER( a2065_device::lance_irq_w )
{
	// default is irq 2, can be changed via jumper
	m_slot->int2_w(!state);
}
