// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/******************************************************************************

    Alpha Denshi "NeoGeo" palette devices

    Notes:
    - URL reference: https://wiki.neogeodev.org/index.php?title=Palettes

    TODO:
    - Make mods to support NeoGeo HW (palette bank, shadows);
    - Are alpha68k.cpp/snk68.cpp with or without shadows?
    - Reference color, research exact consequences about this wiki claim:
      "It always has to be pure black ($8000)(*) otherwise monitors won't
       be happy and other colors won't be displayed correctly."
      (*) tested nam1975/skyadvnt, they actually setup $0000.
      Update: Gold Medalist actually setup $0fff when starter pistol is shot
      on dash events, according to a reference video it causes generally darker
      colors on playfield and a color overflow in the border area.
      We currently just dim the palette to simulate the effect, it's totally
      possible that the side effects are different depending on type of monitor
      used, and maybe the dimming is caused by the capture card uncapable of
      catching up the actual signal and intended behaviour is actually a bright
      flash (without palette clamp?).

******************************************************************************/

#include "emu.h"
#include "alpha68k_palette.h"
#include "video/resnet.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ALPHA68K_PALETTE, alpha68k_palette_device, "alpha68k_palette", "Alpha Denshi Palette device")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  alpha68k_palette_device - constructor
//-------------------------------------------------

alpha68k_palette_device::alpha68k_palette_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ALPHA68K_PALETTE, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
{
}






//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void alpha68k_palette_device::create_rgb_lookups()
{
	static const int resistances[] = {3900, 2200, 1000, 470, 220};

	/* compute four sets of weights - with or without the pulldowns -
	   ensuring that we use the same scaler for all */
	double weights_normal[5];
	double scaler = compute_resistor_weights(0, 255, -1,
											5, resistances, weights_normal, 0, 0,
											0, nullptr, nullptr, 0, 0,
											0, nullptr, nullptr, 0, 0);

	double weights_dark[5];
	compute_resistor_weights(0, 255, scaler,
							5, resistances, weights_dark, 8200, 0,
							0, nullptr, nullptr, 0, 0,
							0, nullptr, nullptr, 0, 0);

	double weights_shadow[5];
	compute_resistor_weights(0, 255, scaler,
							5, resistances, weights_shadow, 150, 0,
							0, nullptr, nullptr, 0, 0,
							0, nullptr, nullptr, 0, 0);

	double weights_dark_shadow[5];
	compute_resistor_weights(0, 255, scaler,
							5, resistances, weights_dark_shadow, 1.0 / ((1.0 / 8200) + (1.0 / 150)), 0,
							0, nullptr, nullptr, 0, 0,
							0, nullptr, nullptr, 0, 0);

	for (int i = 0; i < 32; i++)
	{
		int i4 = (i >> 4) & 1;
		int i3 = (i >> 3) & 1;
		int i2 = (i >> 2) & 1;
		int i1 = (i >> 1) & 1;
		int i0 = (i >> 0) & 1;
		m_palette_lookup[i][0] = combine_weights(weights_normal, i0, i1, i2, i3, i4);
		m_palette_lookup[i][1] = combine_weights(weights_dark, i0, i1, i2, i3, i4);
		m_palette_lookup[i][2] = combine_weights(weights_shadow, i0, i1, i2, i3, i4);
		m_palette_lookup[i][3] = combine_weights(weights_dark_shadow, i0, i1, i2, i3, i4);
	}
}

void alpha68k_palette_device::device_start()
{
	m_paletteram.resize(m_entries);

	create_rgb_lookups();
	save_item(NAME(m_paletteram));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void alpha68k_palette_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint16_t alpha68k_palette_device::read(offs_t offset)
{
	return m_paletteram[offset];
}

inline void alpha68k_palette_device::set_color_entry(u16 offset, u16 pal_data, int shift)
{
	int dark = pal_data >> 15;
	int r = ((pal_data >> 14) & 0x1) | ((pal_data >> 7) & 0x1e);
	int g = ((pal_data >> 13) & 0x1) | ((pal_data >> 3) & 0x1e);
	int b = ((pal_data >> 12) & 0x1) | ((pal_data << 1) & 0x1e);

	r >>= shift;
	g >>= shift;
	b >>= shift;

	set_pen_color(offset, m_palette_lookup[r][dark], m_palette_lookup[g][dark], m_palette_lookup[b][dark]);
}

void alpha68k_palette_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	u16 pal_data = m_paletteram[offset];
	set_color_entry(offset, pal_data, 0);

	// reference color
	if (offset == 0)
	{
		// TODO: actual behaviour, needs HW tests.
		bool is_sync_color = (pal_data & 0x7fff) == 0;
		int sync_color_shift = is_sync_color ? 0 : 2;

		for (int i=0; i<m_entries; i++)
			set_color_entry(i, m_paletteram[i], sync_color_shift);
	}
}
