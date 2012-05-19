/******************************************************************************
    Exidy Max-A-Flex driver

    by Mariusz Wojcieszek

    Based on Atari 400/800 MESS Driver by Juergen Buchmueller

    TODO:
    - fix LCD digits display controlling

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6805/m6805.h"
#include "includes/atari.h"
#include "sound/speaker.h"
#include "sound/pokey.h"
#include "machine/6821pia.h"
#include "video/gtia.h"

#include "maxaflex.lh"


class maxaflex_state : public driver_device
{
public:
	maxaflex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_portA_in;
	UINT8 m_portA_out;
	UINT8 m_ddrA;
	UINT8 m_portB_in;
	UINT8 m_portB_out;
	UINT8 m_ddrB;
	UINT8 m_portC_in;
	UINT8 m_portC_out;
	UINT8 m_ddrC;
	UINT8 m_tdr;
	UINT8 m_tcr;
	timer_device *m_mcu_timer;
	DECLARE_READ8_MEMBER(mcu_portA_r);
	DECLARE_WRITE8_MEMBER(mcu_portA_w);
	DECLARE_READ8_MEMBER(mcu_portB_r);
	DECLARE_WRITE8_MEMBER(mcu_portB_w);
	DECLARE_READ8_MEMBER(mcu_portC_r);
	DECLARE_WRITE8_MEMBER(mcu_portC_w);
	DECLARE_READ8_MEMBER(mcu_ddr_r);
	DECLARE_WRITE8_MEMBER(mcu_portA_ddr_w);
	DECLARE_WRITE8_MEMBER(mcu_portB_ddr_w);
	DECLARE_WRITE8_MEMBER(mcu_portC_ddr_w);
	DECLARE_READ8_MEMBER(mcu_tdr_r);
	DECLARE_WRITE8_MEMBER(mcu_tdr_w);
	DECLARE_READ8_MEMBER(mcu_tcr_r);
	DECLARE_WRITE8_MEMBER(mcu_tcr_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
};



/* Supervisor board emulation */


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

READ8_MEMBER(maxaflex_state::mcu_portA_r)
{
	m_portA_in = ioport("dsw")->read() | (ioport("coin")->read() << 4) | (ioport("console")->read() << 5);
	return (m_portA_in & ~m_ddrA) | (m_portA_out & m_ddrA);
}

WRITE8_MEMBER(maxaflex_state::mcu_portA_w)
{
	m_portA_out = data;
	speaker_level_w(machine().device("speaker"), data >> 7);
}

/* Port B:
    0   (out)   Select 7-segment display to control by writing port C
    1   (out)
    2   (out)   clear coin interrupt
    3   (out)   STRKEY - line connected to keyboard input in 600XL, seems to be not used
    4   (out)   RES600 - reset 600
    5   (out)   AUDMUTE - mutes audio
    6   (out)   latch for lamps
    7   (out)   TOFF - enables/disables user controls
*/

READ8_MEMBER(maxaflex_state::mcu_portB_r)
{
	return (m_portB_in & ~m_ddrB) | (m_portB_out & m_ddrB);
}

WRITE8_MEMBER(maxaflex_state::mcu_portB_w)
{
	UINT8 diff = data ^ m_portB_out;
	m_portB_out = data;

	/* clear coin interrupt */
	if (data & 0x04)
		cputag_set_input_line(machine(), "mcu", M6805_IRQ_LINE, CLEAR_LINE );

	/* AUDMUTE */
	machine().sound().system_enable((data >> 5) & 1);

	/* RES600 */
	if (diff & 0x10)
		cputag_set_input_line(machine(), "maincpu", INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);

	/* latch for lamps */
	if ((diff & 0x40) && !(data & 0x40))
	{
		output_set_lamp_value(0, (m_portC_out >> 0) & 1);
		output_set_lamp_value(1, (m_portC_out >> 1) & 1);
		output_set_lamp_value(2, (m_portC_out >> 2) & 1);
		output_set_lamp_value(3, (m_portC_out >> 3) & 1);
	}
}

/* Port C:
    0   (out)   lamp COIN
    1   (out)   lamp PLAY
    2   (out)   lamp START
    3   (out)   lamp OVER */

READ8_MEMBER(maxaflex_state::mcu_portC_r)
{
	return (m_portC_in & ~m_ddrC) | (m_portC_out & m_ddrC);
}

WRITE8_MEMBER(maxaflex_state::mcu_portC_w)
{
	/* uses a 7447A, which is equivalent to an LS47/48 */
	static const UINT8 ls48_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

	m_portC_out = data & 0x0f;

	/* displays */
	switch( m_portB_out & 0x3 )
	{
		case 0x0: output_set_digit_value(0, ls48_map[m_portC_out]); break;
		case 0x1: output_set_digit_value(1, ls48_map[m_portC_out]); break;
		case 0x2: output_set_digit_value(2, ls48_map[m_portC_out]); break;
		case 0x3: break;
	}
}

