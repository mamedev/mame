// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Aamber Pegasus computer (New Zealand)

    http://web.mac.com/lord_philip/aamber_pegasus/Aamber_Pegasus.html

    Each copy of the monitor rom was made for an individual machine.
    The early bios versions checked that it was running on that
    particular computer.

    This computer has no sound.

    The usual way of loading a new rom was to plug it into the board.
    We have replaced this with cartslots, to save having to recompile
    whenever a new rom is found. Single rom programs will usually work in
    any slot (if it is going to work at all). A working rom will appear
    in the menu. Press the first letter to run it.

    If a machine language program is loaded via cassette, do it in the
    Monitor (L command), when loaded press Enter, and it will be in the
    menu.

    Basic cassettes are loaded in the usual way, that is, start Basic,
    type LOAD, press Enter. When done, RUN or LIST as needed.

    The Aamber Pegasus uses a MCM66710P, which is functionally
    equivalent to the MCM6571.

    Note that datasheet is incorrect for the number "9".
    The first byte is really 0x3E rather than 0x3F, confirmed on real
    hardware.

    TO DO:
    - Work on the other non-working programs

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"


class pegasus_state : public driver_device
{
public:
	pegasus_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_pia_s(*this, "pia_s")
		, m_pia_u(*this, "pia_u")
		, m_exp_00(*this, "exp00")
		, m_exp_01(*this, "exp01")
		, m_exp_02(*this, "exp02")
		, m_exp_0c(*this, "exp0c")
		, m_exp_0d(*this, "exp0d")
		, m_p_videoram(*this, "videoram")
		, m_io_keyboard(*this, "KEY")
	{ }

