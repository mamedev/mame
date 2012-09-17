/*

CASIO PV-2000

(preliminary work by anondumper)
Thanks for the loaner (Ianoid)

NOTE:
The PCB has printed names of components, not ICXY, etc
but: "hn613128pc64.bin"

SEE
http://hou4gong1.mo-blog.jp/.shared/image.html?/photos/uncategorized/pv_2000_k1.jpg
http://hou4gong1.mo-blog.jp/.shared/image.html?/photos/uncategorized/pv_2000_14.jpg
http://hou4gong1.mo-blog.jp/.shared/image.html?/photos/uncategorized/pv_2000_15.jpg

Keyboard inputs are partially supported. Keys missing from the input ports:
- GAME - no beep in basic - is this really a key?

Todo:
- Add joystick support
- Cassette support

Also See:
http://www2.odn.ne.jp/~haf09260/Pv2000/EnrPV.htm
For BIOS CRC confirmation
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"


class pv2000_state : public driver_device
{
public:
	pv2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_cass(*this, CASSETTE_TAG),
	m_last_state(0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	DECLARE_WRITE8_MEMBER(pv2000_cass_conf_w);
	DECLARE_WRITE8_MEMBER(pv2000_keys_w);
	DECLARE_READ8_MEMBER(pv2000_keys_hi_r);
	DECLARE_READ8_MEMBER(pv2000_keys_lo_r);
	DECLARE_READ8_MEMBER(pv2000_keys_mod_r);
	DECLARE_WRITE_LINE_MEMBER(pv2000_vdp_interrupt);
	DECLARE_READ8_MEMBER(cass_in);
	DECLARE_WRITE8_MEMBER(cass_out);
	bool m_last_state;
	UINT8 m_key_pressed;
	UINT8 m_keyb_column;
	UINT8 m_cass_conf;
	virtual void machine_start();
	virtual void machine_reset();
};


WRITE8_MEMBER( pv2000_state::pv2000_cass_conf_w )
{
	logerror( "%s: pv2000_cass_conf_w %02x\n", machine().describe_context(), data );

	m_cass_conf = data & 0x0f;

	if ( m_cass_conf & 0x01 )
		m_cass->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR );
	else
		m_cass->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR );
}


WRITE8_MEMBER( pv2000_state::pv2000_keys_w )
{
	logerror( "%s: pv2000_keys_w %02x\n", machine().describe_context(), data );

	m_keyb_column = data & 0x0f;

	machine().device("maincpu")->execute().set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}


READ8_MEMBER( pv2000_state::pv2000_keys_hi_r )
{
	UINT8 data = 0;
	char kbdrow[6];

	switch ( m_keyb_column )
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		sprintf(kbdrow,"IN%d",m_keyb_column);
		data = ioport( kbdrow )->read() >> 4;
	}

	return data;
}


READ8_MEMBER( pv2000_state::pv2000_keys_lo_r )
{
	UINT8 data = 0;
	char kbdrow[6];

	logerror("%s: pv2000_keys_r\n", machine().describe_context() );

	switch ( m_keyb_column )
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		sprintf(kbdrow,"IN%d",m_keyb_column);
		data = ioport( kbdrow )->read() & 0x0f;
	}

	return 0xf0 | data;
}


READ8_MEMBER( pv2000_state::pv2000_keys_mod_r )
{
	return 0xf0 | ioport( "MOD" )->read();
}

READ8_MEMBER( pv2000_state::cass_in )
{
    // from what i can tell,
    // 0 = data in
    // 1 = must be high
    // 2 = must be low
    // bits 1 & 2 are checked while reading and writing tapes
    // Press STOP key (F1) to cancel LOAD or SAVE

	return 2 | ((m_cass->input() > +0.03) ? 1 : 0);
}

WRITE8_MEMBER( pv2000_state::cass_out )
{
    // it outputs 8-bit values here which are not the bytes in the file
    // result is not readable

	m_cass->output( BIT(data, 0) ? -1.0 : +1.0);
}


/* Memory Maps */

static ADDRESS_MAP_START( pv2000_map, AS_PROGRAM, 8, pv2000_state )
	AM_RANGE(0x0000, 0x3FFF) AM_ROM

	AM_RANGE(0x4000, 0x4000) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0x4001, 0x4001) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)

	AM_RANGE(0x7000, 0x7FFF) AM_RAM
  //AM_RANGE(0x8000, 0xBFFF) ext ram?
	AM_RANGE(0xC000, 0xFFFF) AM_ROM  //cart
ADDRESS_MAP_END


static ADDRESS_MAP_START( pv2000_io_map, AS_IO, 8, pv2000_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)

	//theres also printer and tape I/O (TODO)
	AM_RANGE(0x00, 0x00) AM_WRITE(pv2000_cass_conf_w)

	//keyboard/joystick
	AM_RANGE(0x10, 0x10) AM_READ(pv2000_keys_hi_r)
	AM_RANGE(0x20, 0x20) AM_READWRITE(pv2000_keys_lo_r, pv2000_keys_w)

	//sn76489a
	AM_RANGE(0x40, 0x40) AM_READ(pv2000_keys_mod_r) AM_DEVWRITE("sn76489a", sn76489a_new_device, write)

	/* Cassette input. Gets hit a lot after a GLOAD command */
	AM_RANGE(0x60, 0x60) AM_READWRITE(cass_in,cass_out)
