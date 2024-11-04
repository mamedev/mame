// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Irem M27 hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "redalert.h"


#define NUM_CHARMAP_PENS    0x200
#define NUM_BITMAP_PENS     8


/*************************************
 *
 *  Bitmap videoram write handler
 *
 *************************************/

void redalert_state::redalert_bitmap_videoram_w(offs_t offset, uint8_t data)
{
	m_bitmap_videoram[offset     ] = data;
	m_bitmap_colorram[offset >> 3] = *m_bitmap_color & 0x07;
}



/*************************************
 *
 *  Color generation
 *
 *************************************/

// TODO: clean these functions, add F4 viewer, initialize on boot?
void redalert_state::get_redalert_pens(pen_t *pens)
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
	const uint8_t *prom = memregion("proms")->base();

	scaler = compute_resistor_weights(0, 0xff, -1,
										1, resistances_bitmap,     bitmap_weight,      470, 0,
										3, resistances_charmap_rg, charmap_rg_weights, 470, 0,
										2, resistances_charmap_b,  charmap_b_weights,  470, 0);

				compute_resistor_weights(0, 0xff, scaler,
										1, resistances_back_r,     back_r_weight,      470, 0,
										1, resistances_back_gb,    back_gb_weight,     470, 0,
										0, nullptr, nullptr, 0, 0);

	/* the character layer colors come from the PROM */
	for (offs = 0; offs < NUM_CHARMAP_PENS; offs++)
	{
		uint8_t data = prom[offs];

		/* very strange mapping */
		uint8_t r0_bit = (data >> 2) & 0x01;
		uint8_t r1_bit = (data >> 6) & 0x01;
		uint8_t r2_bit = (data >> 4) & 0x01;
		uint8_t g0_bit = (data >> 1) & 0x01;
		uint8_t g1_bit = (data >> 3) & 0x01;
		uint8_t g2_bit = (data >> 5) & 0x01;
		uint8_t b0_bit = (data >> 0) & 0x01;
		uint8_t b1_bit = (data >> 7) & 0x01;

		uint8_t r = combine_weights(charmap_rg_weights, r0_bit, r1_bit, r2_bit);
		uint8_t g = combine_weights(charmap_rg_weights, g0_bit, g1_bit, g2_bit);
		uint8_t b = combine_weights(charmap_b_weights,  b0_bit, b1_bit);

		pens[offs] = rgb_t(r, g, b);
	}

	/* the bitmap layer colors are directly mapped */
	for (offs = 0; offs < NUM_BITMAP_PENS; offs++)
	{
		uint8_t r = bitmap_weight[(offs >> 2) & 0x01];
		uint8_t g = bitmap_weight[(offs >> 1) & 0x01];
		uint8_t b = bitmap_weight[(offs >> 0) & 0x01];

		pens[NUM_CHARMAP_PENS + offs] = rgb_t(r, g, b);
	}

	/* background color */
	pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS] = rgb_t(back_r_weight[0], back_gb_weight[0], back_gb_weight[0]);
}

/* this uses the same color hook-up between bitmap and chars. */
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
	const uint8_t *prom = memregion("proms")->base();

	scaler = compute_resistor_weights(0, 0xff, -1,
										1, resistances_bitmap,     bitmap_weight,      470, 0,
										3, resistances_charmap_rg, charmap_rg_weights, 470, 0,
										2, resistances_charmap_b,  charmap_b_weights,  470, 0);

				compute_resistor_weights(0, 0xff, scaler,
										1, resistances_back_r,     back_r_weight,      470, 0,
										1, resistances_back_gb,    back_gb_weight,     470, 0,
										0, nullptr, nullptr, 0, 0);

	/* the character layer colors come from the PROM */
	for (offs = 0; offs < NUM_CHARMAP_PENS; offs++)
	{
		uint8_t data = prom[offs];

		uint8_t r = bitmap_weight[(~data >> 2) & 0x01];
		uint8_t g = bitmap_weight[(~data >> 1) & 0x01];
		uint8_t b = bitmap_weight[(~data >> 0) & 0x01];

		pens[offs] = rgb_t(r, g, b);
	}

	/* the bitmap layer colors are directly mapped */
	for (offs = 0; offs < NUM_BITMAP_PENS; offs++)
	{
		uint8_t r = bitmap_weight[(offs >> 2) & 0x01];
		uint8_t g = bitmap_weight[(offs >> 1) & 0x01];
		uint8_t b = bitmap_weight[(offs >> 0) & 0x01];

		pens[NUM_CHARMAP_PENS + offs] = rgb_t(r, g, b);
	}

	/* background color */
	// TODO: verify if really black
	pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS] = rgb_t(0, 0, 0);
}

