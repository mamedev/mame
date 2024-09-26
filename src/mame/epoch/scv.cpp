// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Driver for Epoch Super Cassette Vision

***************************************************************************/

#include "emu.h"
#include "cpu/upd7810/upd7810.h"
#include "sound/upd1771.h"
#include "bus/scv/slot.h"
#include "bus/scv/rom.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class scv_state : public driver_device
{
public:
	scv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this,"videoram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_upd1771c(*this, "upd1771c"),
		m_cart(*this, "cartslot"),
		m_pa(*this, "PA.%u", 0),
		m_pc0(*this, "PC0"),
		m_charrom(*this, "charrom")
	{ }

	void scv(machine_config &config);
	void scv_pal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(vblank_update);

private:
	void porta_w(uint8_t data);
	uint8_t portb_r();
	uint8_t portc_r();
	void portc_w(uint8_t data);
	void upd1771_ack_w(int state);
	void scv_palette(palette_device &palette) const;
	uint32_t screen_update_scv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void plot_sprite_part(bitmap_ind16 &bitmap, uint8_t x, uint8_t y, uint8_t pat, uint8_t col, uint8_t screen_sprite_start_line);
	void draw_sprite(bitmap_ind16 &bitmap, uint8_t x, uint8_t y, uint8_t tile_idx, uint8_t col, uint8_t left, uint8_t right, uint8_t top, uint8_t bottom, uint8_t clip_y, uint8_t screen_sprite_start_line);
	void draw_text(bitmap_ind16 &bitmap, uint8_t x, uint8_t y, uint8_t *char_data, uint8_t fg, uint8_t bg);
	void draw_semi_graph(bitmap_ind16 &bitmap, uint8_t x, uint8_t y, uint8_t data, uint8_t fg);
	void draw_block_graph(bitmap_ind16 &bitmap, uint8_t x, uint8_t y, uint8_t col);

	void scv_mem(address_map &map) ATTR_COLD;

	uint8_t m_porta;
	uint8_t m_portc;
	emu_timer *m_vb_timer;

	required_shared_ptr<uint8_t> m_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<upd1771c_device> m_upd1771c;
	required_device<scv_cart_slot_device> m_cart;
	required_ioport_array<8> m_pa;
	required_ioport m_pc0;
	required_memory_region m_charrom;
};


void scv_state::scv_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom();   // BIOS

	map(0x2000, 0x3403).ram().share("videoram");  // VRAM + 4 registers
	map(0x3600, 0x3600).w(m_upd1771c, FUNC(upd1771c_device::write));

	map(0x8000, 0xff7f).rw(m_cart, FUNC(scv_cart_slot_device::read_cart), FUNC(scv_cart_slot_device::write_cart)); // cartridge
}


static INPUT_PORTS_START( scv )
	PORT_START( "PA.0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "PA.1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "PA.2" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)

	PORT_START( "PA.3" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)

	PORT_START( "PA.4" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)

	PORT_START( "PA.5" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)

	PORT_START( "PA.6" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)

	PORT_START( "PA.7" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Cl") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("En") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START( "PC0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_O)
INPUT_PORTS_END


void scv_state::porta_w(uint8_t data)
{
	m_porta = data;
}


uint8_t scv_state::portb_r()
{
	uint8_t data = 0xff;

	for (int i = 0; i < 8; i++)
	{
		if (!BIT(m_porta, i))
			data &= m_pa[i]->read();
	}

	return data;
}


uint8_t scv_state::portc_r()
{
	uint8_t data = m_portc;

	data = (data & 0xfe) | (m_pc0->read() & 0x01);

	return data;
}


void scv_state::portc_w(uint8_t data)
{
	//logerror("%04x: scv_portc_w: data = 0x%02x\n", m_maincpu->pc(), data );
	m_portc = data;
	m_cart->write_bank(m_portc);
	m_upd1771c->pcm_write(m_portc & 0x08);
}


