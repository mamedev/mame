/*******************************************************************************************************

  coleco.c

  Driver file to handle emulation of the Colecovision.

  Marat Fayzullin (ColEm source)
  Marcel de Kogel (AdamEm source)
  Mike Balfour
  Ben Bruscella
  Sean Young

  NEWS:
    - Modified memory map, now it has only 1k of RAM mapped on 8k Slot
    - Modified I/O map, now it is handled as on a real ColecoVision:
        The I/O map is broken into 4 write and 4 read ports:
            80-9F (W) = Set both controllers to keypad mode
            80-9F (R) = Not Connected

            A0-BF (W) = Video Chip (TMS9928A), A0=0 -> Write Register 0 , A0=1 -> Write Register 1
            A0-BF (R) = Video Chip (TMS9928A), A0=0 -> Read Register 0 , A0=1 -> Read Register 1

            C0-DF (W) = Set both controllers to joystick mode
            C0-DF (R) = Not Connected

            E0-FF (W) = Sound Chip (SN76489A)
            E0-FF (R) = Read Controller data, A1=0 -> read controller 1, A1=1 -> read controller 2

    - Modified paddle handler, now it is handled as on a real ColecoVision
    - Added support for a Roller Controller (Trackball), enabled via category
    - Added support for two Super Action Controller, enabled via category

    EXTRA CONTROLLERS INFO:

    -Driving Controller (Expansion Module #2). It consist of a steering wheel and a gas pedal. Only one
    can be used on a real ColecoVision. The gas pedal is not analog, internally it is just a switch.
    On a real ColecoVision, when the Driving Controller is enabled, the controller 1 do not work because
    have been replaced by the Driving Controller, and controller 2 have to be used to start game, gear
    shift, etc.
    Driving Controller is just a spinner on controller 1 socket similar to the one on Roller Controller
    and Super Action Controllers so you can use Roller Controller or Super Action Controllers to play
    games requiring Driving Controller.

    -Roller Controller. Basically a trackball with four buttons (the two fire buttons from player 1 and
    the two fire buttons from player 2). Only one Roller Controller can be used on a real ColecoVision.
    Roller Controller is connected to both controller sockets and both controllers are conected to the Roller
    Controller, it uses the spinner pins of both sockets to generate the X and Y signals (X from controller 1
    and the Y from controller 2)

    -Super Action Controllers. It is a hand controller with a keypad, four buttons (the two from
    the player pad and two more), and a spinner. This was made primarily for two player sport games, but
    will work for every other ColecoVision game.

*******************************************************************************************************/

/*

    TODO:

    - Dina SG-1000 mode

*/

#include "includes/coleco.h"

/* Read/Write Handlers */

READ8_MEMBER( coleco_state::paddle_1_r )
{
	return m_joy_d7_state[0] | coleco_paddle_read(machine(), 0, m_joy_mode, m_joy_analog_state[0]);
}

READ8_MEMBER( coleco_state::paddle_2_r )
{
	return m_joy_d7_state[1] | coleco_paddle_read(machine(), 1, m_joy_mode, m_joy_analog_state[1]);
}

WRITE8_MEMBER( coleco_state::paddle_off_w )
{
	m_joy_mode = 0;
}

WRITE8_MEMBER( coleco_state::paddle_on_w )
{
	m_joy_mode = 1;
}

/* Memory Maps */

static ADDRESS_MAP_START( coleco_map, AS_PROGRAM, 8, coleco_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_MIRROR(0x1c00)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( coleco_io_map, AS_IO, 8, coleco_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x1f) AM_WRITE(paddle_off_w)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x1e) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0xa1, 0xa1) AM_MIRROR(0x1e) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x1f) AM_WRITE(paddle_on_w)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x1f) AM_DEVWRITE("sn76489a", sn76489a_new_device, write)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x1d) AM_READ(paddle_1_r)
	AM_RANGE(0xe2, 0xe2) AM_MIRROR(0x1d) AM_READ(paddle_2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( czz50_map, AS_PROGRAM, 8, coleco_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_MIRROR(0x1c00)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( czz50_io_map, AS_IO, 8, coleco_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x1f) AM_WRITE(paddle_off_w)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x1e) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0xa1, 0xa1) AM_MIRROR(0x1e) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x1f) AM_WRITE(paddle_on_w)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x1f) AM_DEVWRITE("sn76489a", sn76489a_new_device, write)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x1d) AM_READ(paddle_1_r)
	AM_RANGE(0xe2, 0xe2) AM_MIRROR(0x1d) AM_READ(paddle_2_r)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( czz50 )
	PORT_START("STD_KEYPAD1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("#") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('#')
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME(".") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('.')
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xb000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STD_JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xb0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STD_KEYPAD2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xb000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STD_JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xb0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Interrupts */

static WRITE_LINE_DEVICE_HANDLER(coleco_vdp_interrupt)
{
	coleco_state *drvstate = device->machine().driver_data<coleco_state>();

    // NMI on rising edge
	if (state && !drvstate->m_last_nmi_state)
		drvstate->m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);

	drvstate->m_last_nmi_state = state;
}

