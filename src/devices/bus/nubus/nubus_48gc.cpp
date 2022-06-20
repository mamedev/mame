// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Apple 4*8 Graphics Card (model 630-0400) emulation
  Apple 8*24 Graphics Card emulation (cards have the same framebuffer chip
      w/different ROMs and RAMDACs, apparently)

***************************************************************************/

#include "emu.h"
#include "nubus_48gc.h"
#include "screen.h"

#include <algorithm>


#define VRAM_SIZE  (0x200000)  // 2 megs, maxed out

#define GC48_SCREEN_NAME    "48gc_screen"
#define GC48_ROM_REGION     "48gc_rom"

ROM_START( gc48 )
	ROM_REGION(0x8000, GC48_ROM_REGION, 0)
	ROM_LOAD( "3410801.bin",  0x0000, 0x8000, CRC(e283da91) SHA1(4ae21d6d7bbaa6fc7aa301bee2b791ed33b1dcf9) )
ROM_END

ROM_START( gc824 )
	ROM_REGION(0x8000, GC48_ROM_REGION, 0)
	ROM_LOAD( "3410868.bin",  0x000000, 0x008000, CRC(57f925fa) SHA1(4d3c0632711b7b31c8e0c5cfdd7ec1904f178336) ) /* Label: "341-0868 // (C)APPLE COMPUTER // INC. 1986-1991 // ALL RIGHTS // RESERVED    W5" */
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_48GC,  nubus_48gc_device,  "nb_48gc",  "Apple Macintosh Display Card 4*8")
DEFINE_DEVICE_TYPE(NUBUS_824GC, nubus_824gc_device, "nb_824gc", "Apple Macintosh Display Card 8*24")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void jmfb_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, GC48_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(jmfb_device::screen_update));
	screen.set_raw(25175000, 800, 0, 640, 525, 0, 480);
//  screen.set_size(1152, 870);
//  screen.set_visarea(0, 1152-1, 0, 870-1);
//  screen.set_refresh_hz(75);
//  screen.set_vblank_time(ATTOSECONDS_IN_USEC(1260));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *jmfb_device::device_rom_region() const
{
	return ROM_NAME( gc48 );
}

const tiny_rom_entry *nubus_824gc_device::device_rom_region() const
{
	return ROM_NAME( gc824 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  jmfb_device - constructor
//-------------------------------------------------

jmfb_device::jmfb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool is824) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_screen(nullptr), m_timer(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_stride(0), m_base(0), m_count(0), m_clutoffs(0), m_xres(0), m_yres(0),
	m_is824(is824)
{
	set_screen(*this, GC48_SCREEN_NAME);
}

nubus_48gc_device::nubus_48gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jmfb_device(mconfig, NUBUS_48GC, tag, owner, clock, false)
{
}

nubus_824gc_device::nubus_824gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jmfb_device(mconfig, NUBUS_824GC, tag, owner, clock, true)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void jmfb_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(GC48_ROM_REGION);

	slotspace = get_slotspace();

//  printf("[JMFB %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE / sizeof(uint32_t));
	install_bank(slotspace, slotspace+VRAM_SIZE-1, &m_vram[0]);

	nubus().install_device(slotspace+0x200000, slotspace+0x2003ff, read32s_delegate(*this, FUNC(jmfb_device::mac_48gc_r)), write32s_delegate(*this, FUNC(jmfb_device::mac_48gc_w)));

	m_timer = timer_alloc(FUNC(jmfb_device::vbl_tick), this);
	m_screen = nullptr;    // can we look this up now?
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void jmfb_device::device_reset()
{
	m_toggle = 0;
	m_clutoffs = 0;
	m_count = 0;
	m_vbl_disable = 1;
	m_stride = 80/4;
	m_base = 0;
	m_xres = 640;
	m_yres = 480;
	m_mode = 0;
	std::fill(m_vram.begin(), m_vram.end(), 0);
	memset(m_palette, 0, sizeof(m_palette));
}

/***************************************************************************

  Apple 4*8 Graphics Card section

***************************************************************************/

TIMER_CALLBACK_MEMBER(jmfb_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(m_screen->time_until_pos(479, 0), 0);
}

uint32_t jmfb_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]) + (m_base << 5);

	// first time?  kick off the VBL timer
	if (!m_screen)
	{
		m_screen = &screen;
		m_timer->adjust(m_screen->time_until_pos(479, 0), 0);
	}

	switch (m_mode)
	{
		case 0: // 1bpp
			for (int y = 0; y < m_yres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_xres/8; x++)
				{
					uint8_t const pixels = vram8[(y * m_stride * 4) + x];

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

		case 1: // 2bpp
			for (int y = 0; y < m_yres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_xres/4; x++)
				{
					uint8_t const pixels = vram8[(y * m_stride * 4) + x];

					*scanline++ = m_palette[(pixels>>6)&0x3];
					*scanline++ = m_palette[(pixels>>4)&0x3];
					*scanline++ = m_palette[(pixels>>2)&0x3];
					*scanline++ = m_palette[pixels&3];
				}
			}
			break;

		case 2: // 4 bpp
			for (int y = 0; y < m_yres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_xres/2; x++)
				{
					uint8_t const pixels = vram8[(y * m_stride * 4) + x];

					*scanline++ = m_palette[(pixels>>4)&0xf];
					*scanline++ = m_palette[pixels&0xf];
				}
			}
			break;

		case 3: // 8 bpp
			for (int y = 0; y < m_yres; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_xres; x++)
				{
					uint8_t const pixels = vram8[(y * m_stride * 4) + x];
					*scanline++ = m_palette[pixels];
				}
			}
			break;

		case 4: // 24 bpp
			{
				uint32_t const base = (m_base * 8 / 3) << 3;
				uint32_t const stride = m_stride * 8 / 3;
				for (int y = 0; y < m_yres; y++)
				{
					std::copy_n(&m_vram[base + (y * stride)], m_xres, &bitmap.pix(y));
				}
			}
			break;
	}

	return 0;
}

