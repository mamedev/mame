// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/******************************************************************************

    Alpha Denshi "NeoGeo" palette devices

    Notes:
    - URL reference: https://wiki.neogeodev.org/index.php?title=Palettes

    TODO:
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
	, m_sync_color_shift(nullptr)
	, m_banknum(1)
	, m_has_shadow(false)
	, m_entries(4096)
	, m_bank(0)
	, m_bank_base(0)
	, m_shadow_base(m_entries)
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
	// set shadow base
	if (m_has_shadow)
		m_shadow_base = m_entries * m_banknum;

	m_paletteram.resize(m_entries * m_banknum);
	m_sync_color_shift = make_unique_clear<int[]>(m_banknum);

	create_rgb_lookups();

	// initialize to black, neogeo only?
	for (int i = 0; i < m_entries; i++)
	{
		for (int b = m_banknum - 1; b >= 0; b--)
		{
			set_bank(b);
			write(i, 0x8000);
		}
	}

	save_item(NAME(m_paletteram));
	save_pointer(NAME(m_sync_color_shift), m_banknum);
	save_item(NAME(m_bank));
	save_item(NAME(m_bank_base));
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
	return m_paletteram[offset + m_bank_base];
}

void alpha68k_palette_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset + m_bank_base]);
	u16 pal_data = m_paletteram[offset + m_bank_base];
	set_color_entry(offset, pal_data);

	// reference color
	if (offset == 0)
	{
		// TODO: actual behaviour, needs HW tests.
		bool is_sync_color = (pal_data & 0x7fff) == 0;
		int sync_color_shift = is_sync_color ? 0 : 2;

		if (sync_color_shift != m_sync_color_shift[m_bank])
		{
			m_sync_color_shift[m_bank] = sync_color_shift;
			for (int i=0; i<m_entries; i++)
				set_color_entry(i, m_paletteram[i + m_bank_base]);
		}
	}
}

inline void alpha68k_palette_device::set_color_entry(u16 offset, u16 pal_data)
{
	offset += m_bank_base;
	int dark = pal_data >> 15;
	int r = ((pal_data >> 14) & 0x1) | ((pal_data >> 7) & 0x1e);
	int g = ((pal_data >> 13) & 0x1) | ((pal_data >> 3) & 0x1e);
	int b = ((pal_data >> 12) & 0x1) | ((pal_data << 1) & 0x1e);

	r >>= m_sync_color_shift[m_bank];
	g >>= m_sync_color_shift[m_bank];
	b >>= m_sync_color_shift[m_bank];

	set_pen_color(offset, m_palette_lookup[r][dark], m_palette_lookup[g][dark], m_palette_lookup[b][dark]);
	if (m_has_shadow)
		set_pen_color(offset + m_shadow_base, m_palette_lookup[r][dark+2], m_palette_lookup[g][dark+2], m_palette_lookup[b][dark+2]);
}
