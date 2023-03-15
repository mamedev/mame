// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Lapis ProColor Server 8*16 video card

  FsFF6001: DAC color # (seems to have the data bits perfectly reversed)
  FsFF6003: DAC color write (not bitswapped)
  FsFF6017: Mode (13 = 1bpp, 17 = 2bpp, 1b = 4bpp, 1e = 8bpp, 0a = 15bpp)
  FsFF7000: Bit 2 is VBL IRQ enable/ack
  FsFF7001: Bit 0 is VBL status

***************************************************************************/

#include "emu.h"
#include "pds30_procolor816.h"
#include "screen.h"

#include <algorithm>


#define PROCOLOR816_SCREEN_NAME "cb264_screen"
#define PROCOLOR816_ROM_REGION  "cb264_rom"

#define VRAM_SIZE   (0x200000)  // 2 megs?


ROM_START( procolor816 )
	ROM_REGION(0x8000, PROCOLOR816_ROM_REGION, 0)
	ROM_LOAD( "procolor_ver60590.bin", 0x000000, 0x008000, CRC(ebef6168) SHA1(e41ecc7d12fc13bc74f9223ca02920e8a7eb072b) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PDS030_PROCOLOR816, nubus_procolor816_device, "pd3_pc16", "Lapis ProColor Server 8*16")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_procolor816_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, PROCOLOR816_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_procolor816_device::screen_update));
	screen.set_raw(25175000, 800, 0, 640, 525, 0, 480);
	screen.set_size(1024, 768);
	screen.set_visarea(0, 640-1, 0, 480-1);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_procolor816_device::device_rom_region() const
{
	return ROM_NAME( procolor816 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_procolor816_device - constructor
//-------------------------------------------------

nubus_procolor816_device::nubus_procolor816_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_procolor816_device(mconfig, PDS030_PROCOLOR816, tag, owner, clock)
{
}

nubus_procolor816_device::nubus_procolor816_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	set_screen(*this, PROCOLOR816_SCREEN_NAME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_procolor816_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(PROCOLOR816_ROM_REGION);

	slotspace = get_slotspace();

//  printf("[procolor816 %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE / sizeof(uint32_t));

	nubus().install_device(slotspace, slotspace+VRAM_SIZE-1, read32s_delegate(*this, FUNC(nubus_procolor816_device::vram_r)), write32s_delegate(*this, FUNC(nubus_procolor816_device::vram_w)));
	nubus().install_device(slotspace+0x900000, slotspace+VRAM_SIZE-1+0x900000, read32s_delegate(*this, FUNC(nubus_procolor816_device::vram_r)), write32s_delegate(*this, FUNC(nubus_procolor816_device::vram_w)));
	nubus().install_device(slotspace+0xf00000, slotspace+0xff7fff, read32s_delegate(*this, FUNC(nubus_procolor816_device::procolor816_r)), write32s_delegate(*this, FUNC(nubus_procolor816_device::procolor816_w)));

	m_timer = timer_alloc(FUNC(nubus_procolor816_device::vbl_tick), this);
	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_procolor816_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_vbl_disable = 1;
	m_mode = 3;
	std::fill(m_vram.begin(), m_vram.end(), 0);
	memset(m_palette, 0, sizeof(m_palette));

	m_palette[0] = rgb_t(255, 255, 255);
	m_palette[0x80] = rgb_t(0, 0, 0);
}


TIMER_CALLBACK_MEMBER(nubus_procolor816_device::vbl_tick)
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

uint32_t nubus_procolor816_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]) + 4;

	switch (m_mode)
	{
		case 0: // 1 bpp?
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/8; x++)
				{
					uint8_t const pixels = vram8[(y * 640/8) + x];

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
					uint8_t const pixels = vram8[(y * 640/4) + x];

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
					uint8_t const pixels = vram8[(y * 640/2) + x];

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
					uint8_t const pixels = vram8[(y * 640) + x];
					*scanline++ = m_palette[pixels];
				}
			}
			break;

		case 4: // 15 bpp
			{
				auto const vram16 = util::big_endian_cast<uint16_t const>(&m_vram[0]);

				for (int y = 0; y < 480; y++)
				{
					uint32_t *scanline = &bitmap.pix(y);
					for (int x = 0; x < 640; x++)
					{
						uint16_t const pixels = vram16[(y * 640) + x];
						*scanline++ = rgb_t(pal5bit((pixels>>10) & 0x1f), pal5bit((pixels>>5) & 0x1f), pal5bit(pixels & 0x1f));
					}
				}
			}
			break;

		default:
			fatalerror("procolor816: unknown video mode %d\n", m_mode);
	}
	return 0;
}

void nubus_procolor816_device::procolor816_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x3d805:           // mode
			if (mem_mask == 0xff)
			{
				switch (data & 0xff)
				{
					case 0x13:
						m_mode = 0;
						break;

					case 0x17:
						m_mode = 1;
						break;

					case 0x1b:
						m_mode = 2;
						break;

					case 0x1e:
						m_mode = 3;
						break;

					case 0x0a:
						m_mode = 4;
						break;
				}
			}
			break;

		case 0x3d800:
			if (mem_mask == 0x00ff0000)
			{
		//          printf("%s %08x to DAC control\n", machine().describe_context().c_str(), data);
					m_clutoffs = bitswap<8>((data>>16)&0xff, 0, 1, 2, 3, 4, 5, 6, 7);
			}
			else if (mem_mask == 0x000000ff)
			{
					m_colors[m_count++] = (data & 0xff);

					if (m_count == 3)
					{
//                        printf("%s RAMDAC: color %02x = %02x %02x %02x\n", machine().describe_context().c_str(), m_clutoffs, m_colors[0], m_colors[1], m_colors[2]);
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

	case 0x3dc00:   // VBL control
			if (mem_mask == 0xff000000)
			{
				if (data & 0x04000000)
				{
					m_vbl_disable = 0;
					lower_slot_irq();
				}
				else
				{
					m_vbl_disable = 1;
				}
			}
			break;

		default:
//            printf("%s procolor816_w: %08x @ %x, mask %08x\n", machine().describe_context().c_str(), data, offset, mem_mask);
			break;
	}
}

uint32_t nubus_procolor816_device::procolor816_r(offs_t offset, uint32_t mem_mask)
{
	if (offset == 0x3dc00)
	{
		m_toggle ^= 0xffffffff;
		return m_toggle;
	}
	else if (offset == 0x3d807)
	{
		return 0;
	}
	else
	{
//      printf("procolor816_r: @ %x, mask %08x [PC=%x]\n", offset, mem_mask, machine().device<cpu_device>("maincpu")->pc());
	}

	return 0;
}

void nubus_procolor816_device::vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

uint32_t nubus_procolor816_device::vram_r(offs_t offset, uint32_t mem_mask)
{
	return m_vram[offset];
}
