// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Sigma Designs L-View card

***************************************************************************/

#include "emu.h"
#include "pds30_sigmalview.h"

#define LVIEW_SCREEN_NAME "lview_screen"
#define LVIEW_ROM_REGION  "lview_rom"

#define VRAM_SIZE   (0x80000)  // 512K?

MACHINE_CONFIG_FRAGMENT( lview )
	MCFG_SCREEN_ADD( LVIEW_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, nubus_lview_device, screen_update)
	MCFG_SCREEN_SIZE(832,600)
	MCFG_SCREEN_REFRESH_RATE(70)
	MCFG_SCREEN_VISIBLE_AREA(0, 832-1, 0, 600-1)
MACHINE_CONFIG_END

ROM_START( lview )
	ROM_REGION(0x4000, LVIEW_ROM_REGION, 0)
	ROM_LOAD( "lv_asi_4_00.bin", 0x000000, 0x004000, CRC(b806f875) SHA1(1e58593b1a8720193d1651b0d8a0d43e4e47563d) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type PDS030_LVIEW = &device_creator<nubus_lview_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nubus_lview_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( lview );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *nubus_lview_device::device_rom_region() const
{
	return ROM_NAME( lview );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_lview_device - constructor
//-------------------------------------------------

nubus_lview_device::nubus_lview_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, PDS030_LVIEW, "Sigma Designs L-View", tag, owner, clock, "pd3_lviw", __FILE__),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this), m_vram32(nullptr), m_vbl_disable(0), m_toggle(0), m_timer(nullptr), m_protstate(0)
{
	m_assembled_tag = std::string(tag).append(":").append(LVIEW_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

nubus_lview_device::nubus_lview_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_video_interface(mconfig, *this),
		device_nubus_card_interface(mconfig, *this), m_vram32(nullptr), m_vbl_disable(0), m_toggle(0), m_timer(nullptr), m_protstate(0)
{
	m_assembled_tag = std::string(tag).append(":").append(LVIEW_SCREEN_NAME);
	m_screen_tag = m_assembled_tag.c_str();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_lview_device::device_start()
{
	UINT32 slotspace;

	// set_nubus_device makes m_slot valid
	set_nubus_device();
	install_declaration_rom(this, LVIEW_ROM_REGION);

	slotspace = get_slotspace();

//  printf("[lview %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE);
	m_vram32 = (UINT32 *)&m_vram[0];

	m_nubus->install_device(slotspace, slotspace+VRAM_SIZE-1, read32_delegate(FUNC(nubus_lview_device::vram_r), this), write32_delegate(FUNC(nubus_lview_device::vram_w), this));
	m_nubus->install_device(slotspace+0x900000, slotspace+VRAM_SIZE-1+0x900000, read32_delegate(FUNC(nubus_lview_device::vram_r), this), write32_delegate(FUNC(nubus_lview_device::vram_w), this));
	m_nubus->install_device(slotspace+0xb0000, slotspace+0xbffff, read32_delegate(FUNC(nubus_lview_device::lview_r), this), write32_delegate(FUNC(nubus_lview_device::lview_w), this));

	m_timer = timer_alloc(0, nullptr);
	m_timer->adjust(m_screen->time_until_pos(599, 0), 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_lview_device::device_reset()
{
	m_vbl_disable = 1;
	m_protstate = 0;
	memset(&m_vram[0], 0, VRAM_SIZE);
	memset(m_palette, 0, sizeof(m_palette));

	m_palette[0] = rgb_t(255, 255, 255);
	m_palette[0x80] = rgb_t(0, 0, 0);
}


void nubus_lview_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	m_timer->adjust(m_screen->time_until_pos(599, 0), 0);
}

/***************************************************************************

  CB264 section

***************************************************************************/

UINT32 nubus_lview_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;
	UINT8 pixels, *vram;

	vram = &m_vram[0x20];

	for (y = 0; y < 600; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 832/8; x++)
		{
			pixels = vram[(y * (832/8)) + (BYTE4_XOR_BE(x))];

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

	return 0;
}

READ32_MEMBER( nubus_lview_device::lview_r )
{
	UINT32 rv = 0;

//    printf("prot_r: @ %x, mask %08x [PC=%x  state %d]\n", offset, mem_mask, machine().device("maincpu")->safe_pc(), m_protstate);

	if ((m_protstate == 1) || (m_protstate == 10) || (machine().device("maincpu")->safe_pc() == 0x5aac))
	{
		rv = 0x02020202;
	}

	if (m_protstate == 8)
	{
		rv = 0x01010101;
	}

	m_protstate++;
	return rv;
}

WRITE32_MEMBER( nubus_lview_device::lview_w )
{
//    if (offset != 0x7a && offset != 0x3ffb) printf("prot_w: %08x @ %x, mask %08x (PC=%x)\n", data, offset, mem_mask, space.device().safe_pc());

	if (offset == 0x7a)
	{
		if (data == 1)
		{
			m_vbl_disable = 0;
			lower_slot_irq();
		}
		else
		{
			m_vbl_disable = 1;
		}
	}
}

WRITE32_MEMBER( nubus_lview_device::vram_w )
{
	COMBINE_DATA(&m_vram32[offset]);
}

READ32_MEMBER( nubus_lview_device::vram_r )
{
	return m_vram32[offset];
}