void redalert_state::get_demoneye_pens(pen_t *pens)
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
	const uint8_t *prom = memregion("proms")->base();

	scaler = compute_resistor_weights(0, 0xff, -1,
										1, resistances_bitmap,     bitmap_weight,      470, 0,
										3, resistances_charmap_rg, charmap_rg_weights, 470, 0,
										2, resistances_charmap_b,  charmap_b_weights,  470, 0);

				compute_resistor_weights(0, 0xff, scaler,
										1, resistances_back_r,     back_r_weight,      470, 0,
										1, resistances_back_gb,    back_gb_weight,     470, 0,
										0, nullptr, nullptr, 0, 0);

	/* the character layer colors come from the PROM */
	for (offs = 0; offs < NUM_CHARMAP_PENS; offs++)
	{
		uint8_t data = prom[offs];

		/* very strange mapping */
		uint8_t r0_bit = (data >> 2) & 0x01;
		uint8_t r1_bit = (data >> 6) & 0x01;
		uint8_t r2_bit = (data >> 4) & 0x01;
		uint8_t g0_bit = (data >> 1) & 0x01;
		uint8_t g1_bit = (data >> 3) & 0x01;
		uint8_t g2_bit = (data >> 5) & 0x01;
		uint8_t b0_bit = (data >> 0) & 0x01;
		uint8_t b1_bit = (data >> 7) & 0x01;

		uint8_t r = combine_weights(charmap_rg_weights, r0_bit, r1_bit, r2_bit);
		uint8_t g = combine_weights(charmap_rg_weights, g0_bit, g1_bit, g2_bit);
		uint8_t b = combine_weights(charmap_b_weights,  b0_bit, b1_bit);

		pens[offs] = rgb_t(r, g, b);
	}

	/* the bitmap layer colors are directly mapped */
	for (offs = 0; offs < NUM_BITMAP_PENS; offs++)
	{
		uint8_t r = bitmap_weight[(offs >> 2) & 0x01];
		uint8_t g = bitmap_weight[(offs >> 1) & 0x01];
		uint8_t b = bitmap_weight[(offs >> 0) & 0x01];

		pens[NUM_CHARMAP_PENS + offs] = rgb_t(r, g, b);
	}

	/* background color */
	pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS] = rgb_t(0,0,0);//rgb_t(back_r_weight[0], back_gb_weight[0], back_gb_weight[0]);
}

/*************************************
 *
 *  Video hardware start
 *
 *************************************/

VIDEO_START_MEMBER(redalert_state,redalert)
{
	m_bitmap_colorram = std::make_unique<uint8_t[]>(0x0400);

	save_pointer(NAME(m_bitmap_colorram), 0x0400);

	m_control_xor = 0x00;
}

VIDEO_START_MEMBER(redalert_state,demoneye)
{
	VIDEO_START_CALL_MEMBER( redalert );

	save_item(NAME(m_demoneye_bitmap_reg));
	save_item(NAME(m_demoneye_bitmap_yoffs));
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

uint32_t redalert_state::screen_update_redalert(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS + 1];

	get_redalert_pens(pens);

	for (offs_t offs = 0; offs < 0x2000; offs++)
	{
		uint8_t y = offs & 0xff;
		uint8_t x = (~offs >> 8) << 3;

		uint8_t bitmap_data = m_bitmap_videoram[offs];
		uint8_t bitmap_color = m_bitmap_colorram[offs >> 3];

		uint8_t charmap_code = m_charmap_videoram[0x0000 | (offs >> 3)];
		offs_t charmap_data_base = ((charmap_code & 0x7f) << 3) | (offs & 0x07);

		/* D7 of the char code selects the char set to use */
		uint8_t charmap_data_1, charmap_data_2;
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

		for (int i = 0; i < 8; i++)
		{
			int bitmap_bit = bitmap_data & 0x80;
			uint8_t color_prom_a0_a1 = ((charmap_data_2 & 0x80) >> 6) | ((charmap_data_1 & 0x80) >> 7);

			/* determine priority */
			pen_t pen;
			if ((color_prom_a0_a1 == 0) || (bitmap_bit && ((charmap_code & 0xc0) == 0xc0)))
				pen = bitmap_bit ? pens[NUM_CHARMAP_PENS + bitmap_color] : pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS];
			else
				pen = pens[((charmap_code & 0xfe) << 1) | color_prom_a0_a1];

			if ((*m_video_control ^ m_control_xor) & 0x04)
				bitmap.pix(y, x) = pen;
			else
				bitmap.pix(y ^ 0xff, x ^ 0xff) = pen;

			/* next pixel */
			x++;

			bitmap_data    <<= 1;
			charmap_data_1 <<= 1;
			charmap_data_2 <<= 1;
		}
	}

	return 0;
}



