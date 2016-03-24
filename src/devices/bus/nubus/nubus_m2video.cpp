// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Apple Macintosh II Video Card (630-0153) emulation

  Video ASIC is "TFB" 344-0001
  RAMDAC is Bt453: control at Fs09001C, data at Fs090018
  Fs08000x is general framebuffer control (video mode at 0)
  Fs0D0000 bit 0 is VBL status

***************************************************************************/

#include "emu.h"
#include "nubus_m2video.h"

#define M2VIDEO_SCREEN_NAME "m2video_screen"
#define M2VIDEO_ROM_REGION  "m2video_rom"

#define VRAM_SIZE   (0x80000)   // 512k max

MACHINE_CONFIG_FRAGMENT( m2video )
	MCFG_SCREEN_ADD(M2VIDEO_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, nubus_m2video_device, screen_update)
	MCFG_SCREEN_RAW_PARAMS(25175000, 800, 0, 640, 525, 0, 480)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
MACHINE_CONFIG_END

ROM_START( m2video )
	ROM_REGION(0x1000, M2VIDEO_ROM_REGION, 0)
	ROM_LOAD( "342-0008-a.bin", 0x000000, 0x001000, CRC(bf50850d) SHA1(abe85d8a882bb2b8187a28bd6707fc2f5d77eedd) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NUBUS_M2VIDEO = &device_creator<nubus_m2video_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nubus_m2video_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( m2video );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *nubus_m2video_device::device_rom_region() const
{
	return ROM_NAME( m2video );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_m2video_device - constructor
//-------------------------------------------------

nubus_m2video_device::nubus_m2video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, NUBUS_M2VIDEO, "Macintosh II Video Card", tag, owner, clock, "nb_m2vc", __FILE__),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this), m_vram32(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	m_assembled_tag = std::string(tag).append(":").append(M2VIDEO_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

nubus_m2video_device::nubus_m2video_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this), m_vram32(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	m_assembled_tag = std::string(tag).append(":").append(M2VIDEO_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_m2video_device::device_start()
{
	UINT32 slotspace;

	// set_nubus_device makes m_slot valid
	set_nubus_device();
	install_declaration_rom(this, M2VIDEO_ROM_REGION, true, true);

	slotspace = get_slotspace();

//  printf("[m2video %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE);
	m_vram32 = (UINT32 *)&m_vram[0];

	m_nubus->install_device(slotspace, slotspace+VRAM_SIZE-1, read32_delegate(FUNC(nubus_m2video_device::vram_r), this), write32_delegate(FUNC(nubus_m2video_device::vram_w), this));
	m_nubus->install_device(slotspace+0x900000, slotspace+VRAM_SIZE-1+0x900000, read32_delegate(FUNC(nubus_m2video_device::vram_r), this), write32_delegate(FUNC(nubus_m2video_device::vram_w), this));
	m_nubus->install_device(slotspace+0x80000, slotspace+0xeffff, read32_delegate(FUNC(nubus_m2video_device::m2video_r), this), write32_delegate(FUNC(nubus_m2video_device::m2video_w), this));

	m_timer = timer_alloc(0, nullptr);
	m_timer->adjust(m_screen->time_until_pos(479, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_m2video_device::device_reset()
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


void nubus_m2video_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(m_screen->time_until_pos(479, 0), 0);
}

/***************************************************************************

  "TFB" section

***************************************************************************/

UINT32 nubus_m2video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;
	UINT8 pixels, *vram;

	vram = &m_vram[0x20];

	switch (m_mode)
	{
		case 0: // 1 bpp?
			for (y = 0; y < 480; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 640/8; x++)
				{
					pixels = vram[(y * 128) + (BYTE4_XOR_BE(x))];

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
					pixels = vram[(y * 256) + (BYTE4_XOR_BE(x))];

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
					pixels = vram[(y * 512) + (BYTE4_XOR_BE(x))];

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

		default:
			fatalerror("m2video: unknown video mode %d\n", m_mode);
	}
	return 0;
}

WRITE32_MEMBER( nubus_m2video_device::m2video_w )
{
	data ^= 0xffffffff;
	switch (offset)
	{
		case 0:         // mode
			switch (data & 0xff000000)
			{
				case 0x20000000:
					m_mode = 0;
					break;

				case 0x40000000:
					m_mode = 1;
					break;

				case 0x80000000:
					m_mode = 2;
					break;

				case 0x00000000:
					m_mode = 3;
					break;
			}
			break;

		case 0x4007:    // DAC control
//          printf("%08x to DAC control (PC=%x)\n", data, space.device().safe_pc());
			m_clutoffs = (data>>24)&0xff;
			break;

		case 0x4006: // DAC data
			m_colors[m_count++] = (data>>24) & 0xff;

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

		case 0x8000:    // enable and ack VBL
			m_vbl_disable = 0;
			lower_slot_irq();
			break;

		case 0x8001:    // disable VBL
			m_vbl_disable = 1;
			break;

		default:
//          printf("m2video_w: %08x @ %x, mask %08x (PC=%x)\n", data, offset, mem_mask, space.device().safe_pc());
			break;
	}
}

READ32_MEMBER( nubus_m2video_device::m2video_r )
{
	if (offset == 0x50000/4)    // bit 0 is VBL status
	{
		m_toggle ^= 1;
		return m_toggle;
	}
	else
	{
//      printf("m2video_r: @ %x, mask %08x (PC=%x)\n", offset, mem_mask, space.device().safe_pc());
	}

	return 0;
}

WRITE32_MEMBER( nubus_m2video_device::vram_w )
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram32[offset]);
}

READ32_MEMBER( nubus_m2video_device::vram_r )
{
	return m_vram32[offset] ^ 0xffffffff;
}
