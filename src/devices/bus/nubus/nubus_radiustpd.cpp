// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Radius Two Page Display (1280x960?)

  Fsx8000a = DAC data
  Fsx8000e = DAC control
  Fsx00000 = VRAM (offset 0x200, stride 0x1b0)

***************************************************************************/

#include "emu.h"
#include "nubus_radiustpd.h"
#include "screen.h"

#include <algorithm>


#define RADIUSTPD_SCREEN_NAME "tpd_screen"
#define RADIUSTPD_ROM_REGION  "tpd_rom"

#define VRAM_SIZE   (0x40000)   // 256k.  1152x880 1 bit per pixel fits nicely.


ROM_START( radiustpd )
	ROM_REGION(0x8000, RADIUSTPD_ROM_REGION, 0)
	ROM_LOAD( "tpd_v22.bin",  0x0000, 0x8000, CRC(7dc5ed05) SHA1(4abb64e49201e966c17a255a94b670564b229934) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_RADIUSTPD, nubus_radiustpd_device, "nb_rtpd", "Radius Two Page Display video card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_radiustpd_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, RADIUSTPD_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_radiustpd_device::screen_update));
	screen.set_raw(99.522_MHz_XTAL, 1536, 0, 1152, 900, 0, 880); // 64.79 kHz horizontal, 72 Hz refresh
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_radiustpd_device::device_rom_region() const
{
	return ROM_NAME( radiustpd );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_radiustpd_device - constructor
//-------------------------------------------------

nubus_radiustpd_device::nubus_radiustpd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_radiustpd_device(mconfig, NUBUS_RADIUSTPD, tag, owner, clock)
{
	(void)m_toggle;
	(void)&m_colors[0];
}

nubus_radiustpd_device::nubus_radiustpd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	set_screen(*this, RADIUSTPD_SCREEN_NAME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_radiustpd_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(RADIUSTPD_ROM_REGION, true, true);

	slotspace = get_slotspace();

//  printf("[radiustpd %p] slotspace = %x\n", (void *)this, slotspace);

	m_vram.resize(VRAM_SIZE / sizeof(uint32_t));

	nubus().install_device(slotspace, slotspace+VRAM_SIZE-1, read32s_delegate(*this, FUNC(nubus_radiustpd_device::vram_r)), write32s_delegate(*this, FUNC(nubus_radiustpd_device::vram_w)));
	nubus().install_device(slotspace+0x900000, slotspace+VRAM_SIZE-1+0x900000, read32s_delegate(*this, FUNC(nubus_radiustpd_device::vram_r)), write32s_delegate(*this, FUNC(nubus_radiustpd_device::vram_w)));
	nubus().install_device(slotspace+0x80000, slotspace+0xeffff, read32s_delegate(*this, FUNC(nubus_radiustpd_device::radiustpd_r)), write32s_delegate(*this, FUNC(nubus_radiustpd_device::radiustpd_w)));
	nubus().install_device(slotspace+0x980000, slotspace+0x9effff, read32s_delegate(*this, FUNC(nubus_radiustpd_device::radiustpd_r)), write32s_delegate(*this, FUNC(nubus_radiustpd_device::radiustpd_w)));

	m_timer = timer_alloc(FUNC(nubus_radiustpd_device::vbl_tick), this);
	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_radiustpd_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_vbl_disable = 1;
	m_mode = 0;
	std::fill(m_vram.begin(), m_vram.end(), 0);
	memset(m_palette, 0, sizeof(m_palette));

	m_palette[1] = rgb_t(255, 255, 255);
	m_palette[0] = rgb_t(0, 0, 0);
}


TIMER_CALLBACK_MEMBER(nubus_radiustpd_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

/***************************************************************************

  Two Page Display section

***************************************************************************/

uint32_t nubus_radiustpd_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]) + 0x200;

	for (int y = 0; y < 880; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);
		for (int x = 0; x < 1152/8; x++)
		{
			uint8_t const pixels = vram8[(y * (1152/8)) + x];

			*scanline++ = m_palette[BIT(pixels, 7)];
			*scanline++ = m_palette[BIT(pixels, 6)];
			*scanline++ = m_palette[BIT(pixels, 5)];
			*scanline++ = m_palette[BIT(pixels, 4)];
			*scanline++ = m_palette[BIT(pixels, 3)];
			*scanline++ = m_palette[BIT(pixels, 2)];
			*scanline++ = m_palette[BIT(pixels, 1)];
			*scanline++ = m_palette[BIT(pixels, 0)];
		}
	}

	return 0;
}

void nubus_radiustpd_device::radiustpd_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("TPD: write %08x to %x, mask %08x\n", data, offset, mem_mask);
}

uint32_t nubus_radiustpd_device::radiustpd_r(offs_t offset, uint32_t mem_mask)
{
//  printf("TPD: read @ %x, mask %08x\n", offset, mem_mask);

	if (offset == 0)
	{
		lower_slot_irq();
		m_vbl_disable = true;
	}

	if (offset == 0x8000)
	{
		m_vbl_disable = false;
	}

	if (offset == 0x18000)
	{
		return 0xffffffff;
	}

	return 0;
}

void nubus_radiustpd_device::vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram[offset]);
}

uint32_t nubus_radiustpd_device::vram_r(offs_t offset, uint32_t mem_mask)
{
	return m_vram[offset] ^ 0xffffffff;
}