void scv_state::scv_palette(palette_device &palette) const
{
	/*
	  SCV Epoch-1A chip RGB voltage readouts from paused BIOS color test:

	  (values in millivolts)

	        R   G   B
	  0    29  29 325
	  1    29  27  22
	  2    25  24 510
	  3   337  28 508
	  4    29 515  22
	  5   336 512 329
	  6    26 515 511
	  7    29 337  25
	  8   520  24  22
	  9   517 338  21
	  10  520  25 512
	  11  521 336 333
	  12  518 515  21
	  13  342 336  22
	  14  337 336 330
	  15  516 511 508

	  Only tree 'bins' of values are obviously captured
	   25 ish
	  330 ish
	  520 ish.

	  Quantizing/scaling/rounding between 0 and 255 we thus get:

	*/
	palette.set_pen_color( 0,   0,   0, 155);
	palette.set_pen_color( 1,   0,   0,   0);
	palette.set_pen_color( 2,   0,   0, 255);
	palette.set_pen_color( 3, 161,   0, 255);
	palette.set_pen_color( 4,   0, 255,   0);
	palette.set_pen_color( 5, 160, 255, 157);
	palette.set_pen_color( 6,   0, 255, 255);
	palette.set_pen_color( 7,   0, 161,   0);
	palette.set_pen_color( 8, 255,   0,   0);
	palette.set_pen_color( 9, 255, 161,   0);
	palette.set_pen_color(10, 255,   0, 255);
	palette.set_pen_color(11, 255, 160, 159);
	palette.set_pen_color(12, 255, 255,   0);
	palette.set_pen_color(13, 163, 160,   0);
	palette.set_pen_color(14, 161, 160, 157);
	palette.set_pen_color(15, 255, 255, 255);
}


TIMER_CALLBACK_MEMBER( scv_state::vblank_update )
{
	int vpos = m_screen->vpos();

	switch ( vpos )
	{
	case 240:
		m_maincpu->set_input_line(UPD7810_INTF2, ASSERT_LINE);
		break;
	case 0:
		m_maincpu->set_input_line(UPD7810_INTF2, CLEAR_LINE);
		break;
	}

	m_vb_timer->adjust(m_screen->time_until_pos((vpos + 1) % 262, 0));
}


inline void scv_state::plot_sprite_part( bitmap_ind16 &bitmap, uint8_t x, uint8_t y, uint8_t pat, uint8_t col, uint8_t screen_sprite_start_line )
{
	if ((x >= 4) && ((y + 2) >= screen_sprite_start_line))
	{
		x -= 4;

		if (pat & 0x08)
			bitmap.pix(y + 2, x) = col;

		if (pat & 0x04 && x < 255 )
			bitmap.pix(y + 2, x + 1) = col;

		if (pat & 0x02 && x < 254)
			bitmap.pix(y + 2, x + 2) = col;

		if (pat & 0x01 && x < 253)
			bitmap.pix(y + 2, x + 3) = col;
	}
}


inline void scv_state::draw_sprite( bitmap_ind16 &bitmap, uint8_t x, uint8_t y, uint8_t tile_idx, uint8_t col, uint8_t left, uint8_t right, uint8_t top, uint8_t bottom, uint8_t clip_y, uint8_t screen_sprite_start_line )
{
	y += clip_y * 2;
	for (int j = clip_y * 4; j < 32; j += 4, y += 2)
	{
		uint8_t const pat0 = m_videoram[tile_idx * 32 + j + 0];
		uint8_t const pat1 = m_videoram[tile_idx * 32 + j + 1];
		uint8_t const pat2 = m_videoram[tile_idx * 32 + j + 2];
		uint8_t const pat3 = m_videoram[tile_idx * 32 + j + 3];

		if ((top && (j < 16)) || (bottom && (j >= 16)))
		{
			if (left)
			{
				plot_sprite_part(bitmap, x     , y, pat0 >> 4, col, screen_sprite_start_line);
				plot_sprite_part(bitmap, x +  4, y, pat1 >> 4, col, screen_sprite_start_line);
			}
			if (right)
			{
				plot_sprite_part(bitmap, x +  8, y, pat2 >> 4, col, screen_sprite_start_line);
				plot_sprite_part(bitmap, x + 12, y, pat3 >> 4, col, screen_sprite_start_line);
			}

			if (left)
			{
				plot_sprite_part(bitmap, x     , y + 1, pat0 & 0x0f, col, screen_sprite_start_line);
				plot_sprite_part(bitmap, x +  4, y + 1, pat1 & 0x0f, col, screen_sprite_start_line);
			}
			if (right)
			{
				plot_sprite_part(bitmap, x +  8, y + 1, pat2 & 0x0f, col, screen_sprite_start_line);
				plot_sprite_part(bitmap, x + 12, y + 1, pat3 & 0x0f, col, screen_sprite_start_line);
			}
		}
	}
}