	DECLARE_READ8_MEMBER(pegasus_keyboard_r);
	DECLARE_READ8_MEMBER(pegasus_protection_r);
	DECLARE_READ8_MEMBER(pegasus_pcg_r);
	DECLARE_WRITE8_MEMBER(pegasus_controls_w);
	DECLARE_WRITE8_MEMBER(pegasus_keyboard_w);
	DECLARE_WRITE8_MEMBER(pegasus_pcg_w);
	DECLARE_READ_LINE_MEMBER(pegasus_keyboard_irq);
	DECLARE_READ_LINE_MEMBER(pegasus_cassette_r);
	DECLARE_WRITE_LINE_MEMBER(pegasus_cassette_w);
	DECLARE_WRITE_LINE_MEMBER(pegasus_firq_clr);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_DRIVER_INIT(pegasus);
	TIMER_DEVICE_CALLBACK_MEMBER(pegasus_firq);
	int load_cart(device_image_interface &image, generic_slot_device *slot, const char *reg_tag);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp00_load) { return load_cart(image, m_exp_00, "0000"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp01_load) { return load_cart(image, m_exp_01, "1000"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp02_load) { return load_cart(image, m_exp_02, "2000"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp0c_load) { return load_cart(image, m_exp_0c, "c000"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp0d_load) { return load_cart(image, m_exp_0d, "d000"); }
private:
	UINT8 m_kbd_row;
	bool m_kbd_irq;
	UINT8 *m_p_pcgram;
	const UINT8 *m_p_chargen;
	UINT8 m_control_bits;
	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void video_start() override;
	void pegasus_decrypt_rom(UINT8 *ROM);
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<pia6821_device> m_pia_s;
	required_device<pia6821_device> m_pia_u;
	required_device<generic_slot_device> m_exp_00;
	required_device<generic_slot_device> m_exp_01;
	required_device<generic_slot_device> m_exp_02;
	required_device<generic_slot_device> m_exp_0c;
	required_device<generic_slot_device> m_exp_0d;
	required_shared_ptr<UINT8> m_p_videoram;
	required_ioport_array<8> m_io_keyboard;
};

TIMER_DEVICE_CALLBACK_MEMBER(pegasus_state::pegasus_firq)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

WRITE_LINE_MEMBER( pegasus_state::pegasus_firq_clr )
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}

READ8_MEMBER( pegasus_state::pegasus_keyboard_r )
{
	UINT8 i,data = 0xff;
	for (i = 0; i < 8; i++)
		if (!BIT(m_kbd_row, i)) data &= m_io_keyboard[i]->read();

	m_kbd_irq = (data == 0xff) ? 1 : 0;
	if BIT(m_control_bits, 3)
		data<<=4;
	return data;
}

WRITE8_MEMBER( pegasus_state::pegasus_keyboard_w )
{
	m_kbd_row = data;
}

WRITE8_MEMBER( pegasus_state::pegasus_controls_w )
{
/*  d0,d2 - not emulated
    d0 - Blank - Video blanking
    d1 - Char - select character rom or ram
    d2 - Page - enables writing to video ram
    d3 - Asc - Select which half of the keyboard to read
*/

	m_control_bits = data;
}

READ_LINE_MEMBER( pegasus_state::pegasus_keyboard_irq )
{
	return m_kbd_irq;
}

READ_LINE_MEMBER( pegasus_state::pegasus_cassette_r )
{
	return m_cass->input();
}

WRITE_LINE_MEMBER( pegasus_state::pegasus_cassette_w )
{
	m_cass->output(state ? 1 : -1);
}

READ8_MEMBER( pegasus_state::pegasus_pcg_r )
{
	UINT8 code = m_p_videoram[offset] & 0x7f;
	return m_p_pcgram[(code << 4) | (~m_kbd_row & 15)];
}

WRITE8_MEMBER( pegasus_state::pegasus_pcg_w )
{
//  if BIT(m_control_bits, 1)
	{
		UINT8 code = m_p_videoram[offset] & 0x7f;
		m_p_pcgram[(code << 4) | (~m_kbd_row & 15)] = data;
	}
}

/* Must return the A register except when it is doing a rom search */
READ8_MEMBER( pegasus_state::pegasus_protection_r )
{
	UINT8 data = m_maincpu->state_int(M6809_A);
	if (data == 0x20) data = 0xff;
	return data;
}

static ADDRESS_MAP_START(pegasus_mem, AS_PROGRAM, 8, pegasus_state)
	ADDRESS_MAP_UNMAP_HIGH
	//AM_RANGE(0x0000, 0x2fff)      // mapped by the cartslots 1-3
	AM_RANGE(0xb000, 0xbdff) AM_RAM
	AM_RANGE(0xbe00, 0xbfff) AM_RAM AM_SHARE("videoram")
	//AM_RANGE(0xc000, 0xdfff)      // mapped by the cartslots 4-5
	AM_RANGE(0xe000, 0xe1ff) AM_READ(pegasus_protection_r)
	AM_RANGE(0xe200, 0xe3ff) AM_READWRITE(pegasus_pcg_r,pegasus_pcg_w)
	AM_RANGE(0xe400, 0xe403) AM_MIRROR(0x1fc) AM_DEVREADWRITE("pia_u", pia6821_device, read, write)
	AM_RANGE(0xe600, 0xe603) AM_MIRROR(0x1fc) AM_DEVREADWRITE("pia_s", pia6821_device, read, write)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(pegasusm_mem, AS_PROGRAM, 8, pegasus_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_IMPORT_FROM(pegasus_mem)
	AM_RANGE(0x5000, 0xafff) AM_RAM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pegasus )
	PORT_START("KEY.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BackSpace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHAR(8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Break") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(20)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l') PORT_CHAR(13)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p') PORT_CHAR(16)

	PORT_START("KEY.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Tab") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("[ ]") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Delete") PORT_CODE(KEYCODE_DEL) PORT_CHAR(127) PORT_CHAR('_')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\' \"") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i') PORT_CHAR(9)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k') PORT_CHAR(11)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o') PORT_CHAR(15)

	PORT_START("KEY.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 *") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("` ~") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j') PORT_CHAR(10)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u') PORT_CHAR(21)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g') PORT_CHAR(7)

	PORT_START("KEY.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t') PORT_CHAR(20)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 &") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // outputs a space
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ShiftR") PORT_NAME("ShiftR") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h') PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y') PORT_CHAR(25)

	PORT_START("KEY.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r') PORT_CHAR(18)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w') PORT_CHAR(23)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // outputs a space
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c') PORT_CHAR(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f') PORT_CHAR(6)

	PORT_START("KEY.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e') PORT_CHAR(5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q') PORT_CHAR(17)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) // REPEAT key which is disconnected - outputs a space
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LineFeed") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(10)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m') PORT_CHAR(13)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d') PORT_CHAR(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n') PORT_CHAR(14)

	PORT_START("KEY.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v') PORT_CHAR(22)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a') PORT_CHAR(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x') PORT_CHAR(24)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BlankL") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(16)

	PORT_START("KEY.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b') PORT_CHAR(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s') PORT_CHAR(19)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CapsLock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(19)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ShiftL") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BlankR") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(21)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z') PORT_CHAR(26)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("{ }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('{') PORT_CHAR('}')
INPUT_PORTS_END

void pegasus_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

static const UINT8 mcm6571a_shift[] =
{
	0,1,1,0,0,0,1,0,0,0,0,1,0,0,0,0,
	1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,
	1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0
};


UINT32 pegasus_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx,inv;
	UINT16 sy=0,ma=0,x;
	bool pcg_mode = BIT(m_control_bits, 1);

	for(y = 0; y < 16; y++ )
	{
		for(ra = 0; ra < 16; ra++ )
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for(x = ma; x < ma + 32; x++ )
			{
				inv = 0xff;
				chr = m_p_videoram[x];

				if (BIT(chr, 7))
				{
					inv = 0;
					chr &= 0x7f;
				}

				if (pcg_mode)
				{
					gfx = m_p_pcgram[(chr << 4) | ra] ^ inv;
				}
				else
				if (mcm6571a_shift[chr])
				{
					if (ra < 3)
						gfx = inv;
					else
						gfx = m_p_chargen[(chr<<4) | (ra-3) ] ^ inv;
				}
				else
				{
					if (ra < 10)
						gfx = m_p_chargen[(chr<<4) | ra ] ^ inv;
					else
						gfx = inv;
				}

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=32;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout pegasus_charlayout =
{
	8, 16,                  /* text = 7 x 9, pcg = 8 x 16 */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( pegasus )
	GFXDECODE_ENTRY( "chargen", 0x0000, pegasus_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "pcg", 0x0000, pegasus_charlayout, 0, 1 )
GFXDECODE_END


// An encrypted single rom starts with 02, decrypted with 20.
// The 2nd and 3rd part of a multi-rom set will have no obvious byte,
// so we check the first 4 bytes for a signature, and decrypt if found.
void pegasus_state::pegasus_decrypt_rom(UINT8 *ROM)
{
	bool doit = FALSE;
	UINT8 b;
	UINT16 j;
	dynamic_buffer temp_copy;
	temp_copy.resize(0x1000);

	if (ROM[0] == 0x02) doit = TRUE;
	if (ROM[0] == 0x1e && ROM[1] == 0xfa && ROM[2] == 0x60 && ROM[3] == 0x71) doit = TRUE; // xbasic 2nd rom
	if (ROM[0] == 0x72 && ROM[1] == 0x62 && ROM[2] == 0xc6 && ROM[3] == 0x36) doit = TRUE; // xbasic 3rd rom
	if (ROM[0] == 0xf0 && ROM[1] == 0x40 && ROM[2] == 0xce && ROM[3] == 0x80) doit = TRUE; // forth 2nd rom (both sets)
	if (ROM[0] == 0x80 && ROM[1] == 0x06 && ROM[2] == 0x68 && ROM[3] == 0x14) doit = TRUE; // pascal 2nd rom

	if (doit)
	{
		for (int i = 0; i < 0x1000; i++)
		{
			b = ROM[i];
			j = BITSWAP16(i, 15, 14, 13, 12, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 7);
			b = BITSWAP8(b, 3, 2, 1, 0, 7, 6, 5, 4);
			temp_copy[j & 0xfff] = b;
		}
		memcpy(ROM, &temp_copy[0], 0x1000);
	}
}

int pegasus_state::load_cart(device_image_interface &image, generic_slot_device *slot, const char *reg_tag)
{
	UINT32 size = slot->common_get_size(reg_tag);
	bool any_socket = false;

	if (size > 0x1000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return IMAGE_INIT_FAIL;
	}

	if (image.software_entry() != nullptr && size == 0)
	{
		// we might be loading a cart compatible with all sockets!
		// so try to get region "rom"
		size = slot->common_get_size("rom");
		any_socket = true;

		if (size == 0)
		{
			std::string errmsg;
			strprintf(errmsg,"Attempted to load a file that does not work in this socket.\nPlease check \"Usage\" field in the software list for the correct socket(s) to use.");
			image.seterror(IMAGE_ERROR_UNSPECIFIED, errmsg.c_str());
			return IMAGE_INIT_FAIL;
		}
	}

	slot->rom_alloc(0x1000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE); // we alloc 0x1000 also for smaller roms!
	slot->common_load_rom(slot->get_rom_base(), size, any_socket ? "rom" : reg_tag);

	// raw images have to be decrypted (in particular the ones from softlist)
	pegasus_decrypt_rom(slot->get_rom_base());

	return IMAGE_INIT_PASS;
}

void pegasus_state::machine_start()
{
	m_p_pcgram = memregion("pcg")->base();

	if (m_exp_00->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x0fff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_exp_00));
	if (m_exp_01->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_exp_01));
	if (m_exp_02->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x2000, 0x2fff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_exp_02));
	if (m_exp_0c->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xc000, 0xcfff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_exp_0c));
	if (m_exp_0d->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xd000, 0xdfff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_exp_0d));
}

