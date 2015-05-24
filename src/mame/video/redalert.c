// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Irem Red Alert hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/redalert.h"


#define NUM_CHARMAP_PENS    0x200
#define NUM_BITMAP_PENS     8


/*************************************
 *
 *  Bitmap videoram write handler
 *
 *************************************/

WRITE8_MEMBER(redalert_state::redalert_bitmap_videoram_w)
{
	m_bitmap_videoram[offset     ] = data;
	m_bitmap_colorram[offset >> 3] = *m_bitmap_color & 0x07;
}



/*************************************
 *
 *  Color generation
 *
 *************************************/

void redalert_state::get_pens(pen_t *pens)
{
	static const int resistances_bitmap[]     = { 100 };
	static const int resistances_charmap_rg[] = { 390, 220, 180 };
	static const int resistances_charmap_b[]  = { 220, 100 };
	static const int resistances_back_r[]     = { 1000 + 100 };
	static const int resistances_back_gb[]    = { 100 + 470 };

	offs_t offs;
	double scaler;
	double bitmap_weight[2];
	double charmap_rg_weights[3];
	double charmap_b_weights[2];
	double back_r_weight[1];
	double back_gb_weight[1];
	const UINT8 *prom = memregion("proms")->base();

	scaler = compute_resistor_weights(0, 0xff, -1,
										1, resistances_bitmap,     bitmap_weight,      470, 0,
										3, resistances_charmap_rg, charmap_rg_weights, 470, 0,
										2, resistances_charmap_b,  charmap_b_weights,  470, 0);

				compute_resistor_weights(0, 0xff, scaler,
										1, resistances_back_r,     back_r_weight,      470, 0,
										1, resistances_back_gb,    back_gb_weight,     470, 0,
										0, 0, 0, 0, 0);

	/* the character layer colors come from the PROM */
	for (offs = 0; offs < NUM_CHARMAP_PENS; offs++)
	{
		UINT8 data = prom[offs];

		/* very strange mapping */
		UINT8 r0_bit = (data >> 2) & 0x01;
		UINT8 r1_bit = (data >> 6) & 0x01;
		UINT8 r2_bit = (data >> 4) & 0x01;
		UINT8 g0_bit = (data >> 1) & 0x01;
		UINT8 g1_bit = (data >> 3) & 0x01;
		UINT8 g2_bit = (data >> 5) & 0x01;
		UINT8 b0_bit = (data >> 0) & 0x01;
		UINT8 b1_bit = (data >> 7) & 0x01;

		UINT8 r = combine_3_weights(charmap_rg_weights, r0_bit, r1_bit, r2_bit);
		UINT8 g = combine_3_weights(charmap_rg_weights, g0_bit, g1_bit, g2_bit);
		UINT8 b = combine_2_weights(charmap_b_weights,  b0_bit, b1_bit);

		pens[offs] = rgb_t(r, g, b);
	}

	/* the bitmap layer colors are directly mapped */
	for (offs = 0; offs < NUM_BITMAP_PENS; offs++)
	{
		UINT8 r = bitmap_weight[(offs >> 2) & 0x01];
		UINT8 g = bitmap_weight[(offs >> 1) & 0x01];
		UINT8 b = bitmap_weight[(offs >> 0) & 0x01];

		pens[NUM_CHARMAP_PENS + offs] = rgb_t(r, g, b);
	}

	/* background color */
	pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS] = rgb_t(back_r_weight[0], back_gb_weight[0], back_gb_weight[0]);
}

/* this uses the same color hook-up between bitmap and chars. */
/* TODO: clean me up */
void redalert_state::get_panther_pens(pen_t *pens)
{
	static const int resistances_bitmap[]     = { 100 };
	static const int resistances_charmap_rg[] = { 390, 220, 180 };
	static const int resistances_charmap_b[]  = { 220, 100 };
	static const int resistances_back_r[]     = { 1000 + 100 };
	static const int resistances_back_gb[]    = { 100 + 470 };

	offs_t offs;
	double scaler;
	double bitmap_weight[2];
	double charmap_rg_weights[3];
	double charmap_b_weights[2];
	double back_r_weight[1];
	double back_gb_weight[1];
	const UINT8 *prom = memregion("proms")->base();

	scaler = compute_resistor_weights(0, 0xff, -1,
										1, resistances_bitmap,     bitmap_weight,      470, 0,
										3, resistances_charmap_rg, charmap_rg_weights, 470, 0,
										2, resistances_charmap_b,  charmap_b_weights,  470, 0);

				compute_resistor_weights(0, 0xff, scaler,
										1, resistances_back_r,     back_r_weight,      470, 0,
										1, resistances_back_gb,    back_gb_weight,     470, 0,
										0, 0, 0, 0, 0);

	/* the character layer colors come from the PROM */
	for (offs = 0; offs < NUM_CHARMAP_PENS; offs++)
	{
		UINT8 data = prom[offs];

		UINT8 r = bitmap_weight[(~data >> 2) & 0x01];
		UINT8 g = bitmap_weight[(~data >> 1) & 0x01];
		UINT8 b = bitmap_weight[(~data >> 0) & 0x01];

		pens[offs] = rgb_t(r, g, b);
	}

	/* the bitmap layer colors are directly mapped */
	for (offs = 0; offs < NUM_BITMAP_PENS; offs++)
	{
		UINT8 r = bitmap_weight[(offs >> 2) & 0x01];
		UINT8 g = bitmap_weight[(offs >> 1) & 0x01];
		UINT8 b = bitmap_weight[(offs >> 0) & 0x01];

		pens[NUM_CHARMAP_PENS + offs] = rgb_t(r, g, b);
	}

	/* background color */
	pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS] = rgb_t(back_r_weight[0], back_gb_weight[0], back_gb_weight[0]);
}

