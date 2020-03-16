// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Sigma Designs fixed-resolution monochrome video card
  1664x1200 or 832x600 according to the ad, can't find the 1664 mode.

  VRAM at Fs000000, mirrored at Fs900000.
  Fs0BFFEC: write 0x04 to enable VBL, 0x01 to ack VBL

***************************************************************************/

#include "emu.h"
#include "laserview.h"
#include "screen.h"

#define LASERVIEW_SCREEN_NAME   "laserview_screen"
#define LASERVIEW_ROM_REGION    "laserview_rom"

#define VRAM_SIZE   (0x40000)

ROM_START( laserview )
	ROM_REGION(0x8000, LASERVIEW_ROM_REGION, 0)
	ROM_LOAD( "lva-m2-00020_v3.00.bin", 0x000000, 0x008000, CRC(569d1fb7) SHA1(fd505505226abb5fea7c10ed14e8841077ef1be6) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_LASERVIEW, nubus_laserview_device, "nb_laserview", "Sigma Designs LaserView video card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_laserview_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, LASERVIEW_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_laserview_device::screen_update));
	screen.set_size(832,600);
	screen.set_visarea(0, 832-1, 0, 600-1);
	screen.set_refresh_hz(70);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_laserview_device::device_rom_region() const
{
	return ROM_NAME( laserview );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_laserview_device - constructor
//-------------------------------------------------

nubus_laserview_device::nubus_laserview_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_laserview_device(mconfig, NUBUS_LASERVIEW, tag, owner, clock)
{
}

nubus_laserview_device::nubus_laserview_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_vbl_disable(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_laserview_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(this, LASERVIEW_ROM_REGION, true);

	slotspace = get_slotspace();

//  printf("[laserview %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE);
	install_bank(slotspace, slotspace+VRAM_SIZE-1, "bank_laserview", &m_vram[0]);
	install_bank(slotspace+0x900000, slotspace+0x900000+VRAM_SIZE-1, "bank_laserview2", &m_vram[0]);

	nubus().install_device(slotspace+0xB0000, slotspace+0xBFFFF, read32_delegate(*this, FUNC(nubus_laserview_device::regs_r)), write32_delegate(*this, FUNC(nubus_laserview_device::regs_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_laserview_device::device_reset()
{
	m_vbl_disable = 1;
	m_prot_state = 0;
	m_toggle = 0;
	memset(&m_vram[0], 0, VRAM_SIZE);

	m_palette[0] = rgb_t(255, 255, 255);
	m_palette[1] = rgb_t(0, 0, 0);
}

/***************************************************************************

  Viking 1024x768 B&W card section

***************************************************************************/

uint32_t nubus_laserview_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint8_t pixels;

	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	for (y = 0; y < 600; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 832/8; x++)
		{
			pixels = m_vram[(y * 104) + (BYTE4_XOR_BE(x)) + 0x20];

			*scanline++ = m_palette[(pixels>>7)&1];
			*scanline++ = m_palette[(pixels>>6)&1];
			*scanline++ = m_palette[(pixels>>5)&1];
			*scanline++ = m_palette[(pixels>>4)&1];
			*scanline++ = m_palette[(pixels>>3)&1];
			*scanline++ = m_palette[(pixels>>2)&1];
			*scanline++ = m_palette[(pixels>>1)&1];
			*scanline++ = m_palette[(pixels&1)];
		}
	}

	return 0;
}

WRITE32_MEMBER( nubus_laserview_device::regs_w )
{
//  printf("%08x to regs @ %x mask %08x\n", data, offset, mem_mask);

	switch (offset)
	{
		case 0x3ffb:
			if ((data & 0xff) == 0x04)
			{
				m_vbl_disable = 0;
			}
			else if ((data & 0xff) == 01)
			{
				lower_slot_irq();
			}
			else
			{
				m_vbl_disable = 1;
				lower_slot_irq();
			}
			break;
	}

}

READ32_MEMBER( nubus_laserview_device::regs_r )
{
	//f (offset != 0x3fc1) printf("Read regs_r @ %x mask %08x\n", offset, mem_mask);

	switch (offset)
	{
		case 0x3fc1:
			m_toggle ^= 1;
			if (m_toggle)
			{
				return 0x01010101;
			}
			else
			{
				return 0;
			}
			machine().debug_break();
			break;

		case 0x3fc2:
			if (m_prot_state == 0)
			{
				m_prot_state = 1;
				return 0;
			}
			return 0x02000000;

		case 0x3fff:
			m_prot_state = 0;
			return 0xfeffffff;
	}

	return 0xffffffff;
}

