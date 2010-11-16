/******************************************************************************
    Exidy Max-A-Flex driver

    by Mariusz Wojcieszek

    Based on Atari 400/800 MESS Driver by Juergen Buchmueller

    TODO:
    - fix LCD digits display controlling

******************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6805/m6805.h"
#include "includes/atari.h"
#include "sound/speaker.h"
#include "sound/pokey.h"
#include "machine/6821pia.h"
#include "video/gtia.h"

#include "maxaflex.lh"


/* Supervisor board emulation */

static UINT8 portA_in,portA_out,ddrA;
static UINT8 portB_in,portB_out,ddrB;
static UINT8 portC_in,portC_out,ddrC;
static UINT8 tdr,tcr;
static timer_device *mcu_timer;

/* Port A:
    0   (in)  DSW
    1   (in)  DSW
    2   (in)  DSW
    3   (in)  DSW
    4   (in)  coin
    5   (in)  START button
    6   -
    7   (out) AUDIO
*/

static READ8_HANDLER( mcu_portA_r )
{
	portA_in = input_port_read(space->machine, "dsw") | (input_port_read(space->machine, "coin") << 4) | (input_port_read(space->machine, "console") << 5);
	return (portA_in & ~ddrA) | (portA_out & ddrA);
}

static WRITE8_HANDLER( mcu_portA_w )
{
	portA_out = data;
	speaker_level_w(space->machine->device("speaker"), data >> 7);
}

/* Port B:
    0   (out)   Select 7-segment display to control by writing port C
    1   (out)
    2   (out)   clear coin interupt
    3   (out)   STRKEY - line connected to keyboard input in 600XL, seems to be not used
    4   (out)   RES600 - reset 600
    5   (out)   AUDMUTE - mutes audio
    6   (out)   latch for lamps
    7   (out)   TOFF - enables/disables user controls
*/

static READ8_HANDLER( mcu_portB_r )
{
	return (portB_in & ~ddrB) | (portB_out & ddrB);
}

static WRITE8_HANDLER( mcu_portB_w )
{
	UINT8 diff = data ^ portB_out;
	portB_out = data;

	/* clear coin interrupt */
	if (data & 0x04)
		cputag_set_input_line(space->machine, "mcu", M6805_IRQ_LINE, CLEAR_LINE );

	/* AUDMUTE */
	sound_global_enable(space->machine, (data >> 5) & 1);

	/* RES600 */
	if (diff & 0x10)
		cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);

	/* latch for lamps */
	if ((diff & 0x40) && !(data & 0x40))
	{
		output_set_lamp_value(0, (portC_out >> 0) & 1);
		output_set_lamp_value(1, (portC_out >> 1) & 1);
		output_set_lamp_value(2, (portC_out >> 2) & 1);
		output_set_lamp_value(3, (portC_out >> 3) & 1);
	}
}

/* Port C:
    0   (out)   lamp COIN
    1   (out)   lamp PLAY
    2   (out)   lamp START
    3   (out)   lamp OVER */

static READ8_HANDLER( mcu_portC_r )
{
	return (portC_in & ~ddrC) | (portC_out & ddrC);
}

static WRITE8_HANDLER( mcu_portC_w )
{
	/* uses a 7447A, which is equivalent to an LS47/48 */
	static const UINT8 ls48_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

	portC_out = data & 0x0f;

	/* displays */
	switch( portB_out & 0x3 )
	{
		case 0x0: output_set_digit_value(0, ls48_map[portC_out]); break;
		case 0x1: output_set_digit_value(1, ls48_map[portC_out]); break;
		case 0x2: output_set_digit_value(2, ls48_map[portC_out]); break;
		case 0x3: break;
	}
}

static READ8_HANDLER( mcu_ddr_r )
{
	return 0xff;
}

static WRITE8_HANDLER( mcu_portA_ddr_w )
{
	ddrA = data;
}

static WRITE8_HANDLER( mcu_portB_ddr_w )
{
	ddrB = data;
}

static WRITE8_HANDLER( mcu_portC_ddr_w )
{
	ddrC = data;
}

static TIMER_DEVICE_CALLBACK( mcu_timer_proc )
{
	if ( --tdr == 0x00 )
	{
		if ( (tcr & 0x40) == 0 )
		{
			//timer interrupt!
			generic_pulse_irq_line(timer.machine->device("mcu"), M68705_INT_TIMER);
		}
	}
}

/* Timer Data Reg */
static READ8_HANDLER( mcu_tdr_r )
{
	return tdr;
}

static WRITE8_HANDLER( mcu_tdr_w )
{
	tdr = data;
}

/* Timer control reg */
static READ8_HANDLER( mcu_tcr_r )
{
	return tcr & ~0x08;
}

