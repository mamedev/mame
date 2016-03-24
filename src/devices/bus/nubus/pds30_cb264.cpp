// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  RasterOps ColorBoard 264/SE30 video card emulation

***************************************************************************/

#include "emu.h"
#include "pds30_cb264.h"

#define CB264SE30_SCREEN_NAME "cb264_screen"
#define CB264SE30_ROM_REGION  "cb264_rom"

#define VRAM_SIZE   (0x200000)

MACHINE_CONFIG_FRAGMENT( cb264se30 )
	MCFG_SCREEN_ADD( CB264SE30_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, nubus_cb264se30_device, screen_update)
	MCFG_SCREEN_RAW_PARAMS(25175000, 800, 0, 640, 525, 0, 480)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
MACHINE_CONFIG_END

ROM_START( cb264se30 )
	ROM_REGION(0x8000, CB264SE30_ROM_REGION, 0)
	ROM_LOAD( "0002-2019_10-02-90.bin", 0x000000, 0x008000, CRC(5b5b2fab) SHA1(0584deb38b402718f2abef456b0035b34fddb473) )  // EPROM label "264/30 V1.3 0002-2019 10/02/90"
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type PDS030_CB264SE30 = &device_creator<nubus_cb264se30_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nubus_cb264se30_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cb264se30 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *nubus_cb264se30_device::device_rom_region() const
{
	return ROM_NAME( cb264se30 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_cb264se30_device - constructor
//-------------------------------------------------

nubus_cb264se30_device::nubus_cb264se30_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, PDS030_CB264SE30, "RasterOps Colorboard 264/SE30", tag, owner, clock, "pd3_c264", __FILE__),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this), m_vram32(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	m_assembled_tag = std::string(tag).append(":").append(CB264SE30_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

nubus_cb264se30_device::nubus_cb264se30_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this), m_vram32(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	m_assembled_tag = std::string(tag).append(":").append(CB264SE30_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_cb264se30_device::device_start()
{
	UINT32 slotspace;

	// set_nubus_device makes m_slot valid
	set_nubus_device();
	install_declaration_rom(this, CB264SE30_ROM_REGION);

	slotspace = get_slotspace();

//  printf("[cb264se30 %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE);
	m_vram32 = (UINT32 *)&m_vram[0];

	m_nubus->install_device(slotspace, slotspace+VRAM_SIZE-1, read32_delegate(FUNC(nubus_cb264se30_device::vram_r), this), write32_delegate(FUNC(nubus_cb264se30_device::vram_w), this));
	m_nubus->install_device(slotspace+0xf00000, slotspace+0xfeffff, read32_delegate(FUNC(nubus_cb264se30_device::cb264se30_r), this), write32_delegate(FUNC(nubus_cb264se30_device::cb264se30_w), this));

	m_timer = timer_alloc(0, nullptr);
	m_timer->adjust(m_screen->time_until_pos(479, 0), 0);
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


void nubus_cb264se30_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(m_screen->time_until_pos(479, 0), 0);
}

/***************************************************************************

  CB264 section

***************************************************************************/

UINT32 nubus_cb264se30_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;
	UINT8 pixels, *vram;

	vram = &m_vram[8*1024];

	switch (m_mode)
	{
		case 0: // 1 bpp?
			for (y = 0; y < 480; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 640/8; x++)
				{
					pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];

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
			for (y = 0; y < 480; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 640/4; x++)
				{
					pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[(pixels&0xc0)];
					*scanline++ = m_palette[((pixels<<2)&0xc0)];
					*scanline++ = m_palette[((pixels<<4)&0xc0)];
					*scanline++ = m_palette[((pixels<<6)&0xc0)];
				}
			}
			break;

		case 2: // 4 bpp
			for (y = 0; y < 480; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < 640/2; x++)
				{
					pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[(pixels&0xf0)];
					*scanline++ = m_palette[((pixels&0x0f)<<4)];
				}
			}
			break;

		case 3: // 8 bpp
			for (y = 0; y < 480; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < 640; x++)
				{
					pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_palette[pixels];
				}
			}
			break;

		case 4: // 24 bpp
			{
				UINT32 *vram32 = (UINT32 *)&m_vram[0];
				UINT32 *base;

				for (y = 0; y < 480; y++)
				{
					scanline = &bitmap.pix32(y);
					base = &vram32[y * 1024];
					for (x = 0; x < 640; x++)
					{
						*scanline++ = *base++;
					}
				}
			}
			break;

		default:
			fatalerror("cb264se30: unknown video mode %d\n", m_mode);
	}
	return 0;
}

WRITE32_MEMBER( nubus_cb264se30_device::cb264se30_w )
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
		//          printf("%08x to DAC control (PC=%x)\n", data, space.device().safe_pc());
					m_clutoffs = (data>>24)&0xff;
			}
			else if (mem_mask == 0x0000ff00)
			{
					m_colors[m_count++] = (data>>8) & 0xff;

					if (m_count == 3)
					{
//                        printf("RAMDAC: color %02x = %02x %02x %02x (PC=%x)\n", m_clutoffs, m_colors[0], m_colors[1], m_colors[2], space.device().safe_pc() );
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
//            printf("cb264se30_w: %08x @ %x, mask %08x (PC=%x)\n", data, offset, mem_mask, space.device().safe_pc());
			break;
	}
}

READ32_MEMBER( nubus_cb264se30_device::cb264se30_r )
{
	return 0;
}

WRITE32_MEMBER( nubus_cb264se30_device::vram_w )
{
	COMBINE_DATA(&m_vram32[offset]);
}

READ32_MEMBER( nubus_cb264se30_device::vram_r )
{
	return m_vram32[offset];
}
