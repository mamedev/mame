/***************************************************************************

    Neo-Geo hardware

****************************************************************************/

#include "emu.h"
#include "includes/neogeo.h"
#include "video/resnet.h"

#define NUM_PENS    (0x1000)




/*************************************
 *
 *  Palette handling
 *
 *************************************/

void neogeo_state::compute_rgb_weights(  )
{
	static const int resistances[] = { 220, 470, 1000, 2200, 3900 };

	/* compute four sets of weights - with or without the pulldowns -
	   ensuring that we use the same scaler for all */

	double scaler = compute_resistor_weights(0, 0xff, -1,
								5, resistances, m_rgb_weights_normal, 0, 0,
								0, 0, 0, 0, 0,
								0, 0, 0, 0, 0);

	compute_resistor_weights(0, 0xff, scaler,
								5, resistances, m_rgb_weights_normal_bit15, 8200, 0,
								0, 0, 0, 0, 0,
								0, 0, 0, 0, 0);

	compute_resistor_weights(0, 0xff, scaler,
								5, resistances, m_rgb_weights_dark, 150, 0,
								0, 0, 0, 0, 0,
								0, 0, 0, 0, 0);

	compute_resistor_weights(0, 0xff, scaler,
								5, resistances, m_rgb_weights_dark_bit15, 1 / ((1.0 / 8200) + (1.0 / 150)), 0,
								0, 0, 0, 0, 0,
								0, 0, 0, 0, 0);
}


pen_t neogeo_state::get_pen( UINT16 data )
{
	double *weights;
	UINT8 r, g, b;

	if (m_screen_dark)
	{
		if (data & 0x8000)
			weights = m_rgb_weights_dark_bit15;
		else
			weights = m_rgb_weights_dark;
	}
	else
	{
		if (data & 0x8000)
			weights = m_rgb_weights_normal_bit15;
		else
			weights = m_rgb_weights_normal;
	}

	r = combine_5_weights(weights,
							(data >> 11) & 0x01,
							(data >> 10) & 0x01,
							(data >>  9) & 0x01,
							(data >>  8) & 0x01,
							(data >> 14) & 0x01);

	g = combine_5_weights(weights,
							(data >>  7) & 0x01,
							(data >>  6) & 0x01,
							(data >>  5) & 0x01,
							(data >>  4) & 0x01,
							(data >> 13) & 0x01);

	b = combine_5_weights(weights,
							(data >>  3) & 0x01,
							(data >>  2) & 0x01,
							(data >>  1) & 0x01,
							(data >>  0) & 0x01,
							(data >> 12) & 0x01);

	return rgb_t(r, g, b);
}


void neogeo_state::regenerate_pens()
{
	int i;

	for (i = 0; i < NUM_PENS; i++)
		m_pens[i] = get_pen(m_palettes[m_palette_bank][i]);
}


void neogeo_state::neogeo_set_palette_bank( UINT8 data )
{
	if (data != m_palette_bank)
	{
		m_palette_bank = data;

		regenerate_pens();
	}
}


void neogeo_state::neogeo_set_screen_dark( UINT8 data )
{
	if (data != m_screen_dark)
	{
		m_screen_dark = data;

		regenerate_pens();
	}
}


READ16_MEMBER(neogeo_state::neogeo_paletteram_r)
{
	return m_palettes[m_palette_bank][offset];
}


WRITE16_MEMBER(neogeo_state::neogeo_paletteram_w)
{
	UINT16 *addr = &m_palettes[m_palette_bank][offset];

	COMBINE_DATA(addr);

	m_pens[offset] = get_pen(*addr);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void neogeo_state::video_start()
{
	/* allocate memory not directly mapped */
	m_palettes[0] = auto_alloc_array(machine(), UINT16, NUM_PENS);
	m_palettes[1] = auto_alloc_array(machine(), UINT16, NUM_PENS);
	m_pens = auto_alloc_array(machine(), pen_t, NUM_PENS);

	compute_rgb_weights();

	memset(m_palettes[0], 0x00, NUM_PENS * sizeof(UINT16));
	memset(m_palettes[1], 0x00, NUM_PENS * sizeof(UINT16));
	memset(m_pens, 0x00, NUM_PENS * sizeof(pen_t));

	save_pointer(NAME(m_palettes[0]), NUM_PENS);
	save_pointer(NAME(m_palettes[1]), NUM_PENS);
	save_item(NAME(m_screen_dark));
	save_item(NAME(m_palette_bank));
	machine().save().register_postload(save_prepost_delegate(FUNC(neogeo_state::regenerate_pens), this));

	m_sprgen->set_pens(m_pens);
}



/*************************************
 *
 *  Video system reset
 *
 *************************************/

void neogeo_state::video_reset()
{

}



/*************************************
 *
 *  Video update
 *
 *************************************/

UINT32 neogeo_state::screen_update_neogeo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// fill with background color first
	bitmap.fill(m_pens[0x0fff], cliprect);

	m_sprgen->draw_sprites(bitmap, cliprect.min_y);

	m_sprgen->draw_fixed_layer(bitmap, cliprect.min_y);

	return 0;
}