static WRITE8_HANDLER( mcu_tcr_w )
{
	tcr = data;
	if ( (tcr & 0x40) == 0 )
	{
		int divider;
		attotime period;

		if ( !(tcr & 0x20) )
		{
			/* internal clock / 4*/
			divider = 4;
		}
		else
		{
			/* external clock */
			divider = 1;
		}

		if ( tcr & 0x07 )
		{
			/* use prescaler */
			divider = divider * (1 << (tcr & 0x7));
		}

		period = attotime_mul(ATTOTIME_IN_HZ(3579545), divider);
		mcu_timer->adjust(period, 0, period);
	}
}

static MACHINE_RESET(supervisor_board)
{
	portA_in = portA_out = ddrA	= 0;
	portB_in = portB_out = ddrB	= 0;
	portC_in = portC_out = ddrC	= 0;
	tdr = tcr = 0;
	mcu_timer = machine->device<timer_device>("mcu_timer");

	output_set_lamp_value(0, 0);
	output_set_lamp_value(1, 0);
	output_set_lamp_value(2, 0);
	output_set_lamp_value(3, 0);
	output_set_digit_value(0, 0x00);
	output_set_digit_value(1, 0x00);
	output_set_digit_value(2, 0x00);
}

static INPUT_CHANGED( coin_inserted )
{
	if (!newval)
		cputag_set_input_line(field->port->machine, "mcu", M6805_IRQ_LINE, HOLD_LINE );
}

int atari_input_disabled(void)
{
	return (portB_out & 0x80) == 0x00;
}



