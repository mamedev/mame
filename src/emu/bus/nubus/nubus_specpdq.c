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

#define SPECPDQ_SCREEN_NAME "specpdq_screen"
#define SPECPDQ_ROM_REGION  "specpdq_rom"

#define VRAM_SIZE   (0x400000)

MACHINE_CONFIG_FRAGMENT( specpdq )
	MCFG_SCREEN_ADD( SPECPDQ_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, nubus_specpdq_device, screen_update)
	MCFG_SCREEN_RAW_PARAMS(25175000, 800, 0, 640, 525, 0, 480)
	MCFG_SCREEN_SIZE(1280,1024)
	MCFG_SCREEN_VISIBLE_AREA(0, 1152-1, 0, 844-1)

	MCFG_PALETTE_ADD("palette", 256)
MACHINE_CONFIG_END

ROM_START( specpdq )
	ROM_REGION(0x10000, SPECPDQ_ROM_REGION, 0)
	ROM_LOAD( "specpdq.bin",  0x000000, 0x010000, CRC(82a35f78) SHA1(9511c2df47140f4279196d3b8836b53429879dd9) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NUBUS_SPECPDQ = &device_creator<nubus_specpdq_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nubus_specpdq_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( specpdq );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *nubus_specpdq_device::device_rom_region() const
{
	return ROM_NAME( specpdq );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_specpdq_device - constructor
//-------------------------------------------------

nubus_specpdq_device::nubus_specpdq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, NUBUS_SPECPDQ, "SuperMac Spectrum PDQ video card", tag, owner, clock, "nb_spdq", __FILE__),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this),
		m_palette(*this, "palette")
{
	m_assembled_tag = std::string(tag).append(":").append(SPECPDQ_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

nubus_specpdq_device::nubus_specpdq_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this),
		m_palette(*this, "palette")
{
	m_assembled_tag = std::string(tag).append(":").append(SPECPDQ_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_specpdq_device::device_start()
{
	UINT32 slotspace;

	// set_nubus_device makes m_slot valid
	set_nubus_device();
	install_declaration_rom(this, SPECPDQ_ROM_REGION);

	slotspace = get_slotspace();

//  printf("[specpdq %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE);
	m_vram32 = (UINT32 *)&m_vram[0];
	m_nubus->install_device(slotspace, slotspace+VRAM_SIZE-1, read32_delegate(FUNC(nubus_specpdq_device::vram_r), this), write32_delegate(FUNC(nubus_specpdq_device::vram_w), this));
	m_nubus->install_device(slotspace+0x400000, slotspace+0xfbffff, read32_delegate(FUNC(nubus_specpdq_device::specpdq_r), this), write32_delegate(FUNC(nubus_specpdq_device::specpdq_w), this));

	m_timer = timer_alloc(0, NULL);
	m_timer->adjust(m_screen->time_until_pos(843, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_specpdq_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_vbl_disable = 1;
	m_mode = 0;
	memset(&m_vram[0], 0, VRAM_SIZE);
	memset(m_palette_val, 0, sizeof(m_palette_val));

	m_palette_val[0] = rgb_t(255, 255, 255);
	m_palette_val[0x80] = rgb_t(0, 0, 0);
}


void nubus_specpdq_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(m_screen->time_until_pos(843, 0), 0);
}

/***************************************************************************

  Spectrum 24 PDQ section

***************************************************************************/

UINT32 nubus_specpdq_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;
	UINT8 pixels, *vram;

	// first time?  kick off the VBL timer
	vram = &m_vram[0x9000];

	switch (m_mode)
	{
		case 0: // 1 bpp
			for (y = 0; y < 844; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 1152/8; x++)
				{
					pixels = vram[(y * 512) + (BYTE4_XOR_BE(x))];

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
			for (y = 0; y < 844; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 1152/4; x++)
				{
					pixels = vram[(y * 512) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette_val[(pixels&0xc0)];
					*scanline++ = m_palette_val[((pixels<<2)&0xc0)];
					*scanline++ = m_palette_val[((pixels<<4)&0xc0)];
					*scanline++ = m_palette_val[((pixels<<6)&0xc0)];
				}
			}
			break;

		case 2: // 4 bpp
			for (y = 0; y < 844; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < 1152/2; x++)
				{
					pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette_val[(pixels&0xf0)];
					*scanline++ = m_palette_val[((pixels<<4)&0xf0)];
				}
			}
			break;

		case 3: // 8 bpp
			for (y = 0; y < 844; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < 1152; x++)
				{
					pixels = vram[(y * 1152) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_palette_val[pixels];
				}
			}
			break;

		default:
			fatalerror("specpdq: unknown video mode %d\n", m_mode);
	}
	return 0;
}

WRITE32_MEMBER( nubus_specpdq_device::specpdq_w )
{
	if (offset >= 0xc0000 && offset < 0x100000)
	{
		COMBINE_DATA(&m_7xxxxx_regs[offset-0xc0000]);
	}

	switch (offset)
	{
		case 0xc0054:   // mode 1
//          printf("%x to mode1\n", data);
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
//          printf("%x to mode2\n", data);

			switch (data)
			{
				case 0xff7fffff:
					m_mode = 0;
					break;

				case 0xfeffffff:
					m_mode = 2;
					break;

				case 0xfedfffff:
					m_mode = 3;
					break;
			}

//          printf("m_mode = %d\n", m_mode);
			break;

		case 0x120000:  // DAC address
//          printf("%08x to DAC control (PC=%x)\n", data, space.device().safe_pc());
			m_clutoffs = ((data>>8)&0xff)^0xff;
			break;

		case 0x120001:  // DAC data
			m_colors[m_count++] = ((data>>8)&0xff)^0xff;

			if (m_count == 3)
			{
//              printf("RAMDAC: color %d = %02x %02x %02x (PC=%x)\n", m_clutoffs, m_colors[0], m_colors[1], m_colors[2], space.device().safe_pc() );
				m_palette->set_pen_color(m_clutoffs, rgb_t(m_colors[0], m_colors[1], m_colors[2]));
				m_palette_val[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
				m_clutoffs++;
				if (m_clutoffs > 255)
				{
					m_clutoffs = 0;
				}
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
//          printf("Pattern %08x @ %x\n", data ^ 0xffffffff, offset);
			m_fillbytes[((offset&0x3f)*4)] = ((data>>24) & 0xff) ^ 0xff;
			m_fillbytes[((offset&0x3f)*4)+1] = ((data>>16) & 0xff) ^ 0xff;
			m_fillbytes[((offset&0x3f)*4)+2] = ((data>>8) & 0xff) ^ 0xff;
			m_fillbytes[((offset&0x3f)*4)+3] = (data& 0xff) ^ 0xff;
			break;

		// blitter control
		case 0x182006:
//          printf("%08x (%d) to blitter ctrl 1 (PC=%x)\n", data^0xffffffff, data^0xffffffff, space.device().safe_pc());
			break;

		case 0x182008:
//          printf("%08x (%d) to blitter ctrl 2 (PC=%x)\n", data^0xffffffff, data^0xffffffff, space.device().safe_pc());
			m_patofsx = (data ^ 0xffffffff) & 7;
			m_patofsy = ((data ^ 0xffffffff)>>3) & 7;
			break;

		case 0x18200e:
//          printf("%08x (%d) to blitter ctrl 3 (PC=%x)\n", data^0xffffffff, data^0xffffffff, space.device().safe_pc());
			m_width = data ^ 0xffffffff;
			break;

		case 0x18200b:
//          printf("%08x (%d) to blitter ctrl 4 (PC=%x)\n", data^0xffffffff, data^0xffffffff, space.device().safe_pc());
			m_height = (data ^ 0xffffffff) & 0xffff;
			break;

		case 0x18200a:
			data ^= 0xffffffff;
//          printf("%08x to blitter ctrl 5 (PC=%x)\n", data, space.device().safe_pc());
			m_vram_src = data>>2;
			break;

		case 0x182009:
			data ^= 0xffffffff;
//          printf("%08x to blitter ctrl 6 (PC=%x)\n", data, space.device().safe_pc());
			m_vram_addr = data>>2;
			break;

		case 0x182007:
			data ^= 0xffffffff;
//          printf("%08x to blitter ctrl 7 (PC=%x)\n", data, space.device().safe_pc());

			// fill rectangle
			if (data == 2)
			{
				int x, y;
				UINT8 *vram = &m_vram[m_vram_addr + m_patofsx]; // m_vram_addr is missing the low 2 bits, we add them back here

//              printf("Fill rectangle with %02x %02x %02x %02x, width %d height %d\n", m_fillbytes[0], m_fillbytes[1], m_fillbytes[2], m_fillbytes[3], m_width, m_height);

				for (y = 0; y < m_height; y++)
				{
					for (x = 0; x < m_width; x++)
					{
						vram[(y * 1152)+x] = m_fillbytes[((m_patofsx + x) & 0x1f)+(((m_patofsy + y) & 0x7)*32)];
					}
				}
			}
			else if ((data == 0x101) || (data == 0x100))
			{
				int x, y;
				UINT8 *vram = &m_vram[m_vram_addr];
				UINT8 *vramsrc = &m_vram[m_vram_src];

//              printf("Copy rectangle, width %d height %d  src %x dst %x\n", m_width, m_height, m_vram_addr, m_vram_src);

				for (y = 0; y < m_height; y++)
				{
					for (x = 0; x < m_width; x++)
					{
						vram[(y * 1152)+x] = vramsrc[(y * 1152)+x];
					}
				}
			}
			else
			{
				printf("Unknown blitter command %08x\n", data);
			}
			break;

		default:
//          printf("specpdq_w: %08x @ %x (mask %08x  PC=%x)\n", data^0xffffffff, offset, mem_mask, space.device().safe_pc());
			break;
	}
}

READ32_MEMBER( nubus_specpdq_device::specpdq_r )
{
//  if (offset != 0xc005c && offset != 0xc005e) printf("specpdq_r: @ %x (mask %08x  PC=%x)\n", offset, mem_mask, space.device().safe_pc());

	if (offset >= 0xc0000 && offset < 0x100000)
	{
		return m_7xxxxx_regs[offset-0xc0000];
	}

	return 0;
}

WRITE32_MEMBER( nubus_specpdq_device::vram_w )
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram32[offset]);
}

READ32_MEMBER( nubus_specpdq_device::vram_r )
{
	return m_vram32[offset] ^ 0xffffffff;
}