/*************************************
 *
 *  Video control
 *
 *************************************/

UINT16 neogeo_state::get_video_control(  )
{
	UINT16 ret;
	UINT16 v_counter;

	/*
	    The format of this very important location is:  AAAA AAAA A??? BCCC

	    A is the raster line counter. mosyougi relies solely on this to do the
	      raster effects on the title screen; sdodgeb loops waiting for the top
	      bit to be 1; zedblade heavily depends on it to work correctly (it
	      checks the top bit in the IRQ2 handler).
	    B is definitely a PAL/NTSC flag. (LSPC2 only) Evidence:
	      1) trally changes the position of the speed indicator depending on
	         it (0 = lower 1 = higher).
	      2) samsho3 sets a variable to 60 when the bit is 0 and 50 when it's 1.
	         This is obviously the video refresh rate in Hz.
	      3) samsho3 sets another variable to 256 or 307. This could be the total
	         screen height (including vblank), or close to that.
	      Some games (e.g. lstbld2, samsho3) do this (or similar):
	      bclr    #$0, $3c000e.l
	      when the bit is set, so 3c000e (whose function is unknown) has to be
	      related
	    C animation counter lower 3 bits
	*/

	/* the vertical counter chain goes from 0xf8 - 0x1ff */
	v_counter = m_screen->vpos() + 0x100;

	if (v_counter >= 0x200)
		v_counter = v_counter - NEOGEO_VTOTAL;

	ret = (v_counter << 7) | (m_sprgen->neogeo_get_auto_animation_counter() & 0x0007);

	if (VERBOSE) logerror("%s: video_control read (%04x)\n", machine().describe_context(), ret);

	return ret;
}


void neogeo_state::set_video_control( UINT16 data )
{
	if (VERBOSE) logerror("%s: video control write %04x\n", machine().describe_context(), data);

	m_sprgen->set_auto_animation_speed(data >> 8);
	m_sprgen->set_auto_animation_disabled(data & 0x0008);

	neogeo_set_display_position_interrupt_control(data & 0x00f0);
}


READ16_MEMBER(neogeo_state::neogeo_video_register_r)
{
	UINT16 ret;

	/* accessing the LSB only is not mapped */
	if (mem_mask == 0x00ff)
		ret = neogeo_unmapped_r(space, 0, 0xffff) & 0x00ff;
	else
	{
		switch (offset)
		{
		default:
		case 0x00:
		case 0x01: ret = m_sprgen->get_videoram_data(); break;
		case 0x02: ret = m_sprgen->get_videoram_modulo(); break;
		case 0x03: ret = get_video_control(); break;
		}
	}

	return ret;
}


WRITE16_MEMBER(neogeo_state::neogeo_video_register_w)
{
	/* accessing the LSB only is not mapped */
	if (mem_mask != 0x00ff)
	{
		/* accessing the MSB only stores same data in MSB and LSB */
		if (mem_mask == 0xff00)
			data = (data & 0xff00) | (data >> 8);

		switch (offset)
		{
		case 0x00: m_sprgen->set_videoram_offset(data); break;
		case 0x01: m_sprgen->set_videoram_data(data); break;
		case 0x02: m_sprgen->set_videoram_modulo(data); break;
		case 0x03: set_video_control(data); break;
		case 0x04: neogeo_set_display_counter_msb(data); break;
		case 0x05: neogeo_set_display_counter_lsb(data); break;
		case 0x06: neogeo_acknowledge_interrupt(data); break;
		case 0x07: break; // d0: pause timer for 32 lines when in PAL mode (LSPC2 only)
		}
	}
}