/*************************************
 *
 *  Video hardware start
 *
 *************************************/

VIDEO_START_MEMBER(redalert_state,redalert)
{
	m_bitmap_colorram = auto_alloc_array(machine(), UINT8, 0x0400);

	save_pointer(NAME(m_bitmap_colorram), 0x0400);

	m_control_xor = 0x00;
}

VIDEO_START_MEMBER(redalert_state,ww3)
{
	VIDEO_START_CALL_MEMBER( redalert );

	m_control_xor = 0x04;
}


/*************************************
 *
 *  Red Alert video update
 *
 *************************************/

UINT32 redalert_state::screen_update_redalert(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS + 1];
	offs_t offs;

	get_pens(pens);

	for (offs = 0; offs < 0x2000; offs++)
	{
		int i;
		UINT8 charmap_data_1;
		UINT8 charmap_data_2;

		UINT8 y = offs & 0xff;
		UINT8 x = (~offs >> 8) << 3;

		UINT8 bitmap_data = m_bitmap_videoram[offs];
		UINT8 bitmap_color = m_bitmap_colorram[offs >> 3];

		UINT8 charmap_code = m_charmap_videoram[0x0000 | (offs >> 3)];
		offs_t charmap_data_base = ((charmap_code & 0x7f) << 3) | (offs & 0x07);

		/* D7 of the char code selects the char set to use */
		if (charmap_code & 0x80)
		{
			charmap_data_1 = m_charmap_videoram[0x0400 | charmap_data_base];
			charmap_data_2 = m_charmap_videoram[0x0c00 | charmap_data_base];
		}
		else
		{
			charmap_data_1 = 0; /* effectively disables A0 of the color PROM */
			charmap_data_2 = m_charmap_videoram[0x0800 | charmap_data_base];
		}

		for (i = 0; i < 8; i++)
		{
			pen_t pen;

			int bitmap_bit = bitmap_data & 0x80;
			UINT8 color_prom_a0_a1 = ((charmap_data_2 & 0x80) >> 6) | ((charmap_data_1 & 0x80) >> 7);

			/* determine priority */
			if ((color_prom_a0_a1 == 0) || (bitmap_bit && ((charmap_code & 0xc0) == 0xc0)))
				pen = bitmap_bit ? pens[NUM_CHARMAP_PENS + bitmap_color] : pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS];
			else
				pen = pens[((charmap_code & 0xfe) << 1) | color_prom_a0_a1];

			if ((*m_video_control ^ m_control_xor) & 0x04)
				bitmap.pix32(y, x) = pen;
			else
				bitmap.pix32(y ^ 0xff, x ^ 0xff) = pen;

			/* next pixel */
			x = x + 1;

			bitmap_data    = bitmap_data    << 1;
			charmap_data_1 = charmap_data_1 << 1;
			charmap_data_2 = charmap_data_2 << 1;
		}
	}

	return 0;
}



/*************************************
 *
 *  Demoneye-X video update
 *
 *************************************/

