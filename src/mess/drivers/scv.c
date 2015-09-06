// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Driver for Epoch Super Cassette Vision

***************************************************************************/

#include "emu.h"
#include "cpu/upd7810/upd7810.h"
#include "audio/upd1771.h"
#include "bus/scv/slot.h"
#include "bus/scv/rom.h"


class scv_state : public driver_device
{
public:
	scv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this,"videoram"),
		m_maincpu(*this, "maincpu"),
		m_upd1771c(*this, "upd1771c"),
		m_cart(*this, "cartslot"),
		m_pa(*this, "PA"),
		m_pc0(*this, "PC0"),
		m_charrom(*this, "charrom") { }

	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_READ8_MEMBER(portc_r);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_WRITE_LINE_MEMBER(upd1771_ack_w);
	required_shared_ptr<UINT8> m_videoram;
	UINT8 m_porta;
	UINT8 m_portc;
	emu_timer *m_vb_timer;
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_PALETTE_INIT(scv);
	UINT32 screen_update_scv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	enum
	{
		TIMER_VB
	};

	required_device<cpu_device> m_maincpu;
	required_device<upd1771c_device> m_upd1771c;
	required_device<scv_cart_slot_device> m_cart;
	required_ioport_array<8> m_pa;
	required_ioport m_pc0;
	required_memory_region m_charrom;

	ioport_port *m_key[8];
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void scv_set_banks();
	inline void plot_sprite_part( bitmap_ind16 &bitmap, UINT8 x, UINT8 y, UINT8 pat, UINT8 col, UINT8 screen_sprite_start_line );
	inline void draw_sprite( bitmap_ind16 &bitmap, UINT8 x, UINT8 y, UINT8 tile_idx, UINT8 col, UINT8 left, UINT8 right, UINT8 top, UINT8 bottom, UINT8 clip_y, UINT8 screen_sprite_start_line );
	inline void draw_text( bitmap_ind16 &bitmap, UINT8 x, UINT8 y, UINT8 *char_data, UINT8 fg, UINT8 bg );
	inline void draw_semi_graph( bitmap_ind16 &bitmap, UINT8 x, UINT8 y, UINT8 data, UINT8 fg );
	inline void draw_block_graph( bitmap_ind16 &bitmap, UINT8 x, UINT8 y, UINT8 col );
};


static ADDRESS_MAP_START( scv_mem, AS_PROGRAM, 8, scv_state )
	AM_RANGE( 0x0000, 0x0fff ) AM_ROM   // BIOS

	AM_RANGE( 0x2000, 0x3403 ) AM_RAM AM_SHARE("videoram")  // VRAM + 4 registers
	AM_RANGE( 0x3600, 0x3600 ) AM_DEVWRITE("upd1771c", upd1771c_device, write)

	AM_RANGE( 0x8000, 0xff7f ) AM_DEVREADWRITE("cartslot", scv_cart_slot_device, read_cart, write_cart) // cartridge
	AM_RANGE( 0xff80, 0xffff ) AM_RAM   // upd7801 internal RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( scv_io, AS_IO, 8, scv_state )
	AM_RANGE( 0x00, 0x00 ) AM_WRITE(porta_w)
	AM_RANGE( 0x01, 0x01 ) AM_READ(portb_r)
	AM_RANGE( 0x02, 0x02 ) AM_READWRITE(portc_r, portc_w)
ADDRESS_MAP_END


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


WRITE8_MEMBER( scv_state::porta_w )
{
	m_porta = data;
}


READ8_MEMBER( scv_state::portb_r )
{
	UINT8 data = 0xff;

	for (int i = 0; i < 8; i++)
	{
		if (!BIT(m_porta, i))
			data &= m_pa[i]->read();
	}

	return data;
}


READ8_MEMBER( scv_state::portc_r )
{
	UINT8 data = m_portc;

	data = (data & 0xfe) | (m_pc0->read() & 0x01);

	return data;
}


WRITE8_MEMBER( scv_state::portc_w )
{
	//logerror("%04x: scv_portc_w: data = 0x%02x\n", m_maincpu->pc(), data );
	m_portc = data;
	m_cart->write_bank(space, 0, m_portc);
	m_upd1771c->pcm_write(m_portc & 0x08);
}


