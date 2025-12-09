// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Village Tronic Picasso II/Picasso II+

    RTG graphics card for Amiga 2000/3000/4000

    Hardware:
    - Cirrus Logic CL-GD5426 or CL-GD5428
    - 1 or 2 MB RAM
    - 25 MHz (only II+) and 14.31818 MHz XTAL

    TODO:
    - Not working, VGA core needs work
    - Interrupts?
    - Segmented mode (jumper setting, autoconfig id 13)

***************************************************************************/

#include "emu.h"
#include "picasso2.h"
#include "screen.h"

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_PICASSO2P, bus::amiga::zorro::picasso2p_device, "amiga_picasso2p", "Picasso II+ RTG")

namespace bus::amiga::zorro {

picasso2p_device::picasso2p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_PICASSO2P, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_zorro2_card_interface(mconfig, *this),
	m_vga(*this, "vga"),
	m_autoconfig_memory_done(false)
{
	m_vga_space_config = address_space_config("vga_regs", ENDIANNESS_BIG, 8, 12, 0, address_map_constructor(FUNC(picasso2p_device::vga_map), this));
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void picasso2p_device::mmio_map(address_map &map)
{
	map(0x0000, 0x0fff).rw(FUNC(picasso2p_device::vga0_r), FUNC(picasso2p_device::vga0_w)).umask16(0xffff);
	map(0x1000, 0x1fff).rw(FUNC(picasso2p_device::vga1_r), FUNC(picasso2p_device::vga1_w)).umask16(0xffff);
	map(0x46e8, 0x46e8).w(m_vga, FUNC(cirrus_gd5428_vga_device::mode_setup_w));
}

void picasso2p_device::vga_map(address_map &map)
{
	map(0x102, 0x102).unmaprw(); // TODO
	map(0x3b0, 0x3df).m(m_vga, FUNC(cirrus_gd5428_vga_device::io_map));
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void picasso2p_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.1748_MHz_XTAL, 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(cirrus_gd5428_vga_device::screen_update));

	CIRRUS_GD5428_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x200000);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void picasso2p_device::device_start()
{
}

void picasso2p_device::busrst_w(int state)
{
	if (state == 0)
		m_autoconfig_memory_done = false;
}

device_memory_interface::space_config_vector picasso2p_device::memory_space_config() const
{
		return space_config_vector {
				std::make_pair(0, &m_vga_space_config)
		};
}

uint8_t picasso2p_device::vga0_r(offs_t offset)
{
	LOG("vga0_r: %04x\n", offset);
	return space(0).read_byte(offset);
}

void picasso2p_device::vga0_w(offs_t offset, uint8_t data)
{
	LOG("vga0_w: %04x = %02x\n", offset, data);
	space(0).write_byte(offset, data);
}

uint8_t picasso2p_device::vga1_r(offs_t offset)
{
	LOG("vga1_r: %04x (%04x)\n", offset, offset | 1);
	return space(0).read_byte(offset | 1);
}

void picasso2p_device::vga1_w(offs_t offset, uint8_t data)
{
	LOG("vga1_w: %04x (%04x) = %02x\n", offset, offset | 1, data);
	space(0).write_byte(offset | 1, data);
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void picasso2p_device::autoconfig_base_address(offs_t address)
{
	LOG("autoconfig_base_address received: 0x%06x\n", address);

	if (!m_autoconfig_memory_done)
	{
		LOG("-> installing picasso2p memory\n");

		m_zorro->space().install_readwrite_handler(address, address + 0x1fffff,
			emu::rw_delegate(m_vga, FUNC(cirrus_gd5428_vga_device::mem_r)),
			emu::rw_delegate(m_vga, FUNC(cirrus_gd5428_vga_device::mem_w)), 0xffff);

		m_autoconfig_memory_done = true;

		// configure next
		cfgin_w(0);
	}
	else
	{
		LOG("-> installing picasso2p registers\n");

		// install picasso registers
		m_zorro->space().install_device(address, address + 0x0ffff, *this, &picasso2p_device::mmio_map);

		// stop responding to default autoconfig
		m_zorro->space().unmap_readwrite(0xe80000, 0xe8007f);

		// we're done
		m_zorro->cfgout_w(0);
	}
}

void picasso2p_device::cfgin_w(int state)
{
	LOG("cfgin_w (%d)\n", state);

	if (state != 0)
		return;

	if (!m_autoconfig_memory_done)
	{
		LOG("autoconfig for memory\n");

		// setup autoconfig for memory
		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		autoconfig_board_size(BOARD_SIZE_2M);
		autoconfig_link_into_memory(false);
		autoconfig_rom_vector_valid(false);
		autoconfig_multi_device(false); // ?
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true); // ?
		autoconfig_product(11);
		autoconfig_manufacturer(2167);
		autoconfig_serial(0x00000000);
		autoconfig_rom_vector(0x0000);

		// install autoconfig handler
		m_zorro->space().install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
			write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
	}
	else
	{
		LOG("autoconfig for registers\n");

		// setup autoconfig for registers
		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		autoconfig_board_size(BOARD_SIZE_64K);
		autoconfig_link_into_memory(false);
		autoconfig_rom_vector_valid(false);
		autoconfig_multi_device(false); // ?
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true); // ?
		autoconfig_product(12);
		autoconfig_manufacturer(2167);
		autoconfig_serial(0x00000000);
		autoconfig_rom_vector(0x0000);
	}
}

} // namespace bus::amiga::zorro
