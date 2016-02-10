// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Zandona'
/***************************************************************************

    JR-200 (c) 1982 National / Panasonic

    driver by Roberto Zandona' and Angelo Salese

    http://www.armchairarcade.com/neo/node/1598

    TODO:
    - Timings are basically screwed, it takes too much to load the POST but
      then the cursor blink is too fast
    - keyboard MCU irq and data polling simulation should be inside a timer
      callback
    - MN1544 4-bit CPU core and ROM dump

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "sound/beep.h"


class jr200_state : public driver_device
{
public:
	jr200_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_cram(*this, "cram"),
		m_mn1271_ram(*this, "mn1271_ram"),
		m_maincpu(*this, "maincpu"),
		m_beeper(*this, "beeper"),
		m_pcg(*this, "pcg"),
		m_gfx_rom(*this, "gfx_rom"),
		m_gfx_ram(*this, "gfx_ram"),
		m_row0(*this, "ROW0"),
		m_row1(*this, "ROW1"),
		m_row2(*this, "ROW2"),
		m_row3(*this, "ROW3"),
		m_row4(*this, "ROW4"),
		m_row5(*this, "ROW5"),
		m_row6(*this, "ROW6"),
		m_row7(*this, "ROW7"),
		m_row8(*this, "ROW8"),
		m_row9(*this, "ROW9"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_vram;
	required_shared_ptr<UINT8> m_cram;
	required_shared_ptr<UINT8> m_mn1271_ram;
	UINT8 m_border_col;
	UINT8 m_old_keydata;
	UINT8 m_freq_reg[2];
	emu_timer *m_timer_d;
	DECLARE_READ8_MEMBER(jr200_pcg_1_r);
	DECLARE_READ8_MEMBER(jr200_pcg_2_r);
	DECLARE_WRITE8_MEMBER(jr200_pcg_1_w);
	DECLARE_WRITE8_MEMBER(jr200_pcg_2_w);
	DECLARE_READ8_MEMBER(jr200_bios_char_r);
	DECLARE_WRITE8_MEMBER(jr200_bios_char_w);
	DECLARE_READ8_MEMBER(mcu_keyb_r);
	DECLARE_WRITE8_MEMBER(jr200_beep_w);
	DECLARE_WRITE8_MEMBER(jr200_beep_freq_w);
	DECLARE_WRITE8_MEMBER(jr200_border_col_w);
	DECLARE_READ8_MEMBER(mn1271_io_r);
	DECLARE_WRITE8_MEMBER(mn1271_io_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_jr200(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(timer_d_callback);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beeper;
	required_memory_region m_pcg;
	required_memory_region m_gfx_rom;
	required_memory_region m_gfx_ram;
	required_ioport m_row0;
	required_ioport m_row1;
	required_ioport m_row2;
	required_ioport m_row3;
	required_ioport m_row4;
	required_ioport m_row5;
	required_ioport m_row6;
	required_ioport m_row7;
	required_ioport m_row8;
	required_ioport m_row9;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};



/* TODO: double check this */
static const UINT8 jr200_keycodes[4][9][8] =
{
	/* unshifted */
	{
	{ 0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
	{ 0x38, 0x39, 0x30, 0x2d, 0x5e, 0x08, 0x7f, 0x2d },
	{ 0x37, 0x38, 0x39, 0x09, 0x71, 0x77, 0x65, 0x72 },
	{ 0x74, 0x79, 0x75, 0x69, 0x6f, 0x70, 0x40, 0x5b },
	{ 0x1b, 0x2b, 0x34, 0x35, 0x36, 0x61, 0x73, 0x64 },
	{ 0x66, 0x67, 0x68, 0x6a, 0x6b, 0x6c, 0x3b, 0x27 },
	{ 0x0d, 0x0a, 0x1e, 0x31, 0x32, 0x33, 0x7a, 0x78 },
	{ 0x63, 0x76, 0x62, 0x6e, 0x6d, 0x2c, 0x2e, 0x2f },
	{ 0x1d, 0x1f, 0x1c, 0x30, 0x2e, 0x20, 0x03, 0x00 }
	},

	/* shifted */
	{
	{ 0x00, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5e, 0x26 },
	{ 0x2a, 0x28, 0x29, 0x3d, 0x10, 0x08, 0x7f, 0x2d },
	{ 0x37, 0x38, 0x39, 0x09, 0x51, 0x57, 0x45, 0x52 },
	{ 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x40, 0x7b },
	{ 0x1b, 0x2b, 0x34, 0x35, 0x36, 0x41, 0x53, 0x44 },
	{ 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x3a, 0x22 },
	{ 0x0d, 0x0a, 0x1e, 0x31, 0x32, 0x33, 0x5a, 0x58 },
	{ 0x43, 0x56, 0x42, 0x4e, 0x4d, 0x3c, 0x3e, 0x3f },
	{ 0x1d, 0x1f, 0x1c, 0x30, 0x2e, 0x20, 0x00, 0x00 }
	},

	/* graph on */
	{
	{ 0x00, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97 },
	{ 0x98, 0x99, 0x90, 0x1f, 0x9a, 0x88, 0xff, 0xad },
	{ 0xb7, 0xb8, 0xb9, 0x89, 0x11, 0x17, 0x05, 0x12 },
	{ 0x14, 0x19, 0x15, 0x09, 0x0f, 0x10, 0x1b, 0x1d },
	{ 0x9b, 0xab, 0xb4, 0xb5, 0xb6, 0x01, 0x13, 0x04 },
	{ 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0c, 0x7e, 0x60 },
	{ 0x8d, 0x8a, 0x81, 0xb1, 0xb2, 0xb3, 0x1a, 0x18 },
	{ 0x03, 0x16, 0x02, 0x0e, 0x0d, 0x1c, 0x7c, 0x5c },
	{ 0x84, 0x82, 0x83, 0xb0, 0xae, 0x00, 0x00, 0x00 }
	},

	/* graph on shifted*/
	{
	{ 0x9e, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97 },
	{ 0x98, 0x99, 0x90, 0x1f, 0x9a, 0x88, 0xff, 0xad },
	{ 0xb7, 0xb8, 0xb9, 0x89, 0x11, 0x17, 0x05, 0x12 },
	{ 0x14, 0x19, 0x15, 0x09, 0x0f, 0x10, 0x1b, 0x1d },
	{ 0x9b, 0xab, 0xb4, 0xb5, 0xb6, 0x01, 0x13, 0x04 },
	{ 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0c, 0x7e, 0x60 },
	{ 0x8d, 0x8a, 0x81, 0xb1, 0xb2, 0xb3, 0x1a, 0x18 },
	{ 0x03, 0x16, 0x02, 0x0e, 0x0d, 0x1c, 0x7c, 0x5c },
	{ 0x84, 0x82, 0x83, 0xb0, 0xae, 0x00, 0x00, 0x00 }
	}
};


void jr200_state::video_start()
{
}

UINT32 jr200_state::screen_update_jr200(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,xi,yi,pen;

	bitmap.fill(m_border_col, cliprect);

	for (y = 0; y < 24; y++)
	{
		for (x = 0; x < 32; x++)
		{
			UINT8 tile = m_vram[x + y*32];
			UINT8 attr = m_cram[x + y*32];

			for(yi=0;yi<8;yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					UINT8 *gfx_data;

					if(attr & 0x80) //bitmap mode
					{
						/*
						    this mode draws 4 x 4 dot blocks, by combining lower 6 bits of tile and attribute vram

						    tile def
						    00xx x--- up-right
						    00-- -xxx up-left
						    attr def
						    10xx x--- down-right
						    10-- -xxx down-left
						*/
						int step;

						step = ((xi & 4) ? 3 : 0);
						step+= ((yi & 4) ? 6 : 0);

						pen = ((((attr & 0x3f) << 6) | (tile & 0x3f)) >> (step)) & 0x07;
					}
					else // tile mode
					{
						gfx_data = (attr & 0x40) ? m_pcg->base() : m_gfx_ram->base();

						pen = (gfx_data[(tile*8)+yi]>>(7-xi) & 1) ? (attr & 0x7) : ((attr & 0x38) >> 3);
					}

					bitmap.pix16(y*8+yi+16, x*8+xi+16) = m_palette->pen(pen);
				}
			}
		}
	}

	return 0;
}