/*************************************
 *
 *  Demoneye-X video update
 *
 *************************************/

/*
    [0]
    xxxx xxxx X position
    [1]
    -??x ---- tile bank * 0x20 (?)
    ---- xx-- <used, unknown purpose>
    ---- --x- (1) 8x8 tile width 4, (0) 4x4
    ---- ---x enable layer
    [2]
    ---- x--- boss second form, <unknown purpose>
    ---- --xx tile bank * 0x100 (?)
    [3]
    ---- --xx <3 on normal/first form boss, 1 on second form>
*/
void redalert_state::demoneye_bitmap_layer_w(offs_t offset, uint8_t data)
{
	m_demoneye_bitmap_reg[offset] = data;
}

uint32_t redalert_state::screen_update_demoneye(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS + 1];

	get_demoneye_pens(pens);

	for (offs_t offs = 0; offs < 0x2000; offs++)
	{
		uint8_t y = offs & 0xff;
		uint8_t x = (~offs >> 8) << 3;

		uint8_t bitmap_data = m_bitmap_videoram[offs];
		uint8_t bitmap_color = m_bitmap_colorram[offs >> 3];

		uint8_t charmap_code = m_charmap_videoram[0x1000 | (offs >> 3)];
		offs_t charmap_data_base = ((charmap_code & 0x7f) << 3) | (offs & 0x07);

		/* D7 of the char code selects the char set to use */
		uint8_t charmap_data_1, charmap_data_2;
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
		//charmap_data_2 = m_charmap_videoram[0x1800 | charmap_data_base];

		for (int i = 0; i < 8; i++)
		{
			int bitmap_bit = bitmap_data & 0x80;
			uint8_t color_prom_a0_a1 = ((charmap_data_2 & 0x80) >> 6) | ((charmap_data_1 & 0x80) >> 7);

			/* determine priority */
			pen_t pen;
			if ((color_prom_a0_a1 == 0) || (bitmap_bit && ((charmap_code & 0xc0) == 0xc0)))
				pen = bitmap_bit ? pens[NUM_CHARMAP_PENS + bitmap_color] : pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS];
			else
				pen = pens[((charmap_code & 0xfe) << 1) | color_prom_a0_a1];

			if (*m_video_control & 0x04)
				bitmap.pix(y ^ 0xff, x ^ 0xff) = pen;
			else
				bitmap.pix(y, x) = pen;

			/* next pixel */
			x++;

			bitmap_data    <<= 1;
			charmap_data_1 <<= 1;
			charmap_data_2 <<= 1;
		}
	}

	u8 x = m_demoneye_bitmap_reg[0];
	u8 y = m_demoneye_bitmap_yoffs;
	u8 control = m_demoneye_bitmap_reg[1];


	if(control&1)
	{
		// TODO: pinpoint what the unknown bits are for (more zooming? color offset?)
		// TODO: layer is offset wrt bullets collision
		// Note: boss second form has even more offsetted collision (hence bigger?)
		int width = (control&2)?8:4;
		int base = 0x1400;
		base += (control & 0x10) ? 0x20 : 0;
		base += (m_demoneye_bitmap_reg[2] & 3) * 0x100;

		for(int x_block=0; x_block<8; ++x_block)
		{
			for(int y_block=0; y_block<8; ++y_block)
			{
				if(y_block<width && x_block<width)
				{
					for(int iy=0;iy<8;++iy)
					{
						int l0 = m_charmap_videoram[base+iy];
						int l1 = m_charmap_videoram[base+0x400+iy];
						int l2 = m_charmap_videoram[base+0x800+iy];
						for(int ix=0;ix<8;++ix)
						{
							int ccc = ((l0&0x80)>>5) | ((l1&0x80)>>6) | ((l2&0x80)>>7);
							if(ccc)
							{
								// both are clearly reversed,
								// cfr. boss first form (when opens the eye)
								// or second form (follows player position)
								int y_dst = 8*width - (y_block*8+iy);
								int x_dst = 8*width - (x_block*8+7-ix);
								ccc=pens[NUM_CHARMAP_PENS+ccc];
								bitmap.pix(y+y_dst, x+x_dst) = ccc;
							}

							l0<<=1;
							l1<<=1;
							l2<<=1;
						}
					}
				}

				base+=8;
			}
		}
	}

	//popmessage("%02x: %02x %02x %02x %02x %04x",m_demoneye_bitmap_yoffs, m_demoneye_bitmap_reg[0], m_demoneye_bitmap_reg[1], m_demoneye_bitmap_reg[2], m_demoneye_bitmap_reg[3], test);
	return 0;
}

