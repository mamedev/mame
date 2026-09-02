// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, Angelo Salese, Brian Johnson
/***************************************************************************

    QX-series uPD7220 (GDC) video cards

***************************************************************************/

#include "emu.h"
#include "qx_gdc_cards.h"

#include "screen.h"


#define SCREEN_TAG ":screen"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(QX10_VIDEO_GMS, bus::epson_qx::video::q10gms_device, "qx10_video_gms", "Epson Q10GMS Monochrome Graphics Card")
DEFINE_DEVICE_TYPE(QX10_VIDEO_CMS, bus::epson_qx::video::q10cms_device, "qx10_video_cms", "Epson Q10CMS Color Graphics Card")


namespace bus::epson_qx::video {

//-------------------------------------------------
//  shared chargen ROM
//-------------------------------------------------

ROM_START(qx_gdc_card)
	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("qga.2e", 0x0000, 0x1000, CRC(4120b128) SHA1(9b96f6d78cfd402f8aec7c063ffb70a21b78eff0))
ROM_END


//-------------------------------------------------
//  shared gfxdecode
//-------------------------------------------------

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 16,                         /* 8 x 16 characters */
	RGN_FRAC(1,1),                 /* 128 characters */
	1,                             /* 1 bits per pixel */
	{ 0 },                         /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                           /* every char takes 16 bytes */
};

static GFXDECODE_START(gfx_qx_gdc)
	GFXDECODE_ENTRY("chargen", 0x0000, charlayout, 1, 1)
GFXDECODE_END


//**************************************************************************
//  ABSTRACT BASE
//**************************************************************************

qx_gdc_card_device::qx_gdc_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, size_t vram_size)
	: device_t(mconfig, type, tag, owner, clock)
	, device_qx_video_interface(mconfig, *this)
	, m_hgdc(*this, "upd7220")
	, m_screen(*this, SCREEN_TAG)
	, m_palette(*this, "palette")
	, m_chargen(*this, "chargen")
	, m_vram(*this, "vram", vram_size, ENDIANNESS_LITTLE)
	, m_zoom(0)
{
}

const tiny_rom_entry *qx_gdc_card_device::device_rom_region() const
{
	return ROM_NAME(qx_gdc_card);
}

void qx_gdc_card_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette, FUNC(qx_gdc_card_device::palette_init), 8);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_qx_gdc);

	UPD7220(config, m_hgdc, 16.67_MHz_XTAL/4/2);
	m_hgdc->set_addrmap(0, &qx_gdc_card_device::upd7220_map);
	m_hgdc->set_display_pixels(FUNC(qx_gdc_card_device::hgdc_display_pixels));
	m_hgdc->set_draw_text(FUNC(qx_gdc_card_device::hgdc_draw_text));
	m_hgdc->drq_wr_callback().set(*m_slot, FUNC(video_slot_device::drq_w));
	m_hgdc->set_screen(SCREEN_TAG);
}

void qx_gdc_card_device::device_start()
{
	save_item(NAME(m_zoom));
}

void qx_gdc_card_device::install_io(address_space &space)
{
	space.install_device(0x2c, 0x2c, *this, &qx_gdc_card_device::id_map);
	space.install_device(0x38, 0x39, *this, &qx_gdc_card_device::gdc_map);
	space.install_device(0x3a, 0x3a, *this, &qx_gdc_card_device::zoom_map);
}

void qx_gdc_card_device::device_reset()
{
	m_zoom = 0;
}

uint32_t qx_gdc_card_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	m_hgdc->screen_update(screen, bitmap, cliprect);
	return 0;
}

void qx_gdc_card_device::id_map(address_map &map)
{
	map(0x00, 0x00).mirror(0xff00).r(FUNC(qx_gdc_card_device::id_r));
}

void qx_gdc_card_device::gdc_map(address_map &map)
{
	map(0x00, 0x01).mirror(0xff00).rw(m_hgdc, FUNC(upd7220_device::read), FUNC(upd7220_device::write));
}

void qx_gdc_card_device::zoom_map(address_map &map)
{
	map(0x00, 0x00).mirror(0xff00).w(FUNC(qx_gdc_card_device::zoom_w));
}

UPD7220_DRAW_TEXT_LINE_MEMBER(qx_gdc_card_device::hgdc_draw_text)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	addr &= 0xffff;

	for (int x = 0; x < pitch; x++)
	{
		int tile = m_vram[((addr + x) * 2) >> 1] & 0xff;
		int attr = m_vram[((addr + x) * 2) >> 1] >> 8;

		uint8_t color = text_color(attr);

		for (int yi = 0; yi < lr; yi++)
		{
			uint8_t tile_data = m_chargen[tile * 16 + yi];

			if (attr & 8)
				tile_data ^= 0xff;

			if (cursor_on && cursor_addr == addr + x)
				tile_data ^= 0xff;

			if (attr & 0x80 && m_screen->frame_number() & 0x10)
				tile_data = 0;

			for (int xi = 0; xi < 8; xi++)
			{
				int res_x = ((x * 8) + xi) * (m_zoom + 1);
				int res_y = y + (yi * (m_zoom + 1));

				// TODO: cpm22mf:flop2 display random character test will go out of bounds here
				if (!m_screen->visible_area().contains(res_x, res_y))
					continue;

				uint8_t pen;
				if (yi >= 16)
					pen = 0;
				else
					pen = ((tile_data >> xi) & 1) ? color : 0;

				for (int zx = 0; zx <= m_zoom; ++zx)
				{
					for (int zy = 0; zy <= m_zoom; ++zy)
					{
						if (pen)
							bitmap.pix(res_y + zy, res_x + zx) = palette[pen];
					}
				}
			}
		}
	}
}