READ8_MEMBER(jr200_state::jr200_pcg_1_r)
{
	return m_pcg->base()[offset+0x000];
}

READ8_MEMBER(jr200_state::jr200_pcg_2_r)
{
	return m_pcg->base()[offset+0x400];
}

WRITE8_MEMBER(jr200_state::jr200_pcg_1_w)
{
	m_pcg->base()[offset+0x000] = data;
	m_gfxdecode->gfx(1)->mark_dirty((offset+0x000) >> 3);
}

WRITE8_MEMBER(jr200_state::jr200_pcg_2_w)
{
	m_pcg->base()[offset+0x400] = data;
	m_gfxdecode->gfx(1)->mark_dirty((offset+0x400) >> 3);
}

READ8_MEMBER(jr200_state::jr200_bios_char_r)
{
	return m_gfx_ram->base()[offset];
}


WRITE8_MEMBER(jr200_state::jr200_bios_char_w)
{
	/* TODO: writing is presumably controlled by an I/O bit */
//  m_gfx_ram->base()[offset] = data;
//  m_gfxdecode->gfx(0)->mark_dirty(offset >> 3);
}

/*

I/O Device

*/

READ8_MEMBER(jr200_state::mcu_keyb_r)
{
	int row, col, table = 0;
	UINT8 keydata = 0;

	if (m_row9->read() & 0x07)
	{
		/* shift, upper case */
		table = 1;
	}

	/* scan keyboard */
	for (row = 0; row < 9; row++)
	{
		UINT8 data = 0xff;

		switch ( row )
		{
			case 0: data = m_row0->read(); break;
			case 1: data = m_row1->read(); break;
			case 2: data = m_row2->read(); break;
			case 3: data = m_row3->read(); break;
			case 4: data = m_row4->read(); break;
			case 5: data = m_row5->read(); break;
			case 6: data = m_row6->read(); break;
			case 7: data = m_row7->read(); break;
			case 8: data = m_row8->read(); break;
		}

		for (col = 0; col < 8; col++)
		{
			if (!BIT(data, col))
			{
				/* latch key data */
				keydata = jr200_keycodes[table][row][col];
			}
		}
	}

	if(m_old_keydata == keydata)
		return 0x00;

	m_old_keydata = keydata;

	return keydata;
}