inline void scv_state::draw_text( bitmap_ind16 &bitmap, uint8_t x, uint8_t y, uint8_t *char_data, uint8_t fg, uint8_t bg )
{
	for ( int i = 0; i < 8; i++ )
	{
		uint8_t const d = char_data[i];

		bitmap.pix(y + i, x + 0 ) = ( d & 0x80 ) ? fg : bg;
		bitmap.pix(y + i, x + 1 ) = ( d & 0x40 ) ? fg : bg;
		bitmap.pix(y + i, x + 2 ) = ( d & 0x20 ) ? fg : bg;
		bitmap.pix(y + i, x + 3 ) = ( d & 0x10 ) ? fg : bg;
		bitmap.pix(y + i, x + 4 ) = ( d & 0x08 ) ? fg : bg;
		bitmap.pix(y + i, x + 5 ) = ( d & 0x04 ) ? fg : bg;
		bitmap.pix(y + i, x + 6 ) = ( d & 0x02 ) ? fg : bg;
		bitmap.pix(y + i, x + 7 ) = ( d & 0x01 ) ? fg : bg;
	}

	for ( int i = 8; i < 16; i++ )
	{
		bitmap.pix(y + i, x + 0 ) = bg;
		bitmap.pix(y + i, x + 1 ) = bg;
		bitmap.pix(y + i, x + 2 ) = bg;
		bitmap.pix(y + i, x + 3 ) = bg;
		bitmap.pix(y + i, x + 4 ) = bg;
		bitmap.pix(y + i, x + 5 ) = bg;
		bitmap.pix(y + i, x + 6 ) = bg;
		bitmap.pix(y + i, x + 7 ) = bg;
	}

}


inline void scv_state::draw_semi_graph( bitmap_ind16 &bitmap, uint8_t x, uint8_t y, uint8_t data, uint8_t fg )
{
	if ( ! data )
		return;

	for ( int i = 0; i < 4; i++ )
	{
		bitmap.pix(y + i, x + 0) = fg;
		bitmap.pix(y + i, x + 1) = fg;
		bitmap.pix(y + i, x + 2) = fg;
		bitmap.pix(y + i, x + 3) = fg;
	}
}


inline void scv_state::draw_block_graph( bitmap_ind16 &bitmap, uint8_t x, uint8_t y, uint8_t col )
{
	for ( int i = 0; i < 8; i++ )
	{
		bitmap.pix(y + i, x + 0) = col;
		bitmap.pix(y + i, x + 1) = col;
		bitmap.pix(y + i, x + 2) = col;
		bitmap.pix(y + i, x + 3) = col;
		bitmap.pix(y + i, x + 4) = col;
		bitmap.pix(y + i, x + 5) = col;
		bitmap.pix(y + i, x + 6) = col;
		bitmap.pix(y + i, x + 7) = col;
	}
}


