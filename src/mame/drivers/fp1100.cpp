// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Casio FP-1100

    Info found at various sites:

    Casio FP1000 and FP1100 are "pre-PC" personal computers, with Cassette,
    Floppy Disk, Printer and 2 cart/expansion slots. They had 32K ROM, 64K
    main RAM, 80x25 text display, 320x200, 640x200, 640x400 graphics display.
    Floppy disk is 2x 5 1/4.

    The FP1000 had 16K videoram and monochrome only. The monitor had a switch
    to invert the display (swap foreground and background colours).

    The FP1100 had 48K videoram and 8 colours.

    Processors: Z80 @ 4MHz, uPD7801G @ 2MHz

    Came with Basic built in, and you could run CP/M 2.2 from the floppy disk.

    The keyboard is a separate unit. It contains a beeper.

    TODO:
    - irq sources and communications;
    - unimplemented instruction PER triggered (can be ignored)
    - Most of this code is guesswork, because although schematics exist,
      they are too blurry to read.
    - Display can be interlaced or non-interlaced. Interlaced not emulated.
    - Cassette Load is quite complex, using 6 pins of the sub-cpu. Not done.
    - subcpu is supposed to be in WAIT except in horizontal blanking period.
      WAIT is not emulated in our cpu.
    - Keyboard not working.
    - FDC not done.


    To get it to boot while in debugger:
    - focus 1
    - g
    - after a second it will get stuck in a loop at fc2-fc4
    - press enter until it is at fc4
    - pc++
    - g
    Now it will start. You will most likely get an error message, although
    sometimes you'll get the Casio Basic title. Cursor will be flashing.
    - The message will most likely be blue, but it should be white.
    - Sometimes it will be in inverse video, and sometimes not.
    - Sometimes it will beep, and sometimes not.
    - The interrupt timing is quite critical.


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/upd7810/upd7810.h"
#include "video/mc6845.h"
#include "sound/beep.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"

#define MAIN_CLOCK 15974400
#define LOG 0

class fp1100_state : public driver_device
{
public:
	fp1100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_crtc(*this, "crtc")
		, m_p_videoram(*this, "videoram")
		, m_keyboard(*this, "KEY")
		, m_beep(*this, "beeper")
		, m_centronics(*this, "centronics")
		, m_cass(*this, "cassette")
	{ }

	DECLARE_WRITE8_MEMBER(main_bank_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(main_to_sub_w);
	DECLARE_READ8_MEMBER(sub_to_main_r);
	DECLARE_WRITE8_MEMBER(slot_bank_w);
	DECLARE_READ8_MEMBER(slot_id_r);
	DECLARE_READ8_MEMBER(main_to_sub_r);
	DECLARE_WRITE8_MEMBER(sub_to_main_w);
	DECLARE_WRITE8_MEMBER(colour_control_w);
	DECLARE_WRITE8_MEMBER(kbd_row_w);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_READ8_MEMBER(portc_r);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_WRITE_LINE_MEMBER(centronics_busy_w);
	DECLARE_WRITE_LINE_MEMBER(cass_w);
	INTERRUPT_GEN_MEMBER(fp1100_vblank_irq);
	DECLARE_DRIVER_INIT(fp1100);
	DECLARE_MACHINE_RESET(fp1100);
	MC6845_UPDATE_ROW(fp1100_update_row);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_c);
	required_device<palette_device> m_palette;
private:
	UINT8 m_irq_mask;
	UINT8 m_main_latch;
	UINT8 m_sub_latch;
	UINT8 m_slot_num;
	UINT8 m_kbd_row;
	UINT8 m_col_border;
	UINT8 m_col_cursor;
	UINT8 m_col_display;
	UINT8 m_centronics_busy;
	UINT8 m_cass_data[4];
	bool m_cass_state;
	bool m_cassold;

	struct {
		UINT8 id;
	}m_slot[8];

	struct {
		UINT8 porta;
		UINT8 portb;
		UINT8 portc;
	}m_upd7801;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<mc6845_device> m_crtc;
	required_shared_ptr<UINT8> m_p_videoram;
	required_ioport_array<16> m_keyboard;
	required_device<beep_device> m_beep;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cass;
};

