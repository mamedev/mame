// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Irisha driver by Miodrag Milanovic

        2008-03-27 Preliminary driver.

        Jump addresses:
        Option 1: 0800 (Monitor - the only choice that works)
        Option 2: 046E
        Option 3: 0423 (then jumps to 4000)
        Option 4: 0501
        Option 5: 042E

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/i8251.h"
#include "sound/speaker.h"

class irisha_state : public driver_device
{
public:
	irisha_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_p_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_pit(*this, "pit8253"),
		m_speaker(*this, "speaker"),
		m_uart(*this, "uart")
	{
	}

	DECLARE_READ8_MEMBER(irisha_keyboard_r);
	DECLARE_READ8_MEMBER(irisha_8255_portb_r);
	DECLARE_READ8_MEMBER(irisha_8255_portc_r);
	DECLARE_WRITE8_MEMBER(irisha_8255_porta_w);
	DECLARE_WRITE8_MEMBER(irisha_8255_portb_w);
	DECLARE_WRITE8_MEMBER(irisha_8255_portc_w);
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	DECLARE_WRITE_LINE_MEMBER(write_uart_clock);
	TIMER_CALLBACK_MEMBER(irisha_key);
	DECLARE_WRITE_LINE_MEMBER(irisha_pic_set_int_line);
	UINT32 screen_update_irisha(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_shared_ptr<UINT8> m_p_videoram;

private:
	bool m_sg1_line;
	bool m_keypressed;
	UINT8 m_keyboard_cnt;
	UINT8 m_ppi_porta;
	UINT8 m_ppi_portc;
	void update_speaker();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	ioport_port *m_io_ports[10];
	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<speaker_sound_device> m_speaker;
	required_device<i8251_device> m_uart;
};


/* Address maps */
static ADDRESS_MAP_START(irisha_mem, AS_PROGRAM, 8, irisha_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM  // ROM
	AM_RANGE(0x4000, 0xdfff) AM_RAM  // RAM
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( irisha_io , AS_IO, 8, irisha_state )
	AM_RANGE(0x04, 0x05) AM_READ(irisha_keyboard_r)
	AM_RANGE(0x06, 0x06) AM_DEVREADWRITE("uart",i8251_device, data_r, data_w)
	AM_RANGE(0x07, 0x07) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x08, 0x0B) AM_DEVREADWRITE("pit8253", pit8253_device, read, write )
	AM_RANGE(0x0C, 0x0F) AM_DEVREADWRITE("pic8259", pic8259_device, read, write ) AM_MASK( 0x01 )
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( irisha )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('\xA6')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('_')

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK)

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Back") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD) //
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD)

	PORT_START("LINE9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
INPUT_PORTS_END

/*************************************************

    Video

*************************************************/

UINT32 irisha_state::screen_update_irisha(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 gfx;
	UINT16 y,ma=0,x;
	UINT16 *p;

	for (y = 0; y < 200; y++)
	{
		p = &bitmap.pix16(y);

		for (x = ma; x < ma+40; x++)
		{
			gfx = m_p_videoram[x];

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
		ma+=40;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout irisha_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( irisha )
	GFXDECODE_ENTRY( "maincpu", 0x3800, irisha_charlayout, 0, 1 )
GFXDECODE_END


/*************************************************

    i8255

*************************************************/

READ8_MEMBER(irisha_state::irisha_8255_portb_r)
{
	if (m_keypressed==1)
	{
		m_keypressed = 0;
		return 0x80;
	}

	return 0x00;
}

READ8_MEMBER(irisha_state::irisha_8255_portc_r)
{
	logerror("irisha_8255_portc_r\n");
	return 0;
}

WRITE8_MEMBER(irisha_state::irisha_8255_porta_w)
{
	logerror("irisha_8255_porta_w %02x\n",data);

	m_ppi_porta = data;

	update_speaker();
}

WRITE8_MEMBER(irisha_state::irisha_8255_portb_w)
{
	logerror("irisha_8255_portb_w %02x\n",data);
}

WRITE8_MEMBER(irisha_state::irisha_8255_portc_w)
{
	//logerror("irisha_8255_portc_w %02x\n",data);

	if BIT(data, 6)
		m_pit->write_gate2((BIT(m_ppi_porta, 5) && !BIT(data, 5)) ? 1 : 0);

	m_ppi_portc = data;

	update_speaker();
}

/*************************************************

    i8259

*************************************************/

WRITE_LINE_MEMBER(irisha_state::irisha_pic_set_int_line)
{
	m_maincpu->set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}


/*************************************************

    Sound

*************************************************/

void irisha_state::update_speaker()
{
	int level = ( (BIT(m_ppi_portc, 5)) || (BIT(m_ppi_porta, 4)) || !m_sg1_line) ? 1 : 0;

	m_speaker->level_w(level);
}


WRITE_LINE_MEMBER(irisha_state::speaker_w)
{
	m_sg1_line = state;
	update_speaker();
}


/*************************************************

    Keyboard

*************************************************/

TIMER_CALLBACK_MEMBER(irisha_state::irisha_key)
{
	m_keypressed = 1;
	m_keyboard_cnt = 0;
}

READ8_MEMBER(irisha_state::irisha_keyboard_r)
{
	UINT8 keycode;

	if (m_keyboard_cnt!=0 && m_keyboard_cnt<11)
		keycode = m_io_ports[m_keyboard_cnt-1]->read() ^ 0xff;
	else
		keycode = 0xff;

	m_keyboard_cnt++;

	return keycode;
}

WRITE_LINE_MEMBER(irisha_state::write_uart_clock)
{
	m_uart->write_txc(state);
	m_uart->write_rxc(state);
}


/*************************************************

    Machine

*************************************************/

void irisha_state::machine_start()
{
	static const char *const keynames[] = {
		"LINE0", "LINE1", "LINE2", "LINE3", "LINE4",
		"LINE5", "LINE6", "LINE7", "LINE8", "LINE9"
	};

	for ( UINT8 i = 0; i < 10; i++ )
		m_io_ports[i] = ioport( keynames[i] );

	machine().scheduler().timer_pulse(attotime::from_msec(30), timer_expired_delegate(FUNC(irisha_state::irisha_key),this));
}

void irisha_state::machine_reset()
{
	m_sg1_line = 0;
	m_keypressed = 0;
	m_keyboard_cnt = 0;
	m_ppi_porta = 0;
	m_ppi_portc = 0;
}

/* Machine driver */
static MACHINE_CONFIG_START( irisha, irisha_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, XTAL_16MHz / 9)
	MCFG_CPU_PROGRAM_MAP(irisha_mem)
	MCFG_CPU_IO_MAP(irisha_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(irisha_state, screen_update_irisha)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", irisha)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	MCFG_DEVICE_ADD("uart", I8251, 0)

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_16MHz / 9)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL_16MHz / 9 / 8 / 8)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(irisha_state, write_uart_clock))
	MCFG_PIT8253_CLK2(XTAL_16MHz / 9)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(irisha_state, speaker_w))

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(irisha_state, irisha_8255_porta_w))
	MCFG_I8255_IN_PORTB_CB(READ8(irisha_state, irisha_8255_portb_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(irisha_state, irisha_8255_portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(irisha_state, irisha_8255_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(irisha_state, irisha_8255_portc_w))

	MCFG_PIC8259_ADD( "pic8259", WRITELINE(irisha_state,irisha_pic_set_int_line), VCC, NULL )
MACHINE_CONFIG_END

/* ROM definition */

ROM_START( irisha )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ir_bootm.bin", 0x0000, 0x2000, CRC(7f9f4f0e) SHA1(05f97e1a1d7a15f4451129dba6c0bddc87ea748e))
	ROM_LOAD( "ir_conou.bin", 0x2000, 0x2000, CRC(bf92beed) SHA1(696c482ba53bc6261db11061ecc7141c67f1d820))
ROM_END

/* Driver */
/*    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT   CLASS           INIT    COMPANY  FULLNAME   FLAGS */
COMP( 1983, irisha,      0,      0, irisha,     irisha, driver_device,  0,      "MGU",   "Irisha",  MACHINE_NOT_WORKING)