static ADDRESS_MAP_START(a600xl_mem, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0x5000, 0x57ff) AM_ROM AM_REGION("maincpu", 0x5000)	/* self test */
	AM_RANGE(0x8000, 0xbfff) AM_ROM	/* game cartridge */
	AM_RANGE(0xc000, 0xcfff) AM_ROM /* OS */
	AM_RANGE(0xd000, 0xd0ff) AM_READWRITE(atari_gtia_r, atari_gtia_w)
	AM_RANGE(0xd100, 0xd1ff) AM_NOP
	AM_RANGE(0xd200, 0xd2ff) AM_DEVREADWRITE("pokey", pokey_r, pokey_w)
    AM_RANGE(0xd300, 0xd3ff) AM_DEVREADWRITE("pia", pia6821_alt_r, pia6821_alt_w)
	AM_RANGE(0xd400, 0xd4ff) AM_READWRITE(atari_antic_r, atari_antic_w)
	AM_RANGE(0xd500, 0xd7ff) AM_NOP
	AM_RANGE(0xd800, 0xffff) AM_ROM /* OS */
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_mem, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READ( mcu_portA_r ) AM_WRITE( mcu_portA_w )
	AM_RANGE(0x0001, 0x0001) AM_READ( mcu_portB_r ) AM_WRITE( mcu_portB_w )
	AM_RANGE(0x0002, 0x0002) AM_READ( mcu_portC_r ) AM_WRITE( mcu_portC_w )
	AM_RANGE(0x0004, 0x0004) AM_READ( mcu_ddr_r ) AM_WRITE( mcu_portA_ddr_w )
	AM_RANGE(0x0005, 0x0005) AM_READ( mcu_ddr_r ) AM_WRITE( mcu_portB_ddr_w )
	AM_RANGE(0x0006, 0x0006) AM_READ( mcu_ddr_r ) AM_WRITE( mcu_portC_ddr_w )
	AM_RANGE(0x0008, 0x0008) AM_READ( mcu_tdr_r ) AM_WRITE( mcu_tdr_w )
	AM_RANGE(0x0009, 0x0009) AM_READ( mcu_tcr_r ) AM_WRITE( mcu_tcr_w )
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( a600xl )

    PORT_START("console")  /* IN0 console keys & switch settings */
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_NAME("Option") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_NAME("Select") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x01, 0x01, IPT_START1 )

	PORT_START("djoy_0_1")	/* IN1 digital joystick #1 + #2 (PIA port A) */
	PORT_BIT(0x01, 0x01, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT(0x02, 0x02, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
	PORT_BIT(0x04, 0x04, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(0x08, 0x08, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	/* player #2 input is not connected */
	PORT_BIT(0x10, 0x10, IPT_JOYSTICK_UP) PORT_PLAYER(2)
	PORT_BIT(0x20, 0x20, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
	PORT_BIT(0x40, 0x40, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT(0x80, 0x80, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)

	PORT_START("djoy_2_3")	/* IN2 digital joystick #3 + #4 (PIA port B) */
	/* not connected */
	PORT_BIT(0x01, 0x01, IPT_JOYSTICK_UP) PORT_PLAYER(3)
	PORT_BIT(0x02, 0x02, IPT_JOYSTICK_DOWN) PORT_PLAYER(3)
	PORT_BIT(0x04, 0x04, IPT_JOYSTICK_LEFT) PORT_PLAYER(3)
	PORT_BIT(0x08, 0x08, IPT_JOYSTICK_RIGHT) PORT_PLAYER(3)
	PORT_BIT(0x10, 0x10, IPT_JOYSTICK_UP) PORT_PLAYER(4)
	PORT_BIT(0x20, 0x20, IPT_JOYSTICK_DOWN) PORT_PLAYER(4)
	PORT_BIT(0x40, 0x40, IPT_JOYSTICK_LEFT) PORT_PLAYER(4)
	PORT_BIT(0x80, 0x80, IPT_JOYSTICK_RIGHT) PORT_PLAYER(4)

	PORT_START("djoy_b")	/* IN3 digital joystick buttons (GTIA button bits) */
	PORT_BIT(0x01, 0x01, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x02, 0x02, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x04, 0x04, IPT_BUTTON1) PORT_PLAYER(3)
	PORT_BIT(0x08, 0x08, IPT_BUTTON1) PORT_PLAYER(4)
	PORT_BIT(0x10, 0x10, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x20, 0x20, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, 0x40, IPT_BUTTON2) PORT_PLAYER(3)
	PORT_BIT(0x80, 0x80, IPT_BUTTON2) PORT_PLAYER(4)

	/* Max-A-Flex specific ports */
	PORT_START("coin")	/* IN4 coin */
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED(coin_inserted, 0)

	PORT_START("dsw")	/* IN5 DSW */
	PORT_DIPNAME(0xf, 0x9, "Coin/Time" )
	PORT_DIPSETTING( 0x0, "30 sec" )
	PORT_DIPSETTING( 0x1, "60 sec" )
	PORT_DIPSETTING( 0x2, "90 sec" )
	PORT_DIPSETTING( 0x3, "120 sec" )
	PORT_DIPSETTING( 0x4, "150 sec" )
	PORT_DIPSETTING( 0x5, "180 sec" )
	PORT_DIPSETTING( 0x6, "210 sec" )
	PORT_DIPSETTING( 0x7, "240 sec" )
	PORT_DIPSETTING( 0x8, "270 sec" )
	PORT_DIPSETTING( 0x9, "300 sec" )
	PORT_DIPSETTING( 0xa, "330 sec" )
	PORT_DIPSETTING( 0xb, "360 sec" )
	PORT_DIPSETTING( 0xc, "390 sec" )
	PORT_DIPSETTING( 0xd, "420 sec" )
	PORT_DIPSETTING( 0xe, "450 sec" )
	PORT_DIPSETTING( 0xf, "480 sec" )

INPUT_PORTS_END


static const pokey_interface pokey_config = {
	{ DEVCB_NULL },
	DEVCB_NULL,
	DEVCB_NULL,DEVCB_NULL,
	atari_interrupt_cb
};

static MACHINE_CONFIG_START( a600xl, driver_device )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502, FREQ_17_EXACT)
	MDRV_CPU_PROGRAM_MAP(a600xl_mem)
	MDRV_CPU_VBLANK_INT_HACK(a800xl_interrupt, TOTAL_LINES_60HZ)

	MDRV_CPU_ADD("mcu", M68705, 3579545)
	MDRV_CPU_PROGRAM_MAP(mcu_mem)

	MDRV_PIA6821_ADD("pia", atarixl_pia_interface)
	MDRV_TIMER_ADD("mcu_timer", mcu_timer_proc)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_VISIBLE_AREA(MIN_X, MAX_X, MIN_Y, MAX_Y)
	MDRV_SCREEN_REFRESH_RATE(FRAME_RATE_60HZ)
	MDRV_SCREEN_SIZE(HWIDTH*8, TOTAL_LINES_60HZ)

	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(atari)
	MDRV_DEFAULT_LAYOUT(layout_maxaflex)

	MDRV_VIDEO_START(atari)
	MDRV_VIDEO_UPDATE(atari)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("pokey", POKEY, FREQ_17_EXACT)
	MDRV_SOUND_CONFIG(pokey_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_MACHINE_START( atarixl )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maxaflex, a600xl )
	MDRV_MACHINE_RESET( supervisor_board )
MACHINE_CONFIG_END

ROM_START(maxaflex)
	ROM_REGION(0x10000,"maincpu",0) /* 64K for the CPU */
    ROM_LOAD("atarixl.rom", 0xc000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55))

	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD("maxaflex.uc",  0x0000, 0x0800, CRC(fe9cf53c) SHA1(4b02bc2f0c8a1eab799814fac82d5812c0160206))

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("maxprom.prm", 0x0000, 0x0200, CRC(edf5c950) SHA1(9ad046ea41a61585dd8d2f2d4167a3cc39d2928f))	/* for simulating keystrokes ?*/
ROM_END

ROM_START(mf_bdash)
	ROM_REGION(0x10000,"maincpu",0) /* 64K for the CPU */
	ROM_LOAD("bd-acs-1.rom",	0x8000, 0x2000, CRC(2b11750e) SHA1(43e9ae44eb1767621920bb94a4370ed602d81056))
	ROM_LOAD("bd-acs-2.rom",	0xa000, 0x2000, CRC(e9ea2658) SHA1(189ede7201ef122cf2b72fc847a896b9dbe007e5))
    ROM_LOAD("atarixl.rom",		0xc000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55))

	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD("maxaflex.uc",  0x0000, 0x0800, CRC(fe9cf53c) SHA1(4b02bc2f0c8a1eab799814fac82d5812c0160206))

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("maxprom.prm", 0x0000, 0x0200, CRC(edf5c950) SHA1(9ad046ea41a61585dd8d2f2d4167a3cc39d2928f))	/* for simulating keystrokes ?*/
ROM_END

ROM_START(mf_achas)
	ROM_REGION(0x10000,"maincpu",0) /* 64K for the CPU */
	ROM_LOAD("ac.rom",			0x8000, 0x4000, CRC(18752991) SHA1(f508b89d2251c53d017cff6cb23b8e9880a0cc0b))
    ROM_LOAD("atarixl.rom",		0xc000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55))

	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD("maxaflex.uc",  0x0000, 0x0800, CRC(fe9cf53c) SHA1(4b02bc2f0c8a1eab799814fac82d5812c0160206))

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("maxprom.prm", 0x0000, 0x0200, CRC(edf5c950) SHA1(9ad046ea41a61585dd8d2f2d4167a3cc39d2928f))	/* for simulating keystrokes ?*/
ROM_END

ROM_START(mf_brist)
	ROM_REGION(0x10000,"maincpu",0) /* 64K for the CPU */
	ROM_LOAD("brist.rom",		0x8000, 0x4000, CRC(4263d64d) SHA1(80a041bceb499e1466516488013aa4439b3db6f2))
    ROM_LOAD("atarixl.rom",		0xc000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55))

	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD("maxaflex.uc",  0x0000, 0x0800, CRC(fe9cf53c) SHA1(4b02bc2f0c8a1eab799814fac82d5812c0160206))

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("maxprom.prm", 0x0000, 0x0200, CRC(edf5c950) SHA1(9ad046ea41a61585dd8d2f2d4167a3cc39d2928f))	/* for simulating keystrokes ?*/
ROM_END

ROM_START(mf_flip)
	ROM_REGION(0x10000,"maincpu",0) /* 64K for the CPU */
	ROM_LOAD("flipflop.rom",	0x8000, 0x4000, CRC(8ae057be) SHA1(ba26d6a3790ebdb754c1192b2c28f0fe93aca377))
    ROM_LOAD("atarixl.rom",		0xc000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55))

	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD("maxaflex.uc",  0x0000, 0x0800, CRC(fe9cf53c) SHA1(4b02bc2f0c8a1eab799814fac82d5812c0160206))

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("maxprom.prm", 0x0000, 0x0200, CRC(edf5c950) SHA1(9ad046ea41a61585dd8d2f2d4167a3cc39d2928f))	/* for simulating keystrokes ?*/
ROM_END

static DRIVER_INIT( a600xl )
{
	UINT8 *rom = memory_region(machine, "maincpu");
	memcpy( rom + 0x5000, rom + 0xd000, 0x800 );
}

GAME( 1984, maxaflex, 0,        maxaflex, a600xl, a600xl, ROT0, "Exidy", "Max-A-Flex", GAME_IS_BIOS_ROOT )
GAME( 1982, mf_achas, maxaflex, maxaflex, a600xl, a600xl, ROT0, "Exidy / First Star Software", "Astro Chase (Max-A-Flex)", 0 )
GAME( 1983, mf_brist, maxaflex, maxaflex, a600xl, a600xl, ROT0, "Exidy / First Star Software", "Bristles (Max-A-Flex)", 0 )
GAME( 1983, mf_flip,  maxaflex, maxaflex, a600xl, a600xl, ROT0, "Exidy / First Star Software", "Flip & Flop (Max-A-Flex)", 0 )
GAME( 1984, mf_bdash, maxaflex, maxaflex, a600xl, a600xl, ROT0, "Exidy / First Star Software", "Boulder Dash (Max-A-Flex)", 0 )
