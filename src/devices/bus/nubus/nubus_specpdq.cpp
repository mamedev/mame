// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  SuperMac Spectrum PDQ video card

  Accelerated only in 256 color mode.  Accleration is not yet emulated
  properly (pattern fill works but has glitches).  Use in B&W or 16 colors
  for full functionality right now.

  blitter info:

  ctrl 1 = ?
  ctrl 2 = low 3 bits of Y position in bits 3-5, low 3 bits of X position in bits 0-2
  ctrl 3 = width
  ctrl 4 = height
  ctrl 5 = ?
  ctrl 6 = VRAM offset * 4
  ctrl 7 = command/execute (00000002 for pattern fill, 00000100 for copy)

  Busy flag at Fs800000 (bit 8)

  There is 256 bytes of pattern RAM arranged as 32 pixels horizontally by 8
  vertically.

***************************************************************************/

#include "emu.h"
#include "nubus_specpdq.h"
#include "screen.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


#define SPECPDQ_SCREEN_NAME "specpdq_screen"
#define SPECPDQ_ROM_REGION  "specpdq_rom"

#define VRAM_SIZE   (0x400000)


ROM_START( specpdq )
	ROM_REGION(0x10000, SPECPDQ_ROM_REGION, 0)
	ROM_LOAD( "specpdq.bin",  0x000000, 0x010000, CRC(82a35f78) SHA1(9511c2df47140f4279196d3b8836b53429879dd9) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_SPECPDQ, nubus_specpdq_device, "nb_spdq", "SuperMac Spectrum PDQ video card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_specpdq_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SPECPDQ_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_specpdq_device::screen_update));
	screen.set_raw(25175000, 800, 0, 640, 525, 0, 480);
	screen.set_size(1280, 1024);
	screen.set_visarea(0, 1152-1, 0, 844-1);

	PALETTE(config, m_palette).set_entries(256);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_specpdq_device::device_rom_region() const
{
	return ROM_NAME( specpdq );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_specpdq_device - constructor
//-------------------------------------------------

nubus_specpdq_device::nubus_specpdq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_specpdq_device(mconfig, NUBUS_SPECPDQ, tag, owner, clock)
{
}

nubus_specpdq_device::nubus_specpdq_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_palette(*this, "palette"),
	m_timer(nullptr),
	m_mode(0), m_vbl_disable(0),
	m_count(0), m_clutoffs(0),
	m_width(0), m_height(0), m_patofsx(0), m_patofsy(0), m_vram_addr(0), m_vram_src(0)
{
	set_screen(*this, SPECPDQ_SCREEN_NAME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_specpdq_device::device_start()
{
	install_declaration_rom(SPECPDQ_ROM_REGION);

	uint32_t const slotspace = get_slotspace();
	LOG("[specpdq %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE / sizeof(uint32_t));
	nubus().install_device(slotspace, slotspace+VRAM_SIZE-1, read32s_delegate(*this, FUNC(nubus_specpdq_device::vram_r)), write32s_delegate(*this, FUNC(nubus_specpdq_device::vram_w)));
	nubus().install_device(slotspace+0x400000, slotspace+0xfbffff, read32s_delegate(*this, FUNC(nubus_specpdq_device::specpdq_r)), write32s_delegate(*this, FUNC(nubus_specpdq_device::specpdq_w)));

	m_timer = timer_alloc(FUNC(nubus_specpdq_device::vbl_tick), this);
	m_timer->adjust(screen().time_until_pos(843, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_specpdq_device::device_reset()
{
	std::fill(m_vram.begin(), m_vram.end(), 0);
	m_mode = 0;
	m_vbl_disable = 1;
	std::fill(std::begin(m_palette_val), std::end(m_palette_val), 0);
	std::fill(std::begin(m_colors), std::end(m_colors), 0);
	m_count = 0;
	m_clutoffs = 0;

	std::fill(std::begin(m_7xxxxx_regs), std::end(m_7xxxxx_regs), 0);
	m_width = 0;
	m_height = 0;
	m_patofsx = 0;
	m_patofsy = 0;
	m_vram_addr = 0;
	m_vram_src = 0;

	m_palette_val[0] = rgb_t(255, 255, 255);
	m_palette_val[0x80] = rgb_t(0, 0, 0);
}


TIMER_CALLBACK_MEMBER(nubus_specpdq_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(screen().time_until_pos(843, 0), 0);
}

/***************************************************************************

  Spectrum 24 PDQ section

***************************************************************************/

uint32_t nubus_specpdq_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// first time?  kick off the VBL timer
	auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]) + 0x9000;

	switch (m_mode)
	{
		case 0: // 1 bpp
			for (int y = 0; y < 844; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 1152/8; x++)
				{
					uint8_t const pixels = vram8[(y * 512) + x];

					*scanline++ = m_palette_val[(pixels&0x80)];
					*scanline++ = m_palette_val[((pixels<<1)&0x80)];
					*scanline++ = m_palette_val[((pixels<<2)&0x80)];
					*scanline++ = m_palette_val[((pixels<<3)&0x80)];
					*scanline++ = m_palette_val[((pixels<<4)&0x80)];
					*scanline++ = m_palette_val[((pixels<<5)&0x80)];
					*scanline++ = m_palette_val[((pixels<<6)&0x80)];
					*scanline++ = m_palette_val[((pixels<<7)&0x80)];
				}
			}
			break;

		case 1: // 2 bpp
			for (int y = 0; y < 844; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 1152/4; x++)
				{
					uint8_t const pixels = vram8[(y * 512) + x];

					*scanline++ = m_palette_val[(pixels&0xc0)];
					*scanline++ = m_palette_val[((pixels<<2)&0xc0)];
					*scanline++ = m_palette_val[((pixels<<4)&0xc0)];
					*scanline++ = m_palette_val[((pixels<<6)&0xc0)];
				}
			}
			break;

		case 2: // 4 bpp
			for (int y = 0; y < 844; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 1152/2; x++)
				{
					uint8_t const pixels = vram8[(y * 1024) + x];

					*scanline++ = m_palette_val[(pixels&0xf0)];
					*scanline++ = m_palette_val[((pixels<<4)&0xf0)];
				}
			}
			break;

		case 3: // 8 bpp
			for (int y = 0; y < 844; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 1152; x++)
				{
					uint8_t const pixels = vram8[(y * 1152) + x];
					*scanline++ = m_palette_val[pixels];
				}
			}
			break;

		default:
			fatalerror("specpdq: unknown video mode %d\n", m_mode);
	}
	return 0;
}

void nubus_specpdq_device::specpdq_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset >= 0xc0000 && offset < 0x100000)
	{
		COMBINE_DATA(&m_7xxxxx_regs[offset-0xc0000]);
	}

	switch (offset)
	{
		case 0xc0054:   // mode 1
			LOG("%x to mode1\n", data);
			break;

		case 0xc005c:   // interrupt control
			if (!(data & 0x8000))
			{
				m_vbl_disable = 1;
			}
			else
			{
				m_vbl_disable = 0;
				lower_slot_irq();
			}
			break;

		case 0xc005e:   // not sure, interrupt related?
			break;

		case 0xc007a:
			LOG("%x to mode2\n", data);

			switch (data)
			{
				case 0xffffffff:
					m_mode = 0;
					break;

				case 0xff7fffff:
					m_mode = 1;
					break;

				case 0xfeffffff:
					m_mode = 2;
					break;

				case 0xfedfffff:
					m_mode = 3;
					break;
			}

			LOG("m_mode = %d\n", m_mode);
			break;

		case 0x120000:  // DAC address
			LOG("%08x to DAC control %s\n", data,machine().describe_context());
			m_clutoffs = ((data >> 8) & 0xff) ^ 0xff;
			break;

		case 0x120001:  // DAC data
			m_colors[m_count++] = ((data >> 8) & 0xff) ^ 0xff;

			if (m_count == 3)
			{
				LOG("RAMDAC: color %d = %02x %02x %02x %s\n", m_clutoffs, m_colors[0], m_colors[1], m_colors[2], machine().describe_context());
				m_palette->set_pen_color(m_clutoffs, rgb_t(m_colors[0], m_colors[1], m_colors[2]));
				m_palette_val[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
				m_clutoffs = (m_clutoffs + 1) & 0xff;
				m_count = 0;
			}
			break;

		// blitter texture? pattern?  256 pixels worth at 8bpp
		case 0x181000:
		case 0x181001:
		case 0x181002:
		case 0x181003:
		case 0x181004:
		case 0x181005:
		case 0x181006:
		case 0x181007:
		case 0x181008:
		case 0x181009:
		case 0x18100a:
		case 0x18100b:
		case 0x18100c:
		case 0x18100d:
		case 0x18100e:
		case 0x18100f:
		case 0x181010:
		case 0x181011:
		case 0x181012:
		case 0x181013:
		case 0x181014:
		case 0x181015:
		case 0x181016:
		case 0x181017:
		case 0x181018:
		case 0x181019:
		case 0x18101a:
		case 0x18101b:
		case 0x18101c:
		case 0x18101d:
		case 0x18101e:
		case 0x18101f:
		case 0x181020:
		case 0x181021:
		case 0x181022:
		case 0x181023:
		case 0x181024:
		case 0x181025:
		case 0x181026:
		case 0x181027:
		case 0x181028:
		case 0x181029:
		case 0x18102a:
		case 0x18102b:
		case 0x18102c:
		case 0x18102d:
		case 0x18102e:
		case 0x18102f:
		case 0x181030:
		case 0x181031:
		case 0x181032:
		case 0x181033:
		case 0x181034:
		case 0x181035:
		case 0x181036:
		case 0x181037:
		case 0x181038:
		case 0x181039:
		case 0x18103a:
		case 0x18103b:
		case 0x18103c:
		case 0x18103d:
		case 0x18103e:
		case 0x18103f:
			if(offset == 0x181000) {
				machine().debug_break();
				LOG("Pattern %08x @ %x\n", data ^ 0xffffffff, offset);
			}
			m_fillbytes[((offset&0x3f)*4)] = ((data>>24) & 0xff) ^ 0xff;
			m_fillbytes[((offset&0x3f)*4)+1] = ((data>>16) & 0xff) ^ 0xff;
			m_fillbytes[((offset&0x3f)*4)+2] = ((data>>8) & 0xff) ^ 0xff;
			m_fillbytes[((offset&0x3f)*4)+3] = (data& 0xff) ^ 0xff;
			break;

		// blitter control
		case 0x182006:
			LOG("%08x (%d) to blitter ctrl 1 %s rectangle\n", data^0xffffffff, data^0xffffffff, machine().describe_context());
			break;

		case 0x182008:
			LOG("%08x (%d) to blitter ctrl 2 %s rectangle\n", data^0xffffffff, data^0xffffffff, machine().describe_context());
			m_patofsx = (data ^ 0xffffffff) & 7;
			m_patofsy = ((data ^ 0xffffffff)>>3) & 7;
			break;

		case 0x18200e:
			LOG("%08x (%d) to blitter ctrl 3 %s\n", data^0xffffffff, data^0xffffffff, machine().describe_context());
			m_width = data ^ 0xffffffff;
			break;

		case 0x18200b:
			LOG("%08x (%d) to blitter ctrl 4 %s\n", data^0xffffffff, data^0xffffffff, machine().describe_context());
			m_height = (data ^ 0xffffffff) & 0xffff;
			break;

		case 0x18200a:
			data ^= 0xffffffff;
			LOG("%08x to blitter ctrl 5 %s\n", data, machine().describe_context());
			m_vram_src = data>>2;
			break;

		case 0x182009:
			data ^= 0xffffffff;
			LOG("%08x to blitter ctrl 6 %s\n", data, machine().describe_context());
			m_vram_addr = data>>2;
			break;

		case 0x182007:
			data ^= 0xffffffff;
			LOG("%08x to blitter ctrl 7 %s\n", data, machine().describe_context());

			// fill rectangle
			if (data == 2)
			{
				auto const vram8 = util::big_endian_cast<uint8_t>(&m_vram[0]) + m_vram_addr;

				LOG("Fill rectangle with %02x %02x %02x %02x, adr %x (%d, %d) width %d height %d delta %d %d\n", m_fillbytes[0], m_fillbytes[1], m_fillbytes[2], m_fillbytes[3], m_vram_addr, m_vram_addr % 1152, m_vram_addr / 1152, m_width, m_height, m_patofsx, m_patofsy);

				for (int y = 0; y <= m_height; y++)
				{
					for (int x = 0; x <= m_width; x++)
					{
						vram8[(y * 1152)+x] = m_fillbytes[((m_patofsx + x) & 0x1f)+(((m_patofsy + y) & 0x7) << 5)];
					}
				}
			}
			else if (data == 0x100)
			{
				auto const vram8 = util::big_endian_cast<uint8_t>(&m_vram[0]) + m_vram_addr;
				auto const vramsrc8 = util::big_endian_cast<uint8_t const>(&m_vram[0]) + m_vram_src;

				LOG("Copy rectangle forwards, width %d height %d dst %x (%d, %d) src %x (%d, %d)\n", m_width, m_height, m_vram_addr, m_vram_addr % 1152, m_vram_addr / 1152, m_vram_src, m_vram_src % 1152, m_vram_src / 1152);

				for (int y = 0; y <= m_height; y++)
				{
					for (int x = 0; x <= m_width; x++)
					{
						vram8[(y * 1152)+x] = vramsrc8[(y * 1152)+x];
					}
				}
			}
			else if (data == 0x101)
			{
				auto const vram8 = util::big_endian_cast<uint8_t>(&m_vram[0]) + m_vram_addr;
				auto const vramsrc8 = util::big_endian_cast<uint8_t const>(&m_vram[0]) + m_vram_src;

				LOG("Copy rectangle backwards, width %d height %d dst %x (%d, %d) src %x (%d, %d)\n", m_width, m_height, m_vram_addr, m_vram_addr % 1152, m_vram_addr / 1152, m_vram_src, m_vram_src % 1152, m_vram_src / 1152);

				for (int y = 0; y < m_height; y++)
				{
					for (int x = 0; x < m_width; x++)
					{
						vram8[(-y * 1152)-x] = vramsrc8[(-y * 1152)-x];
					}
				}
			}
			else
			{
				LOG("Unknown blitter command %08x\n", data);
			}
			break;

		default:
			LOG("specpdq_w: %08x @ %x (mask %08x  %s)\n", data^0xffffffff, offset, mem_mask, machine().describe_context());
			break;
	}
}

uint32_t nubus_specpdq_device::specpdq_r(offs_t offset, uint32_t mem_mask)
{
//  if (offset != 0xc005c && offset != 0xc005e) logerror("specpdq_r: @ %x (mask %08x  %s)\n", offset, mem_mask, machine().describe_context());

	if (offset >= 0xc0000 && offset < 0x100000)
	{
		return m_7xxxxx_regs[offset-0xc0000];
	}

	return 0;
}

void nubus_specpdq_device::vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram[offset]);
}

uint32_t nubus_specpdq_device::vram_r(offs_t offset, uint32_t mem_mask)
{
	return m_vram[offset] ^ 0xffffffff;
}
