// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    PBJ Word-Pak/Word-Pak II - 80 Column Video Cartridge

    TODO:
    - the R6545 Transparent Mode used to update video RAM needs some attention.
    - original PBJ WorkPak (not II or RS) had an optional Basic Driver ROM (undumped).
    - does WordPak II use different character ROM?

***************************************************************************/

#include "emu.h"
#include "coco_wpk.h"
#include "render.h"
#include "screen.h"


//-------------------------------------------------
//  ROM( coco_wpk )
//-------------------------------------------------

ROM_START(coco_wpk)
	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("chr_gen_4.0.rom", 0x0000, 0x0800, CRC(6e9a671a) SHA1(0ff5fae512c7f1abe138504d51492404dd86be03)) // from PBJ WordPak RS
ROM_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(COCO_WPK, coco_wpk_device, "coco_wpk", "CoCo WordPak")
DEFINE_DEVICE_TYPE(COCO_WPK2, coco_wpk2_device, "coco_wpk2", "CoCo WordPak II")
DEFINE_DEVICE_TYPE(COCO_WPKRS, coco_wpkrs_device, "coco_wpkrs", "CoCo WordPak RS")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_wpk_device - constructor
//-------------------------------------------------

coco_wpk_device::coco_wpk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_cococart_interface(mconfig, *this )
	, m_crtc(*this, "crtc")
	, m_palette(*this, "palette")
	, m_chargen(*this, "chargen")
	, m_video_addr(0)
{
}

coco_wpk_device::coco_wpk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: coco_wpk_device(mconfig, COCO_WPK, tag, owner, clock)
{
}

coco_wpk2_device::coco_wpk2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: coco_wpk_device(mconfig, COCO_WPK2, tag, owner, clock)
{
}

coco_wpkrs_device::coco_wpkrs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: coco_wpk_device(mconfig, COCO_WPKRS, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_wpk_device::device_start()
{
	m_video_ram = std::make_unique<u8[]>(0x800);

	save_pointer(NAME(m_video_ram), 0x800);
	save_item(NAME(m_video_addr));
}

void coco_wpk2_device::device_start()
{
	coco_wpk_device::device_start();
}

void coco_wpkrs_device::device_start()
{
	coco_wpk_device::device_start();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void coco_wpk_device::device_reset()
{
	install_readwrite_handler(0xff98, 0xff98, read8smo_delegate(*m_crtc, FUNC(r6545_1_device::status_r)), write8smo_delegate(*m_crtc, FUNC(r6545_1_device::address_w)));
	install_readwrite_handler(0xff99, 0xff99, read8smo_delegate(*m_crtc, FUNC(r6545_1_device::register_r)), write8smo_delegate(*m_crtc, FUNC(r6545_1_device::register_w)));
	install_write_handler(0xff9b, 0xff9b, write8smo_delegate(*this, FUNC(coco_wpkrs_device::crtc_display_w)));
}

void coco_wpk2_device::device_reset()
{
	coco_wpk_device::device_reset();

	install_write_handler(0xff9c, 0xff9c, write8smo_delegate(*this, FUNC(coco_wpk2_device::video_select_w)));

	machine().render().first_target()->set_view(0);
}

void coco_wpkrs_device::device_reset()
{
	install_readwrite_handler(0xff76, 0xff76, read8smo_delegate(*m_crtc, FUNC(r6545_1_device::status_r)), write8smo_delegate(*m_crtc, FUNC(r6545_1_device::address_w)));
	install_readwrite_handler(0xff77, 0xff77, read8smo_delegate(*m_crtc, FUNC(r6545_1_device::register_r)), write8smo_delegate(*m_crtc, FUNC(r6545_1_device::register_w)));
	install_write_handler(0xff79, 0xff79, write8smo_delegate(*this, FUNC(coco_wpkrs_device::crtc_display_w)));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void coco_wpk_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.318181_MHz_XTAL, 896, 0, 640, 290, 0, 240);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	R6545_1(config, m_crtc, 14.318181_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_on_update_addr_change_callback(FUNC(coco_wpkrs_device::crtc_addr));
	m_crtc->set_update_row_callback(FUNC(coco_wpkrs_device::crtc_update_row));
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *coco_wpk_device::device_rom_region() const
{
	return ROM_NAME( coco_wpk );
}


void coco_wpk_device::crtc_display_w(u8 data)
{
	//logerror("crtc_display_w: %02x\n", data);
	m_video_ram[m_video_addr] = data;

	m_crtc->register_r();
}


MC6845_ON_UPDATE_ADDR_CHANGED(coco_wpk_device::crtc_addr)
{
	//logerror("crtc_addr: %04x %d\n", address, strobe);
	m_video_addr = address;
}


MC6845_UPDATE_ROW(coco_wpk_device::crtc_update_row)
{
	u32 *p = &bitmap.pix(y);

	for (int column = 0; column < x_count; column++)
	{
		u8 code = m_video_ram[(ma + column) & 0x7ff];
		u16 addr = (code << 4) | (ra & 0x0f);
		u8 data = m_chargen->base()[addr & 0x7ff];

		if (column == cursor_x)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			*p++ = m_palette->pen(BIT(data, 0) && de);

			data >>= 1;
		}
	}
}


void coco_wpk2_device::video_select_w(u8 data)
{
	// software video switch (Word-Pak II only)
	machine().render().first_target()->set_view(BIT(data, 6));
}