READ8_MEMBER(maxaflex_state::mcu_ddr_r)
{
	return 0xff;
}

WRITE8_MEMBER(maxaflex_state::mcu_portA_ddr_w)
{
	m_ddrA = data;
}

WRITE8_MEMBER(maxaflex_state::mcu_portB_ddr_w)
{
	m_ddrB = data;
}

WRITE8_MEMBER(maxaflex_state::mcu_portC_ddr_w)
{
	m_ddrC = data;
}

static TIMER_DEVICE_CALLBACK( mcu_timer_proc )
{
	maxaflex_state *state = timer.machine().driver_data<maxaflex_state>();
	if ( --state->m_tdr == 0x00 )
	{
		if ( (state->m_tcr & 0x40) == 0 )
		{
			//timer interrupt!
			generic_pulse_irq_line(timer.machine().device("mcu"), M68705_INT_TIMER, 1);
		}
	}
}

/* Timer Data Reg */
READ8_MEMBER(maxaflex_state::mcu_tdr_r)
{
	return m_tdr;
}

WRITE8_MEMBER(maxaflex_state::mcu_tdr_w)
{
	m_tdr = data;
}

/* Timer control reg */
READ8_MEMBER(maxaflex_state::mcu_tcr_r)
{
	return m_tcr & ~0x08;
}

WRITE8_MEMBER(maxaflex_state::mcu_tcr_w)
{
	m_tcr = data;
	if ( (m_tcr & 0x40) == 0 )
	{
		int divider;
		attotime period;

		if ( !(m_tcr & 0x20) )
		{
			/* internal clock / 4*/
			divider = 4;
		}
		else
		{
			/* external clock */
			divider = 1;
		}

		if ( m_tcr & 0x07 )
		{
			/* use prescaler */
			divider = divider * (1 << (m_tcr & 0x7));
		}

		period = attotime::from_hz(3579545) * divider;
		m_mcu_timer->adjust(period, 0, period);
	}
}

static MACHINE_RESET(supervisor_board)
{
	maxaflex_state *state = machine.driver_data<maxaflex_state>();
	state->m_portA_in = state->m_portA_out = state->m_ddrA	= 0;
	state->m_portB_in = state->m_portB_out = state->m_ddrB	= 0;
	state->m_portC_in = state->m_portC_out = state->m_ddrC	= 0;
	state->m_tdr = state->m_tcr = 0;
	state->m_mcu_timer = machine.device<timer_device>("mcu_timer");

	output_set_lamp_value(0, 0);
	output_set_lamp_value(1, 0);
	output_set_lamp_value(2, 0);
	output_set_lamp_value(3, 0);
	output_set_digit_value(0, 0x00);
	output_set_digit_value(1, 0x00);
	output_set_digit_value(2, 0x00);
}

INPUT_CHANGED_MEMBER(maxaflex_state::coin_inserted)
{
	if (!newval)
		cputag_set_input_line(machine(), "mcu", M6805_IRQ_LINE, HOLD_LINE );
}

int atari_input_disabled(running_machine &machine)
{
	maxaflex_state *state = machine.driver_data<maxaflex_state>();
	return (state->m_portB_out & 0x80) == 0x00;
}