ADDRESS_MAP_END


static INPUT_PORTS_START( pv2000 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Hiragana")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Yen")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN4_1") /* Game?? DEL / MODE / STOP ??, no beep in basic, START in galaga */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN4_2") /* DEL / MODE / STOP ??, no beep in basic, SELECT in galaga */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CRSR Up+Left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CRSR Down+Left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CRSR Up+Right")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CRSR Down+Right")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN6_2") /* Unknown ??, no beep in basic */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN6_3") /* Unknown ??, no beep in basic */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN7_2") /* Unknown ??, no beep in basic */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN7_3") /* Unknown ??, no beep in basic */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')

	PORT_START("IN8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("Attack 0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_NAME("Attack 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN8_2") /* Unknown ?, no beep in basic */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN8_3") /* Unknown ?, no beep in basic */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mode")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Del")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN8_7") /* Unknown ?, no beeep in basic */

	PORT_START("IN9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_NAME("Stop")

	PORT_START("MOD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RALT) PORT_NAME("Color")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_NAME("Func")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Shift")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END


WRITE_LINE_MEMBER( pv2000_state::pv2000_vdp_interrupt )
{
    // only if it goes up
	if (state && !m_last_state)
		machine().device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);

	m_last_state = state;

	/* Check if irq triggering from keyboard presses is enabled */
	if ( m_keyb_column == 0x0f )
	{
		/* Check if a key is pressed */
		UINT8 key_pressed;

		key_pressed = ioport( "IN0" )->read()
			| ioport( "IN1" )->read()
			| ioport( "IN2" )->read()
			| ioport( "IN3" )->read()
			| ioport( "IN4" )->read()
			| ioport( "IN5" )->read()
			| ioport( "IN6" )->read()
			| ioport( "IN7" )->read()
			| ioport( "IN8" )->read();

		if ( key_pressed && m_key_pressed != key_pressed )
			machine().device("maincpu")->execute().set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);

		m_key_pressed = key_pressed;
	}
}



/* Machine Initialization */

static TMS9928A_INTERFACE(pv2000_tms9928a_interface)
{
	"screen",
	0x4000,
	DEVCB_DRIVER_LINE_MEMBER(pv2000_state, pv2000_vdp_interrupt)
};


/*************************************
 *
 *  Sound interface
 *
 *************************************/


//-------------------------------------------------
//  sn76496_config psg_intf
//-------------------------------------------------

static const sn76496_config psg_intf =
{
    DEVCB_NULL
};


void pv2000_state::machine_start()
{
}

void pv2000_state::machine_reset()
{

	m_last_state = 0;
	m_key_pressed = 0;
	m_keyb_column = 0;

	machine().device("maincpu")->execute().set_input_line_vector(INPUT_LINE_IRQ0, 0xff);
	memset(&memregion("maincpu")->base()[0x7000], 0xff, 0x1000);	// initialize RAM
}

static DEVICE_IMAGE_LOAD( pv2000_cart )
{
	UINT8 *cart = image.device().machine().root_device().memregion("maincpu")->base() + 0xC000;
	UINT32 size;

	if (image.software_entry() == NULL)
		size = image.length();
	else
		size = image.get_software_region_length("rom");

	if (size != 0x2000 && size != 0x4000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return IMAGE_INIT_FAIL;
	}

	if (image.software_entry() == NULL)
	{
		if (image.fread( cart, size) != size)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unable to fully read from file");
			return IMAGE_INIT_FAIL;
		}
	}
	else
		memcpy(cart, image.get_software_region("rom"), size);

	return IMAGE_INIT_PASS;
}

static const cassette_interface pv2000_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED),
	NULL,
	NULL
};


/* Machine Drivers */
static MACHINE_CONFIG_START( pv2000, pv2000_state )

	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, XTAL_7_15909MHz/2)	// 3.579545 MHz
	MCFG_CPU_PROGRAM_MAP(pv2000_map)
	MCFG_CPU_IO_MAP(pv2000_io_map)


	// video hardware
	MCFG_TMS9928A_ADD( "tms9928a", TMS9928A, pv2000_tms9928a_interface )
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn76489a", SN76489A_NEW, XTAL_7_15909MHz/2)	/* 3.579545 MHz */
	MCFG_SOUND_CONFIG(psg_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* cassette */
	MCFG_CASSETTE_ADD( CASSETTE_TAG, pv2000_cassette_interface )

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,col,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(pv2000_cart)
	MCFG_CARTSLOT_INTERFACE("pv2000_cart")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","pv2000")
MACHINE_CONFIG_END



/* ROMs */
ROM_START (pv2000)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hn613128pc64.bin", 0x0000, 0x4000, CRC(8f31f297) SHA1(94b5f54dd7bce321e377fdaaf592acd3870cf621) )
	ROM_CART_LOAD("cart", 0xC000, 0x4000, ROM_OPTIONAL)
ROM_END


/* System Drivers */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT     INIT COMPANY   FULLNAME    FLAGS
CONS( 1983, pv2000,  0,      0,      pv2000,  pv2000, driver_device,   0,   "Casio",  "PV-2000",  GAME_NOT_WORKING )