UINT32 redalert_state::screen_update_demoneye(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS + 1];
	offs_t offs;

	get_pens(pens);

	for (offs = 0; offs < 0x2000; offs++)
	{
		int i;
		UINT8 charmap_data_1;
		UINT8 charmap_data_2;

		UINT8 y = offs & 0xff;
		UINT8 x = (~offs >> 8) << 3;

		UINT8 bitmap_data = m_bitmap_videoram[offs];
		UINT8 bitmap_color = m_bitmap_colorram[offs >> 3];

		UINT8 charmap_code = m_charmap_videoram[0x1000 | (offs >> 3)];
		offs_t charmap_data_base = ((charmap_code & 0x7f) << 3) | (offs & 0x07);

		/* D7 of the char code selects the char set to use */
		if (charmap_code & 0x80)
		{
			charmap_data_1 = m_charmap_videoram[0x0400 | charmap_data_base];
			charmap_data_2 = m_charmap_videoram[0x0c00 | charmap_data_base];
		}
		else
		{
			charmap_data_1 = m_charmap_videoram[0x0000 | charmap_data_base];
			charmap_data_2 = m_charmap_videoram[0x0800 | charmap_data_base];
		}

		/* this is the mapping of the 3rd char set */
		//charmap_data_1 = m_charmap_videoram[0x1400 | charmap_data_base];
		//charmap_data_2 = m_charmap_videoram[0x1c00 | charmap_data_base];

		for (i = 0; i < 8; i++)
		{
			pen_t pen;

			int bitmap_bit = bitmap_data & 0x80;
			UINT8 color_prom_a0_a1 = ((charmap_data_2 & 0x80) >> 6) | ((charmap_data_1 & 0x80) >> 7);

			/* determine priority */
			if ((color_prom_a0_a1 == 0) || (bitmap_bit && ((charmap_code & 0xc0) == 0xc0)))
				pen = bitmap_bit ? pens[NUM_CHARMAP_PENS + bitmap_color] : pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS];
			else
				pen = pens[((charmap_code & 0xfe) << 1) | color_prom_a0_a1];

			if (*m_video_control & 0x04)
				bitmap.pix32(y ^ 0xff, x ^ 0xff) = pen;
			else
				bitmap.pix32(y, x) = pen;

			/* next pixel */
			x = x + 1;

			bitmap_data    = bitmap_data    << 1;
			charmap_data_1 = charmap_data_1 << 1;
			charmap_data_2 = charmap_data_2 << 1;
		}
	}

	return 0;
}

/*************************************
 *
 *  Panther video update
 *
 *************************************/

UINT32 redalert_state::screen_update_panther(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS + 1];
	offs_t offs;

	get_panther_pens(pens);

	for (offs = 0; offs < 0x2000; offs++)
	{
		int i;
		UINT8 charmap_data_1;
		UINT8 charmap_data_2;

		UINT8 y = offs & 0xff;
		UINT8 x = (~offs >> 8) << 3;

		UINT8 bitmap_data = m_bitmap_videoram[offs];
		UINT8 bitmap_color = m_bitmap_colorram[offs >> 3];

		UINT8 charmap_code = m_charmap_videoram[0x0000 | (offs >> 3)];
		offs_t charmap_data_base = ((charmap_code & 0x7f) << 3) | (offs & 0x07);

		/* D7 of the char code selects the char set to use */
		if (charmap_code & 0x80)
		{
			charmap_data_1 = m_charmap_videoram[0x0400 | charmap_data_base];
			charmap_data_2 = m_charmap_videoram[0x0c00 | charmap_data_base];
		}
		else
		{
			charmap_data_1 = 0; /* effectively disables A0 of the color PROM */
			charmap_data_2 = m_charmap_videoram[0x0800 | charmap_data_base];
		}

		for (i = 0; i < 8; i++)
		{
			pen_t pen;

			int bitmap_bit = bitmap_data & 0x80;
			UINT8 color_prom_a0_a1 = ((charmap_data_2 & 0x80) >> 6) | ((charmap_data_1 & 0x80) >> 7);

			/* determine priority */
			if ((color_prom_a0_a1 == 0) || (bitmap_bit && ((charmap_code & 0xc0) == 0xc0)))
				pen = bitmap_bit ? pens[NUM_CHARMAP_PENS + bitmap_color] : pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS];
			else
				pen = pens[((charmap_code & 0xfe) << 1) | color_prom_a0_a1];

			if ((*m_video_control ^ m_control_xor) & 0x04)
				bitmap.pix32(y, x) = pen;
			else
				bitmap.pix32(y ^ 0xff, x ^ 0xff) = pen;

			/* next pixel */
			x = x + 1;

			bitmap_data    = bitmap_data    << 1;
			charmap_data_1 = charmap_data_1 << 1;
			charmap_data_2 = charmap_data_2 << 1;
		}
	}

	return 0;
}


/*************************************
 *
 *  Red Alert machine driver
 *
 *************************************/

static MACHINE_CONFIG_FRAGMENT( redalert_video_common )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(redalert_state, screen_update_redalert)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( redalert_video )

	MCFG_VIDEO_START_OVERRIDE(redalert_state,redalert)
	MCFG_FRAGMENT_ADD( redalert_video_common )

MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( ww3_video )

	MCFG_VIDEO_START_OVERRIDE(redalert_state, ww3 )
	MCFG_FRAGMENT_ADD( redalert_video_common )

MACHINE_CONFIG_END


/*************************************
 *
 *  Demoneye-X machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( demoneye_video )
	MCFG_VIDEO_START_OVERRIDE(redalert_state,redalert)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(redalert_state, screen_update_demoneye)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( panther_video )

	MCFG_VIDEO_START_OVERRIDE(redalert_state,ww3)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(redalert_state, screen_update_panther)
MACHINE_CONFIG_END
