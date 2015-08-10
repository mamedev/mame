// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Ben Bruscella, Sean Young
/*******************************************************************************************************

  coleco.c

  Driver file to handle emulation of the ColecoVision.

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
	return m_joy_d7_state[0] | coleco_paddle_read(0, m_joy_mode, m_joy_analog_state[0]);
}

READ8_MEMBER( coleco_state::paddle_2_r )
{
	return m_joy_d7_state[1] | coleco_paddle_read(1, m_joy_mode, m_joy_analog_state[1]);
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
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_MIRROR(0x1c00) AM_SHARE("ram")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( coleco_io_map, AS_IO, 8, coleco_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x1f) AM_WRITE(paddle_off_w)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x1e) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0xa1, 0xa1) AM_MIRROR(0x1e) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x1f) AM_WRITE(paddle_on_w)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x1f) AM_DEVWRITE("sn76489a", sn76489a_device, write)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x1d) AM_READ(paddle_1_r)
	AM_RANGE(0xe2, 0xe2) AM_MIRROR(0x1d) AM_READ(paddle_2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( czz50_map, AS_PROGRAM, 8, coleco_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_MIRROR(0x1c00) AM_SHARE("ram")
	AM_RANGE(0x8000, 0xffff) AM_ROM
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
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("*") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('*')
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

WRITE_LINE_MEMBER(coleco_state::coleco_vdp_interrupt)
{
	// NMI on rising edge
	if (state && !m_last_nmi_state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);

	m_last_nmi_state = state;
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
	coleco_scan_paddles(&m_joy_analog_reload[0], &m_joy_analog_reload[1]);

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

READ8_MEMBER( coleco_state::cart_r )
{
	return m_cart->bd_r(space, offset & 0x7fff, 0, 0, 0, 0, 0);
}

UINT8 coleco_state::coleco_scan_paddles(UINT8 *joy_status0, UINT8 *joy_status1)
{
	UINT8 ctrl_sel = (m_ctrlsel != NULL) ? m_ctrlsel->read() : 0;

	/* which controller shall we read? */
	if ((ctrl_sel & 0x07) == 0x02)          // Super Action Controller P1
		*joy_status0 = (m_sac_slide1 != NULL) ? m_sac_slide1->read() : 0;
	else if ((ctrl_sel & 0x07) == 0x03)     // Driving Controller P1
		*joy_status0 = (m_driv_wheel1 != NULL) ? m_driv_wheel1->read() : 0;

	if ((ctrl_sel & 0x70) == 0x20)          // Super Action Controller P2
		*joy_status1 = (m_sac_slide2 != NULL) ? m_sac_slide2->read() : 0;
	else if ((ctrl_sel & 0x70) == 0x30)     // Driving Controller P2
		*joy_status1 = (m_driv_wheel2 != NULL) ? m_driv_wheel2->read() : 0;

	/* In principle, even if not supported by any game, I guess we could have two Super
	   Action Controllers plugged into the Roller controller ports. Since I found no info
	   about the behavior of sliders in such a configuration, we overwrite SAC sliders with
	   the Roller trackball inputs and actually use the latter ones, when both are selected. */
	if (ctrl_sel & 0x80)                    // Roller controller
	{
		*joy_status0 = (m_roller_x != NULL) ? m_roller_x->read() : 0;
		*joy_status1 = (m_roller_y != NULL) ? m_roller_y->read() : 0;
	}

	return *joy_status0 | *joy_status1;
}


