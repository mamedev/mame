// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "upd777_dev.h"

DEFINE_DEVICE_TYPE(UPD777,    upd777_device,    "upd777",    "uPD777")

upd777_device::upd777_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	upd777_cpu_device(mconfig, UPD777, tag, owner, clock),
	m_gfxdecode(*this, "gfxdecode"),
	m_palette(*this, "palette")
{
}

uint32_t upd777_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	gfx_element *gfx = m_gfxdecode->gfx(0);
	gfx_element *gfx2 = m_gfxdecode->gfx(0);

	for (int i = 0; i <= 0x18; i++)
	{
		u8 s0 = m_datamem[(i * 4) + 0];
		u8 s1 = m_datamem[(i * 4) + 1];
		u8 s2 = m_datamem[(i * 4) + 2];
		u8 s3 = m_datamem[(i * 4) + 3];

		int ypos = (s0 & 0x7e) >> 1;
		//int prio = (s0 & 0x01);
		int xpos = (s1 & 0x7f);
		int patn = (s2 & 0x7f);
		//int ylow = (s3 & 0x70);
		int pal = (s3 & 0x0e) >> 1;
		//int ysub = (s3 & 0x01);

		if (patn<0x70)
			gfx->zoom_transpen(bitmap, cliprect, patn, pal, 0, 0, xpos * 4, ypos * 4, 0x40000, 0x40000, 0);
		else
			gfx2->zoom_transpen(bitmap, cliprect, patn-0x70, pal, 0, 0, xpos * 4, ypos * 4, 0x40000, 0x40000, 0);
	}

	return 0;
}

// documentation says patterns 0x00 - 0x6e are 7x7
// and patterns 0x70 - 0x7e are 8x7
// but they all seem to be stored at 11x7, just with some columns blank?
// this is probably because of how they were read out, over 11 data lines

// 0x00-0x2f are 'Normal (7x7)
// 0x30-0x67 are 'Bent' (7x7)
// 0x68-0x6f are 'Y Repeat' (7x7)
// 0x70-0x77 are 'XY Repeat' (8x7)
// 0x78-0x7f are 'X Repeat' (8x8)
// 
// NOTE, sprite patterns *7 and *f are unused so documentation expresses these ranges as to 66, 6e etc. rather than 67 6f
//
// it isn't clear how the 'Bent' effect etc. is enabled, as clearly not all patterns in this range should use it?

static const gfx_layout test_layout =
{
	7,7,
	0x70,
	1,
	{ 0 },
	{ 4,5,6,7,8,9,10 },
	{ 0*11,1*11,2*11,3*11,4*11,5*11,6*11 },
	7*11
};

static const gfx_layout test2_layout =
{
	8,7,
	0x10,
	1,
	{ 0 },
	{ 3,4,5,6,7,8,9,10 },
	{ 0*11,1*11,2*11,3*11,4*11,5*11,6*11 },
	7*11
};

static GFXDECODE_START( gfx_ud777 )
	GFXDECODE_ENTRY( "patterns", 0x000, test_layout,  0, 8 )
	GFXDECODE_ENTRY( "patterns", 0x436, test2_layout, 0, 8 )
GFXDECODE_END


void upd777_device::palette_init(palette_device &palette) const
{
	// just a fake palette for now
	for (int i = 0; i < palette.entries(); i++)
	{
		if (i & 1)
		{
			palette.set_pen_color(i, rgb_t(((i >> 1) & 1) ? 0xff : 0x7f, ((i >> 2) & 1) ? 0xff : 0x7f, ((i >> 3) & 1) ? 0xff : 0x7f));
		}
		else
		{
			palette.set_pen_color(i, rgb_t(0, 0, 0));
		}
	}
}

void upd777_device::device_add_mconfig(machine_config &config)
{
	// or pass the screen from the driver?
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-0-1);
	screen.set_screen_update(FUNC(upd777_device::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ud777);
	PALETTE(config, m_palette, FUNC(upd777_device::palette_init), 32 * 3).set_entries(0x10);
}

ROM_START( upd777 )
	ROM_REGION16_LE( 0x1000, "prg", ROMREGION_ERASEFF )
	ROM_REGION( 0x4d0, "patterns", ROMREGION_ERASEFF )
ROM_END

const tiny_rom_entry *upd777_device::device_rom_region() const
{
	return ROM_NAME(upd777);
}
