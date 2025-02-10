// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Omega Datentechnik Rainbow II (2145/32)
    Ingenieurbuero Helfrich Rainbow II (2195/32)
    BSC FrameMaster (2049/32, 2092/32)

    24-bit framebuffer for Amiga 2000/3000/4000

    Hardware:
    - ADV7120 RAMDAC
    - 30 MHz XTAL
    - 1.5 MB RAM with space for 0.5 more as alpha channel
    - Fixed resolution 768x576 (PAL) and 768x480 (NTSC)
    - 15.75 kHz or 31.5 kHz
    - 24-bit color

    TODO:
    - ROM sockets are unsupported
    - Rest of the jumpers
    - Verify autoconfig IDs and include additional models
    - Interlace (pending MAME support)

***************************************************************************/

#include "emu.h"
#include "rainbow2.h"

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_RAINBOW2, bus::amiga::zorro::rainbow2_device, "amiga_rainbow2", "Rainbow II Framebuffer")
DEFINE_DEVICE_TYPE(AMIGA_FRAMEMASTER, bus::amiga::zorro::framemaster_device, "amiga_framemaster", "FrameMaster Framebuffer")

namespace bus::amiga::zorro {

rainbow2_device::rainbow2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type, uint16_t manufacturer) :
	device_t(mconfig, type, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_screen(*this, "screen"),
	m_jumper(*this, "jumper")
{
	m_manufacturer = manufacturer;
}

rainbow2_device::rainbow2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	rainbow2_device(mconfig, tag, owner, clock, AMIGA_RAINBOW2, 2145)
{
}

framemaster_device::framemaster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	rainbow2_device(mconfig, tag, owner, clock, AMIGA_FRAMEMASTER, 2092)
{
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( rainbow2 )
	PORT_START("jumper")
	PORT_CONFNAME(0x01, 0x01, "JP5")
	PORT_CONFSETTING(0x00, "Test Signal")
	PORT_CONFSETTING(0x01, DEF_STR(Normal))
INPUT_PORTS_END

ioport_constructor rainbow2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rainbow2 );
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void rainbow2_device::device_add_mconfig(machine_config &config)
{
	// default to PAL
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(30_MHz_XTAL, 960, 0, 768, 625, 0, 576); // exact values not known
	m_screen->set_screen_update(FUNC(rainbow2_device::screen_update));
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void rainbow2_device::device_start()
{
	// setup ram
	m_vram = make_unique_clear<uint16_t[]>(0x200000/2);

	// register for save states
	save_pointer(NAME(m_vram), 0x200000/2);
	save_item(NAME(m_control));

}

void rainbow2_device::busrst_w(int state)
{
	if (state == 0)
		m_control = 0;
}

void rainbow2_device::control_w(offs_t offset, uint8_t data)
{
	// 3---  enable ROM
	// -2--  normal (PAL) or complement mode (NTSC)
	// --1-  video out enable
	// ---0  interlace (0) or non-interlace (1)

	LOG("control_w: %02x\n", data);

	if (BIT(data, 2))
	{
		// NTSC
		rectangle v(0, 768 - 1, 0, 480 - 1);
		m_screen->configure(960, 525, v, attotime::from_ticks(960 * 525, 30_MHz_XTAL).as_attoseconds());
	}
	else
	{
		// PAL
		rectangle v(0, 768 - 1, 0, 576 - 1);
		m_screen->configure(960, 625, v, attotime::from_ticks(960 * 625, 30_MHz_XTAL).as_attoseconds());
	}

	m_control = data & 0x0f;
}

uint32_t rainbow2_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_jumper->read(), 0) == 0)
	{
		// test signal enabled
		bitmap.fill(rgb_t::white(), cliprect);
	}
	else if (BIT(m_control, 1) == 0)
	{
		// video disabled
		bitmap.fill(rgb_t::black(), cliprect);
	}
	else
	{
		for (unsigned y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			unsigned const i = y * 768 * 2; // line data start in memory
			auto *const dst = &bitmap.pix(y);

			for (unsigned x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				uint16_t const v1 = m_vram[(i + (x << 1)) | 0]; // green, blue
				uint16_t const v2 = m_vram[(i + (x << 1)) | 1]; // alpha, red

				dst[x] = (uint32_t(v2) << 16) | v1;
			}
		}
	}

	return 0;
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void rainbow2_device::autoconfig_base_address(offs_t address)
{
	LOG("autoconfig_base_address received: 0x%06x\n", address);
	LOG("-> installing rainbow2\n");

	// stop responding to default autoconfig
	m_zorro->space().unmap_readwrite(0xe80000, 0xe8007f);

	// video memory
	m_zorro->space().install_ram(address, address + 0x1fffff, m_vram.get());

	// control register
	m_zorro->space().install_write_handler(address + 0x1ffff8, address + 0x1ffff8,
		emu::rw_delegate(*this, FUNC(rainbow2_device::control_w)));

	// we're done
	m_zorro->cfgout_w(0);
}

void rainbow2_device::cfgin_w(int state)
{
	LOG("cfgin_w (%d)\n", state);

	if (state == 0)
	{
		// setup autoconfig
		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		autoconfig_board_size(BOARD_SIZE_2M);
		autoconfig_link_into_memory(false);
		autoconfig_rom_vector_valid(false);
		autoconfig_multi_device(false);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true); // ?
		autoconfig_product(32);
		autoconfig_manufacturer(m_manufacturer);
		autoconfig_serial(0x00000000);
		autoconfig_rom_vector(0x0000);

		// install autoconfig handler
		m_zorro->space().install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
			write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
	}
}

} // namespace bus::amiga::zorro