TIMER_CALLBACK_MEMBER(coleco_state::paddle_d7reset_callback)
{

	m_joy_d7_state[param] = 0;
	m_joy_analog_state[param] = 0;
}

TIMER_CALLBACK_MEMBER(coleco_state::paddle_irqreset_callback)
{

	m_joy_irq_state[param] = 0;

	if (!m_joy_irq_state[param ^ 1])
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(coleco_state::paddle_pulse_callback)
{

	if (m_joy_analog_reload[param])
	{
		m_joy_analog_state[param] = m_joy_analog_reload[param];

		// on movement, controller port d7 is set for a short period and an irq is fired on d7 rising edge
		m_joy_d7_state[param] = 0x80;
		m_joy_d7_timer[param]->adjust(attotime::from_usec(500), param); // TODO: measure duration

		// irq on rising edge, PULSE_LINE is not supported in this case, so clear it manually
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_joy_irq_timer[param]->adjust(attotime::from_usec(11), param); // TODO: measure duration
		m_joy_irq_state[param] = 1;

		// reload timer
		m_joy_pulse_timer[param]->adjust(m_joy_pulse_reload[param], param);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(coleco_state::paddle_update_callback)
{
	// arbitrary timer for reading analog controls
	coleco_scan_paddles(machine(), &m_joy_analog_reload[0], &m_joy_analog_reload[1]);

	for (int port = 0; port < 2; port++)
	{
		if (m_joy_analog_reload[port])
		{
			const int sensitivity = 500;
			int ipt = m_joy_analog_reload[port];
			if (ipt & 0x80) ipt = 0x100 - ipt;
			attotime freq = attotime::from_msec(sensitivity / ipt);

			// change pulse intervals relative to spinner/trackball speed
			m_joy_pulse_reload[port] = freq;
			m_joy_pulse_timer[port]->adjust(min(freq, m_joy_pulse_timer[port]->remaining()), port);
		}
	}
}

/* Machine Initialization */

static TMS9928A_INTERFACE(coleco_tms9928a_interface)
{
	"screen",
	0x4000,
	DEVCB_LINE(coleco_vdp_interrupt)
};


//-------------------------------------------------
//  sn76496_config psg_intf
//-------------------------------------------------

static const sn76496_config psg_intf =
{
    DEVCB_NULL
};


void coleco_state::machine_start()
{
	// init paddles
	for (int port = 0; port < 2; port++)
	{
		m_joy_pulse_timer[port] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(coleco_state::paddle_pulse_callback),this));
		m_joy_d7_timer[port] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(coleco_state::paddle_d7reset_callback),this));
		m_joy_irq_timer[port] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(coleco_state::paddle_irqreset_callback),this));

		m_joy_irq_state[port] = 0;
		m_joy_d7_state[port] = 0;
		m_joy_analog_state[port] = 0;
	}
}

void coleco_state::machine_reset()
{
	m_last_nmi_state = 0;

	m_maincpu->set_input_line_vector(INPUT_LINE_IRQ0, 0xff);

	memset(&machine().root_device().memregion(Z80_TAG)->base()[0x6000], 0xff, 0x400);	// initialize RAM
}

//static int coleco_cart_verify(const UINT8 *cartdata, size_t size)
//{
//  int retval = IMAGE_VERIFY_FAIL;
//
//  /* Verify the file is in Colecovision format */
//  if ((cartdata[0] == 0xAA) && (cartdata[1] == 0x55)) /* Production Cartridge */
//      retval = IMAGE_VERIFY_PASS;
//  if ((cartdata[0] == 0x55) && (cartdata[1] == 0xAA)) /* "Test" Cartridge. Some games use this method to skip ColecoVision title screen and delay */
//      retval = IMAGE_VERIFY_PASS;
//
//  return retval;
//}

