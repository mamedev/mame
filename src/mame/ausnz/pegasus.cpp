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

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6809/m6809.h"
#include "imagedev/cassette.h"
#include "machine/6821pia.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class pegasus_state : public driver_device
{
public:
	pegasus_state(const machine_config &mconfig, device_type type, const char *tag)
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
		, m_vram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_io_keyboard(*this, "KEY.%u", 0U)
	{ }

	void pegasusm(machine_config &config);
	void pegasus(machine_config &config);

	void init_pegasus();

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	u8 pegasus_keyboard_r();
	u8 pegasus_protection_r();
	u8 pegasus_pcg_r(offs_t offset);
	void pegasus_controls_w(u8 data);
	void pegasus_keyboard_w(u8 data);
	void pegasus_pcg_w(offs_t offset, u8 data);
	int pegasus_cassette_r();
	void pegasus_cassette_w(int state);
	void pegasus_firq_clr(int state);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(pegasus_firq);
	std::pair<std::error_condition, std::string> load_cart(device_image_interface &image, generic_slot_device *slot, const char *reg_tag);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp00_load) { return load_cart(image, m_exp_00, "0000"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp01_load) { return load_cart(image, m_exp_01, "1000"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp02_load) { return load_cart(image, m_exp_02, "2000"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp0c_load) { return load_cart(image, m_exp_0c, "c000"); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp0d_load) { return load_cart(image, m_exp_0d, "d000"); }

	void pegasus_mem(address_map &map) ATTR_COLD;
	void pegasusm_mem(address_map &map) ATTR_COLD;

	u8 m_kbd_row = 0U;
	u8 m_control_bits = 0U;
	std::unique_ptr<u8[]> m_pcg;
	void pegasus_decrypt_rom(u8 *ROM);
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<pia6821_device> m_pia_s;
	required_device<pia6821_device> m_pia_u;
	required_device<generic_slot_device> m_exp_00;
	required_device<generic_slot_device> m_exp_01;
	required_device<generic_slot_device> m_exp_02;
	required_device<generic_slot_device> m_exp_0c;
	required_device<generic_slot_device> m_exp_0d;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_p_chargen;
	required_ioport_array<8> m_io_keyboard;
};

TIMER_DEVICE_CALLBACK_MEMBER(pegasus_state::pegasus_firq)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

void pegasus_state::pegasus_firq_clr(int state)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}

u8 pegasus_state::pegasus_keyboard_r()
{
	u8 data = 0xff;
	for (unsigned i = 0; i < 8; i++)
		if (!BIT(m_kbd_row, i)) data &= m_io_keyboard[i]->read();

	m_pia_s->cb1_w((data == 0xff) ? 1 : 0);
	if (BIT(m_control_bits, 3))
		data <<= 4;
	return data;
}

void pegasus_state::pegasus_keyboard_w(u8 data)
{
	m_kbd_row = data;
}

void pegasus_state::pegasus_controls_w(u8 data)
{
/*  d0,d2 - not emulated
    d0 - Blank - Video blanking
    d1 - Char - select character rom or ram
    d2 - Page - enables writing to video ram
    d3 - Asc - Select which half of the keyboard to read
*/

	m_control_bits = data;
}

int pegasus_state::pegasus_cassette_r()
{
	return m_cass->input();
}

void pegasus_state::pegasus_cassette_w(int state)
{
	m_cass->output(state ? 1 : -1);
}

u8  pegasus_state::pegasus_pcg_r(offs_t offset)
{
	u8 code = m_vram[offset] & 0x7f;
	return m_pcg[(code << 4) | (~m_kbd_row & 15)];
}

void pegasus_state::pegasus_pcg_w(offs_t offset, u8 data)
{
//  if (BIT(m_control_bits, 1))
	{
		u8 code = m_vram[offset] & 0x7f;
		m_pcg[(code << 4) | (~m_kbd_row & 15)] = data;
	}
}

/* Must return the A register except when it is doing a rom search */
u8 pegasus_state::pegasus_protection_r()
{
	u8 data = m_maincpu->state_int(M6809_A);
	if (data == 0x20) data = 0xff;
	return data;
}

void pegasus_state::pegasus_mem(address_map &map)
{
	map.unmap_value_high();
	//map(0x0000, 0x2fff)      // mapped by the cartslots 1-3
	map(0xb000, 0xbdff).ram();
	map(0xbe00, 0xbfff).ram().share("videoram");
	//map(0xc000, 0xdfff)      // mapped by the cartslots 4-5
	map(0xe000, 0xe1ff).r(FUNC(pegasus_state::pegasus_protection_r));
	map(0xe200, 0xe3ff).rw(FUNC(pegasus_state::pegasus_pcg_r), FUNC(pegasus_state::pegasus_pcg_w));
	map(0xe400, 0xe403).mirror(0x1fc).rw(m_pia_u, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xe600, 0xe603).mirror(0x1fc).rw(m_pia_s, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xf000, 0xffff).rom().region("maincpu", 0);
}

void pegasus_state::pegasusm_mem(address_map &map)
{
	map.unmap_value_high();
	pegasus_mem(map);
	map(0x5000, 0xafff).ram();
}

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

static const u8 mcm6571a_shift[] =
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


u32 pegasus_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy=0, ma=0;
	const bool pcg_mode = BIT(m_control_bits, 1);

	for (u8 y = 0; y < 16; y++)
	{
		for (u8 ra = 0; ra < 16; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = ma; x < ma + 32; x++)
			{
				u8 inv = 0xff;
				u8 chr = m_vram[x];

				if (BIT(chr, 7))
				{
					inv = 0;
					chr &= 0x7f;
				}

				u8 gfx;
				if (pcg_mode)
				{
					gfx = m_pcg[(chr << 4) | ra] ^ inv;
				}
				else if (mcm6571a_shift[chr])
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
	8, 16,                  // text = 7 x 9, pcg = 8 x 16
	128,                    // 128 characters
	1,                      // 1 bits per pixel
	{ 0 },                  // no bitplanes
	// x offsets
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	// y offsets
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    // every char takes 16 bytes
};

static GFXDECODE_START( gfx_pegasus )
	GFXDECODE_ENTRY( "chargen", 0x0000, pegasus_charlayout, 0, 1 )
	//GFXDECODE_ENTRY( "pcg", 0x0000, pegasus_charlayout, 0, 1 )
GFXDECODE_END


// An encrypted single rom starts with 02, decrypted with 20.
// The 2nd and 3rd part of a multi-rom set will have no obvious byte,
// so we check the first 4 bytes for a signature, and decrypt if found.
void pegasus_state::pegasus_decrypt_rom(u8 *ROM)
{
	bool doit = false;

	if (ROM[0] == 0x02) doit = true;
	if (ROM[0] == 0x1e && ROM[1] == 0xfa && ROM[2] == 0x60 && ROM[3] == 0x71) doit = true; // xbasic 2nd rom
	if (ROM[0] == 0x72 && ROM[1] == 0x62 && ROM[2] == 0xc6 && ROM[3] == 0x36) doit = true; // xbasic 3rd rom
	if (ROM[0] == 0xf0 && ROM[1] == 0x40 && ROM[2] == 0xce && ROM[3] == 0x80) doit = true; // forth 2nd rom (both sets)
	if (ROM[0] == 0x80 && ROM[1] == 0x06 && ROM[2] == 0x68 && ROM[3] == 0x14) doit = true; // pascal 2nd rom

	if (doit)
	{
		std::vector<u8> temp_copy;
		temp_copy.resize(0x1000);
		for (int i = 0; i < 0x1000; i++)
		{
			const u16 j = bitswap<16>(i, 15, 14, 13, 12, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 7);
			const u8 b = bitswap<8>(ROM[i], 3, 2, 1, 0, 7, 6, 5, 4);
			temp_copy[j & 0xfff] = b;
		}
		memcpy(ROM, &temp_copy[0], 0x1000);
	}
}

std::pair<std::error_condition, std::string> pegasus_state::load_cart(device_image_interface &image, generic_slot_device *slot, const char *reg_tag)
{
	u32 size = slot->common_get_size(reg_tag);
	bool any_socket = false;

	if (size > 0x1000)
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be no larger than 4K)");

	if (image.loaded_through_softlist() && size == 0)
	{
		// we might be loading a cart compatible with all sockets!
		// so try to get region "rom"
		size = slot->common_get_size("rom");
		any_socket = true;

		if (size == 0)
		{
			// FIXME: multi-line message may not be displayed properly
			return std::make_pair(
					image_error::INVALIDIMAGE,
					"Attempted to load a file that does not work in this socket.\n"
					"Please check \"Usage\" field in the software list for the correct socket(s) to use.");
		}
	}

	slot->rom_alloc(0x1000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE); // we alloc 0x1000 also for smaller ROMs!
	slot->common_load_rom(slot->get_rom_base(), size, any_socket ? "rom" : reg_tag);

	// raw images have to be decrypted (in particular the ones from softlist)
	pegasus_decrypt_rom(slot->get_rom_base());

	return std::make_pair(std::error_condition(), std::string());
}

void pegasus_state::machine_start()
{
	if (m_exp_00->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x0fff, read8sm_delegate(*m_exp_00, FUNC(generic_slot_device::read_rom)));
	if (m_exp_01->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8sm_delegate(*m_exp_01, FUNC(generic_slot_device::read_rom)));
	if (m_exp_02->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x2000, 0x2fff, read8sm_delegate(*m_exp_02, FUNC(generic_slot_device::read_rom)));
	if (m_exp_0c->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xc000, 0xcfff, read8sm_delegate(*m_exp_0c, FUNC(generic_slot_device::read_rom)));
	if (m_exp_0d->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xd000, 0xdfff, read8sm_delegate(*m_exp_0d, FUNC(generic_slot_device::read_rom)));
	m_pcg = make_unique_clear<u8[]>(0x0800);
	save_pointer(NAME(m_pcg), 0x0800);
	save_item(NAME(m_kbd_row));
	save_item(NAME(m_control_bits));
}

void pegasus_state::machine_reset()
{
	m_kbd_row = 0;
	m_pia_s->cb1_w(1);
	m_control_bits = 0;
}

void pegasus_state::init_pegasus()
{
	// decrypt monitor
	u8 *base = memregion("maincpu")->base();
	pegasus_decrypt_rom(base);
}

void pegasus_state::pegasus(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, XTAL(4'000'000));  // actually a 6809C - 4MHZ clock coming in, 1MHZ internally
	m_maincpu->set_addrmap(AS_PROGRAM, &pegasus_state::pegasus_mem);

	TIMER(config, "pegasus_firq").configure_periodic(FUNC(pegasus_state::pegasus_firq), attotime::from_hz(400));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(pegasus_state::screen_update));
	screen.set_size(32*8, 16*16);
	screen.set_visarea(0, 32*8-1, 0, 16*16-1);
	screen.set_palette("palette");
	GFXDECODE(config, "gfxdecode", "palette", gfx_pegasus);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* devices */
	PIA6821(config, m_pia_s);
	m_pia_s->readpb_handler().set(FUNC(pegasus_state::pegasus_keyboard_r));
	m_pia_s->readca1_handler().set(FUNC(pegasus_state::pegasus_cassette_r));
	m_pia_s->writepa_handler().set(FUNC(pegasus_state::pegasus_keyboard_w));
	m_pia_s->writepb_handler().set(FUNC(pegasus_state::pegasus_controls_w));
	m_pia_s->ca2_handler().set(FUNC(pegasus_state::pegasus_cassette_w));
	m_pia_s->cb2_handler().set(FUNC(pegasus_state::pegasus_firq_clr));
	m_pia_s->irqa_handler().set_inputline("maincpu", M6809_IRQ_LINE);
	m_pia_s->irqb_handler().set_inputline("maincpu", M6809_IRQ_LINE);

	PIA6821(config, m_pia_u);
	m_pia_u->irqa_handler().set_inputline("maincpu", M6809_IRQ_LINE);
	m_pia_u->irqb_handler().set_inputline("maincpu", M6809_IRQ_LINE);

	GENERIC_SOCKET(config, "exp00", generic_plain_slot, "pegasus_cart").set_device_load(FUNC(pegasus_state::exp00_load));
	GENERIC_SOCKET(config, "exp01", generic_plain_slot, "pegasus_cart").set_device_load(FUNC(pegasus_state::exp01_load));
	GENERIC_SOCKET(config, "exp02", generic_plain_slot, "pegasus_cart").set_device_load(FUNC(pegasus_state::exp02_load));
	GENERIC_SOCKET(config, "exp0c", generic_plain_slot, "pegasus_cart").set_device_load(FUNC(pegasus_state::exp0c_load));
	GENERIC_SOCKET(config, "exp0d", generic_plain_slot, "pegasus_cart").set_device_load(FUNC(pegasus_state::exp0d_load));

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED|CASSETTE_MOTOR_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("pegasus_cart");
}

