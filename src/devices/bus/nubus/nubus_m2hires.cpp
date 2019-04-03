// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Apple Hi-Resolution Video Card emulation

  RAMDAC: control at Fs0940e0, data at Fs0940e4
  Fs090010 bit 16 is vbl status, bit 17 must be "1" for proper operation
  Fs08000x are the control registers

***************************************************************************/

#include "emu.h"
#include "nubus_m2hires.h"
#include "screen.h"

#define M2HIRES_SCREEN_NAME "m2hires_screen"
#define M2HIRES_ROM_REGION  "m2hires_rom"

#define VRAM_SIZE   (0x80000)   // 512k max


ROM_START( m2hires )
	ROM_REGION(0x2000, M2HIRES_ROM_REGION, 0)
	ROM_LOAD( "341-0660.bin", 0x0000, 0x2000, CRC(ea6f7913) SHA1(37c59f38ae34021d0cb86c2e76a598b7e6077c0d) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_M2HIRES, nubus_m2hires_device, "nb_m2hr", "Macintosh II Hi-Resolution video card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_m2hires_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, M2HIRES_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_m2hires_device::screen_update));
	screen.set_raw(25175000, 800, 0, 640, 525, 0, 480);
	screen.set_size(1024, 768);
	screen.set_visarea(0, 640-1, 0, 480-1);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_m2hires_device::device_rom_region() const
{
	return ROM_NAME( m2hires );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_m2hires_device - constructor
//-------------------------------------------------

nubus_m2hires_device::nubus_m2hires_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_m2hires_device(mconfig, NUBUS_M2HIRES, tag, owner, clock)
{
}

nubus_m2hires_device::nubus_m2hires_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_vram32(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	set_screen(*this, M2HIRES_SCREEN_NAME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_m2hires_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(this, M2HIRES_ROM_REGION, true);

	slotspace = get_slotspace();

//  logerror("[m2hires %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE);
	m_vram32 = (uint32_t *)&m_vram[0];

	nubus().install_device(slotspace, slotspace+VRAM_SIZE-1, read32_delegate(FUNC(nubus_m2hires_device::vram_r), this), write32_delegate(FUNC(nubus_m2hires_device::vram_w), this));
	nubus().install_device(slotspace+0x900000, slotspace+VRAM_SIZE-1+0x900000, read32_delegate(FUNC(nubus_m2hires_device::vram_r), this), write32_delegate(FUNC(nubus_m2hires_device::vram_w), this));
	nubus().install_device(slotspace+0x80000, slotspace+0xeffff, read32_delegate(FUNC(nubus_m2hires_device::m2hires_r), this), write32_delegate(FUNC(nubus_m2hires_device::m2hires_w), this));

	m_timer = timer_alloc(0, nullptr);
	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_m2hires_device::device_reset()
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


void nubus_m2hires_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

/***************************************************************************

  Spectrum 24 PDQ section

***************************************************************************/

uint32_t nubus_m2hires_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint8_t pixels, *vram;

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
			fatalerror("m2hires: unknown video mode %d\n", m_mode);
	}
	return 0;
}

WRITE32_MEMBER( nubus_m2hires_device::m2hires_w )
{
	data ^= 0xffffffff;

	switch (offset)
	{
		case 1:         // mode
			switch (data)
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

				case 0x00010000:
					m_mode = 3;
					break;
			}
			break;

		case 0x5038:    // DAC control
//          logerror("%08x to DAC control %s\n", data, machine().describe_context());
			m_clutoffs = (data>>24)&0xff;
			break;

		case 0x5039: // DAC data
			m_colors[m_count++] = (data>>24) & 0xff;

			if (m_count == 3)
			{
//              logerror("RAMDAC: color %d = %02x %02x %02x %s\n", m_clutoffs, m_colors[0], m_colors[1], m_colors[2], machine().describe_context() );
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
//          logerror("m2hires_w: %08x @ %x, mask %08x %s\n", data, offset, mem_mask, machine().describe_context());
			break;
	}
}

READ32_MEMBER( nubus_m2hires_device::m2hires_r )
{
	if (offset == 0x10010/4)
	{
		m_toggle ^= (1<<16);
		return m_toggle | (1<<17);  // bit 17 indicates a 4/8bpp capable ASIC apparently; the firmware won't enter those modes otherwise (although they show in the list)
	}
/*  else
    {
        logerror("m2hires_r: @ %x, mask %08x %s\n", offset, mem_mask, machine().describe_context());
    }*/

	return 0;
}

WRITE32_MEMBER( nubus_m2hires_device::vram_w )
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram32[offset]);
}

READ32_MEMBER( nubus_m2hires_device::vram_r )
{
	return m_vram32[offset] ^ 0xffffffff;
}