uint32_t scv_state::screen_update_scv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;
	uint8_t fg = m_videoram[0x1403] >> 4;
	uint8_t bg = m_videoram[0x1403] & 0x0f;
	uint8_t gr_fg = m_videoram[0x1401] >> 4;
	uint8_t gr_bg = m_videoram[0x1401] & 0x0f;
	int clip_x = ( m_videoram[0x1402] & 0x0f ) * 2;
	int clip_y = m_videoram[0x1402] >> 4;

	/* Clear the screen */
	bitmap.fill(gr_bg , cliprect);

	/* Draw background */
	for ( y = 0; y < 16; y++ )
	{
		int text_y = 0;

		if ( y < clip_y )
		{
			text_y = ( m_videoram[0x1400] & 0x80 ) ? 0 : 1;
		}
		else
		{
			text_y = ( m_videoram[0x1400] & 0x80 ) ? 1 : 0;
		}

		for ( x = 0; x < 32; x++ )
		{
			int text_x = 0;
			uint8_t d = m_videoram[ 0x1000 + y * 32 + x ];

			if ( x < clip_x )
			{
				text_x = ( m_videoram[0x1400] & 0x40 ) ? 0 : 1;
			}
			else
			{
				text_x = ( m_videoram[0x1400] & 0x40 ) ? 1 : 0;
			}

			if ( text_x && text_y )
			{
				/* Text mode */
				uint8_t *char_data = m_charrom->base() + ( d & 0x7f ) * 8;
				draw_text( bitmap, x * 8, y * 16, char_data, fg, bg );
			}
			else
			{
				switch ( m_videoram[0x1400] & 0x03 )
				{
				case 0x01:      /* Semi graphics mode */
					draw_semi_graph( bitmap, x * 8    , y * 16     , d & 0x80, gr_fg );
					draw_semi_graph( bitmap, x * 8 + 4, y * 16     , d & 0x40, gr_fg );
					draw_semi_graph( bitmap, x * 8    , y * 16 +  4, d & 0x20, gr_fg );
					draw_semi_graph( bitmap, x * 8 + 4, y * 16 +  4, d & 0x10, gr_fg );
					draw_semi_graph( bitmap, x * 8    , y * 16 +  8, d & 0x08, gr_fg );
					draw_semi_graph( bitmap, x * 8 + 4, y * 16 +  8, d & 0x04, gr_fg );
					draw_semi_graph( bitmap, x * 8    , y * 16 + 12, d & 0x02, gr_fg );
					draw_semi_graph( bitmap, x * 8 + 4, y * 16 + 12, d & 0x01, gr_fg );
					break;

				case 0x03:      /* Block graphics mode */
					draw_block_graph( bitmap, x * 8, y * 16    , d >> 4 );
					draw_block_graph( bitmap, x * 8, y * 16 + 8, d & 0x0f );
					break;

				default:        /* Otherwise draw nothing? */
					break;
				}
			}
		}
	}

	/* Draw sprites if enabled */
	if ( m_videoram[0x1400] & 0x10 )
	{
		uint8_t screen_start_sprite_line = ( ( ( m_videoram[0x1400] & 0xf7 ) == 0x17 ) && ( ( m_videoram[0x1402] & 0xef ) == 0x4f ) ) ? 21 + 32 : 0 ;
		int i;

		for ( i = 0; i < 128; i++ )
		{
			uint8_t spr_y = m_videoram[ 0x1200 + i * 4 ] & 0xfe;
			uint8_t y_32 = m_videoram[ 0x1200 + i * 4 ] & 0x01;       /* Xx32 sprite */
			uint8_t clip = m_videoram[ 0x1201 + i * 4 ] >> 4;
			uint8_t col = m_videoram[ 0x1201 + i * 4 ] & 0x0f;
			uint8_t spr_x = m_videoram[ 0x1202 + i * 4 ] & 0xfe;
			uint8_t x_32 = m_videoram[ 0x1202 + i * 4 ] & 0x01;       /* 32xX sprite */
			uint8_t tile_idx = m_videoram[ 0x1203 + i * 4 ] & 0x7f;
			uint8_t half = m_videoram[ 0x1203 + i * 4] & 0x80;
			uint8_t left = 1;
			uint8_t right = 1;
			uint8_t top = 1;
			uint8_t bottom = 1;

			if ( !col )
			{
				continue;
			}

			if ( !spr_y )
			{
				continue;
			}

			if ( half )
			{
				if ( tile_idx & 0x40 )
				{
					if ( y_32 )
					{
						spr_y -= 8;
						top = 0;
						bottom = 1;
						y_32 = 0;
					}
					else
					{
						top = 1;
						bottom = 0;
					}
				}
				if ( x_32 )
				{
					spr_x -= 8;
					left = 0;
					right = 1;
					x_32 = 0;
				}
				else
				{
					left = 1;
					right = 0;
				}
			}

			/* Check if 2 color sprites are enabled */
			if ( ( m_videoram[0x1400] & 0x20 ) && ( i & 0x20 ) )
			{
				/* 2 color sprite handling */
				draw_sprite( bitmap, spr_x, spr_y, tile_idx, col, left, right, top, bottom, clip, screen_start_sprite_line );
				if ( x_32 || y_32 )
				{
					static const uint8_t spr_2col_lut0[16] = { 0, 15, 12, 13, 10, 11,  8, 9, 6, 7,  4,  5, 2, 3,  1,  1 };
					static const uint8_t spr_2col_lut1[16] = { 0,  1,  8, 11,  2,  3, 10, 9, 4, 5, 12, 13, 6, 7, 14, 15 };

					draw_sprite( bitmap, spr_x, spr_y, tile_idx ^ ( 8 * x_32 + y_32 ), ( i & 0x40 ) ? spr_2col_lut1[col] : spr_2col_lut0[col], left, right, top, bottom, clip, screen_start_sprite_line );
				}
			}
			else
			{
				/* regular sprite handling */
				draw_sprite( bitmap, spr_x, spr_y, tile_idx, col, left, right, top, bottom, clip, screen_start_sprite_line );
				if ( x_32 )
				{
					draw_sprite( bitmap, spr_x + 16, spr_y, tile_idx | 8, col, 1, 1, top, bottom, clip, screen_start_sprite_line );
				}

				if ( y_32 )
				{
					clip = ( clip & 0x08 ) ? ( clip & 0x07 ) : 0;
					draw_sprite( bitmap, spr_x, spr_y + 16, tile_idx | 1, col, left, right, 1, 1, clip, screen_start_sprite_line );
					if ( x_32 )
					{
						draw_sprite( bitmap, spr_x + 16, spr_y + 16, tile_idx | 9, col, 1, 1, 1, 1, clip, screen_start_sprite_line );
					}
				}
			}
		}
	}

	return 0;
}