/*************************************
 *
 *  Panther video update
 *
 *************************************/

uint32_t redalert_state::screen_update_panther(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS + 1];

	get_panther_pens(pens);

	for (offs_t offs = 0; offs < 0x2000; offs++)
	{
		uint8_t y = offs & 0xff;
		uint8_t x = (~offs >> 8) << 3;

		uint8_t bitmap_data = m_bitmap_videoram[offs];
		uint8_t bitmap_color = m_bitmap_colorram[offs >> 3];

		uint8_t charmap_code = m_charmap_videoram[0x0000 | (offs >> 3)];
		offs_t charmap_data_base = ((charmap_code & 0x7f) << 3) | (offs & 0x07);

		/* D7 of the char code selects the char set to use */
		uint8_t charmap_data_1, charmap_data_2;
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

		for (int i = 0; i < 8; i++)
		{
			int bitmap_bit = bitmap_data & 0x80;
			uint8_t color_prom_a0_a1 = ((charmap_data_2 & 0x80) >> 6) | ((charmap_data_1 & 0x80) >> 7);

			/* determine priority */
			pen_t pen;
			if ((color_prom_a0_a1 == 0) || (bitmap_bit && ((charmap_code & 0xc0) == 0xc0)))
				pen = bitmap_bit ? pens[NUM_CHARMAP_PENS + bitmap_color] : pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS];
			else
				pen = pens[((charmap_code & 0xfe) << 1) | color_prom_a0_a1];

			if ((*m_video_control ^ m_control_xor) & 0x04)
				bitmap.pix(y, x) = pen;
			else
				bitmap.pix(y ^ 0xff, x ^ 0xff) = pen;

			/* next pixel */
			x++;

			bitmap_data    <<= 1;
			charmap_data_1 <<= 1;
			charmap_data_2 <<= 1;
		}
	}

	return 0;
}


/*************************************
 *
 *  Red Alert machine driver
 *
 *************************************/

void redalert_state::redalert_video_common(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(redalert_state::screen_update_redalert));
}

void redalert_state::redalert_video(machine_config &config)
{
	MCFG_VIDEO_START_OVERRIDE(redalert_state,redalert)
	redalert_video_common(config);
}

void redalert_state::ww3_video(machine_config &config)
{
	MCFG_VIDEO_START_OVERRIDE(redalert_state, ww3 )
	redalert_video_common(config);
}


/*************************************
 *
 *  Demoneye-X machine driver
 *
 *************************************/

void redalert_state::demoneye_video(machine_config &config)
{
	MCFG_VIDEO_START_OVERRIDE(redalert_state,demoneye)

	redalert_video_common(config);

	m_screen->set_screen_update(FUNC(redalert_state::screen_update_demoneye));
}


void redalert_state::panther_video(machine_config &config)
{
	MCFG_VIDEO_START_OVERRIDE(redalert_state,ww3)

	redalert_video_common(config);

	m_screen->set_screen_update(FUNC(redalert_state::screen_update_panther));
}