static DEVICE_IMAGE_LOAD( czz50_cart )
{
	UINT8 *ptr = image.device().machine().root_device().memregion(Z80_TAG)->base() + 0x8000;
	UINT32 size;

	if (image.software_entry() == NULL)
	{
		size = image.length();
		if (image.fread(ptr, size) != size)
			return IMAGE_INIT_FAIL;
		return IMAGE_INIT_PASS;
	}
	else
	{
		memcpy(ptr, image.get_software_region("rom"), image.get_software_region_length("rom"));
		return IMAGE_INIT_PASS;
	}
}

/* Machine Drivers */

static MACHINE_CONFIG_START( coleco, coleco_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_7_15909MHz/2)	// 3.579545 MHz
	MCFG_CPU_PROGRAM_MAP(coleco_map)
	MCFG_CPU_IO_MAP(coleco_io_map)

	// video hardware
	MCFG_TMS9928A_ADD( "tms9928a", TMS9928A, coleco_tms9928a_interface )
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn76489a", SN76489A_NEW, XTAL_7_15909MHz/2)	/* 3.579545 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_CONFIG(psg_intf)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,col,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("coleco_cart")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","coleco")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("paddle_timer", coleco_state, paddle_update_callback, attotime::from_msec(20))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( czz50, coleco_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_7_15909MHz/2)	// ???
	MCFG_CPU_PROGRAM_MAP(czz50_map)
	MCFG_CPU_IO_MAP(czz50_io_map)

	// video hardware
	MCFG_TMS9928A_ADD( "tms9928a", TMS9928A, coleco_tms9928a_interface )
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn76489a", SN76489A_NEW, XTAL_7_15909MHz/2)	// ???
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_CONFIG(psg_intf)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,col,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(czz50_cart)
	MCFG_CARTSLOT_INTERFACE("coleco_cart")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","coleco")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("paddle_timer", coleco_state, paddle_update_callback, attotime::from_msec(20))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dina, czz50 )
	MCFG_DEVICE_REMOVE("tms9928a")
	MCFG_DEVICE_REMOVE("screen")

	MCFG_TMS9928A_ADD( "tms9928a", TMS9929A, coleco_tms9928a_interface )
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )
MACHINE_CONFIG_END

/* ROMs */

ROM_START (coleco)
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "coleco.rom", 0x0000, 0x2000, CRC(3aa93ef3) SHA1(45bedc4cbdeac66c7df59e9e599195c778d86a92) )
	ROM_CART_LOAD("cart", 0x8000, 0x8000, ROM_NOMIRROR | ROM_OPTIONAL)
ROM_END

ROM_START (colecoa)
	// differences to 0x3aa93ef3 modified characters, added a pad 2 related fix
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "colecoa.rom", 0x0000, 0x2000, CRC(39bb16fc) SHA1(99ba9be24ada3e86e5c17aeecb7a2d68c5edfe59) )
	ROM_CART_LOAD("cart", 0x8000, 0x8000, ROM_NOMIRROR | ROM_OPTIONAL)
ROM_END

ROM_START (colecob)
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "svi603.rom", 0x0000, 0x2000, CRC(19e91b82) SHA1(8a30abe5ffef810b0f99b86db38b1b3c9d259b78) )
	ROM_CART_LOAD("cart", 0x8000, 0x8000, ROM_NOMIRROR | ROM_OPTIONAL)
ROM_END

ROM_START( czz50 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "czz50.rom", 0x0000, 0x2000, CRC(4999abc6) SHA1(96aecec3712c94517103d894405bc98a7dafa440) )
	ROM_CONTINUE( 0x8000, 0x2000 )
ROM_END

#define rom_dina rom_czz50
#define rom_prsarcde rom_czz50


/* System Drivers */

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     INIT     COMPANY             FULLNAME                            FLAGS
CONS( 1982, coleco,   0,        0,      coleco,   coleco, driver_device,   0,       "Coleco",           "ColecoVision",                     0 )
CONS( 1982, colecoa,  coleco,   0,      coleco,   coleco, driver_device,   0,       "Coleco",           "ColecoVision (Thick Characters)",  0 )
CONS( 1983, colecob,  coleco,   0,      coleco,   coleco, driver_device,   0,       "Spectravideo",     "SVI-603 Coleco Game Adapter",      0 )
CONS( 1986, czz50,    0,   coleco,      czz50,	  czz50, driver_device,    0,       "Bit Corporation",  "Chuang Zao Zhe 50",                0 )
CONS( 1988, dina,     czz50,    0,      dina,	  czz50, driver_device,    0,       "Telegames",        "Dina",                             0 )
CONS( 1988, prsarcde, czz50,    0,      czz50,    czz50, driver_device,    0,       "Telegames",        "Personal Arcade",                  0 )