void jmfb_device::mac_48gc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_registers[offset&0xff]);

	switch (offset)
	{
		case 0x8/4: // base
//          printf("%x to base\n", data);
			m_base = data;
			break;

		case 0x00c/4: // stride
//          printf("%x to stride\n", data);
			// this value is in DWORDs for 1-8 bpp and, uhh, strange for 24bpp
			m_stride = data;
			break;

		case 0x200/4:   // DAC control
//          printf("%08x to DAC control\n", data);
			if (m_is824)
			{
				m_clutoffs = data&0xff;
			}
			else
			{
				m_clutoffs = data>>24;
			}
			m_count = 0;
			break;

		case 0x204/4:   // DAC data
			if (m_is824)
			{
				m_colors[m_count++] = data&0xff;
			}
			else
			{
				m_colors[m_count++] = data>>24;
			}

			if (m_count == 3)
			{
//              printf("RAMDAC: color %d = %02x %02x %02x\n", m_clutoffs, m_colors[0], m_colors[1], m_colors[2]);
				m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
				m_clutoffs++;
				m_count = 0;
			}
			break;

		case 0x208/4:   // mode control
			m_mode = (data>>3)&3;
			if (m_mode == 3)    // this can be 8 or 24 bpp
			{
				// check pixel format for 24bpp
				if (m_is824)
				{
					if (data & 2)
					{
						m_mode = 4; // 24bpp
					}
				}
				else
				{
					if (((data>>5)&3) == 0)
					{
						m_mode = 4; // 24bpp
					}
				}
			}
//          printf("%02x to mode (m_mode = %d)\n", data, m_mode);
			break;

		case 0x13c/4:   // bit 1 = VBL disable (1=no interrupts)
			m_vbl_disable = (data & 2) ? 1 : 0;
			break;

		case 0x148/4:   // write 1 here to clear interrupt
			if (data == 1)
			{
				lower_slot_irq();
			}
			break;

		default:
			break;
	}
}

uint32_t jmfb_device::mac_48gc_r(offs_t offset, uint32_t mem_mask)
{
//  printf("%s 48gc_r: @ %x, mask %08x\n", machine().describe_context().c_str(), offset, mem_mask);

	switch (offset)
	{
		case 0:
			return 0x0c00;  // sense 13" RGB for now
//          return 0x0000;  // sense "RGB Kong" monitor

		case 0x1c0/4:
			m_toggle ^= 0xffffffff;
			return m_toggle;
	}

	return 0;
}
