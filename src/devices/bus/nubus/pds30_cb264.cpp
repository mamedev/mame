// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  RasterOps ColorBoard 264/SE30 video card emulation

***************************************************************************/

#include "emu.h"
#include "pds30_cb264.h"
#include "screen.h"

#include <algorithm>


#define CB264SE30_SCREEN_NAME "cb264_screen"
#define CB264SE30_ROM_REGION  "cb264_rom"

#define VRAM_SIZE   (0x200000)


ROM_START( cb264se30 )
	ROM_REGION(0x8000, CB264SE30_ROM_REGION, 0)
	ROM_LOAD( "0002-2019_10-02-90.bin", 0x000000, 0x008000, CRC(5b5b2fab) SHA1(0584deb38b402718f2abef456b0035b34fddb473) )  // EPROM label "264/30 V1.3 0002-2019 10/02/90"
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PDS030_CB264SE30, nubus_cb264se30_device, "pd3_c264", "RasterOps Colorboard 264/SE30")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_cb264se30_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, CB264SE30_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_cb264se30_device::screen_update));
	screen.set_raw(25175000, 800, 0, 640, 525, 0, 480);
	screen.set_size(1024, 768);
	screen.set_visarea(0, 640-1, 0, 480-1);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_cb264se30_device::device_rom_region() const
{
	return ROM_NAME( cb264se30 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_cb264se30_device - constructor
//-------------------------------------------------

nubus_cb264se30_device::nubus_cb264se30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_cb264se30_device(mconfig, PDS030_CB264SE30, tag, owner, clock)
{
	(void)m_toggle;
}

nubus_cb264se30_device::nubus_cb264se30_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_vram32(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	set_screen(*this, CB264SE30_SCREEN_NAME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_cb264se30_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(this, CB264SE30_ROM_REGION);

	slotspace = get_slotspace();

//  printf("[cb264se30 %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE);
	m_vram32 = (uint32_t *)&m_vram[0];

	nubus().install_device(slotspace, slotspace+VRAM_SIZE-1, read32s_delegate(*this, FUNC(nubus_cb264se30_device::vram_r)), write32s_delegate(*this, FUNC(nubus_cb264se30_device::vram_w)));
	nubus().install_device(slotspace+0xf00000, slotspace+0xfeffff, read32s_delegate(*this, FUNC(nubus_cb264se30_device::cb264se30_r)), write32s_delegate(*this, FUNC(nubus_cb264se30_device::cb264se30_w)));

	m_timer = timer_alloc(FUNC(nubus_cb264se30_device::vbl_tick), this);
	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_cb264se30_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_vbl_disable = 1;
	m_mode = 4;
	memset(&m_vram[0], 0, VRAM_SIZE);
	memset(m_palette, 0, sizeof(m_palette));

	m_palette[0] = rgb_t(255, 255, 255);
	m_palette[0x80] = rgb_t(0, 0, 0);
}


TIMER_CALLBACK_MEMBER(nubus_cb264se30_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

/***************************************************************************

  CB264 section

***************************************************************************/

uint32_t nubus_cb264se30_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const vram = &m_vram[8*1024];

	switch (m_mode)
	{
		case 0: // 1 bpp?
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/8; x++)
				{
					uint8_t const pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[(pixels&0x80)];
					*scanline++ = m_palette[((pixels<<1)&0x80)];
					*scanline++ = m_palette[((pixels<<2)&0x80)];
					*scanline++ = m_palette[((pixels<<3)&0x80)];
					*scanline++ = m_palette[((pixels<<4)&0x80)];
					*scanline++ = m_palette[((pixels<<5)&0x80)];
					*scanline++ = m_palette[((pixels<<6)&0x80)];
					*scanline++ = m_palette[((pixels<<7)&0x80)];
				}
			}
			break;

		case 1: // 2 bpp
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/4; x++)
				{
					uint8_t const pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[(pixels&0xc0)];
					*scanline++ = m_palette[((pixels<<2)&0xc0)];
					*scanline++ = m_palette[((pixels<<4)&0xc0)];
					*scanline++ = m_palette[((pixels<<6)&0xc0)];
				}
			}
			break;

		case 2: // 4 bpp
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/2; x++)
				{
					uint8_t const pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[(pixels&0xf0)];
					*scanline++ = m_palette[((pixels&0x0f)<<4)];
				}
			}
			break;

		case 3: // 8 bpp
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640; x++)
				{
					uint8_t const pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_palette[pixels];
				}
			}
			break;

		case 4: // 24 bpp
			{
				uint32_t const *const vram32 = (uint32_t *)&m_vram[0];

				for (int y = 0; y < 480; y++)
				{
					std::copy_n(&vram32[y * 1024], 640, &bitmap.pix(y));
				}
			}
			break;

		default:
			fatalerror("cb264se30: unknown video mode %d\n", m_mode);
	}
	return 0;
}

void nubus_cb264se30_device::cb264se30_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x38003:           // mode
//            if (data != 0x08000000) printf("%08x to mode\n", data);
			switch (data & 0xff000000)
			{
				case 0x38000000:
					m_mode = 0;
					break;

				case 0x39000000:
					m_mode = 1;
					break;

				case 0x3a000000:
					m_mode = 2;
					break;

				case 0x3b000000:
					m_mode = 3;
					break;

				case 0x3f000000:
					m_mode = 4;
					break;
			}
			break;

		case 0x38000:
			if (mem_mask == 0xff000000)
			{
		//          logerror("%08x to DAC control %s\n", data, machine().describe_context());
					m_clutoffs = (data>>24)&0xff;
			}
			else if (mem_mask == 0x0000ff00)
			{
					m_colors[m_count++] = (data>>8) & 0xff;

					if (m_count == 3)
					{
//                        logerror("RAMDAC: color %02x = %02x %02x %02x %s\n", m_clutoffs, m_colors[0], m_colors[1], m_colors[2], machine().describe_context());
						m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
						m_clutoffs++;
						if (m_clutoffs > 255)
						{
							m_clutoffs = 0;
						}
						m_count = 0;
					}
			}
			break;

		case 0x2c017:   // VBL control
			if (data & 0x06000000)
			{
				m_vbl_disable = 0;
				lower_slot_irq();
			}
			else
			{
				m_vbl_disable = 1;
			}
			break;

		default:
//            logerror("cb264se30_w: %08x @ %x, mask %08x %s\n", data, offset, mem_mask, machine().describe_context());
			break;
	}
}

uint32_t nubus_cb264se30_device::cb264se30_r(offs_t offset, uint32_t mem_mask)
{
	return 0;
}

void nubus_cb264se30_device::vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vram32[offset]);
}

uint32_t nubus_cb264se30_device::vram_r(offs_t offset, uint32_t mem_mask)
{
	return m_vram32[offset];
}
