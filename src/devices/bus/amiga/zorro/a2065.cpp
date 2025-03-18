// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Commodore A2065

    Zorro-II Ethernet Network Interface

***************************************************************************/

#include "emu.h"
#include "a2065.h"

#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_A2065, bus::amiga::zorro::a2065_device, "amiga_a2065", "Commodore A2065 Ethernet Card")

namespace bus::amiga::zorro {

a2065_device::a2065_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_A2065, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_lance(*this, "lance")
{
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void a2065_device::device_add_mconfig(machine_config &config)
{
	AM7990(config, m_lance);
	m_lance->intr_out().set(FUNC(a2065_device::lance_irq_w));
	m_lance->dma_in().set(FUNC(a2065_device::lance_ram_r));
	m_lance->dma_out().set(FUNC(a2065_device::lance_ram_w));
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void a2065_device::device_start()
{
	// setup ram
	m_ram = std::make_unique<uint16_t[]>(0x4000);
	memset(m_ram.get(), 0xff, 0x4000 * sizeof(uint16_t));

	// register for save states
	save_pointer(NAME(m_ram), 0x4000);
}

uint16_t a2065_device::host_ram_r(offs_t offset)
{
	// logerror("host read offset %04x\n", offset);
	return m_ram[offset & 0x3fff];
}

void a2065_device::host_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// logerror("host write %04x = %04x\n", offset, data);
	COMBINE_DATA(&m_ram[offset]);
}

uint16_t a2065_device::lance_ram_r(offs_t offset)
{
	offset = (offset >> 1) & 0x3fff;
	// logerror("lance read offset %04x\n", offset);
	return m_ram[offset];
}

void a2065_device::lance_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset = (offset >> 1) & 0x3fff;
	// logerror("lance write %04x = %04x\n", offset, data);
	COMBINE_DATA(&m_ram[offset]);
}

void a2065_device::lance_irq_w(int state)
{
	// default is irq 2, can be changed via jumper
	m_zorro->int2_w(!state);
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void a2065_device::autoconfig_base_address(offs_t address)
{
	LOG("%s: autoconfig_base_address received: 0x%06x\n", shortname(), address);
	LOG("-> installing a2065\n");

	// stop responding to default autoconfig
	m_zorro->space().unmap_readwrite(0xe80000, 0xe8007f);

	// install autoconfig handler to new location
	m_zorro->space().install_readwrite_handler(address, address + 0x7f,
			read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
			write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);

	// install access to lance registers
	m_zorro->space().install_read_handler(address + 0x4000, address + 0x4003,
			read16m_delegate(*m_lance, FUNC(am7990_device::regs_r)), 0xffff);
	m_zorro->space().install_write_handler(address + 0x4000, address + 0x4003,
			write16sm_delegate(*m_lance, FUNC(am7990_device::regs_w)), 0xffff);

	// install access to onboard ram (32k)
	m_zorro->space().install_read_handler(address + 0x8000, address + 0x8000 + 0x7fff,
			read16sm_delegate(*this, FUNC(a2065_device::host_ram_r)), 0xffff);
	m_zorro->space().install_write_handler(address + 0x8000, address + 0x8000 + 0x7fff,
			write16s_delegate(*this, FUNC(a2065_device::host_ram_w)), 0xffff);

	// we're done
	m_zorro->cfgout_w(0);
}

void a2065_device::cfgin_w(int state)
{
	LOG("%s: cfgin_w (%d)\n", shortname(), state);

	if (state == 0)
	{
		// setup autoconfig
		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		autoconfig_board_size(BOARD_SIZE_64K);

		autoconfig_product(112);
		autoconfig_manufacturer(514);
		autoconfig_serial(0x00123456); // last 3 bytes = last 3 bytes of mac address

		autoconfig_link_into_memory(false);
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
