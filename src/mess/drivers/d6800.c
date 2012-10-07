/*

    The Dream 6800 is a CHIP-8 computer roughly modelled on the Cosmac VIP.
    It was decribed in Electronics Australia magazine in 4 articles starting
    in May 1979. It has 1k of ROM and 1k of RAM. The video consists of 64x32
    pixels. The keyboard is a hexcode 4x4 matrix, plus a Function key.

    Designed by Michael Bauer, of the Division of Computing and Mathematics
    at Deakin University, Australia.

    NOTE that the display only updates after each 4 digits is entered, and
    you can't see what you type as you change bytes. This is by design.
    The speaker is supposed to bleep on each keystroke, but it only gets
    one pulse - which is almost inaudible.

    Function keys:
    FN 0 - Modify memory - firstly enter a 4-digit address, then 2-digit data
                    the address will increment by itself, enter the next byte.
                    FN by itself will step to the next address.

    FN 1 - Tape load

    FN 2 - Tape save. You must have entered the start address at 0002, and
           the end address+1 at 0004 (big-endian).

    FN 3 - Run. You must have entered the 4-digit go address first.

    All CHIP-8 programs load at 0x200 (max size 4k), and exec address
    is C000.

    Information and programs can be found at http://chip8.com/?page=78


    TODO:
    - Cassette

*/


#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "sound/beep.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "sound/wave.h"
#include "sound/dac.h"
#include "machine/6821pia.h"


class d6800_state : public driver_device
{
public:
	d6800_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_cass(*this, CASSETTE_TAG),
		  m_pia(*this, "pia"),
		  m_dac(*this, "dac"),
		  m_videoram(*this, "videoram") { }

	DECLARE_READ8_MEMBER( d6800_cassette_r );
	DECLARE_WRITE8_MEMBER( d6800_cassette_w );
	DECLARE_READ8_MEMBER( d6800_keyboard_r );
	DECLARE_WRITE8_MEMBER( d6800_keyboard_w );
	DECLARE_READ_LINE_MEMBER( d6800_fn_key_r );
	DECLARE_READ_LINE_MEMBER( d6800_keydown_r );
	DECLARE_READ_LINE_MEMBER( d6800_rtc_pulse );
	DECLARE_WRITE_LINE_MEMBER( d6800_screen_w );
	UINT8 m_rtc;
	bool m_screen_on;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<device_t> m_pia;
	required_device<dac_device> m_dac;
	required_shared_ptr<UINT8> m_videoram;
private:
	UINT8 m_kbd_s;
	UINT8 m_portb;
	virtual void machine_start();
	virtual void machine_reset();
public:
	UINT32 screen_update_d6800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(d6800_p);
};


/* Memory Maps */

