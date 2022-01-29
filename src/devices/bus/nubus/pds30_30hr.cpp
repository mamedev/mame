// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Micron/XCEED Technologies Color 30HR

  Fs800000 - Mode A
  FsA00000 - Mode B
  FsC00000 - RAMDAC write offset
  FsC00004 - RAMDAC write data
  FsC00008 - RAMDAC write mask
  FsC0000C - RAMDAC read offset

***************************************************************************/

#include "emu.h"
#include "pds30_30hr.h"
#include "screen.h"

#define XCEED30HR_SCREEN_NAME "x30hr_screen"
#define XCEED30HR_ROM_REGION  "x30hr_rom"

#define VRAM_SIZE   (0x100000)  // 1 MB VRAM - max mode is 1024x768 @ 8bpp


ROM_START( xceed30hr )
	ROM_REGION(0x8000, XCEED30HR_ROM_REGION, 0)
	ROM_LOAD( "369c.rom",     0x000000, 0x008000, CRC(b22f0a89) SHA1(be34c8604b8a1ae9c9f3b0b90faba9a1a64a5855) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PDS030_XCEED30HR, nubus_xceed30hr_device, "pd3_30hr", "Micron/XCEED Technology Color 30HR")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_xceed30hr_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, XCEED30HR_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_xceed30hr_device::screen_update));
	screen.set_raw(25175000, 800, 0, 640, 525, 0, 480);
	screen.set_size(1024, 768);
	screen.set_visarea(0, 640-1, 0, 480-1);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_xceed30hr_device::device_rom_region() const
{
	return ROM_NAME( xceed30hr );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_xceed30hr_device - constructor
//-------------------------------------------------

nubus_xceed30hr_device::nubus_xceed30hr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_xceed30hr_device(mconfig, PDS030_XCEED30HR, tag, owner, clock)
{
}

nubus_xceed30hr_device::nubus_xceed30hr_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_vram32(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	set_screen(*this, XCEED30HR_SCREEN_NAME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_xceed30hr_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(this, XCEED30HR_ROM_REGION);

	slotspace = get_slotspace();

//  printf("[xceed30hr %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE);
	m_vram32 = (uint32_t *)&m_vram[0];

	nubus().install_device(slotspace, slotspace+VRAM_SIZE-1, read32s_delegate(*this, FUNC(nubus_xceed30hr_device::vram_r)), write32s_delegate(*this, FUNC(nubus_xceed30hr_device::vram_w)));
	nubus().install_device(slotspace+0x800000, slotspace+0xefffff, read32s_delegate(*this, FUNC(nubus_xceed30hr_device::xceed30hr_r)), write32s_delegate(*this, FUNC(nubus_xceed30hr_device::xceed30hr_w)));

	m_timer = timer_alloc(0);
	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_xceed30hr_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_vbl_disable = 1;
	m_mode = 0;
	memset(&m_vram[0], 0, VRAM_SIZE);
	memset(m_palette, 0, sizeof(m_palette));

	m_palette[0] = rgb_t(255, 255, 255);
	m_palette[0x80] = rgb_t(0, 0, 0);
}


void nubus_xceed30hr_device::device_timer(emu_timer &timer, device_timer_id tid, int param)
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

uint32_t nubus_xceed30hr_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const vram = &m_vram[1024];

	switch (m_mode)
	{
		case 0: // 1 bpp?
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/8; x++)
				{
					uint8_t const pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];

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
			break;

		case 1: // 2 bpp
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 640/4; x++)
				{
					uint8_t const pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[((pixels>>6)&3)];
					*scanline++ = m_palette[((pixels>>4)&3)];
					*scanline++ = m_palette[((pixels>>2)&3)];
					*scanline++ = m_palette[(pixels&3)];
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

					*scanline++ = m_palette[(pixels>>4)];
					*scanline++ = m_palette[(pixels&0xf)];
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

		default:
			fatalerror("xceed30hr: unknown video mode %d\n", m_mode);
	}
	return 0;
}

void nubus_xceed30hr_device::xceed30hr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x80000:           // mode
			switch (data & 0xff000000)
			{
				case 0xfc000000:
					m_mode = 0;
					break;

				case 0xfd000000:
					m_mode = 1;
					break;

				case 0xfe000000:
					m_mode = 2;
					break;

				case 0xff000000:
					m_mode = 3;
					break;
			}
			break;

		case 0x80005:   // ack VBL
			lower_slot_irq();
			break;

		case 0x100000:
//            logerror("%08x to DAC control %s\n", data, machine().describe_context());
			m_clutoffs = (data&0xff);
			m_count = 0;
			break;

		case 0x100001:
//            logerror("%08x to DAC data %s\n", data, machine().describe_context());
			m_colors[m_count++] = (data & 0xff);

			if (m_count == 3)
			{
//                logerror("RAMDAC: color %02x = %02x %02x %02x %s\n", m_clutoffs, m_colors[0], m_colors[1], m_colors[2], machine().describe_context());
				m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
				m_clutoffs++;
				if (m_clutoffs > 255)
				{
					m_clutoffs = 0;
				}
				m_count = 0;
			}
			break;

		case 0x100002:  // VBL control
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
//            logerror("xceed30hr_w: %08x @ %x, mask %08x %s\n", data, offset, mem_mask, machine().describe_context());
			break;
	}
}

uint32_t nubus_xceed30hr_device::xceed30hr_r(offs_t offset, uint32_t mem_mask)
{
//    logerror("xceed30hr_r: @ %x, mask %08x %s\n", offset, mem_mask, machine().describe_context());
	if (offset == 0x80008)
	{
		m_toggle ^= 0x80;
		return m_toggle;
	}

	return 0;
}

void nubus_xceed30hr_device::vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vram32[offset]);
}

uint32_t nubus_xceed30hr_device::vram_r(offs_t offset, uint32_t mem_mask)
{
	return m_vram32[offset];
}
