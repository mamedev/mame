// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    XPert/ProDev Merlin

    RTG graphics card for Amiga 2000/3000/4000

    Hardware:
    - Tseng Labs ET4000W32
    - 1, 2 (maximum in Zorro-II mode) or 4 MB RAM
    - 33 MHz and 14.31818 MHz XTAL
    - BT482KPJ85 RAMDAC
    - Serial EEPROM with stored serial number (unknown type)
    - DG894 (video switcher)
    - Optional video module: X-Calibur (S-VHS/Composite input/output)

    TODO:
    - Skeleton

***************************************************************************/

#include "emu.h"
#include "merlin.h"
#include "screen.h"

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_MERLIN, bus::amiga::zorro::merlin_device, "amiga_merlin", "Merlin RTG")

namespace bus::amiga::zorro {

merlin_device::merlin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_MERLIN, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_vga(*this, "vga"),
	m_ramdac(*this, "ramdac"),
	m_autoconfig_memory_done(false)
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void merlin_device::mmio_map(address_map &map)
{
	map(0x0000, 0xffff).unmaprw();
	map(0x0000, 0x001f).m(m_ramdac, FUNC(bt482_device::map)).umask32(0x00ff0000); // TODO: 16-bit
	map(0x03b0, 0x03df).m(m_vga, FUNC(et4kw32i_vga_device::io_map));
	//map(0x0401, 0x0401) monitor switch
	map(0x210a, 0x210a).mirror(0x70).rw(m_vga, FUNC(et4kw32i_vga_device::acl_index_r), FUNC(et4kw32i_vga_device::acl_index_w));
	map(0x210b, 0x210b).mirror(0x70).rw(m_vga, FUNC(et4kw32i_vga_device::acl_data_r), FUNC(et4kw32i_vga_device::acl_data_w));
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void merlin_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(33_MHz_XTAL, 900, 0, 640, 526, 0, 480); // TODO
	screen.set_screen_update(FUNC(merlin_device::screen_update));

	ET4KW32I_VGA(config, m_vga, 0); // should be ET4000W32
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x200000);
	m_vga->vsync_cb().set([this](int state) { m_zorro->int6_w(state); });

	BT482(config, m_ramdac, 0);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void merlin_device::device_start()
{
}

void merlin_device::busrst_w(int state)
{
	if (state == 0)
		m_autoconfig_memory_done = false;
}

uint32_t merlin_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_vga->screen_update(screen, bitmap, cliprect);
	m_ramdac->screen_update(screen, bitmap, cliprect);

	return 0;
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void merlin_device::autoconfig_base_address(offs_t address)
{
	LOG("autoconfig_base_address received: 0x%06x\n", address);

	if (!m_autoconfig_memory_done)
	{
		LOG("-> installing merlin memory\n");

		m_zorro->space().install_readwrite_handler(address, address + 0x1fffff,
			emu::rw_delegate(m_vga, FUNC(et4kw32i_vga_device::mem_r)),
			emu::rw_delegate(m_vga, FUNC(et4kw32i_vga_device::mem_w)), 0xffff);

		m_autoconfig_memory_done = true;

		// configure next
		cfgin_w(0);
	}
	else
	{
		LOG("-> installing merlin registers\n");

		// install merlin registers
		m_zorro->space().install_device(address, address + 0x0ffff, *this, &merlin_device::mmio_map);

		// stop responding to default autoconfig
		m_zorro->space().unmap_readwrite(0xe80000, 0xe8007f);

		// we're done
		m_zorro->cfgout_w(0);
	}
}

void merlin_device::cfgin_w(int state)
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
		autoconfig_product(3);
		autoconfig_manufacturer(2117);
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
		autoconfig_product(4);
		autoconfig_manufacturer(2117);
		autoconfig_serial(0x00000000);
		autoconfig_rom_vector(0x0000);
	}
}

} // namespace bus::amiga::zorro