MC6845_UPDATE_ROW( fp1100_state::fp1100_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 r,g,b,col,i;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	if BIT(m_upd7801.porta, 4)
	{ // green screen
		for (x = 0; x < x_count; x++)
		{
			mem = (((ma+x)<<3) + ra) & 0x3fff;
			g = m_p_videoram[mem];
			for (i = 0; i < 8; i++)
			{
				col = BIT(g, i);
				if (x == cursor_x) col ^= 1;
				*p++ = palette[col<<1];
			}
		}
	}
	else
	{ // RGB screen
		for (x = 0; x < x_count; x++)
		{
			mem = (((ma+x)<<3) + ra) & 0x3fff;
			b = m_p_videoram[mem];
			r = m_p_videoram[mem+0x4000];
			g = m_p_videoram[mem+0x8000];
			for (i = 0; i < 8; i++)
			{
				col = BIT(r, i) + (BIT(g, i) << 1) + (BIT(b, i) << 2);
				if (x == cursor_x) col = m_col_cursor;
				*p++ = palette[col];
			}
		}
	}
}

/*
d0 - Package select
d1 - Bank select (at boot time)
other bits not used
*/
WRITE8_MEMBER( fp1100_state::main_bank_w )
{
	membank("bankr0")->set_entry( BIT(data,1)); //(1) RAM (0) ROM
	m_slot_num = (m_slot_num & 3) | ((data & 1) << 2); //??
}

WRITE8_MEMBER( fp1100_state::irq_mask_w )
{
	machine().scheduler().synchronize(); // force resync
	m_irq_mask = data;
	m_subcpu->set_input_line(UPD7810_INTF2, BIT(data, 7) ? HOLD_LINE : CLEAR_LINE);
	if (LOG) printf("%s: IRQmask=%X\n",machine().describe_context(),data);
}

WRITE8_MEMBER( fp1100_state::main_to_sub_w )
{
//  machine().scheduler().synchronize(); // force resync
//  m_subcpu->set_input_line(UPD7810_INTF2, ASSERT_LINE);
	m_sub_latch = data;
	if (LOG) printf("%s: From main:%X\n",machine().describe_context(),data);
}

READ8_MEMBER( fp1100_state::sub_to_main_r )
{
//  machine().scheduler().synchronize(); // force resync
//  m_maincpu->set_input_line(0, CLEAR_LINE);
	if (LOG) printf("%s: To main:%X\n",machine().describe_context(),m_main_latch);
	return m_main_latch;
}

WRITE8_MEMBER( fp1100_state::slot_bank_w )
{
	m_slot_num = (data & 3) | (m_slot_num & 4);
}

READ8_MEMBER( fp1100_state::slot_id_r )
{
	//return 0xff;
	return m_slot[m_slot_num & 7].id;
}