void pegasus_state::pegasusm(machine_config &config)
{
	pegasus(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pegasus_state::pegasusm_mem);
}


/* ROM definition */
ROM_START( pegasus )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "11r2674", "Monitor 1.1 r2674")
	ROMX_LOAD( "mon11_2674.bin",  0x0000, 0x1000, CRC(1640ff7e) SHA1(8199643749fb40fb8be05e9f311c75620ca939b1), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "10r2569", "Monitor 1.0 r2569")
	ROMX_LOAD( "mon10_2569.bin",  0x0000, 0x1000, CRC(910fc930) SHA1(a4f6bbe5def0268cc49ee7045616a39017dd8052), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "11r2569", "Monitor 1.1 r2569")
	ROMX_LOAD( "mon11_2569.bin",  0x0000, 0x1000, CRC(07b92002) SHA1(3c434601120870c888944ecd9ade5186432ddbc2), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "11r2669", "Monitor 1.1 r2669")
	ROMX_LOAD( "mon11_2669.bin",  0x0000, 0x1000, CRC(f3ee23c8) SHA1(3ac96935668f5e53799c90db5140393c2ef9ce36), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "22r2856", "Monitor 2.2 r2856")
	ROMX_LOAD( "mon22_2856.bin",  0x0000, 0x1000, CRC(5f5f688a) SHA1(3719eecc347e158dd027ea7aa8a068cdafc00d9b), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(5, "22br2856", "Monitor 2.2B r2856")
	ROMX_LOAD( "mon22b_2856.bin", 0x0000, 0x1000, CRC(a47b0308) SHA1(f215e51aa8df6aed99c10f3df6a3589cb9f63d46), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS(6, "23r2601", "Monitor 2.3 r2601")
	ROMX_LOAD( "mon23_2601.bin",  0x0000, 0x1000, CRC(0e024222) SHA1(9950cba08996931b9d5a3368b44c7309638b4e08), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS(7, "23ar2569", "Monitor 2.3A r2569")
	ROMX_LOAD( "mon23a_2569.bin", 0x0000, 0x1000, CRC(248e62c9) SHA1(adbde27e69b38b29ff89bacf28d0240a8e5d90f3), ROM_BIOS(7) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "6571.bin", 0x0000, 0x0800, CRC(5a25144b) SHA1(7b9fee0c8ef2605b85d12b6d9fe8feb82418c63a) )
ROM_END

#define rom_pegasusm rom_pegasus

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT    CLASS          INIT          COMPANY      FULLNAME                                  FLAGS
COMP( 1981, pegasus,  0,       0,      pegasus,  pegasus, pegasus_state, init_pegasus, "Technosys", "Aamber Pegasus",                         MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1981, pegasusm, pegasus, 0,      pegasusm, pegasus, pegasus_state, init_pegasus, "Technosys", "Aamber Pegasus with RAM expansion unit", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
