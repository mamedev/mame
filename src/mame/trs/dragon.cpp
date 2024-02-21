// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    Dragon Family

    Bug? dragon200e hangs when single-quote is typed.

Dragon Alpha code added 21-Oct-2004,
            Phill Harvey-Smith (afra@aurigae.demon.co.uk)

            Added AY-8912 and FDC code 30-Oct-2004.

Fixed Dragon Alpha NMI enable/disable, following circuit traces on a real machine.
    P.Harvey-Smith, 11-Aug-2005.

Re-implemented Alpha NMI enable/disable, using direct PIA reads, rather than
keeping track of it in a variable in the driver.
    P.Harvey-Smith, 25-Sep-2006.

Radically re-wrote memory emulation code for CoCo 1/2 & Dragon machines, the
new code emulates the memory mapping of the SAM, dependent on what size of
RAM chips it is programed to use, including proper mirroring of the RAM.

Replaced the kludged emulation of the cart line, with a timer based trigger
this is set to toggle at 1Hz, this seems to be good enough to trigger the
cartline, but is so slow in real terms that it should have very little
impact on the emulation speed.

Re-factored the code common to all machines, and separated the code different,
into callbacks/functions unique to the machines, in preparation for splitting
the code for individual machine types into separate files, I have preposed, that
the CoCo 1/2 should stay in coco.c, and that the coco3 and dragon specific code
should go into coco3.c and dragon.c which should (hopefully) make the code
easier to manage.
    P.Harvey-Smith, Dec 2006-Feb 2007

***************************************************************************/

#include "emu.h"
#include "dragon.h"

#include "bus/coco/cococart.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6809/hd6309.h"
#include "cpu/m6809/m6809.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"

#include "softlist_dev.h"

#include "formats/coco_cas.h"
#include "formats/dmk_dsk.h"
#include "formats/sdf_dsk.h"
#include "formats/vdk_dsk.h"



/***************************************************************************
  DRAGON32
***************************************************************************/

//-------------------------------------------------
//  pia1_pa_changed - called when PIA1 PA changes
//-------------------------------------------------

void dragon_state::pia1_pa_changed(uint8_t data)
{
	/* call inherited function */
	coco12_state::pia1_pa_changed(data);

	/* if strobe bit is high send data from pia0 port b to dragon parallel printer */
	if (data & 0x02)
	{
		uint8_t output = pia_0().b_output();
		m_printer->output(output);
	}
}


/***************************************************************************
  DRAGON64
***************************************************************************/

//-------------------------------------------------
//  device_start
//-------------------------------------------------