static ADDRESS_MAP_START(fp1100_map, AS_PROGRAM, 8, fp1100_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x8fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x9000, 0xffff) AM_RAM AM_REGION("wram", 0x9000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(fp1100_io, AS_IO, 8, fp1100_state )
	ADDRESS_MAP_UNMAP_HIGH
	//AM_RANGE(0x0000, 0xfeff) slot memory area
	AM_RANGE(0xff00, 0xff7f) AM_READWRITE(slot_id_r,slot_bank_w)
	AM_RANGE(0xff80, 0xffff) AM_READ(sub_to_main_r)
	AM_RANGE(0xff80, 0xff9f) AM_WRITE(irq_mask_w)
	AM_RANGE(0xffa0, 0xffbf) AM_WRITE(main_bank_w)
	AM_RANGE(0xffc0, 0xffff) AM_WRITE(main_to_sub_w)
ADDRESS_MAP_END

READ8_MEMBER( fp1100_state::main_to_sub_r )
{
//  machine().scheduler().synchronize(); // force resync
//  m_subcpu->set_input_line(UPD7810_INTF2, CLEAR_LINE);
	if (LOG) printf("%s: To sub:%X\n",machine().describe_context(),m_sub_latch);
	return m_sub_latch;
}

WRITE8_MEMBER( fp1100_state::sub_to_main_w )
{
//  machine().scheduler().synchronize(); // force resync
//  m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, 0xf0);
	m_main_latch = data;
	if (LOG) printf("%s: From sub:%X\n",machine().describe_context(),data);
}

/*
d0,1,2 - border colour (B,R,G)
d3     - not used
d4,5,6 - colour of cursor; or display area (B,R,G) (see d7)
d7     - 1=display area; 0=cursor
*/
WRITE8_MEMBER( fp1100_state::colour_control_w )
{
	data = BITSWAP8(data, 7, 4, 6, 5, 3, 0, 2, 1);  // change BRG to RGB

	m_col_border = data & 7;

	if BIT(data, 7)
		m_col_display = (data >> 4) & 7;
	else
		m_col_cursor = data >> 4;
}

/*
d0,1,2,3 - keyboard scan row
         - if 13, turn on shift-lock LED
         - if 14, turn on caps-lock LED
         - if 15, turn off both LEDs
d4       - Beeper
d5       - "3state buffer of key data line (1=open, 0=closed)"
d6,7     - not used
*/
WRITE8_MEMBER( fp1100_state::kbd_row_w )
{
	m_kbd_row = data & 15;
	m_beep->set_state(BIT(data, 4));
}

static ADDRESS_MAP_START(fp1100_slave_map, AS_PROGRAM, 8, fp1100_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION("sub_ipl",0x0000)
	AM_RANGE(0x2000, 0xdfff) AM_RAM AM_SHARE("videoram") //vram B/R/G
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x3fe) AM_DEVREADWRITE("crtc", mc6845_device, status_r,address_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x3fe) AM_DEVREADWRITE("crtc", mc6845_device, register_r,register_w)
	AM_RANGE(0xe400, 0xe7ff) AM_READ_PORT("DSW") AM_WRITE(kbd_row_w)
	AM_RANGE(0xe800, 0xebff) AM_READWRITE(main_to_sub_r,sub_to_main_w)
	//AM_RANGE(0xec00, 0xefff) "Acknowledge of INT0" doesn't seem to be used
	AM_RANGE(0xf000, 0xf3ff) AM_WRITE(colour_control_w)
	AM_RANGE(0xf400, 0xff7f) AM_ROM AM_REGION("sub_ipl",0x2400)
	AM_RANGE(0xff80, 0xffff) AM_RAM     /* upd7801 internal RAM */
ADDRESS_MAP_END

/*
d0,1,2 - enable RGB guns (G,R,B)
d3     - CRTC clock (80 or 40 cols)
d4     - RGB (0) or Green (1)
d5     - clear videoram
d6     - CMT baud rate (1=300; 0=1200)
d7     - CMT load clock
The SO pin is Serial Output to CMT (1=2400Hz; 0=1200Hz)
*/
WRITE8_MEMBER( fp1100_state::porta_w )
{
	m_upd7801.porta = data;

	if BIT(data, 5)
		memset(m_p_videoram, 0, 0xc000);
}

READ8_MEMBER( fp1100_state::portb_r )
{
	UINT8 data = m_keyboard[m_kbd_row]->read() ^ 0xff;
	//m_subcpu->set_input_line(UPD7810_INTF0, BIT(data, 7) ? HOLD_LINE : CLEAR_LINE);
	return data;
}

/*
d0 - Centronics busy
d1 - Centronics error
d2 - CMT load input clock
d7 - CMT load serial data
*/
READ8_MEMBER( fp1100_state::portc_r )
{
	return (m_upd7801.portc & 0x78) | m_centronics_busy;
}

/*
d3 - cause INT on main cpu
d4 - Centronics port is used for input or output
d5 - CMT relay
d6 - Centronics strobe
*/
WRITE8_MEMBER( fp1100_state::portc_w )
{
	if BIT(m_irq_mask, 4)
		m_maincpu->set_input_line_and_vector(0, BIT(data, 3) ? CLEAR_LINE : HOLD_LINE, 0xf0);
	if (LOG) printf("%s: PortC:%X\n",machine().describe_context(),data);
	m_upd7801.portc = data;
	m_cass->change_state(BIT(data, 5) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	m_centronics->write_strobe(BIT(data, 6));
}

static ADDRESS_MAP_START(fp1100_slave_io, AS_IO, 8, fp1100_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_WRITE(porta_w)
	AM_RANGE(0x01, 0x01) AM_READ(portb_r) AM_DEVWRITE("cent_data_out", output_latch_device, write)
	AM_RANGE(0x02, 0x02) AM_READWRITE(portc_r,portc_w)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( fp1100 )
	PORT_START("KEY.0")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break")
	PORT_BIT(0x70, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_NAME("Caps")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Graph")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Ctrl")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Shift") PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("KEY.2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_NAME("PF0")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter") PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("*")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_NAME("Z") PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_NAME("Q") PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP-")    PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_NAME("Esc") PORT_CHAR(27)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_NAME("A") PORT_CHAR('A') PORT_CHAR('a')

	PORT_START("KEY.3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("PF1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_NAME(",")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("/")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_NAME("X") PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_NAME("W") PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP+")    PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_NAME("1") PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_NAME("S") PORT_CHAR('S') PORT_CHAR('s')

	PORT_START("KEY.4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("PF2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME(".")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Del")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_NAME("C") PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_NAME("E") PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP3")    PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_NAME("2") PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_NAME("D") PORT_CHAR('D') PORT_CHAR('d')

	PORT_START("KEY.5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_NAME("PF3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("000")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_NAME("V") PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_NAME("R") PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP6")    PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_NAME("3") PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_NAME("F") PORT_CHAR('F') PORT_CHAR('f')

	PORT_START("KEY.6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("PF4")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Space") PORT_CHAR(32)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Ins")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_NAME("B") PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_NAME("T") PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP9")    PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_NAME("4") PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_NAME("G") PORT_CHAR('G') PORT_CHAR('g')

	PORT_START("KEY.7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("PF5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP0")    PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_NAME("N") PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_NAME("Y") PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP8")    PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_NAME("5") PORT_CHAR('5') PORT_CHAR('^')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_NAME("H") PORT_CHAR('H') PORT_CHAR('h')

	PORT_START("KEY.8")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_NAME("PF6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP2")    PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_NAME("M") PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_NAME("U") PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP5")    PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_NAME("6") PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_NAME("J") PORT_CHAR('J') PORT_CHAR('j')

	PORT_START("KEY.9")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_NAME("PF7")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP1")    PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Home")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_NAME(",") PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_NAME("I") PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP4")    PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_NAME("7") PORT_CHAR('7') PORT_CHAR('%')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_NAME("K") PORT_CHAR('K') PORT_CHAR('k')

	PORT_START("KEY.10")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_NAME("PF8")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("]") PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_NAME(".") PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_NAME("O") PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KP7")    PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_NAME("8") PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_NAME("L") PORT_CHAR('L') PORT_CHAR('l')

	PORT_START("KEY.11")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_NAME("PF9")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("[") PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Rubout") PORT_CHAR(8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("/") PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_NAME("P") PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Return") PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_NAME("9") PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_NAME(";") PORT_CHAR(';') PORT_CHAR('+')

	PORT_START("KEY.12")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stop/Cont")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("-") PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Yen") PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(c)")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("|")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("^") PORT_CHAR('^') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_NAME("0") PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME(":") PORT_CHAR(':') PORT_CHAR('*')

	PORT_START("KEY.13")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.14")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.15")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Text width" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "40 chars/line" )
	PORT_DIPSETTING(    0x01, "80 chars/line" )
	PORT_DIPNAME( 0x02, 0x02, "Screen Mode" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "Screen 0" )
	PORT_DIPSETTING(    0x02, "Screen 1" )
	PORT_DIPNAME( 0x04, 0x04, "FP Mode" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "FP-1000" ) // Green screen
	PORT_DIPSETTING(    0x04, "FP-1100" ) // RGB
	PORT_DIPNAME( 0x08, 0x08, "CMT Baud Rate" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "1200 Baud" )
	PORT_DIPSETTING(    0x08, "300 Baud" )
	PORT_DIPNAME( 0x10, 0x10, "Printer Type" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "<undefined>" )
	PORT_DIPSETTING(    0x10, "FP-1012PR" )
	PORT_DIPNAME( 0x20, 0x20, "Keyboard Type" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "<undefined>" )
	PORT_DIPSETTING(    0x20, "Normal" )
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SLOTS")
	PORT_CONFNAME( 0x0003, 0x0002, "Slot #0" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0001, "ROM" )
	PORT_CONFSETTING(    0x0002, "RAM" )
	PORT_CONFSETTING(    0x0003, "FDC" )
	PORT_CONFNAME( 0x000c, 0x0008, "Slot #1" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0004, "ROM" )
	PORT_CONFSETTING(    0x0008, "RAM" )
	PORT_CONFSETTING(    0x000c, "FDC" )
	PORT_CONFNAME( 0x0030, 0x0020, "Slot #2" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0010, "ROM" )
	PORT_CONFSETTING(    0x0020, "RAM" )
	PORT_CONFSETTING(    0x0030, "FDC" )
	PORT_CONFNAME( 0x00c0, 0x0080, "Slot #3" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0040, "ROM" )
	PORT_CONFSETTING(    0x0080, "RAM" )
	PORT_CONFSETTING(    0x00c0, "FDC" )
	PORT_CONFNAME( 0x0300, 0x0200, "Slot #4" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0100, "ROM" )
	PORT_CONFSETTING(    0x0200, "RAM" )
	PORT_CONFSETTING(    0x0300, "FDC" )
	PORT_CONFNAME( 0x0c00, 0x0800, "Slot #5" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0400, "ROM" )
	PORT_CONFSETTING(    0x0800, "RAM" )
	PORT_CONFSETTING(    0x0c00, "FDC" )
	PORT_CONFNAME( 0x3000, 0x2000, "Slot #6" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x1000, "ROM" )
	PORT_CONFSETTING(    0x2000, "RAM" )
	PORT_CONFSETTING(    0x3000, "FDC" )
	PORT_CONFNAME( 0xc000, 0x8000, "Slot #7" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x4000, "ROM" )
	PORT_CONFSETTING(    0x8000, "RAM" )
	PORT_CONFSETTING(    0xc000, "FDC" )