UINT8 coleco_state::coleco_paddle_read(int port, int joy_mode, UINT8 joy_status)
{
	UINT8 ctrl_sel = (m_ctrlsel != NULL ) ? m_ctrlsel->read() : 0;
	UINT8 ctrl_extra = ctrl_sel & 0x80;
	ctrl_sel = ctrl_sel >> (port*4) & 7;

	/* Keypad and fire 1 (SAC Yellow Button) */
	if (joy_mode == 0)
	{
		/* No key pressed by default */
		UINT8 data = 0x0f;
		UINT16 ipt = 0xffff;

		if (ctrl_sel == 0)          // ColecoVision Controller
			ipt = port ? m_std_keypad2->read() : m_std_keypad1->read();
		else if (ctrl_sel == 2)     // Super Action Controller
			ipt = port ? m_sac_keypad2->read() : m_sac_keypad1->read();

		/* Numeric pad buttons are not independent on a real ColecoVision, if you push more
		   than one, a real ColecoVision think that it is a third button, so we are going to emulate
		   the right behaviour */
		/* Super Action Controller additional buttons are read in the same way */
		if (!(ipt & 0x0001)) data &= 0x0a; /* 0 */
		if (!(ipt & 0x0002)) data &= 0x0d; /* 1 */
		if (!(ipt & 0x0004)) data &= 0x07; /* 2 */
		if (!(ipt & 0x0008)) data &= 0x0c; /* 3 */
		if (!(ipt & 0x0010)) data &= 0x02; /* 4 */
		if (!(ipt & 0x0020)) data &= 0x03; /* 5 */
		if (!(ipt & 0x0040)) data &= 0x0e; /* 6 */
		if (!(ipt & 0x0080)) data &= 0x05; /* 7 */
		if (!(ipt & 0x0100)) data &= 0x01; /* 8 */
		if (!(ipt & 0x0200)) data &= 0x0b; /* 9 */
		if (!(ipt & 0x0400)) data &= 0x06; /* # */
		if (!(ipt & 0x0800)) data &= 0x09; /* * */
		if (!(ipt & 0x1000)) data &= 0x04; /* Blue Action Button */
		if (!(ipt & 0x2000)) data &= 0x08; /* Purple Action Button */

		return ((ipt & 0x4000) >> 8) | 0x30 | data;
	}
	/* Joystick and fire 2 (SAC Red Button) */
	else
	{
		UINT8 data = 0x7f;

		if (ctrl_sel == 0)          // ColecoVision Controller
			data = port ? m_std_joy2->read() : m_std_joy1->read();
		else if (ctrl_sel == 2)     // Super Action Controller
			data = port ? m_sac_joy2->read() : m_sac_joy1->read();
		else if (ctrl_sel == 3)     // Driving Controller
			data = port ? m_driv_pedal2->read() : m_driv_pedal1->read();

		/* If any extra analog contoller enabled */
		if (ctrl_extra || ctrl_sel == 2 || ctrl_sel == 3)
		{
			if (joy_status & 0x80) data ^= 0x30;
			else if (joy_status) data ^= 0x10;
		}

		return data & 0x7f;
	}
}

void coleco_state::machine_start()
{
	memset(m_ram, 0xff, m_ram.bytes()); // initialize RAM

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

	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x8000, 0xffff, read8_delegate(FUNC(coleco_state::cart_r),this));

	save_item(NAME(m_joy_mode));
	save_item(NAME(m_last_nmi_state));
	save_item(NAME(m_joy_irq_state));
	save_item(NAME(m_joy_d7_state));
	save_item(NAME(m_joy_analog_state));
	save_item(NAME(m_joy_analog_reload));
}

void coleco_state::machine_reset()
{
	m_last_nmi_state = 0;
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


/* Machine Drivers */

static MACHINE_CONFIG_START( coleco, coleco_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_7_15909MHz/2) // 3.579545 MHz
	MCFG_CPU_PROGRAM_MAP(coleco_map)
	MCFG_CPU_IO_MAP(coleco_io_map)

	/* video hardware */
	MCFG_DEVICE_ADD( "tms9928a", TMS9928A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(coleco_state, coleco_vdp_interrupt))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn76489a", SN76489A, XTAL_7_15909MHz/2) // 3.579545 MHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* cartridge */
	MCFG_COLECOVISION_CARTRIDGE_SLOT_ADD(COLECOVISION_CARTRIDGE_SLOT_TAG, colecovision_cartridges, NULL)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","coleco")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("paddle_timer", coleco_state, paddle_update_callback, attotime::from_msec(20))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( colecop, coleco )

	/* video hardware */
	MCFG_DEVICE_REMOVE("tms9928a")
	MCFG_DEVICE_REMOVE("screen")

	MCFG_DEVICE_ADD( "tms9928a", TMS9929A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(coleco_state, coleco_vdp_interrupt))
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( czz50, coleco )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu") // note: cpu speed unverified, assume it's the same as ColecoVision
	MCFG_CPU_PROGRAM_MAP(czz50_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dina, czz50 )

	/* video hardware */
	MCFG_DEVICE_REMOVE("tms9928a")
	MCFG_DEVICE_REMOVE("screen")

	MCFG_DEVICE_ADD( "tms9928a", TMS9929A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(coleco_state, coleco_vdp_interrupt))
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )
MACHINE_CONFIG_END


