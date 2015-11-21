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

#define XCEED30HR_SCREEN_NAME "x30hr_screen"
#define XCEED30HR_ROM_REGION  "x30hr_rom"

#define VRAM_SIZE   (0x100000)  // 1 MB VRAM - max mode is 1024x768 @ 8bpp

MACHINE_CONFIG_FRAGMENT( xceed30hr )
	MCFG_SCREEN_ADD( XCEED30HR_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, nubus_xceed30hr_device, screen_update)
	MCFG_SCREEN_RAW_PARAMS(25175000, 800, 0, 640, 525, 0, 480)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
MACHINE_CONFIG_END

ROM_START( xceed30hr )
	ROM_REGION(0x8000, XCEED30HR_ROM_REGION, 0)
	ROM_LOAD( "369c.rom",     0x000000, 0x008000, CRC(b22f0a89) SHA1(be34c8604b8a1ae9c9f3b0b90faba9a1a64a5855) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type PDS030_XCEED30HR = &device_creator<nubus_xceed30hr_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nubus_xceed30hr_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( xceed30hr );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *nubus_xceed30hr_device::device_rom_region() const
{
	return ROM_NAME( xceed30hr );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_xceed30hr_device - constructor
//-------------------------------------------------

nubus_xceed30hr_device::nubus_xceed30hr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, PDS030_XCEED30HR, "Micron/XCEED Technology Color 30HR", tag, owner, clock, "pd3_30hr", __FILE__),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this), m_vram32(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	m_assembled_tag = std::string(tag).append(":").append(XCEED30HR_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

nubus_xceed30hr_device::nubus_xceed30hr_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this), m_vram32(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	m_assembled_tag = std::string(tag).append(":").append(XCEED30HR_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_xceed30hr_device::device_start()
{
	UINT32 slotspace;

	// set_nubus_device makes m_slot valid
	set_nubus_device();
	install_declaration_rom(this, XCEED30HR_ROM_REGION);

	slotspace = get_slotspace();

//  printf("[xceed30hr %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE);
	m_vram32 = (UINT32 *)&m_vram[0];

	m_nubus->install_device(slotspace, slotspace+VRAM_SIZE-1, read32_delegate(FUNC(nubus_xceed30hr_device::vram_r), this), write32_delegate(FUNC(nubus_xceed30hr_device::vram_w), this));
	m_nubus->install_device(slotspace+0x800000, slotspace+0xefffff, read32_delegate(FUNC(nubus_xceed30hr_device::xceed30hr_r), this), write32_delegate(FUNC(nubus_xceed30hr_device::xceed30hr_w), this));

	m_timer = timer_alloc(0, NULL);
	m_timer->adjust(m_screen->time_until_pos(479, 0), 0);
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


void nubus_xceed30hr_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
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

UINT32 nubus_xceed30hr_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;
	UINT8 pixels, *vram;

	vram = &m_vram[1024];

	switch (m_mode)
	{
		case 0: // 1 bpp?
			for (y = 0; y < 480; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 640/8; x++)
				{
					pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[(pixels>>7)&1];
					*scanline++ = m_palette[(pixels>>6)&1];
					*scanline++ = m_palette[(pixels>>5)&1];
					*scanline++ = m_palette[(pixels>>4)&1];
					*scanline++ = m_palette[(pixels>>3)&1];
					*scanline++ = m_palette[(pixels>>2)&1];
					*scanline++ = m_palette[(pixels>>1)&1];
					*scanline++ = m_palette[pixels&1];
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

					*scanline++ = m_palette[((pixels>>6)&3)];
					*scanline++ = m_palette[((pixels>>4)&3)];
					*scanline++ = m_palette[((pixels>>2)&3)];
					*scanline++ = m_palette[(pixels&3)];
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

					*scanline++ = m_palette[(pixels>>4)];
					*scanline++ = m_palette[(pixels&0xf)];
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

		default:
			fatalerror("xceed30hr: unknown video mode %d\n", m_mode);
	}
	return 0;
}

WRITE32_MEMBER( nubus_xceed30hr_device::xceed30hr_w )
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
//            printf("%08x to DAC control (PC=%x)\n", data, space.device().safe_pc());
			m_clutoffs = (data&0xff);
			m_count = 0;
			break;

		case 0x100001:
//            printf("%08x to DAC data (PC=%x)\n", data, space.device().safe_pc());
			m_colors[m_count++] = (data & 0xff);

			if (m_count == 3)
			{
//                printf("RAMDAC: color %02x = %02x %02x %02x (PC=%x)\n", m_clutoffs, m_colors[0], m_colors[1], m_colors[2], space.device().safe_pc() );
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
//            printf("xceed30hr_w: %08x @ %x, mask %08x (PC=%x)\n", data, offset, mem_mask, space.device().safe_pc());
			break;
	}
}

READ32_MEMBER( nubus_xceed30hr_device::xceed30hr_r )
{
//    printf("xceed30hr_r: @ %x, mask %08x [PC=%x]\n", offset, mem_mask, machine().device("maincpu")->safe_pc());
	if (offset == 0x80008)
	{
		m_toggle ^= 0x80;
		return m_toggle;
	}

	return 0;
}

WRITE32_MEMBER( nubus_xceed30hr_device::vram_w )
{
	COMBINE_DATA(&m_vram32[offset]);
}

READ32_MEMBER( nubus_xceed30hr_device::vram_r )
{
	return m_vram32[offset];
}
