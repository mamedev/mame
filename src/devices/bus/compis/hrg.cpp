// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis (Ultra) High Resolution Graphics adapter emulation

**********************************************************************/

#include "emu.h"
#include "hrg.h"
#include "screen.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define UPD7220_TAG     "upd7220"
#define SCREEN_TAG      "screen"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMPIS_HRG,  compis_hrg_device,  "compis_hrg",  "Compis HRG")
DEFINE_DEVICE_TYPE(COMPIS_UHRG, compis_uhrg_device, "compis_uhrg", "Compis UHRG")


//-------------------------------------------------
//  ADDRESS_MAP( upd7220_map )
//-------------------------------------------------

void compis_hrg_device::hrg_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x00000, 0x3fff).ram().share("video_ram");
}

void compis_uhrg_device::uhrg_map(address_map &map)
{
	map.global_mask(0xffff);
	map(0x00000, 0xffff).ram().share("video_ram");
}


//-------------------------------------------------
//  UPD7220_DISPLAY_PIXELS_MEMBER( display_pixels )
//-------------------------------------------------

UPD7220_DISPLAY_PIXELS_MEMBER( compis_hrg_device::display_pixels )
{
	uint16_t const gfx = m_video_ram[(address & 0x3fff)];
	pen_t const *const pen = m_palette->pens();

	for(uint16_t i = 0; i < 16; i++)
		bitmap.pix(y, x + i) = pen[BIT(gfx, i)];
}


//-------------------------------------------------
//  UPD7220_DISPLAY_PIXELS_MEMBER( display_pixels )
//-------------------------------------------------

UPD7220_DISPLAY_PIXELS_MEMBER( compis_uhrg_device::display_pixels )
{
	uint16_t const gfx = m_video_ram[(address & 0xffff)];
	pen_t const *const pen = m_palette->pens();

	for(uint16_t i = 0; i < 16; i++)
		bitmap.pix(y, x + i) = pen[BIT(gfx, i)];
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void compis_hrg_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG).set_color(rgb_t::green()));
	// from upd7220 setup
	screen.set_raw(2252500 * 8, 848, 0, 640, 425, 20, 420);
	screen.set_screen_update(UPD7220_TAG, FUNC(upd7220_device::screen_update));

	UPD7220(config, m_crtc, 2252500); // unknown clock
	m_crtc->set_addrmap(0, &compis_hrg_device::hrg_map);
	m_crtc->set_display_pixels(FUNC(compis_hrg_device::display_pixels));
	m_crtc->set_screen(SCREEN_TAG);

	PALETTE(config, m_palette, palette_device::MONOCHROME);
}


void compis_uhrg_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG).set_color(rgb_t::green()));
	// from upd7220 setup
	// this is just for frontend, system actually goes low res first before determining
	// that UHRG card is connected thru VRAM mirror
	screen.set_raw(2252500 * 8 * 4, 1696, 0, 1280, 850, 20, 820);
	screen.set_screen_update(UPD7220_TAG, FUNC(upd7220_device::screen_update));

	UPD7220(config, m_crtc, 2252500 * 4); // unknown clock
	m_crtc->set_addrmap(0, &compis_uhrg_device::uhrg_map);
	m_crtc->set_display_pixels(FUNC(compis_uhrg_device::display_pixels));
	m_crtc->set_screen(SCREEN_TAG);

	PALETTE(config, m_palette, palette_device::MONOCHROME);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  compis_hrg_device - constructor
//-------------------------------------------------

compis_hrg_device::compis_hrg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_compis_graphics_card_interface(mconfig, *this),
	m_crtc(*this, UPD7220_TAG),
	m_palette(*this, "palette"),
	m_video_ram(*this, "video_ram")
{
}

compis_hrg_device::compis_hrg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	compis_hrg_device(mconfig, COMPIS_HRG, tag, owner, clock)
{
}

compis_uhrg_device::compis_uhrg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	compis_hrg_device(mconfig, COMPIS_UHRG, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void compis_hrg_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void compis_hrg_device::device_reset()
{
}


//-------------------------------------------------
//  pcs6_6_r -
//-------------------------------------------------

uint8_t compis_hrg_device::pcs6_6_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset < 2)
		data = m_crtc->read(offset & 0x01);
	else
	{
		// monochrome only, hblank? vblank?
		if(offset == 2)
		{
			switch(m_unk_video)
			{
				case 0x04:
					m_unk_video = 0x44;
					break;
				case 0x44:
					m_unk_video = 0x64;
					break;
				default:
					m_unk_video = 0x04;
					break;
			}
			data = m_unk_video;
		}
		else
			data = 0;
	}

	//logerror("%s PCS 6:6 read %04x : %02x\n", machine().describe_context(), offset, data);

	return data;
}


//-------------------------------------------------
//  pcs6_6_w -
//-------------------------------------------------

void compis_hrg_device::pcs6_6_w(offs_t offset, uint8_t data)
{
	//logerror("%s PCS 6:6 write %04x : %02x\n", machine().describe_context(), offset, data);

	// 0x336 is likely the color plane register
	if (offset < 2) m_crtc->write(offset & 0x01, data);
}

void compis_uhrg_device::pcs6_6_w(offs_t offset, uint8_t data)
{
	compis_hrg_device::pcs6_6_w(offset, data);
	if (offset == 2)
	{
		// bit 7: 1 high res clock 0: HRG regular clock
		m_crtc->set_unscaled_clock((2252500) << (BIT(data, 7) ? 2 : 0));

		// TODO: bits 3-0, non-linear address scanout?
		// 0xf normally (including regular HRG device), 0xe when it starts to printout stuff in UHRG mode
	}
}
