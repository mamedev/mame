// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Micron/XCEED Technologies MacroColor 30

  Similar to the 30HR, but registers are rearranged and 24bpp support
  was added.

***************************************************************************/

#include "emu.h"
#include "pds30_mc30.h"
#include "screen.h"

#define XCEEDMC30_SCREEN_NAME "x30hr_screen"
#define XCEEDMC30_ROM_REGION  "x30hr_rom"

#define VRAM_SIZE   (0x200000)  // 16 42C4256 256Kx4 VRAMs on the board = 2MB


ROM_START( xceedmc30 )
	ROM_REGION(0x8000, XCEEDMC30_ROM_REGION, 0)
	ROM_LOAD( "0390.bin", 0x000000, 0x008000, CRC(adea7a18) SHA1(9141eb1a0e5061e0409d65a89b4eaeb119ee4ffb) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PDS030_XCEEDMC30, nubus_xceedmc30_device, "pd3_mclr", "Micron/XCEED Technology MacroColor 30")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_xceedmc30_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, XCEEDMC30_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_xceedmc30_device::screen_update));
	screen.set_raw(25175000, 800, 0, 640, 525, 0, 480);
	screen.set_size(1024, 768);
	screen.set_visarea(0, 640-1, 0, 480-1);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_xceedmc30_device::device_rom_region() const
{
	return ROM_NAME( xceedmc30 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_xceedmc30_device - constructor
//-------------------------------------------------

nubus_xceedmc30_device::nubus_xceedmc30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_xceedmc30_device(mconfig, PDS030_XCEEDMC30, tag, owner, clock)
{
}

nubus_xceedmc30_device::nubus_xceedmc30_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nubus_card_interface(mconfig, *this),
	m_vram32(nullptr), m_mode(0), m_vbl_disable(0), m_toggle(0), m_count(0), m_clutoffs(0), m_timer(nullptr)
{
	set_screen(*this, XCEEDMC30_SCREEN_NAME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_xceedmc30_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(this, XCEEDMC30_ROM_REGION);

	slotspace = get_slotspace();

//  printf("[xceedmc30 %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE);
	m_vram32 = (uint32_t *)&m_vram[0];

	nubus().install_device(slotspace, slotspace+VRAM_SIZE-1, read32_delegate(*this, FUNC(nubus_xceedmc30_device::vram_r)), write32_delegate(*this, FUNC(nubus_xceedmc30_device::vram_w)));
	nubus().install_device(slotspace+0x800000, slotspace+0xefffff, read32_delegate(*this, FUNC(nubus_xceedmc30_device::xceedmc30_r)), write32_delegate(*this, FUNC(nubus_xceedmc30_device::xceedmc30_w)));

	m_timer = timer_alloc(0, nullptr);
	m_timer->adjust(screen().time_until_pos(479, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_xceedmc30_device::device_reset()
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


void nubus_xceedmc30_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
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

uint32_t nubus_xceedmc30_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint8_t pixels, *vram;

	vram = &m_vram[4*1024];

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

		case 4: // 24 bpp
			{
				uint32_t *vram32 = (uint32_t *)vram;
				uint32_t *base;

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
			fatalerror("xceedmc30: unknown video mode %d\n", m_mode);
	}
	return 0;
}

WRITE32_MEMBER( nubus_xceedmc30_device::xceedmc30_w )
{
	switch (offset)
	{
		case 0x80000:           // mode
			switch (data & 0xff000000)
			{
				case 0xfb000000:
					m_mode = 0;
					break;

				case 0xfa000000:
					m_mode = 1;
					break;

				case 0xf9000000:
					m_mode = 2;
					break;

				case 0xf8000000:
					m_mode = 3;
					break;

				case 0xff000000:
					m_mode = 4;
					break;
			}
			break;

		case 0x80005:   // ack VBL
			lower_slot_irq();
			break;

		case 0x100000:
//            logerror("%s %08x to DAC control\n", machine().describe_context(), data);
			m_clutoffs = (data&0xff);
			m_count = 0;
			break;

		case 0x100001:
//            printf("%s %08x to DAC data\n", machine().describe_context().c_str(), data);
			m_colors[m_count++] = ((data>>24) & 0xff);

			if (m_count == 3)
			{
//                printf("%s RAMDAC: color %02x = %02x %02x %02x\n", machine().describe_context().c_str(), m_clutoffs, m_colors[0], m_colors[1], m_colors[2]);
				m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
				m_clutoffs++;
				if (m_clutoffs > 255)
				{
					m_clutoffs = 0;
				}
				m_count = 0;
			}
			break;

		case 0x80002:   // VBL control
			if (data == 0xdcef0000)
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
//            printf("%s xceedmc30_w: %08x @ %x, mask %08x\n", machine().describe_context().c_str(), data, offset, mem_mask);
			break;
	}
}

READ32_MEMBER( nubus_xceedmc30_device::xceedmc30_r )
{
//    printf("xceedmc30_r: @ %x, mask %08x [PC=%x]\n", offset, mem_mask, machine().device<cpu_device>("maincpu")->pc());
	if (offset == 0x80008)
	{
		m_toggle ^= 0x04;
		return m_toggle;
	}

	return 0;
}

WRITE32_MEMBER( nubus_xceedmc30_device::vram_w )
{
	COMBINE_DATA(&m_vram32[offset]);
}

READ32_MEMBER( nubus_xceedmc30_device::vram_r )
{
	return m_vram32[offset];
}