void scv_state::upd1771_ack_w(int state)
{
	m_maincpu->set_input_line(UPD7810_INTF1, (state) ? ASSERT_LINE : CLEAR_LINE);
}

void scv_state::machine_start()
{
	m_vb_timer = timer_alloc(FUNC(scv_state::vblank_update), this);

	save_item(NAME(m_porta));
	save_item(NAME(m_portc));
	if (m_cart->exists())
		m_cart->save_ram();

}


void scv_state::machine_reset()
{
	m_vb_timer->adjust(m_screen->time_until_pos(0, 0));
}


/* F4 Character Displayer */
static const gfx_layout scv_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_scv )
	GFXDECODE_ENTRY( "charrom", 0x0000, scv_charlayout, 0, 8 )
GFXDECODE_END


static void scv_cart(device_slot_interface &device)
{
	device.option_add_internal("rom8k",       SCV_ROM8K);
	device.option_add_internal("rom16k",      SCV_ROM16K);
	device.option_add_internal("rom32k",      SCV_ROM32K);
	device.option_add_internal("rom32k_ram",  SCV_ROM32K_RAM8K);
	device.option_add_internal("rom64k",      SCV_ROM64K);
	device.option_add_internal("rom128k",     SCV_ROM128K);
	device.option_add_internal("rom128k_ram", SCV_ROM128K_RAM4K);
}

void scv_state::scv(machine_config &config)
{
	upd7801_device &upd(UPD7801(config, m_maincpu, 4_MHz_XTAL));
	upd.set_addrmap(AS_PROGRAM, &scv_state::scv_mem);
	upd.pa_out_cb().set(FUNC(scv_state::porta_w));
	upd.pb_in_cb().set(FUNC(scv_state::portb_r));
	upd.pc_in_cb().set(FUNC(scv_state::portc_r));
	upd.pc_out_cb().set(FUNC(scv_state::portc_w));

	/* Video chip is EPOCH TV-1 */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(14'318'181)/2, 456, 24, 24+192, 262, 23, 23+222);  // TODO: Verify
	m_screen->set_screen_update(FUNC(scv_state::screen_update_scv));
	m_screen->set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_scv);
	PALETTE(config, "palette", FUNC(scv_state::scv_palette), 16);

	/* Sound is generated by UPD1771C clocked at XTAL(6'000'000) */
	SPEAKER(config, "mono").front_center();
	UPD1771C(config, m_upd1771c, 6_MHz_XTAL);
	m_upd1771c->ack_handler().set(FUNC(scv_state::upd1771_ack_w));
	m_upd1771c->add_route(ALL_OUTPUTS, "mono", 1.00);

	SCV_CART_SLOT(config, m_cart, scv_cart, nullptr);

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("scv");
}


void scv_state::scv_pal(machine_config &config)
{
	scv(config);

	m_maincpu->set_clock(3780000);

	// Video chip is EPOCH TV-1A
	m_screen->set_raw(13.4_MHz_XTAL/2, 456, 24, 24+192, 342, 23, 23+222);     // TODO: Verify
}


/* The same bios is used in both the NTSC and PAL versions of the console */
ROM_START( scv )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "upd7801g.s01", 0, 0x1000, CRC(7ac06182) SHA1(6e89d1227581c76441a53d605f9e324185f1da33) )

	ROM_REGION( 0x400, "charrom", 0 )
	ROM_LOAD( "epochtv.chr", 0, 0x400, BAD_DUMP CRC(db521533) SHA1(40b4e44838c35191f115437a14f200f052e71509) )
ROM_END


ROM_START( scv_pal )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "upd7801g.s01", 0, 0x1000, CRC(7ac06182) SHA1(6e89d1227581c76441a53d605f9e324185f1da33) )

	ROM_REGION( 0x400, "charrom", 0 )
	ROM_LOAD( "epochtv.chr", 0, 0x400, BAD_DUMP CRC(db521533) SHA1(40b4e44838c35191f115437a14f200f052e71509) )
ROM_END

} // anonymous namespace


/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME                       FLAGS */
CONS( 1984, scv,     0,      0,      scv,     scv,   scv_state, empty_init, "Epoch", "Super Cassette Vision",       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
CONS( 198?, scv_pal, scv,    0,      scv_pal, scv,   scv_state, empty_init, "Yeno",  "Super Cassette Vision (PAL)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