void dragon64_state::machine_start()
{
	dragon_state::machine_start();

	uint8_t *rom = memregion("maincpu")->base();
	m_rombank[0]->configure_entries(0, 2, &rom[0x0000], 0x8000);
	m_rombank[1]->configure_entries(0, 2, &rom[0x2000], 0x8000);
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void dragon64_state::machine_reset()
{
	dragon_state::machine_reset();

	m_rombank[0]->set_entry(0);
	m_rombank[1]->set_entry(0);
}


//-------------------------------------------------
//  pia1_pb_changed
//-------------------------------------------------

void dragon64_state::pia1_pb_changed(uint8_t data)
{
	dragon_state::pia1_pb_changed(data);

	uint8_t ddr = ~pia_1().port_b_z_mask();

	/* If bit 2 of the pia1 ddrb is 1 then this pin is an output so use it */
	/* to control the paging of the 32k and 64k basic roms */
	/* Otherwise it set as an input, with an EXTERNAL pull-up so it should */
	/* always be high (enabling 32k basic rom) */
	if (ddr & 0x04)
	{
		page_rom(data & 0x04 ? true : false);
	}
}


//-------------------------------------------------
//  page_rom - Controls rom paging in Dragon 64,
//  and Dragon Alpha.
//
//  On 64, switches between the two versions of the
//  basic rom mapped in at 0x8000
//
//  On the alpha switches between the
//  Boot/Diagnostic rom and the basic rom
//-------------------------------------------------

void dragon64_state::page_rom(bool romswitch)
{
	int bank = romswitch
		? 0    // This is the 32k mode basic(64)/boot rom(alpha)
		: 1;   // This is the 64k mode basic(64)/basic rom(alpha)
	m_rombank[0]->set_entry(bank);      // 0x8000-0x9FFF
	m_rombank[1]->set_entry(bank);      // 0xA000-0xBFFF
}


/***************************************************************************
  DRAGON200-E
***************************************************************************/

uint8_t dragon200e_state::sam_read(offs_t offset)
{
	uint8_t data = sam().display_read(offset);
	m_vdg->as_w(data & 0x80 ? ASSERT_LINE : CLEAR_LINE);
	m_vdg->intext_w(data & 0x80 ? CLEAR_LINE : ASSERT_LINE);
	m_vdg->inv_w(m_lk1->read() ? ASSERT_LINE : CLEAR_LINE);
	return data;
}

MC6847_GET_CHARROM_MEMBER(dragon200e_state::char_rom_r)
{
	uint16_t addr = (line << 8) | (BIT(pia_1().b_output(), 4) << 7) | ch;
	return m_char_rom->base()[addr & 0xfff];
}


/***************************************************************************
  DRAGON64 PLUS
***************************************************************************/

//-------------------------------------------------
//  d64plus_6845_disp_r
//
//  The status of the 6845 display is determined by bit 0 of the $FFE2
//  register as follows:-
//    1  Video display busy, ie. not blanking
//    0  Video display available, ie. blanking
//  The display is blanking during a horizontal or vertical retrace period.
//-------------------------------------------------

uint8_t d64plus_state::d64plus_6845_disp_r()
{
	return m_crtc->de_r() ? 0xff : 0xfe;
}


//-------------------------------------------------
//  d64plus_bank_w
//-------------------------------------------------

void d64plus_state::d64plus_bank_w(uint8_t data)
{
	switch (data & 0x06)
	{
	case 0:  // Standard Dragon 32 Dynamic bank
		m_pram_bank->set_entry(0);
		m_vram_bank->set_entry(0);
		break;
	case 2:  // First extra 32K bank (A)
		m_pram_bank->set_entry(1);
		m_vram_bank->set_entry(1);
		break;
	case 6:  // Second extra 32K bank (B)
		m_pram_bank->set_entry(2);
		m_vram_bank->set_entry(2);
		break;
	default:
		logerror("unknown bank register $FFE2 = %02x\n", data);
		break;
	}
	if (data & 0x01)
	{
		m_vram_bank->set_entry(3);  // Video RAM bank (C)
	}
}


MC6845_UPDATE_ROW(d64plus_state::crtc_update_row)
{
	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_video_ram[((ma + column) & 0x7ff)];
		uint16_t addr = (code << 4) | (ra & 0x0f);
		uint8_t data = m_char_rom->base()[addr & 0xfff];

		if (column == cursor_x)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			int x = (column * 8) + bit;
			bitmap.pix(y, x) = m_palette->pen(BIT(data, 7) && de);
			data <<= 1;
		}
	}
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void d64plus_state::machine_start()
{
	dragon64_state::machine_start();

	m_sam->space(0).install_readwrite_bank(0x0000, 0x7fff, m_pram_bank);
	m_sam->space(0).install_readwrite_bank(0x0000, 0x07ff, m_vram_bank);

	m_pram_bank->configure_entry(0, m_ram->pointer());
	m_pram_bank->configure_entries(1, 2, m_plus_ram, 0x8000);

	m_vram_bank->configure_entry(0, m_ram->pointer());
	m_vram_bank->configure_entries(1, 2, m_plus_ram, 0x8000);
	m_vram_bank->configure_entry(3, m_video_ram);

	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_readwrite_handler(0xffe0, 0xffe0, read8smo_delegate(*m_crtc, FUNC(mc6845_device::status_r)), write8smo_delegate(*m_crtc, FUNC(mc6845_device::address_w)));
	space.install_readwrite_handler(0xffe1, 0xffe1, read8smo_delegate(*m_crtc, FUNC(mc6845_device::register_r)), write8smo_delegate(*m_crtc, FUNC(mc6845_device::register_w)));
	space.install_readwrite_handler(0xffe2, 0xffe2, read8smo_delegate(*this, FUNC(d64plus_state::d64plus_6845_disp_r)), write8smo_delegate(*this, FUNC(d64plus_state::d64plus_bank_w)));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void d64plus_state::machine_reset()
{
	dragon64_state::machine_reset();

	m_pram_bank->set_entry(0);
	m_vram_bank->set_entry(0);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void dragon_state::dragon_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(m_sam, FUNC(sam6883_device::read), FUNC(sam6883_device::write));
}


void dragon64_state::d64_rom0(address_map &map)
{
	// $8000-$9FFF
	map(0x0000, 0x1fff).bankr("rombank0");
}

void dragon64_state::d64_rom1(address_map &map)
{
	// $A000-$BFFF
	map(0x0000, 0x1fff).bankr("rombank1");
}

void dragon64_state::d64_io0(address_map &map)
{
	// $FF00-$FF1F
	map(0x00, 0x03).mirror(0x18).rw(m_pia_0, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x04, 0x07).mirror(0x18).rw(m_acia, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
}


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

/* Dragon keyboard

             PB0 PB1 PB2 PB3 PB4 PB5 PB6 PB7
    PA6: Ent Clr Brk N/c N/c N/c N/c Shift
    PA5: X   Y   Z   Up  Dwn Lft Rgt Space
    PA4: P   Q   R   S   T   U   V   W
    PA3: H   I   J   K   L   M   N   O
    PA2: @   A   B   C   D   E   F   G
    PA1: 8   9   :   ;   ,   -   .   /
    PA0: 0   1   2   3   4   5   6   7
 */
static INPUT_PORTS_START( dragon_keyboard )
	PORT_START("row0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("row1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("row3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("row4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("row5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME(u8"\u2191") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP), '^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME(u8"\u2193") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN), 10) PORT_CHAR('[')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME(u8"\u2190") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT), 8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME(u8"\u2192") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT), 9) PORT_CHAR(']')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("row6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x78, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END

static INPUT_PORTS_START( dragon200e_keyboard )
	PORT_INCLUDE(dragon_keyboard)

	PORT_MODIFY("row0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME(u8"0 \u00c7") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(0xC7)

	PORT_MODIFY("row1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME(u8"\u00d1") PORT_CODE(KEYCODE_COLON) PORT_CHAR(0xD1)

	PORT_MODIFY("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(';') PORT_CHAR('+')

	PORT_MODIFY("row5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME(u8"\u2191") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP), '^') PORT_CHAR('_')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME(u8"\u2193 \u00a1") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(10) PORT_CHAR(0xA1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME(u8"\u2192 \u00bf") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(9) PORT_CHAR(0xBF)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHAR(0xA7)

	PORT_MODIFY("row6")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME("CLEAR @") PORT_CODE(KEYCODE_HOME) PORT_CHAR(12) PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, dragon_state, keyboard_changed, 0) PORT_NAME(u8"BREAK \u00fc") PORT_CODE(KEYCODE_END) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27) PORT_CHAR(0xFC)
INPUT_PORTS_END

INPUT_PORTS_START( dragon )
	PORT_INCLUDE(dragon_keyboard)
	PORT_INCLUDE(coco_joystick)
	PORT_INCLUDE(coco_analog_control)
INPUT_PORTS_END

static INPUT_PORTS_START( dragon200e )
	PORT_INCLUDE(dragon200e_keyboard)
	PORT_INCLUDE(coco_joystick)
	PORT_INCLUDE(coco_analog_control)

	PORT_START("LK1")
	PORT_CONFNAME(0x01, 0x01, "Inverse Video")
	PORT_CONFSETTING(0x00, "Inverse")
	PORT_CONFSETTING(0x01, "Normal")
INPUT_PORTS_END

void dragon_state::dragon_cart(device_slot_interface &device)
{
	dragon_cart_add_basic_devices(device);
	dragon_cart_add_fdcs(device);
	dragon_cart_add_multi_pak(device);
}


// F4 Character Displayer
static const gfx_layout d64plus_charlayout =
{
	8, 12,                  // 8 x 12 characters
	256,                    // 256 characters
	1,                      // 1 bits per pixel
	{ 0 },                  // no bitplanes
	// x offsets
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	// y offsets
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8 * 16                  // every char takes 16 bytes
};

static GFXDECODE_START(gfx_d64plus)
	GFXDECODE_ENTRY("chargen", 0, d64plus_charlayout, 0, 1)
GFXDECODE_END


void dragon_state::dragon_base(machine_config &config)
{
	this->set_clock(14.218_MHz_XTAL / 16);

	// basic machine hardware
	MC6809E(config, m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &dragon_state::dragon_mem);

	// devices
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	INPUT_MERGER_ANY_HIGH(config, m_firqs).output_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);

	PIA6821(config, m_pia_0);
	m_pia_0->writepa_handler().set(FUNC(coco_state::pia0_pa_w));
	m_pia_0->writepb_handler().set(FUNC(coco_state::pia0_pb_w));
	m_pia_0->tspb_handler().set_constant(0xff);
	m_pia_0->ca2_handler().set(FUNC(coco_state::pia0_ca2_w));
	m_pia_0->cb2_handler().set(FUNC(coco_state::pia0_cb2_w));
	m_pia_0->irqa_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_pia_0->irqb_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	PIA6821(config, m_pia_1);
	m_pia_1->readpa_handler().set(FUNC(coco_state::pia1_pa_r));
	m_pia_1->readpb_handler().set(FUNC(coco_state::pia1_pb_r));
	m_pia_1->writepa_handler().set(FUNC(coco_state::pia1_pa_w));
	m_pia_1->writepb_handler().set(FUNC(coco_state::pia1_pb_w));
	m_pia_1->ca2_handler().set(FUNC(coco_state::pia1_ca2_w));
	m_pia_1->cb2_handler().set(FUNC(coco_state::pia1_cb2_w));
	m_pia_1->irqa_handler().set(m_firqs, FUNC(input_merger_device::in_w<0>));
	m_pia_1->irqb_handler().set(m_firqs, FUNC(input_merger_device::in_w<1>));

	SAM6883(config, m_sam, 14.218_MHz_XTAL, m_maincpu);
	m_sam->set_addrmap(0, &dragon_state::coco_ram);
	m_sam->set_addrmap(1, &dragon_state::coco_rom0);
	m_sam->set_addrmap(2, &dragon_state::coco_rom1);
	m_sam->set_addrmap(3, &dragon_state::coco_rom2);
	m_sam->set_addrmap(4, &dragon_state::coco_io0);
	m_sam->set_addrmap(5, &dragon_state::coco_io1);
	m_sam->set_addrmap(6, &dragon_state::coco_io2);
	m_sam->set_addrmap(7, &dragon_state::coco_ff60);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(coco_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_interface("dragon_cass");

	PRINTER(config, m_printer, 0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	MC6847_PAL(config, m_vdg, 14.218_MHz_XTAL / 4);
	m_vdg->set_screen(m_screen);
	m_vdg->hsync_wr_callback().set(FUNC(dragon_state::horizontal_sync));
	m_vdg->fsync_wr_callback().set(FUNC(dragon_state::field_sync));
	m_vdg->input_callback().set(FUNC(dragon_state::sam_read));

	// sound hardware
	coco_sound(config);

	// floating space
	coco_floating(config);

	// software lists
	SOFTWARE_LIST(config, "dragon_cart_list").set_original("dragon_cart");
	SOFTWARE_LIST(config, "dragon_cass_list").set_original("dragon_cass");
	SOFTWARE_LIST(config, "dragon_flop_list").set_original("dragon_flop");
	SOFTWARE_LIST(config, "coco_cart_list").set_compatible("coco_cart");
}

void dragon_state::dragon32(machine_config &config)
{
	dragon_base(config);

	// internal ram
	RAM(config, m_ram).set_default_size("32K").set_extra_options("64K");

	// cartridge
	COCOCART_SLOT(config, m_cococart, DERIVED_CLOCK(1, 1), &dragon_state::dragon_cart, "dragon_fdc");
	m_cococart->cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	m_cococart->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_cococart->halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
}

void dragon64_state::dragon64(machine_config &config)
{
	dragon_base(config);

	// internal ram
	RAM(config, m_ram).set_default_size("64K");

	sam().set_addrmap(1, &dragon64_state::d64_rom0);
	sam().set_addrmap(2, &dragon64_state::d64_rom1);
	sam().set_addrmap(4, &dragon64_state::d64_io0);

	// cartridge
	COCOCART_SLOT(config, m_cococart, DERIVED_CLOCK(1, 1), &dragon64_state::dragon_cart, "dragon_fdc");
	m_cococart->cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	m_cococart->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_cococart->halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);

	// acia
	MOS6551(config, m_acia, 0);
	m_acia->set_xtal(1.8432_MHz_XTAL);
	m_acia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));

	// software lists
	SOFTWARE_LIST(config, "dragon_flex_list").set_original("dragon_flex");
	SOFTWARE_LIST(config, "dragon_os9_list").set_original("dragon_os9");
}

void dragon64_state::dragon64h(machine_config &config)
{
	dragon64(config);
	// Replace M6809 with HD6309
	HD6309E(config.replace(), m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &dragon64_state::dragon_mem);
	m_ram->set_default_size("64K");
}

void dragon200e_state::dragon200e(machine_config &config)
{
	dragon64(config);
	// video hardware
	m_vdg->set_get_char_rom(FUNC(dragon200e_state::char_rom_r));
	m_vdg->input_callback().set(FUNC(dragon200e_state::sam_read));
}

void d64plus_state::d64plus(machine_config &config)
{
	dragon64(config);
	// video hardware
	screen_device &plus_screen(SCREEN(config, "plus_screen", SCREEN_TYPE_RASTER));
	plus_screen.set_raw(14.218_MHz_XTAL, 912, 0, 640, 316, 0, 264);
	plus_screen.set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_d64plus);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// crtc
	HD6845S(config, m_crtc, 14.218_MHz_XTAL / 4 / 2);
	m_crtc->set_screen("plus_screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(d64plus_state::crtc_update_row));
}

void dragon64_state::tanodr64(machine_config &config)
{
	dragon64(config);
	this->set_clock(14.318181_MHz_XTAL / 16);

	m_sam->set_clock(14.318181_MHz_XTAL);

	// video hardware
	MC6847_NTSC(config.replace(), m_vdg, 14.318181_MHz_XTAL / 4);
	m_vdg->set_screen(m_screen);
	m_vdg->hsync_wr_callback().set(FUNC(dragon_state::horizontal_sync));
	m_vdg->fsync_wr_callback().set(FUNC(dragon_state::field_sync));
	m_vdg->input_callback().set(FUNC(dragon_state::sam_read));

	// cartridge
	m_cococart->set_default_option("sdtandy_fdc");
}

void dragon64_state::tanodr64h(machine_config &config)
{
	tanodr64(config);
	// Replace M6809 CPU with HD6309 CPU
	HD6309E(config.replace(), m_maincpu, DERIVED_CLOCK(1, 1));
	m_maincpu->set_addrmap(AS_PROGRAM, &dragon64_state::dragon_mem);
	m_ram->set_default_size("64K");
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(dragon32)
	ROM_REGION(0xC000, "maincpu",0)
	ROM_LOAD("d32.rom",      0x0000,  0x4000, CRC(e3879310) SHA1(f2dab125673e653995a83bf6b793e3390ec7f65a))
ROM_END

ROM_START(dragon64)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("d64_1.rom",    0x0000,  0x4000, CRC(60a4634c) SHA1(f119506eaa3b4b70b9aa0dd83761e8cbe043d042))
	ROM_LOAD("d64_2.rom",    0x8000,  0x4000, CRC(17893a42) SHA1(e3c8986bb1d44269c4587b04f1ca27a70b0aaa2e))
ROM_END

ROM_START(dragon200)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ic18.rom",    0x0000, 0x4000, CRC(84f68bf9) SHA1(1983b4fb398e3dd9668d424c666c5a0b3f1e2b69))
	ROM_LOAD( "ic17.rom",    0x8000, 0x4000, CRC(17893a42) SHA1(e3c8986bb1d44269c4587b04f1ca27a70b0aaa2e))
ROM_END

ROM_START(dragon200e)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ic18_v1.4e.ic34",    0x0000, 0x4000, CRC(95af0a0a) SHA1(628543ee8b47a56df2b2175cfb763c0051517b90))
	ROM_LOAD( "ic17_v1.4e.ic37",    0x8000, 0x4000, CRC(48b985df) SHA1(c25632f3c2cfd1af3ee26b2f233a1ce1eccc365d))
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "rom26.ic1",          0x0000, 0x1000, CRC(565724bc) SHA1(da5b756ba2a9c9ecebaa7daa8ba8bfd984d56a6f))
ROM_END

ROM_START(d64plus)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("d64_1.rom",      0x0000, 0x4000, CRC(60a4634c) SHA1(f119506eaa3b4b70b9aa0dd83761e8cbe043d042))
	ROM_LOAD("d64_2.rom",      0x8000, 0x4000, CRC(17893a42) SHA1(e3c8986bb1d44269c4587b04f1ca27a70b0aaa2e))
	ROM_REGION(0x0200, "prom", 0)
	ROM_LOAD("n82s147an.ic12", 0x0000, 0x0200, CRC(92b6728d) SHA1(bcf7c60c4e5608a58587044458d9cacaca4568aa))
	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("chargen.ic22",   0x0000, 0x2000, CRC(514f1450) SHA1(956c99fcca1b52e79bb5d91dbafc817c992e324a))
ROM_END

ROM_START(tanodr64)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("d64_1.rom",    0x0000,  0x4000, CRC(60a4634c) SHA1(f119506eaa3b4b70b9aa0dd83761e8cbe043d042))
	ROM_LOAD("d64_2.rom",    0x8000,  0x4000, CRC(17893a42) SHA1(e3c8986bb1d44269c4587b04f1ca27a70b0aaa2e))
ROM_END

#define rom_dragon64h rom_dragon64
#define rom_tanodr64h rom_tanodr64

//    YEAR  NAME        PARENT    COMPAT  MACHINE     INPUT       CLASS               INIT        COMPANY                         FULLNAME                          FLAGS
COMP( 1982, dragon32,   0,        0,      dragon32,   dragon,     dragon_state,       empty_init, "Dragon Data Ltd",              "Dragon 32",                      0 )
COMP( 1983, dragon64,   dragon32, 0,      dragon64,   dragon,     dragon64_state,     empty_init, "Dragon Data Ltd",              "Dragon 64",                      0 )
COMP( 19??, dragon64h,  dragon32, 0,      dragon64h,  dragon,     dragon64_state,     empty_init, "Dragon Data Ltd",              "Dragon 64 (HD6309E CPU)",        MACHINE_UNOFFICIAL )
COMP( 1985, dragon200,  dragon32, 0,      dragon64,   dragon,     dragon64_state,     empty_init, "Eurohard S.A.",                "Dragon 200",                     0 )
COMP( 1985, dragon200e, dragon32, 0,      dragon200e, dragon200e, dragon200e_state,   empty_init, "Eurohard S.A.",                "Dragon 200-E",                   0 )
COMP( 1985, d64plus,    dragon32, 0,      d64plus,    dragon,     d64plus_state,      empty_init, "Dragon Data Ltd / Compusense", "Dragon 64 Plus",                 0 )
COMP( 1983, tanodr64,   dragon32, 0,      tanodr64,   dragon,     dragon64_state,     empty_init, "Dragon Data Ltd / Tano Ltd",   "Tano Dragon 64 (NTSC)",          0 )
COMP( 19??, tanodr64h,  dragon32, 0,      tanodr64h,  dragon,     dragon64_state,     empty_init, "Dragon Data Ltd / Tano Ltd",   "Tano Dragon 64 (NTSC; HD6309E)", MACHINE_UNOFFICIAL )