PALETTE_INIT_MEMBER(scv_state, scv)
{
	/*
	  SCV Epoch-1A chip RGB voltage readouts from paused Bios color test:

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

	  Quamtizing/scaling/rounding between 0 and 255 we thus get:

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
	palette.set_pen_color( 10, 255,   0, 255);
	palette.set_pen_color( 11, 255, 160, 159);
	palette.set_pen_color( 12, 255, 255,   0);
	palette.set_pen_color( 13, 163, 160,   0);
	palette.set_pen_color( 14, 161, 160, 157);
	palette.set_pen_color( 15, 255, 255, 255);
}


void scv_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_VB:
			{
				int vpos = machine().first_screen()->vpos();

				switch( vpos )
				{
				case 240:
					m_maincpu->set_input_line(UPD7810_INTF2, ASSERT_LINE);
					break;
				case 0:
					m_maincpu->set_input_line(UPD7810_INTF2, CLEAR_LINE);
					break;
				}

				m_vb_timer->adjust(machine().first_screen()->time_until_pos((vpos + 1) % 262, 0));
			}
			break;

		default:
			assert_always(FALSE, "Unknown id in scv_state::device_timer");
	}
}


inline void scv_state::plot_sprite_part( bitmap_ind16 &bitmap, UINT8 x, UINT8 y, UINT8 pat, UINT8 col, UINT8 screen_sprite_start_line )
{
	if ( x < 4 )
	{
		return;
	}

	x -= 4;

	if ( y + 2 >= screen_sprite_start_line )
	{
		if ( pat & 0x08 )
		{
			bitmap.pix16(y + 2, x ) = col;
		}
		if ( pat & 0x04 && x < 255 )
		{
			bitmap.pix16(y + 2, x + 1 ) = col;
		}
		if ( pat & 0x02 && x < 254 )
		{
			bitmap.pix16(y + 2, x + 2 ) = col;
		}
		if ( pat & 0x01 && x < 253 )
		{
			bitmap.pix16(y + 2, x + 3 ) = col;
		}
	}
}


inline void scv_state::draw_sprite( bitmap_ind16 &bitmap, UINT8 x, UINT8 y, UINT8 tile_idx, UINT8 col, UINT8 left, UINT8 right, UINT8 top, UINT8 bottom, UINT8 clip_y, UINT8 screen_sprite_start_line )
{
	int j;

	y += clip_y * 2;
	for ( j = clip_y * 4; j < 32; j += 4 )
	{
		UINT8 pat0 = m_videoram[ tile_idx * 32 + j + 0 ];
		UINT8 pat1 = m_videoram[ tile_idx * 32 + j + 1 ];
		UINT8 pat2 = m_videoram[ tile_idx * 32 + j + 2 ];
		UINT8 pat3 = m_videoram[ tile_idx * 32 + j + 3 ];

		if ( ( top && j < 16 ) || ( bottom && j >= 16 ) )
		{
			if ( left )
			{
				plot_sprite_part( bitmap, x     , y, pat0 >> 4, col, screen_sprite_start_line );
				plot_sprite_part( bitmap, x +  4, y, pat1 >> 4, col, screen_sprite_start_line );
			}
			if ( right )
			{
				plot_sprite_part( bitmap, x +  8, y, pat2 >> 4, col, screen_sprite_start_line );
				plot_sprite_part( bitmap, x + 12, y, pat3 >> 4, col, screen_sprite_start_line );
			}

			if ( left )
			{
				plot_sprite_part( bitmap, x     , y + 1, pat0 & 0x0f, col, screen_sprite_start_line );
				plot_sprite_part( bitmap, x +  4, y + 1, pat1 & 0x0f, col, screen_sprite_start_line );
			}
			if ( right )
			{
				plot_sprite_part( bitmap, x +  8, y + 1, pat2 & 0x0f, col, screen_sprite_start_line );
				plot_sprite_part( bitmap, x + 12, y + 1, pat3 & 0x0f, col, screen_sprite_start_line );
			}
		}

		y += 2;
	}
}


inline void scv_state::draw_text( bitmap_ind16 &bitmap, UINT8 x, UINT8 y, UINT8 *char_data, UINT8 fg, UINT8 bg )
{
	int i;

	for ( i = 0; i < 8; i++ )
	{
		UINT8 d = char_data[i];

		bitmap.pix16(y + i, x + 0 ) = ( d & 0x80 ) ? fg : bg;
		bitmap.pix16(y + i, x + 1 ) = ( d & 0x40 ) ? fg : bg;
		bitmap.pix16(y + i, x + 2 ) = ( d & 0x20 ) ? fg : bg;
		bitmap.pix16(y + i, x + 3 ) = ( d & 0x10 ) ? fg : bg;
		bitmap.pix16(y + i, x + 4 ) = ( d & 0x08 ) ? fg : bg;
		bitmap.pix16(y + i, x + 5 ) = ( d & 0x04 ) ? fg : bg;
		bitmap.pix16(y + i, x + 6 ) = ( d & 0x02 ) ? fg : bg;
		bitmap.pix16(y + i, x + 7 ) = ( d & 0x01 ) ? fg : bg;
	}

	for ( i = 8; i < 16; i++ )
	{
		bitmap.pix16(y + i, x + 0 ) = bg;
		bitmap.pix16(y + i, x + 1 ) = bg;
		bitmap.pix16(y + i, x + 2 ) = bg;
		bitmap.pix16(y + i, x + 3 ) = bg;
		bitmap.pix16(y + i, x + 4 ) = bg;
		bitmap.pix16(y + i, x + 5 ) = bg;
		bitmap.pix16(y + i, x + 6 ) = bg;
		bitmap.pix16(y + i, x + 7 ) = bg;
	}

}


inline void scv_state::draw_semi_graph( bitmap_ind16 &bitmap, UINT8 x, UINT8 y, UINT8 data, UINT8 fg )
{
	int i;

	if ( ! data )
		return;

	for ( i = 0; i < 4; i++ )
	{
		bitmap.pix16(y + i, x + 0) = fg;
		bitmap.pix16(y + i, x + 1) = fg;
		bitmap.pix16(y + i, x + 2) = fg;
		bitmap.pix16(y + i, x + 3) = fg;
	}
}


inline void scv_state::draw_block_graph( bitmap_ind16 &bitmap, UINT8 x, UINT8 y, UINT8 col )
{
	int i;

	for ( i = 0; i < 8; i++ )
	{
		bitmap.pix16(y + i, x + 0) = col;
		bitmap.pix16(y + i, x + 1) = col;
		bitmap.pix16(y + i, x + 2) = col;
		bitmap.pix16(y + i, x + 3) = col;
		bitmap.pix16(y + i, x + 4) = col;
		bitmap.pix16(y + i, x + 5) = col;
		bitmap.pix16(y + i, x + 6) = col;
		bitmap.pix16(y + i, x + 7) = col;
	}
}


UINT32 scv_state::screen_update_scv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;
	UINT8 fg = m_videoram[0x1403] >> 4;
	UINT8 bg = m_videoram[0x1403] & 0x0f;
	UINT8 gr_fg = m_videoram[0x1401] >> 4;
	UINT8 gr_bg = m_videoram[0x1401] & 0x0f;
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
			UINT8 d = m_videoram[ 0x1000 + y * 32 + x ];

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
				UINT8 *char_data = m_charrom->base() + ( d & 0x7f ) * 8;
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
		UINT8 screen_start_sprite_line = ( ( ( m_videoram[0x1400] & 0xf7 ) == 0x17 ) && ( ( m_videoram[0x1402] & 0xef ) == 0x4f ) ) ? 21 + 32 : 0 ;
		int i;

		for ( i = 0; i < 128; i++ )
		{
			UINT8 spr_y = m_videoram[ 0x1200 + i * 4 ] & 0xfe;
			UINT8 y_32 = m_videoram[ 0x1200 + i * 4 ] & 0x01;       /* Xx32 sprite */
			UINT8 clip = m_videoram[ 0x1201 + i * 4 ] >> 4;
			UINT8 col = m_videoram[ 0x1201 + i * 4 ] & 0x0f;
			UINT8 spr_x = m_videoram[ 0x1202 + i * 4 ] & 0xfe;
			UINT8 x_32 = m_videoram[ 0x1202 + i * 4 ] & 0x01;       /* 32xX sprite */
			UINT8 tile_idx = m_videoram[ 0x1203 + i * 4 ] & 0x7f;
			UINT8 half = m_videoram[ 0x1203 + i * 4] & 0x80;
			UINT8 left = 1;
			UINT8 right = 1;
			UINT8 top = 1;
			UINT8 bottom = 1;

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
					static const UINT8 spr_2col_lut0[16] = { 0, 15, 12, 13, 10, 11,  8, 9, 6, 7,  4,  5, 2, 3,  1,  1 };
					static const UINT8 spr_2col_lut1[16] = { 0,  1,  8, 11,  2,  3, 10, 9, 4, 5, 12, 13, 6, 7, 14, 15 };

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


WRITE_LINE_MEMBER( scv_state::upd1771_ack_w )
{
	m_maincpu->set_input_line(UPD7810_INTF1, (state) ? ASSERT_LINE : CLEAR_LINE);
}

void scv_state::machine_start()
{
	m_vb_timer = timer_alloc(TIMER_VB);

	save_item(NAME(m_porta));
	save_item(NAME(m_portc));
	if (m_cart->exists())
		m_cart->save_ram();

}


void scv_state::machine_reset()
{
	m_vb_timer->adjust(machine().first_screen()->time_until_pos(0, 0));
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

static GFXDECODE_START( scv )
	GFXDECODE_ENTRY( "charrom", 0x0000, scv_charlayout, 0, 8 )
GFXDECODE_END


static SLOT_INTERFACE_START(scv_cart)
	SLOT_INTERFACE_INTERNAL("rom8k",       SCV_ROM8K)
	SLOT_INTERFACE_INTERNAL("rom16k",      SCV_ROM16K)
	SLOT_INTERFACE_INTERNAL("rom32k",      SCV_ROM32K)
	SLOT_INTERFACE_INTERNAL("rom32k_ram",  SCV_ROM32K_RAM8K)
	SLOT_INTERFACE_INTERNAL("rom64k",      SCV_ROM64K)
	SLOT_INTERFACE_INTERNAL("rom128k",     SCV_ROM128K)
	SLOT_INTERFACE_INTERNAL("rom128k_ram", SCV_ROM128K_RAM4K)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( scv, scv_state )

	MCFG_CPU_ADD( "maincpu", UPD7801, XTAL_4MHz )
	MCFG_CPU_PROGRAM_MAP( scv_mem )
	MCFG_CPU_IO_MAP( scv_io )

	/* Video chip is EPOCH TV-1 */
	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_RAW_PARAMS( XTAL_14_31818MHz/2, 456, 24, 24+192, 262, 23, 23+222 )  /* TODO: Verify */
	MCFG_SCREEN_UPDATE_DRIVER(scv_state, screen_update_scv)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", scv)
	MCFG_PALETTE_ADD( "palette", 16 )
	MCFG_PALETTE_INIT_OWNER(scv_state, scv)

	/* Sound is generated by UPD1771C clocked at XTAL_6MHz */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD( "upd1771c", UPD1771C, XTAL_6MHz )
	MCFG_UPD1771_ACK_HANDLER(WRITELINE(scv_state, upd1771_ack_w))
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

	MCFG_SCV_CARTRIDGE_ADD("cartslot", scv_cart, NULL)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","scv")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( scv_pal, scv )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_CLOCK( 3780000 )

	/* Video chip is EPOCH TV-1A */
	MCFG_SCREEN_MODIFY( "screen" )
	MCFG_SCREEN_RAW_PARAMS( XTAL_13_4MHz/2, 456, 24, 24+192, 342, 23, 23+222 )      /* TODO: Verify */
MACHINE_CONFIG_END


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


/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  INIT    COMPANY  FULLNAME                 FLAGS */
CONS( 1984, scv,     0,      0,      scv,     scv, driver_device,   0,      "Epoch", "Super Cassette Vision", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
CONS( 198?, scv_pal, scv,    0,      scv_pal, scv, driver_device,   0,      "Yeno",  "Super Cassette Vision (PAL)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