/* ROMs */

ROM_START (coleco)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "original", "Original" )
	ROMX_LOAD( "313 10031-4005 73108a.u2", 0x0000, 0x2000, CRC(3aa93ef3) SHA1(45bedc4cbdeac66c7df59e9e599195c778d86a92), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "thick", "Thick characters" )
	// differences to 0x3aa93ef3 modified characters, added a pad 2 related fix
	ROMX_LOAD( "colecoa.rom", 0x0000, 0x2000, CRC(39bb16fc) SHA1(99ba9be24ada3e86e5c17aeecb7a2d68c5edfe59), ROM_BIOS(2) )
ROM_END

/*  ONYX (Prototype)
    Unreleased Brazilian Colecovision clone by Microdigital.

    It was never released and the only known prototypes were uncovered by an ex-employee of Microdigital
    called Cl??udio Cassens who donated it to collectors (Eduardo Luccas) in June 2015.
    -- Felipe Sanches
*/
ROM_START (onyx)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "onyx.rom", 0x0000, 0x2000, CRC(011c32e7) SHA1(f44263221e330b2590dffc1a6f43ed2591fe19be) )
ROM_END

/*  PAL Colecovision BIOS

Country: Italy
Serial number: C0039036
Model number: 240020
Circuit board: (C) 1983 91162 rev D

Information about the chip

Motorola logo
(C)1983 COLECO
R72114A
8317   */

ROM_START (colecop)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r72114a_8317.u2", 0x0000, 0x2000, CRC(d393c0cc) SHA1(160077afb139943725c634d6539898db59f33657) )
ROM_END

ROM_START (svi603)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "svi603.rom", 0x0000, 0x2000, CRC(19e91b82) SHA1(8a30abe5ffef810b0f99b86db38b1b3c9d259b78) )
ROM_END

ROM_START( czz50 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "czz50.rom", 0x0000, 0x2000, CRC(4999abc6) SHA1(96aecec3712c94517103d894405bc98a7dafa440) )
	ROM_CONTINUE( 0x8000, 0x2000 )
ROM_END

#define rom_dina rom_czz50
#define rom_prsarcde rom_czz50


/* System Drivers */

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT   INIT              COMPANY             FULLNAME                            FLAGS
CONS( 1982, coleco,   0,        0,      coleco,   coleco, driver_device, 0, "Coleco",           "ColecoVision (NTSC)",              0 )
CONS( 1982, onyx,     coleco,   0,      coleco,   coleco, driver_device, 0, "Microdigital",     "Onyx (Brazil/Prototype)",          0 )
CONS( 1983, colecop,  coleco,   0,      colecop,  coleco, driver_device, 0, "Coleco",           "ColecoVision (PAL)",               0 )
CONS( 1983, svi603,   coleco,   0,      coleco,   coleco, driver_device, 0, "Spectravideo",     "SVI-603 Coleco Game Adapter",      0 )
CONS( 1986, czz50,    0,        coleco, czz50,    czz50,  driver_device, 0, "Bit Corporation",  "Chuang Zao Zhe 50",                0 )
CONS( 1988, dina,     czz50,    0,      dina,     czz50,  driver_device, 0, "Telegames",        "Dina",                             0 )
CONS( 1988, prsarcde, czz50,    0,      czz50,    czz50,  driver_device, 0, "Telegames",        "Personal Arcade",                  0 )