static ADDRESS_MAP_START(a600xl_mem, AS_PROGRAM, 8, maxaflex_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0x5000, 0x57ff) AM_ROM AM_REGION("maincpu", 0x5000)	/* self test */
	AM_RANGE(0x8000, 0xbfff) AM_ROM	/* game cartridge */
	AM_RANGE(0xc000, 0xcfff) AM_ROM /* OS */
	AM_RANGE(0xd000, 0xd0ff) AM_READWRITE_LEGACY(atari_gtia_r, atari_gtia_w)
	AM_RANGE(0xd100, 0xd1ff) AM_NOP
	AM_RANGE(0xd200, 0xd2ff) AM_DEVREADWRITE("pokey", pokeyn_device, read, write)
    AM_RANGE(0xd300, 0xd3ff) AM_DEVREADWRITE("pia", pia6821_device, read_alt, write_alt)
	AM_RANGE(0xd400, 0xd4ff) AM_READWRITE_LEGACY(atari_antic_r, atari_antic_w)
	AM_RANGE(0xd500, 0xd7ff) AM_NOP
	AM_RANGE(0xd800, 0xffff) AM_ROM /* OS */
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_mem, AS_PROGRAM, 8, maxaflex_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READ(mcu_portA_r ) AM_WRITE(mcu_portA_w )
	AM_RANGE(0x0001, 0x0001) AM_READ(mcu_portB_r ) AM_WRITE(mcu_portB_w )
	AM_RANGE(0x0002, 0x0002) AM_READ(mcu_portC_r ) AM_WRITE(mcu_portC_w )
	AM_RANGE(0x0004, 0x0004) AM_READ(mcu_ddr_r ) AM_WRITE(mcu_portA_ddr_w )
	AM_RANGE(0x0005, 0x0005) AM_READ(mcu_ddr_r ) AM_WRITE(mcu_portB_ddr_w )
	AM_RANGE(0x0006, 0x0006) AM_READ(mcu_ddr_r ) AM_WRITE(mcu_portC_ddr_w )
	AM_RANGE(0x0008, 0x0008) AM_READ(mcu_tdr_r ) AM_WRITE(mcu_tdr_w )
	AM_RANGE(0x0009, 0x0009) AM_READ(mcu_tcr_r ) AM_WRITE(mcu_tcr_w )
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( a600xl )

    PORT_START("console")  /* IN0 console keys & switch settings */
	PORT_BIT(0x04, 0x04, IPT_KEYPAD) PORT_NAME("Option") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x02, 0x02, IPT_KEYPAD) PORT_NAME("Select") PORT_CODE(KEYCODE_F1)
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
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, maxaflex_state,coin_inserted, 0)

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

READ8_DEVICE_HANDLER(maxaflex_atari_pia_pa_r)
{
	return atari_input_disabled(device->machine()) ? 0xFF : device->machine().root_device().ioport("djoy_0_1")->read_safe(0);
}

READ8_DEVICE_HANDLER(maxaflex_atari_pia_pb_r)
{
	return atari_input_disabled(device->machine()) ? 0xFF : device->machine().root_device().ioport("djoy_2_3")->read_safe(0);
}


const pia6821_interface maxaflex_atarixl_pia_interface =
{
	DEVCB_HANDLER(maxaflex_atari_pia_pa_r),		/* port A in */
	DEVCB_HANDLER(maxaflex_atari_pia_pb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(a600xl_pia_pb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_LINE(atari_pia_cb2_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};



static MACHINE_CONFIG_START( a600xl, maxaflex_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, FREQ_17_EXACT)
	MCFG_CPU_PROGRAM_MAP(a600xl_mem)
	MCFG_TIMER_ADD_SCANLINE("scantimer", a800xl_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("mcu", M68705, 3579545)
	MCFG_CPU_PROGRAM_MAP(mcu_mem)

	MCFG_PIA6821_ADD("pia", maxaflex_atarixl_pia_interface)
	MCFG_TIMER_ADD("mcu_timer", mcu_timer_proc)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_VISIBLE_AREA(MIN_X, MAX_X, MIN_Y, MAX_Y)
	MCFG_SCREEN_REFRESH_RATE(FRAME_RATE_60HZ)
	MCFG_SCREEN_SIZE(HWIDTH*8, TOTAL_LINES_60HZ)
	MCFG_SCREEN_UPDATE_STATIC(atari)

	MCFG_PALETTE_LENGTH(256)
	MCFG_PALETTE_INIT(atari)
	MCFG_DEFAULT_LAYOUT(layout_maxaflex)

	MCFG_VIDEO_START(atari)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("pokey", POKEYN, FREQ_17_EXACT)
	MCFG_SOUND_CONFIG(pokey_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_MACHINE_START( atarixl )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maxaflex, a600xl )
	MCFG_MACHINE_RESET( supervisor_board )
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
	UINT8 *rom = machine.root_device().memregion("maincpu")->base();
	memcpy( rom + 0x5000, rom + 0xd000, 0x800 );
}

GAME( 1984, maxaflex, 0,        maxaflex, a600xl, a600xl, ROT0, "Exidy", "Max-A-Flex", GAME_IS_BIOS_ROOT )
GAME( 1982, mf_achas, maxaflex, maxaflex, a600xl, a600xl, ROT0, "Exidy / First Star Software", "Astro Chase (Max-A-Flex)", 0 )
GAME( 1983, mf_brist, maxaflex, maxaflex, a600xl, a600xl, ROT0, "Exidy / First Star Software", "Bristles (Max-A-Flex)", 0 )
GAME( 1983, mf_flip,  maxaflex, maxaflex, a600xl, a600xl, ROT0, "Exidy / First Star Software", "Flip & Flop (Max-A-Flex)", 0 )
GAME( 1984, mf_bdash, maxaflex, maxaflex, a600xl, a600xl, ROT0, "Exidy / First Star Software", "Boulder Dash (Max-A-Flex)", 0 )