void pegasus_state::machine_reset()
{
	m_kbd_row = 0;
	m_kbd_irq = 1;
	m_control_bits = 0;
}

DRIVER_INIT_MEMBER(pegasus_state, pegasus)
{
	// decrypt monitor
	UINT8 *base = memregion("maincpu")->base() + 0xf000;
	pegasus_decrypt_rom(base);
}

static MACHINE_CONFIG_START( pegasus, pegasus_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809E, XTAL_4MHz)  // actually a 6809C - 4MHZ clock coming in, 1MHZ internally
	MCFG_CPU_PROGRAM_MAP(pegasus_mem)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("pegasus_firq", pegasus_state, pegasus_firq, attotime::from_hz(400))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(pegasus_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 16*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 16*16-1)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pegasus)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_DEVICE_ADD("pia_s", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(pegasus_state, pegasus_keyboard_r))
	MCFG_PIA_READCA1_HANDLER(READLINE(pegasus_state, pegasus_cassette_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(pegasus_state, pegasus_keyboard_irq))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(pegasus_state, pegasus_keyboard_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(pegasus_state, pegasus_controls_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(pegasus_state, pegasus_cassette_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(pegasus_state, pegasus_firq_clr))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))

	MCFG_DEVICE_ADD("pia_u", PIA6821, 0)
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))

	MCFG_GENERIC_SOCKET_ADD("exp00", generic_plain_slot, "pegasus_cart")
	MCFG_GENERIC_LOAD(pegasus_state, exp00_load)

	MCFG_GENERIC_SOCKET_ADD("exp01", generic_plain_slot, "pegasus_cart")
	MCFG_GENERIC_LOAD(pegasus_state, exp01_load)

	MCFG_GENERIC_SOCKET_ADD("exp02", generic_plain_slot, "pegasus_cart")
	MCFG_GENERIC_LOAD(pegasus_state, exp02_load)

	MCFG_GENERIC_SOCKET_ADD("exp0c", generic_plain_slot, "pegasus_cart")
	MCFG_GENERIC_LOAD(pegasus_state, exp0c_load)

	MCFG_GENERIC_SOCKET_ADD("exp0d", generic_plain_slot, "pegasus_cart")
	MCFG_GENERIC_LOAD(pegasus_state, exp0d_load)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED|CASSETTE_MOTOR_ENABLED)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "pegasus_cart")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pegasusm, pegasus )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(pegasusm_mem)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( pegasus )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "11r2674", "Monitor 1.1 r2674")
	ROMX_LOAD( "mon11_2674.bin", 0xf000, 0x1000, CRC(1640ff7e) SHA1(8199643749fb40fb8be05e9f311c75620ca939b1), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "10r2569", "Monitor 1.0 r2569")
	ROMX_LOAD( "mon10_2569.bin", 0xf000, 0x1000, CRC(910fc930) SHA1(a4f6bbe5def0268cc49ee7045616a39017dd8052), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "11r2569", "Monitor 1.1 r2569")
	ROMX_LOAD( "mon11_2569.bin", 0xf000, 0x1000, CRC(07b92002) SHA1(3c434601120870c888944ecd9ade5186432ddbc2), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "11r2669", "Monitor 1.1 r2669")
	ROMX_LOAD( "mon11_2669.bin", 0xf000, 0x1000, CRC(f3ee23c8) SHA1(3ac96935668f5e53799c90db5140393c2ef9ce36), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(4, "22r2856", "Monitor 2.2 r2856")
	ROMX_LOAD( "mon22_2856.bin", 0xf000, 0x1000, CRC(5f5f688a) SHA1(3719eecc347e158dd027ea7aa8a068cdafc00d9b), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS(5, "22br2856", "Monitor 2.2B r2856")
	ROMX_LOAD( "mon22b_2856.bin", 0xf000, 0x1000, CRC(a47b0308) SHA1(f215e51aa8df6aed99c10f3df6a3589cb9f63d46), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS(6, "23r2601", "Monitor 2.3 r2601")
	ROMX_LOAD( "mon23_2601.bin", 0xf000, 0x1000, CRC(0e024222) SHA1(9950cba08996931b9d5a3368b44c7309638b4e08), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS(7, "23ar2569", "Monitor 2.3A r2569")
	ROMX_LOAD( "mon23a_2569.bin", 0xf000, 0x1000, CRC(248e62c9) SHA1(adbde27e69b38b29ff89bacf28d0240a8e5d90f3), ROM_BIOS(8) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "6571.bin", 0x0000, 0x0800, CRC(5a25144b) SHA1(7b9fee0c8ef2605b85d12b6d9fe8feb82418c63a) )

	ROM_REGION( 0x800, "pcg", ROMREGION_ERASEFF )
ROM_END

#define rom_pegasusm rom_pegasus

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1981, pegasus,  0,       0,    pegasus,   pegasus, pegasus_state, pegasus, "Technosys",   "Aamber Pegasus", MACHINE_NO_SOUND_HW )
COMP( 1981, pegasusm, pegasus, 0,    pegasusm,  pegasus, pegasus_state, pegasus, "Technosys",   "Aamber Pegasus with RAM expansion unit", MACHINE_NO_SOUND_HW )