WRITE8_MEMBER(jr200_state::jr200_beep_w)
{
	/* writing 0x0e enables the beeper, writing anything else disables it */
	m_beeper->set_state(((data & 0xf) == 0x0e) ? 1 : 0);
}

WRITE8_MEMBER(jr200_state::jr200_beep_freq_w)
{
	UINT32 beep_freq;

	m_freq_reg[offset] = data;

	beep_freq = ((m_freq_reg[0]<<8) | (m_freq_reg[1] & 0xff)) + 1;

	m_beeper->set_clock(84000 / beep_freq);
}

WRITE8_MEMBER(jr200_state::jr200_border_col_w)
{
	m_border_col = data;
}


TIMER_CALLBACK_MEMBER(jr200_state::timer_d_callback)
{
	m_maincpu->set_input_line(0, HOLD_LINE);
}

READ8_MEMBER(jr200_state::mn1271_io_r)
{
	UINT8 retVal = m_mn1271_ram[offset];
	if((offset+0xc800) > 0xca00)
		retVal= 0xff;

	switch(offset+0xc800)
	{
		case 0xc801: retVal= mcu_keyb_r(space,0); break;
		case 0xc803: retVal= (m_mn1271_ram[0x03] & 0xcf) | 0x30;  break;//---x ---- printer status ready (ACTIVE HIGH)
		case 0xc807: retVal= (m_mn1271_ram[0x07] & 0x80) | 0x60; break;
		case 0xc80a: retVal= (m_mn1271_ram[0x0a] & 0xfe); break;
		case 0xc80c: retVal= (m_mn1271_ram[0x0c] & 0xdf) | 0x20; break;
		case 0xc80e: retVal= 0; break;
		case 0xc810: retVal= 0; break;
		case 0xc816: retVal= 0x4e; break;
		case 0xc81c: retVal= (m_mn1271_ram[0x1c] & 0xfe) | 1;  break;//bit 0 needs to be high otherwise system refuses to boot
		case 0xc81d: retVal= (m_mn1271_ram[0x1d] & 0xed); break;
	}
	//logerror("mn1271_io_r [%04x] = %02x\n",offset+0xc800,retVal);
	return retVal;
}

WRITE8_MEMBER(jr200_state::mn1271_io_w)
{
	m_mn1271_ram[offset] = data;
	switch(offset+0xc800)
	{
		case 0xc805: break; //LPT printer port W
		case 0xc816: if (data!=0) {
					m_timer_d->adjust(attotime::zero, 0, attotime::from_hz(XTAL_14_31818MHz) * (m_mn1271_ram[0x17]*0x100 + m_mn1271_ram[0x18]));
				} else {
					m_timer_d->adjust(attotime::zero, 0,  attotime::zero);
				}
				break;
		case 0xc819: jr200_beep_w(space,0,data); break;
		case 0xc81a:
		case 0xc81b: jr200_beep_freq_w(space,offset-0x1a,data); break;
		case 0xca00: jr200_border_col_w(space,0,data); break;
	}
}

static ADDRESS_MAP_START(jr200_mem, AS_PROGRAM, 8, jr200_state )
/*
    0000-3fff RAM
    4000-4fff RAM ( 4k expansion)
    4000-7fff RAM (16k expansion)
    4000-bfff RAM (32k expansion)
*/
	AM_RANGE(0x0000, 0x7fff) AM_RAM

	AM_RANGE(0xa000, 0xbfff) AM_ROM

	AM_RANGE(0xc000, 0xc0ff) AM_READWRITE(jr200_pcg_1_r,jr200_pcg_1_w) //PCG area (1)
	AM_RANGE(0xc100, 0xc3ff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xc400, 0xc4ff) AM_READWRITE(jr200_pcg_2_r,jr200_pcg_2_w) //PCG area (2)
	AM_RANGE(0xc500, 0xc7ff) AM_RAM AM_SHARE("cram")

