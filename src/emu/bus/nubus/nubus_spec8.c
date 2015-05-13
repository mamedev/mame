// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  SuperMac Spectrum/8 Series III video card

  There is no sign of acceleration or blitting in any mode, and the acceleration
  code from the Spectrum PDQ ROM is absent on this one.

  On first boot / with clean PRAM, press SPACE repeatedly when it shows the frame
  that fills the entire screen.  If you get it wrong, delete PRAM and try again.

***************************************************************************/

#include "emu.h"
#include "nubus_spec8.h"

#define SPEC8S3_SCREEN_NAME "spec8s3_screen"
#define SPEC8S3_ROM_REGION  "spec8s3_rom"

#define VRAM_SIZE   (0xc0000)   // 768k of VRAM for 1024x768 @ 8 bit

MACHINE_CONFIG_FRAGMENT( spec8s3 )
	MCFG_SCREEN_ADD( SPEC8S3_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, nubus_spec8s3_device, screen_update)
	MCFG_SCREEN_RAW_PARAMS(25175000, 800, 0, 640, 525, 0, 480)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
MACHINE_CONFIG_END

ROM_START( spec8s3 )
	ROM_REGION(0x8000, SPEC8S3_ROM_REGION, 0)
	ROM_LOAD( "1003067-0001d.11b.bin", 0x000000, 0x008000, CRC(12188e2b) SHA1(6552d40364eae99b449842a79843d8c0114c4c70) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NUBUS_SPEC8S3 = &device_creator<nubus_spec8s3_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nubus_spec8s3_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( spec8s3 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *nubus_spec8s3_device::device_rom_region() const
{
	return ROM_NAME( spec8s3 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_spec8s3_device - constructor
//-------------------------------------------------

nubus_spec8s3_device::nubus_spec8s3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, NUBUS_SPEC8S3, "SuperMac Spectrum/8 Series III video card", tag, owner, clock, "nb_sp8s3", __FILE__),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this)
{
	m_assembled_tag = std::string(tag).append(":").append(SPEC8S3_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

nubus_spec8s3_device::nubus_spec8s3_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this)
{
	m_assembled_tag = std::string(tag).append(":").append(SPEC8S3_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_spec8s3_device::device_start()
{
	UINT32 slotspace;

	// set_nubus_device makes m_slot valid
	set_nubus_device();
	install_declaration_rom(this, SPEC8S3_ROM_REGION);

	slotspace = get_slotspace();

//  printf("[SPEC8S3 %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE);
	m_vram32 = (UINT32 *)&m_vram[0];
	m_nubus->install_device(slotspace, slotspace+VRAM_SIZE-1, read32_delegate(FUNC(nubus_spec8s3_device::vram_r), this), write32_delegate(FUNC(nubus_spec8s3_device::vram_w), this));
	m_nubus->install_device(slotspace+0x900000, slotspace+VRAM_SIZE-1+0x900000, read32_delegate(FUNC(nubus_spec8s3_device::vram_r), this), write32_delegate(FUNC(nubus_spec8s3_device::vram_w), this));
	m_nubus->install_device(slotspace+0xd0000, slotspace+0xfffff, read32_delegate(FUNC(nubus_spec8s3_device::spec8s3_r), this), write32_delegate(FUNC(nubus_spec8s3_device::spec8s3_w), this));

	m_timer = timer_alloc(0, NULL);
	m_timer->adjust(m_screen->time_until_pos(767, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_spec8s3_device::device_reset()
{
	m_count = 0;
	m_clutoffs = 0;
	m_vbl_disable = 1;
	m_mode = 0;
	m_vbl_pending = false;
	m_parameter = 0;
	memset(&m_vram[0], 0, VRAM_SIZE);
	memset(m_palette, 0, sizeof(m_palette));

	m_palette[0] = rgb_t(255, 255, 255);
	m_palette[1] = rgb_t(0, 0, 0);
}


void nubus_spec8s3_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
		m_vbl_pending = true;
	}

	m_timer->adjust(m_screen->time_until_pos(767, 0), 0);
}

/***************************************************************************

  Spectrum 24 PDQ section

***************************************************************************/

UINT32 nubus_spec8s3_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;
	UINT8 pixels, *vram;

	vram = &m_vram[0x400];

	switch (m_mode)
	{
		case 0: // 1 bpp
			for (y = 0; y < 768; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 1024/8; x++)
				{
					pixels = vram[(y * 512) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[pixels&0x80];
					*scanline++ = m_palette[(pixels<<1)&0x80];
					*scanline++ = m_palette[(pixels<<2)&0x80];
					*scanline++ = m_palette[(pixels<<3)&0x80];
					*scanline++ = m_palette[(pixels<<4)&0x80];
					*scanline++ = m_palette[(pixels<<5)&0x80];
					*scanline++ = m_palette[(pixels<<6)&0x80];
					*scanline++ = m_palette[(pixels<<7)&0x80];
				}
			}
			break;

		case 1: // 2 bpp
			for (y = 0; y < 768; y++)
			{
				scanline = &bitmap.pix32(y);
				for (x = 0; x < 1024/4; x++)
				{
					pixels = vram[(y * 512) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[pixels&0xc0];
					*scanline++ = m_palette[(pixels<<2)&0xc0];
					*scanline++ = m_palette[(pixels<<4)&0xc0];
					*scanline++ = m_palette[(pixels<<6)&0xc0];
				}
			}
			break;

		case 2: // 4 bpp
			for (y = 0; y < 768; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < 1024/2; x++)
				{
					pixels = vram[(y * 512) + (BYTE4_XOR_BE(x))];

					*scanline++ = m_palette[pixels&0xf0];
					*scanline++ = m_palette[(pixels<<4)&0xf0];
				}
			}
			break;

		case 3: // 8 bpp
			for (y = 0; y < 768; y++)
			{
				scanline = &bitmap.pix32(y);

				for (x = 0; x < 1024; x++)
				{
					pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];
					*scanline++ = m_palette[pixels];
				}
			}
			break;

		default:
			fatalerror("spec8s3: unknown video mode %d\n", m_mode);
	}
	return 0;
}

WRITE32_MEMBER( nubus_spec8s3_device::spec8s3_w )
{
	switch (offset)
	{
		case 0x385c:    // IRQ enable
			if (data & 0x10)
			{
				m_vbl_disable = 1;
				lower_slot_irq();
				m_vbl_pending = false;
			}
			else
			{
				m_vbl_disable = 0;
			}
			break;

		case 0x385e:
			break;

		case 0x386e:
			break;

		case 0x3a00:
			m_clutoffs = (data & 0xff) ^ 0xff;
			break;

		case 0x3a01:
//          printf("%08x to color (%08x invert)\n", data, data ^ 0xffffffff);
			m_colors[m_count++] = (data & 0xff) ^ 0xff;

			if (m_count == 3)
			{
				int actual_color = BITSWAP8(m_clutoffs, 0, 1, 2, 3, 4, 5, 6, 7);

//              printf("RAMDAC: color %d = %02x %02x %02x (PC=%x)\n", actual_color, m_colors[0], m_colors[1], m_colors[2], space.device().safe_pc() );
				m_palette[actual_color] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
				m_clutoffs++;
				if (m_clutoffs > 255)
				{
					m_clutoffs = 0;
				}
				m_count = 0;
			}
			break;

		case 0x3c00:
			if ((m_parameter == 2) && (data != 0xffffffff))
			{
				data &= 0xff;
//              printf("%x to mode\n", data);
				switch (data)
				{
					case 0x5f:
						m_mode = 0;
						break;

					case 0x5e:
						m_mode = 1;
						break;

					case 0x5d:
						m_mode = 2;
						break;

					case 0x5c:
						m_mode = 3;
						break;
				}
			}
			m_parameter++;
			break;

		case 0x3e02:
			if (data == 1)
			{
				m_parameter = 0;
			}
			break;

		default:
//          if (offset >= 0x3800) printf("spec8s3_w: %08x @ %x (mask %08x  PC=%x)\n", data, offset, mem_mask, space.device().safe_pc());
			break;
	}
}

READ32_MEMBER( nubus_spec8s3_device::spec8s3_r )
{
	switch (offset)
	{
		case 0x3826:
		case 0x382e:
			return 0xff;

		case 0x3824:
		case 0x382c:
			return (0xa^0xffffffff);

		case 0x385c:
			if (m_vbl_pending)
			{
				return 0x8;
			}
			return 0;

		case 0x385e:
			return 0;

		default:
//          if (offset >= 0x3800) printf("spec8s3_r: @ %x (mask %08x  PC=%x)\n", offset, mem_mask, space.device().safe_pc());
			break;
	}
	return 0;
}

WRITE32_MEMBER( nubus_spec8s3_device::vram_w )
{
	data ^= 0xffffffff;
	COMBINE_DATA(&m_vram32[offset]);
}

READ32_MEMBER( nubus_spec8s3_device::vram_r )
{
	return m_vram32[offset] ^ 0xffffffff;
}
