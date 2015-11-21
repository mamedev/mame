// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Apple model 820-5037-C "Macintosh II Portrait Video Card"
  PCB is marked "Workstation/Portrait Card"
  640x870, 1, 2 or 4bpp grayscale

  Fs0900e0 = DAC control
  Fs0900e4 = DAC data
  Fs0A0000 = enable / ack VBL IRQ
  Fs0A0004 = disable VBL IRQ

***************************************************************************/

#include "emu.h"
#include "nubus_wsportrait.h"

#define WSPORTRAIT_SCREEN_NAME  "wsport_screen"
#define WSPORTRAIT_ROM_REGION  "wsport_rom"

#define VRAM_SIZE   (0x80000)   // 512k max

MACHINE_CONFIG_FRAGMENT( wsportrait )
	MCFG_SCREEN_ADD( WSPORTRAIT_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, nubus_wsportrait_device, screen_update)
	MCFG_SCREEN_SIZE(1024,960)
	MCFG_SCREEN_REFRESH_RATE(75.0)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 870-1)
MACHINE_CONFIG_END

ROM_START( wsportrait )
	ROM_REGION(0x1000, WSPORTRAIT_ROM_REGION, 0)
	ROM_LOAD( "341-0732.bin", 0x000000, 0x001000, CRC(ddc35b78) SHA1(ce2bf2374bb994c17962dba8f3d11bc1260e2644) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NUBUS_WSPORTRAIT = &device_creator<nubus_wsportrait_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nubus_wsportrait_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( wsportrait );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *nubus_wsportrait_device::device_rom_region() const
{
	return ROM_NAME( wsportrait );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_wsportrait_device - constructor
//-------------------------------------------------

nubus_wsportrait_device::nubus_wsportrait_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, NUBUS_WSPORTRAIT, "Macintosh II Portrait Video Card", tag, owner, clock, "nb_wspt", __FILE__),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this), m_vram32(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	m_assembled_tag = std::string(tag).append(":").append(WSPORTRAIT_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

nubus_wsportrait_device::nubus_wsportrait_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this), m_vram32(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	m_assembled_tag = std::string(tag).append(":").append(WSPORTRAIT_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_wsportrait_device::device_start()
{
	UINT32 slotspace;

	// set_nubus_device makes m_slot valid
	set_nubus_device();
	install_declaration_rom(this, WSPORTRAIT_ROM_REGION, true);

	slotspace = get_slotspace();

	printf("[wsportrait %p] slotspace = %x\n", (void *)this, slotspace);

	m_vram.resize(VRAM_SIZE);
	m_vram32 = (UINT32 *)&m_vram[0];

	m_nubus->install_device(slotspace, slotspace+VRAM_SIZE-1, read32_delegate(FUNC(nubus_wsportrait_device::vram_r), this), write32_delegate(FUNC(nubus_wsportrait_device::vram_w), this));
	m_nubus->install_device(slotspace+0x900000, slotspace+0x900000+VRAM_SIZE-1, read32_delegate(FUNC(nubus_wsportrait_device::vram_r), this), write32_delegate(FUNC(nubus_wsportrait_device::vram_w), this));
	m_nubus->install_device(slotspace+0x80000, slotspace+0xeffff, read32_delegate(FUNC(nubus_wsportrait_device::wsportrait_r), this), write32_delegate(FUNC(nubus_wsportrait_device::wsportrait_w), this));

	m_timer = timer_alloc(0, NULL);
	m_timer->adjust(m_screen->time_until_pos(869, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_wsportrait_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_vbl_disable = 1;
	m_mode = 0;
	memset(&m_vram[0], 0, VRAM_SIZE);
	memset(m_palette, 0, sizeof(m_palette));
}


void nubus_wsportrait_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(m_screen->time_until_pos(869, 0), 0);
}

/***************************************************************************

  Workstation/Portrait emulation

***************************************************************************/

UINT32 nubus_wsportrait_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;
	UINT8 pixels, *vram;

	// first time?  kick off the VBL timer
	vram = &m_vram[0x80];

	switch (m_mode)
	{
		case 0: // 1 bpp?
			for (y = 0; y < 870; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 640/8; x++)
				{
					pixels = vram[(y * 128) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[((pixels>>7)&0x1)];
					*scanline++ = m_palette[((pixels>>6)&0x1)];
					*scanline++ = m_palette[((pixels>>5)&0x1)];
					*scanline++ = m_palette[((pixels>>4)&0x1)];
					*scanline++ = m_palette[((pixels>>3)&0x1)];
					*scanline++ = m_palette[((pixels>>2)&0x1)];
					*scanline++ = m_palette[((pixels>>1)&0x1)];
					*scanline++ = m_palette[(pixels&1)];
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
					pixels = vram[(y * 512) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[((pixels&0xf0)>>4)];
					*scanline++ = m_palette[(pixels&0xf)];
				}
			}
			break;

		default:
			fatalerror("wsportrait: unknown video mode %d\n", m_mode);
	}
	return 0;
}

WRITE32_MEMBER( nubus_wsportrait_device::wsportrait_w )
{
	data ^= 0xffffffff;
//  if (offset != 0x8000) printf("wsportrait: Write %08x @ %x, mask %08x\n", data, offset, mem_mask);

	switch (offset)
	{
		case 1:         // mode control
//          printf("%08x to mode 1\n", data);
			switch (data & 0xff000000)
			{
				case 0x20000000:
				case 0x24000000:
					m_mode = 0;
					break;

				case 0x40000000:
					m_mode = 1;
					break;

				case 0x50000000:
				case 0x80000000:
					m_mode = 2;
					break;
			}
			break;

		case 0x4038:    // DAC control
			m_clutoffs = (data>>24)&0xff;
			break;

		case 0x4039:    // DAC data - only 4 bits per component!
			m_colors[m_count] = (data>>24) & 0x0f;
			m_colors[m_count] |= (m_colors[m_count]<<4);
			m_count++;

			if (m_count == 3)
			{
//              printf("RAMDAC: color %d = %02x %02x %02x (PC=%x)\n", m_clutoffs, m_colors[0], m_colors[1], m_colors[2], space.device().safe_pc() );
				m_palette[m_clutoffs] = rgb_t(m_colors[2], m_colors[2], m_colors[2]);
				m_clutoffs++;
				if (m_clutoffs > 255)
				{
					m_clutoffs = 0;
				}
				m_count = 0;
			}
			break;

		case 0x8000:
			lower_slot_irq();
			m_vbl_disable = false;
			break;

		case 0x8001:
			m_vbl_disable = true;
			break;
	}
}

READ32_MEMBER( nubus_wsportrait_device::wsportrait_r )
{
//  printf("wsportrait: Read @ %x, mask %08x\n", offset, mem_mask);

	/*
	  monitor types

	  0x0 = invalid
	  0x2 = invalid
	  0x4 = color: 640x870 1bpp, 640x480 2bpp and 4bpp
	  0x6 = 1bpp 640x384? and sets weird mode controls
	  0x8 = really odd (bitplaned?)
	  0xa = invalid
	  0xc = 640x480 grayscale
	  0xe = same as 0x6
	*/

	switch (offset)
	{
		case 0x4004:
			m_toggle ^= 0x00010000;
			return m_toggle | 0xfffc0000;   // bit 0 = vbl status, bits 1-3 = monitor type
	}
	return 0;
}

WRITE32_MEMBER( nubus_wsportrait_device::vram_w )
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram32[offset]);
}

READ32_MEMBER( nubus_wsportrait_device::vram_r )
{
	return m_vram32[offset] ^ 0xffffffff;
}