static ADDRESS_MAP_START( d6800_map, AS_PROGRAM, 8, d6800_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x01ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x0200, 0x07ff) AM_RAM
	AM_RANGE(0x8010, 0x8013) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0xc000, 0xc3ff) AM_MIRROR(0x3c00) AM_ROM
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( d6800 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("Y0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("Y1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("Y2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("Y3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("SHIFT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FN") PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("VS")
	/* vblank */
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen")
INPUT_PORTS_END

/* Video */

UINT32 d6800_state::screen_update_d6800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 x,y,gfx=0;

	for (y = 0; y < 32; y++)
	{
		UINT16 *p = &bitmap.pix16(y);

		for (x = 0; x < 8; x++)
		{
			if (m_screen_on)
				gfx = m_videoram[ x | (y<<3)];

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
	return 0;
}

/* PIA6821 Interface */

TIMER_DEVICE_CALLBACK_MEMBER(d6800_state::d6800_p)
{
	m_rtc++;
	m_maincpu->set_input_line(M6800_IRQ_LINE, (m_rtc > 0xf8) ? ASSERT_LINE : CLEAR_LINE);
}


// not used
READ_LINE_MEMBER( d6800_state::d6800_rtc_pulse )
{
	return 1;
}

READ_LINE_MEMBER( d6800_state::d6800_keydown_r )
{
	UINT8 data = ioport("X0")->read()
	           & ioport("X1")->read()
	           & ioport("X2")->read()
	           & ioport("X3")->read();

	m_kbd_s = (data == 15) ? 0 : 1;

	return m_kbd_s;
}

READ_LINE_MEMBER( d6800_state::d6800_fn_key_r )
{
	return ioport("SHIFT")->read();
}

WRITE_LINE_MEMBER( d6800_state::d6800_screen_w )
{
	m_screen_on = state;
}

READ8_MEMBER( d6800_state::d6800_cassette_r )
{
	/*
    Cassette circuit consists of a 741 op-amp, a 74121 oneshot, and a 74LS74.
    When a pulse arrives, the oneshot is set. After a preset time, it triggers
    and the 74LS74 compares this pulse to the output of the 741. Therefore it
    knows if the tone is 1200 or 2400 Hz. Input to PIA is bit 7.
    */

	return ((m_cass->input() > 0.03) ? 1 : 0) | m_portb;
}

WRITE8_MEMBER( d6800_state::d6800_cassette_w )
{
	/*
    Cassette circuit consists of a 566 and a transistor. The 556 runs at 2400
    or 1200 Hz depending on the state of the transistor. This is controlled by
    bit 0 of the PIA. Bit 6 drives the speaker.
    */

	m_cass->output(BIT(data, 0) ? -1.0 : +1.0);

	m_dac->write_unsigned8(data);
	m_portb = data;
}

READ8_MEMBER( d6800_state::d6800_keyboard_r )
{
	/*
    This system reads the key matrix one way, then swaps the input and output
    lines around and reads it another way. This isolates the key that was pressed.
    */

	m_kbd_s++;

	if (m_kbd_s == 3)
		return 0x0f & ioport("X0")->read() & ioport("X1")->read() & ioport("X2")->read() & ioport("X3")->read();
	else
	if (m_kbd_s == 6)
		return 0xf0 & ioport("Y0")->read() & ioport("Y1")->read() & ioport("Y2")->read() & ioport("Y3")->read();
	else
		return 0xff;
}

WRITE8_MEMBER( d6800_state::d6800_keyboard_w )
{
	/*

        bit     description

        PA0     keyboard column 0
        PA1     keyboard column 1
        PA2     keyboard column 2
        PA3     keyboard column 3
        PA4     keyboard row 0
        PA5     keyboard row 1
        PA6     keyboard row 2
        PA7     keyboard row 3

    */

}

static const pia6821_interface d6800_mc6821_intf =
{
	DEVCB_DRIVER_MEMBER(d6800_state, d6800_keyboard_r),	/* port A input */
	DEVCB_DRIVER_MEMBER(d6800_state, d6800_cassette_r),	/* port B input */
	DEVCB_DRIVER_LINE_MEMBER(d6800_state, d6800_keydown_r),	/* CA1 input */
	DEVCB_DRIVER_LINE_MEMBER(d6800_state, d6800_rtc_pulse),	/* CB1 input */
	DEVCB_DRIVER_LINE_MEMBER(d6800_state, d6800_fn_key_r),	/* CA2 input */
	DEVCB_NULL,						/* CB2 input */
	DEVCB_DRIVER_MEMBER(d6800_state, d6800_keyboard_w),	/* port A output */
	DEVCB_DRIVER_MEMBER(d6800_state, d6800_cassette_w),	/* port B output */
	DEVCB_NULL,						/* CA2 output */
	DEVCB_DRIVER_LINE_MEMBER(d6800_state, d6800_screen_w),	/* CB2 output */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE),	/* IRQA output */
	DEVCB_CPU_INPUT_LINE("maincpu", M6800_IRQ_LINE)		/* IRQB output */
};

/* Machine Initialization */

void d6800_state::machine_start()
{
}

void d6800_state::machine_reset()
{
}

/* Machine Drivers */

static const cassette_interface d6800_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};

static QUICKLOAD_LOAD( d6800 )
{
	address_space &space = image.device().machine().device("maincpu")->memory().space(AS_PROGRAM);
	int i;
	int quick_addr = 0x0200;
	int exec_addr = 0xc000;
	int quick_length;
	UINT8 *quick_data;
	int read_;

	quick_length = image.length();
	quick_data = (UINT8*)malloc(quick_length);
	if (!quick_data)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot open file");
		image.message(" Cannot open file");
		return IMAGE_INIT_FAIL;
	}

	read_ = image.fread( quick_data, quick_length);
	if (read_ != quick_length)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Cannot read the file");
		image.message(" Cannot read the file");
		return IMAGE_INIT_FAIL;
	}

	for (i = 0; i < quick_length; i++)
		if ((quick_addr + i) < 0x800)
			space.write_byte(i + quick_addr, quick_data[i]);

	/* display a message about the loaded quickload */
	image.message(" Quickload: size=%04X : start=%04X : end=%04X : exec=%04X",quick_length,quick_addr,quick_addr+quick_length,exec_addr);

	// Start the quickload
	image.device().machine().device("maincpu")->state().set_pc(exec_addr);
	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( d6800, d6800_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6800, XTAL_4MHz/4)
	MCFG_CPU_PROGRAM_MAP(d6800_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(64, 32)
	MCFG_SCREEN_VISIBLE_AREA(0, 63, 0, 31)
	MCFG_SCREEN_UPDATE_DRIVER(d6800_state, screen_update_d6800)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_PIA6821_ADD("pia", d6800_mc6821_intf)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, d6800_cassette_interface)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("d6800_p", d6800_state, d6800_p, attotime::from_hz(40000))

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", d6800, "ch8", 1)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( d6800 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d6800.bin", 0xc000, 0x0400, CRC(3f97ca2e) SHA1(60f26e57a058262b30befceceab4363a5d65d877) )
ROM_END

/*    YEAR  NAME   PARENT  COMPAT  MACHINE   INPUT       INIT        COMPANY             FULLNAME      FLAGS */
COMP( 1979, d6800, 0,      0,      d6800,    d6800, driver_device,      0,   "Electronics Australia", "Dream 6800", GAME_NOT_WORKING )