//  0xc800 - 0xcfff I / O area
	AM_RANGE(0xc800, 0xcfff) AM_READWRITE(mn1271_io_r,mn1271_io_w) AM_SHARE("mn1271_ram")

	AM_RANGE(0xd000, 0xd7ff) AM_READWRITE(jr200_bios_char_r,jr200_bios_char_w) //BIOS PCG RAM area
	AM_RANGE(0xd800, 0xdfff) AM_ROM // cart space (header 0x7e)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( jr200 )
//  PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("ROW8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("ROW9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT SHIFT") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT CTRL") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( jr200 )
	GFXDECODE_ENTRY( "gfx_ram", 0, tiles8x8_layout, 0, 1 )
	GFXDECODE_ENTRY( "pcg", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END

void jr200_state::machine_start()
{
	m_timer_d = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(jr200_state::timer_d_callback),this));
}

void jr200_state::machine_reset()
{
	UINT8 *gfx_rom = m_gfx_rom->base();
	UINT8 *gfx_ram = m_gfx_ram->base();
	int i;
	memset(m_mn1271_ram,0,0x800);

	for(i=0;i<0x800;i++)
		gfx_ram[i] = gfx_rom[i];

	for(i=0;i<0x800;i+=8)
		m_gfxdecode->gfx(0)->mark_dirty(i >> 3);
}


static MACHINE_CONFIG_START( jr200, jr200_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6802, XTAL_14_31818MHz / 4) /* MN1800A, ? Mhz assumption that it is same as JR-100*/
	MCFG_CPU_PROGRAM_MAP(jr200_mem)

//  MCFG_CPU_ADD("mn1544", MN1544, ?)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(16 + 256 + 16, 16 + 192 + 16) /* border size not accurate */
	MCFG_SCREEN_VISIBLE_AREA(0, 16 + 256 + 16 - 1, 0, 16 + 192 + 16 - 1)
	MCFG_SCREEN_UPDATE_DRIVER(jr200_state, screen_update_jr200)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", jr200)
	MCFG_PALETTE_ADD_3BIT_BRG("palette")

	MCFG_SPEAKER_STANDARD_MONO("mono")

	// AY-8910 ?

	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS,"mono",0.50)
MACHINE_CONFIG_END



/* ROM definition */
ROM_START( jr200 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "rom1.bin",   0xa000, 0x2000, CRC(bfed707b) SHA1(551823e7ca63f459eb46eb4c7a3e1e169fba2ca2))
	ROM_LOAD( "rom2.bin",   0xe000, 0x2000, CRC(a1cb5027) SHA1(5da98d4ce9cba8096d98e6f2de60baa1673406d0))

	ROM_REGION( 0x10000, "mn1544", ROMREGION_ERASEFF )
	ROM_LOAD( "mn1544.bin",  0x0000, 0x0400, NO_DUMP )

	ROM_REGION( 0x0800, "gfx_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "char.rom", 0x0000, 0x0800, CRC(cb641624) SHA1(6fe890757ebc65bbde67227f9c7c490d8edd84f2) )

	ROM_REGION( 0x0800, "gfx_ram", ROMREGION_ERASEFF )

	ROM_REGION( 0x0800, "pcg", ROMREGION_ERASEFF )
ROM_END

ROM_START( jr200u )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "basic.rom",  0xa000, 0x2000, CRC(cc53eb52) SHA1(910927b98a8338ba072173d79613422a8cb796da) )
	ROM_LOAD( "jr200u.bin", 0xe000, 0x2000, CRC(37ca3080) SHA1(17d3fdedb4de521da7b10417407fa2b61f01a77a) )

	ROM_REGION( 0x10000, "mn1544", ROMREGION_ERASEFF )
	ROM_LOAD( "mn1544.bin",  0x0000, 0x0400, NO_DUMP )

	ROM_REGION( 0x0800, "gfx_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "char.rom", 0x0000, 0x0800, CRC(cb641624) SHA1(6fe890757ebc65bbde67227f9c7c490d8edd84f2) )

	ROM_REGION( 0x0800, "gfx_ram", ROMREGION_ERASEFF )

	ROM_REGION( 0x0800, "pcg", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT   COMPANY   FULLNAME       FLAGS */
COMP( 1982, jr200,  0,       0,     jr200,  jr200, driver_device,    0,          "National",   "JR-200",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1982, jr200u, jr200,   0,     jr200,  jr200, driver_device,    0,          "Panasonic",   "JR-200U",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