INPUT_PORTS_END


static const gfx_layout fp1100_chars_8x8 =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( fp1100 )
	GFXDECODE_ENTRY( "sub_ipl", 0x2400, fp1100_chars_8x8, 0, 1 )
GFXDECODE_END

WRITE_LINE_MEMBER( fp1100_state::centronics_busy_w )
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER( fp1100_state::cass_w )
{
	m_cass_state = state;
}

TIMER_DEVICE_CALLBACK_MEMBER( fp1100_state::timer_c )
{
	m_cass_data[3]++;

	if (m_cass_state != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cass_state;
	}

	if (m_cass_state)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

INTERRUPT_GEN_MEMBER(fp1100_state::fp1100_vblank_irq)
{
//  if BIT(m_irq_mask, 4)
//      m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xf8);
}

MACHINE_RESET_MEMBER( fp1100_state, fp1100 )
{
	int i;
	UINT8 slot_type;
	const UINT8 id_type[4] = { 0xff, 0x00, 0x01, 0x04};
	for(i=0;i<8;i++)
	{
		slot_type = (ioport("SLOTS")->read() >> i*2) & 3;
		m_slot[i].id = id_type[slot_type];
	}

	m_beep->set_frequency(950);    /* guess */
	m_beep->set_state(0);

	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram

	m_irq_mask = 0;
	m_main_latch = 0;
	m_sub_latch = 0;
	m_slot_num = 0;
	m_kbd_row = 0;
	m_col_border = 0;
	m_col_cursor = 0;
	m_col_display = 0;
	m_upd7801.porta = 0;
	m_upd7801.portb = 0;
	m_upd7801.portc = 0;
}

DRIVER_INIT_MEMBER( fp1100_state, fp1100 )
{
	UINT8 *main = memregion("ipl")->base();
	UINT8 *wram = memregion("wram")->base();

	membank("bankr0")->configure_entry(1, &wram[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x0000]);
	membank("bankw0")->configure_entry(0, &wram[0x0000]);
}

static MACHINE_CONFIG_START( fp1100, fp1100_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(fp1100_map)
	MCFG_CPU_IO_MAP(fp1100_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fp1100_state, fp1100_vblank_irq)

	MCFG_CPU_ADD( "sub", UPD7801, MAIN_CLOCK/4 )
	MCFG_CPU_PROGRAM_MAP( fp1100_slave_map )
	MCFG_CPU_IO_MAP( fp1100_slave_io )
	MCFG_UPD7810_TXD(WRITELINE(fp1100_state, cass_w))

	MCFG_MACHINE_RESET_OVERRIDE(fp1100_state, fp1100)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", h46505_device, screen_update)
	MCFG_PALETTE_ADD("palette", 8)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fp1100)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50) // inside the keyboard

	/* CRTC */
	MCFG_MC6845_ADD("crtc", H46505, "screen", MAIN_CLOCK/8)   /* hand tuned to get ~60 fps */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(fp1100_state, fp1100_update_row)

	/* Printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(fp1100_state, centronics_busy_w))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	/* Cassette */
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_c", fp1100_state, timer_c, attotime::from_hz(4800)) // cass write
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( fp1100 )
	ROM_REGION( 0x9000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "basic.rom", 0x0000, 0x9000, BAD_DUMP CRC(7c7dd17c) SHA1(985757b9c62abd17b0bd77db751d7782f2710ec3))

	ROM_REGION( 0x3000, "sub_ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "sub1.rom", 0x0000, 0x1000, CRC(8feda489) SHA1(917d5b398b9e7b9a6bfa5e2f88c5b99923c3c2a3))
	ROM_LOAD( "sub2.rom", 0x1000, 0x1000, CRC(359f007e) SHA1(0188d5a7b859075cb156ee55318611bd004128d7))
	ROM_LOAD( "sub3.rom", 0x2000, 0xf80, BAD_DUMP CRC(fb2b577a) SHA1(a9ae6b03e06ea2f5db30dfd51ebf5aede01d9672))

	ROM_REGION( 0x10000, "wram", ROMREGION_ERASE00 )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE     INPUT   CLASS            INIT    COMPANY    FULLNAME       FLAGS */
COMP( 1983, fp1100,  0,      0,       fp1100,     fp1100, fp1100_state,  fp1100,   "Casio",   "FP-1100", MACHINE_NOT_WORKING)