//**************************************************************************
//  MONOCHROME BASE
//**************************************************************************

qx_gdc_mono_card_device::qx_gdc_mono_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: qx_gdc_card_device(mconfig, type, tag, owner, clock, VRAM_SIZE)
{
}

void qx_gdc_mono_card_device::palette_init(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i, rgb_t::black());
	palette.set_pen_color(1, rgb_t(0x00, 0x9f, 0x00));
	palette.set_pen_color(2, rgb_t(0x00, 0xff, 0x00));
}

void qx_gdc_mono_card_device::upd7220_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("vram");
}

UPD7220_DISPLAY_PIXELS_MEMBER(qx_gdc_mono_card_device::hgdc_display_pixels)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	address &= 0xffff;
	int gfx = m_vram[address];

	for (int xi = 0; xi < 16; xi++)
	{
		uint8_t pen = ((gfx >> xi) & 1) ? 1 : 0;
		for (int z = 0; z <= m_zoom; ++z)
		{
			int xval = ((x + xi) * (m_zoom + 1)) + z;
			if (xval >= bitmap.cliprect().width() * 2)
				continue;
			bitmap.pix(y, xval) = palette[pen];
		}
	}
}


//**************************************************************************
//  COLOR BASE
//**************************************************************************

qx_gdc_color_card_device::qx_gdc_color_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: qx_gdc_card_device(mconfig, type, tag, owner, clock, VRAM_SIZE)
	, m_vram_bank(*this, "vrambank")
	, m_vram_bank_val(0)
{
}

void qx_gdc_color_card_device::device_start()
{
	qx_gdc_card_device::device_start();

	m_vram_bank->configure_entries(0, 3, &m_vram[0], 0x20000);

	save_item(NAME(m_vram_bank_val));
}

void qx_gdc_color_card_device::install_io(address_space &space)
{
	qx_gdc_card_device::install_io(space);
	space.install_device(0x2d, 0x2d, *this, &qx_gdc_color_card_device::bank_map);
}

void qx_gdc_color_card_device::device_reset()
{
	qx_gdc_card_device::device_reset();
	vram_bank_w(1);
}

void qx_gdc_color_card_device::palette_init(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i, pal1bit((i >> 2) & 1), pal1bit((i >> 1) & 1), pal1bit((i >> 0) & 1));
}

void qx_gdc_color_card_device::upd7220_map(address_map &map)
{
	map(0x0000, 0xffff).bankrw("vrambank").mirror(0x30000);
}

void qx_gdc_color_card_device::vram_bank_w(uint8_t data)
{
	m_vram_bank_val = data;

	int bank = -1;
	if (data & 1)      bank = 0; // B
	else if (data & 2) bank = 1; // G
	else if (data & 4) bank = 2; // R

	if (bank >= 0)
		m_vram_bank->set_entry(bank);
}

void qx_gdc_color_card_device::bank_map(address_map &map)
{
	map(0x00, 0x00).mirror(0xff00).rw(FUNC(qx_gdc_color_card_device::vram_bank_r), FUNC(qx_gdc_color_card_device::vram_bank_w));
}

UPD7220_DISPLAY_PIXELS_MEMBER(qx_gdc_color_card_device::hgdc_display_pixels)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	int gfx[3];
	address &= 0xffff;
	gfx[0] = m_vram[address + 0x00000];
	gfx[1] = m_vram[address + 0x10000];
	gfx[2] = m_vram[address + 0x20000];

	for (int xi = 0; xi < 16; xi++)
	{
		uint8_t pen;
		pen  = ((gfx[0] >> xi) & 1) ? 1 : 0;
		pen |= ((gfx[1] >> xi) & 1) ? 2 : 0;
		pen |= ((gfx[2] >> xi) & 1) ? 4 : 0;
		for (int z = 0; z <= m_zoom; ++z)
		{
			int xval = ((x + xi) * (m_zoom + 1)) + z;
			if (xval >= bitmap.cliprect().width() * 2)
				continue;
			bitmap.pix(y, xval) = palette[pen];
		}
	}
}


//**************************************************************************
//  CONCRETE CARDS
//**************************************************************************

q10gms_device::q10gms_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: qx_gdc_mono_card_device(mconfig, QX10_VIDEO_GMS, tag, owner, clock)
{
}

void q10gms_device::install_io(address_space &space)
{
	qx_gdc_mono_card_device::install_io(space);
	space.install_device(0x3b, 0x3b, *this, &q10gms_device::lightpen_map);
}

void q10gms_device::lightpen_map(address_map &map)
{
	map(0x00, 0x00).mirror(0xff00).r(FUNC(q10gms_device::lightpen_r));
}

q10cms_device::q10cms_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: qx_gdc_color_card_device(mconfig, QX10_VIDEO_CMS, tag, owner, clock)
{
}

void q10cms_device::install_io(address_space &space)
{
	qx_gdc_color_card_device::install_io(space);
	space.install_device(0x3b, 0x3b, *this, &q10cms_device::lightpen_map);
}

void q10cms_device::lightpen_map(address_map &map)
{
	map(0x00, 0x00).mirror(0xff00).r(FUNC(q10cms_device::lightpen_r));
}


void video_cards(device_slot_interface &device)
{
	device.option_add("q10gms", QX10_VIDEO_GMS);
	device.option_add("q10cms", QX10_VIDEO_CMS);
}

}
