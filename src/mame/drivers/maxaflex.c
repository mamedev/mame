/******************************************************************************
    Exidy Max-A-Flex driver

    by Mariusz Wojcieszek

    Based on Atari 400/800 MESS Driver by Juergen Buchmueller

    TODO:
    - add AUDMUTE port - muting all sounds on system
    - fix LCD digits display controlling
    - change lamps and time counter display to use artwork

******************************************************************************/

#include "driver.h"
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
static const device_config *mcu_timer;

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
	speaker_level_w(0, data >> 7);
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
			generic_pulse_irq_line(cputag_get_cpu(timer->machine, "mcu"), M68705_INT_TIMER);
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
		timer_device_adjust_periodic( mcu_timer, period, 0, period);
	}
}

static MACHINE_RESET(supervisor_board)
{
	portA_in = portA_out = ddrA	= 0;
	portB_in = portB_out = ddrB	= 0;
	portC_in = portC_out = ddrC	= 0;
	tdr = tcr = 0;
	mcu_timer = devtag_get_device(machine, "mcu_timer");

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

static const rgb_t atari_palette[256] =
{
	/* Grey */
    MAKE_RGB(0x00,0x00,0x00), MAKE_RGB(0x1c,0x1c,0x1c), MAKE_RGB(0x39,0x39,0x39), MAKE_RGB(0x59,0x59,0x59),
	MAKE_RGB(0x79,0x79,0x79), MAKE_RGB(0x92,0x92,0x92), MAKE_RGB(0xab,0xab,0xab), MAKE_RGB(0xbc,0xbc,0xbc),
	MAKE_RGB(0xcd,0xcd,0xcd), MAKE_RGB(0xd9,0xd9,0xd9), MAKE_RGB(0xe6,0xe6,0xe6), MAKE_RGB(0xec,0xec,0xec),
	MAKE_RGB(0xf2,0xf2,0xf2), MAKE_RGB(0xf8,0xf8,0xf8), MAKE_RGB(0xff,0xff,0xff), MAKE_RGB(0xff,0xff,0xff),
	/* Gold */
    MAKE_RGB(0x39,0x17,0x01), MAKE_RGB(0x5e,0x23,0x04), MAKE_RGB(0x83,0x30,0x08), MAKE_RGB(0xa5,0x47,0x16),
	MAKE_RGB(0xc8,0x5f,0x24), MAKE_RGB(0xe3,0x78,0x20), MAKE_RGB(0xff,0x91,0x1d), MAKE_RGB(0xff,0xab,0x1d),
	MAKE_RGB(0xff,0xc5,0x1d), MAKE_RGB(0xff,0xce,0x34), MAKE_RGB(0xff,0xd8,0x4c), MAKE_RGB(0xff,0xe6,0x51),
	MAKE_RGB(0xff,0xf4,0x56), MAKE_RGB(0xff,0xf9,0x77), MAKE_RGB(0xff,0xff,0x98), MAKE_RGB(0xff,0xff,0x98),
	/* Orange */
    MAKE_RGB(0x45,0x19,0x04), MAKE_RGB(0x72,0x1e,0x11), MAKE_RGB(0x9f,0x24,0x1e), MAKE_RGB(0xb3,0x3a,0x20),
	MAKE_RGB(0xc8,0x51,0x22), MAKE_RGB(0xe3,0x69,0x20), MAKE_RGB(0xff,0x81,0x1e), MAKE_RGB(0xff,0x8c,0x25),
	MAKE_RGB(0xff,0x98,0x2c), MAKE_RGB(0xff,0xae,0x38), MAKE_RGB(0xff,0xc5,0x45), MAKE_RGB(0xff,0xc5,0x59),
	MAKE_RGB(0xff,0xc6,0x6d), MAKE_RGB(0xff,0xd5,0x87), MAKE_RGB(0xff,0xe4,0xa1), MAKE_RGB(0xff,0xe4,0xa1),
	/* Red-Orange */
    MAKE_RGB(0x4a,0x17,0x04), MAKE_RGB(0x7e,0x1a,0x0d), MAKE_RGB(0xb2,0x1d,0x17), MAKE_RGB(0xc8,0x21,0x19),
	MAKE_RGB(0xdf,0x25,0x1c), MAKE_RGB(0xec,0x3b,0x38), MAKE_RGB(0xfa,0x52,0x55), MAKE_RGB(0xfc,0x61,0x61),
	MAKE_RGB(0xff,0x70,0x6e), MAKE_RGB(0xff,0x7f,0x7e), MAKE_RGB(0xff,0x8f,0x8f), MAKE_RGB(0xff,0x9d,0x9e),
	MAKE_RGB(0xff,0xab,0xad), MAKE_RGB(0xff,0xb9,0xbd), MAKE_RGB(0xff,0xc7,0xce), MAKE_RGB(0xff,0xc7,0xce),
	/* Pink */
    MAKE_RGB(0x05,0x05,0x68), MAKE_RGB(0x3b,0x13,0x6d), MAKE_RGB(0x71,0x22,0x72), MAKE_RGB(0x8b,0x2a,0x8c),
	MAKE_RGB(0xa5,0x32,0xa6), MAKE_RGB(0xb9,0x38,0xba), MAKE_RGB(0xcd,0x3e,0xcf), MAKE_RGB(0xdb,0x47,0xdd),
	MAKE_RGB(0xea,0x51,0xeb), MAKE_RGB(0xf4,0x5f,0xf5), MAKE_RGB(0xfe,0x6d,0xff), MAKE_RGB(0xfe,0x7a,0xfd),
	MAKE_RGB(0xff,0x87,0xfb), MAKE_RGB(0xff,0x95,0xfd), MAKE_RGB(0xff,0xa4,0xff), MAKE_RGB(0xff,0xa4,0xff),
	/* Purple */
    MAKE_RGB(0x28,0x04,0x79), MAKE_RGB(0x40,0x09,0x84), MAKE_RGB(0x59,0x0f,0x90), MAKE_RGB(0x70,0x24,0x9d),
	MAKE_RGB(0x88,0x39,0xaa), MAKE_RGB(0xa4,0x41,0xc3), MAKE_RGB(0xc0,0x4a,0xdc), MAKE_RGB(0xd0,0x54,0xed),
	MAKE_RGB(0xe0,0x5e,0xff), MAKE_RGB(0xe9,0x6d,0xff), MAKE_RGB(0xf2,0x7c,0xff), MAKE_RGB(0xf8,0x8a,0xff),
	MAKE_RGB(0xff,0x98,0xff), MAKE_RGB(0xfe,0xa1,0xff), MAKE_RGB(0xfe,0xab,0xff), MAKE_RGB(0xfe,0xab,0xff),
	/* Purple-Blue */
    MAKE_RGB(0x35,0x08,0x8a), MAKE_RGB(0x42,0x0a,0xad), MAKE_RGB(0x50,0x0c,0xd0), MAKE_RGB(0x64,0x28,0xd0),
	MAKE_RGB(0x79,0x45,0xd0), MAKE_RGB(0x8d,0x4b,0xd4), MAKE_RGB(0xa2,0x51,0xd9), MAKE_RGB(0xb0,0x58,0xec),
	MAKE_RGB(0xbe,0x60,0xff), MAKE_RGB(0xc5,0x6b,0xff), MAKE_RGB(0xcc,0x77,0xff), MAKE_RGB(0xd1,0x83,0xff),
	MAKE_RGB(0xd7,0x90,0xff), MAKE_RGB(0xdb,0x9d,0xff), MAKE_RGB(0xdf,0xaa,0xff), MAKE_RGB(0xdf,0xaa,0xff),
	/* Blue 1 */
    MAKE_RGB(0x05,0x1e,0x81), MAKE_RGB(0x06,0x26,0xa5), MAKE_RGB(0x08,0x2f,0xca), MAKE_RGB(0x26,0x3d,0xd4),
	MAKE_RGB(0x44,0x4c,0xde), MAKE_RGB(0x4f,0x5a,0xee), MAKE_RGB(0x5a,0x68,0xff), MAKE_RGB(0x65,0x75,0xff),
	MAKE_RGB(0x71,0x83,0xff), MAKE_RGB(0x80,0x91,0xff), MAKE_RGB(0x90,0xa0,0xff), MAKE_RGB(0x97,0xa9,0xff),
	MAKE_RGB(0x9f,0xb2,0xff), MAKE_RGB(0xaf,0xbe,0xff), MAKE_RGB(0xc0,0xcb,0xff), MAKE_RGB(0xc0,0xcb,0xff),
	/* Blue 2 */
    MAKE_RGB(0x0c,0x04,0x8b), MAKE_RGB(0x22,0x18,0xa0), MAKE_RGB(0x38,0x2d,0xb5), MAKE_RGB(0x48,0x3e,0xc7),
	MAKE_RGB(0x58,0x4f,0xda), MAKE_RGB(0x61,0x59,0xec), MAKE_RGB(0x6b,0x64,0xff), MAKE_RGB(0x7a,0x74,0xff),
	MAKE_RGB(0x8a,0x84,0xff), MAKE_RGB(0x91,0x8e,0xff), MAKE_RGB(0x99,0x98,0xff), MAKE_RGB(0xa5,0xa3,0xff),
	MAKE_RGB(0xb1,0xae,0xff), MAKE_RGB(0xb8,0xb8,0xff), MAKE_RGB(0xc0,0xc2,0xff), MAKE_RGB(0xc0,0xc2,0xff),
	/* Light-Blue */
    MAKE_RGB(0x1d,0x29,0x5a), MAKE_RGB(0x1d,0x38,0x76), MAKE_RGB(0x1d,0x48,0x92), MAKE_RGB(0x1c,0x5c,0xac),
	MAKE_RGB(0x1c,0x71,0xc6), MAKE_RGB(0x32,0x86,0xcf), MAKE_RGB(0x48,0x9b,0xd9), MAKE_RGB(0x4e,0xa8,0xec),
	MAKE_RGB(0x55,0xb6,0xff), MAKE_RGB(0x70,0xc7,0xff), MAKE_RGB(0x8c,0xd8,0xff), MAKE_RGB(0x93,0xdb,0xff),
	MAKE_RGB(0x9b,0xdf,0xff), MAKE_RGB(0xaf,0xe4,0xff), MAKE_RGB(0xc3,0xe9,0xff), MAKE_RGB(0xc3,0xe9,0xff),
	/* Turquoise */
    MAKE_RGB(0x2f,0x43,0x02), MAKE_RGB(0x39,0x52,0x02), MAKE_RGB(0x44,0x61,0x03), MAKE_RGB(0x41,0x7a,0x12),
	MAKE_RGB(0x3e,0x94,0x21), MAKE_RGB(0x4a,0x9f,0x2e), MAKE_RGB(0x57,0xab,0x3b), MAKE_RGB(0x5c,0xbd,0x55),
	MAKE_RGB(0x61,0xd0,0x70), MAKE_RGB(0x69,0xe2,0x7a), MAKE_RGB(0x72,0xf5,0x84), MAKE_RGB(0x7c,0xfa,0x8d),
	MAKE_RGB(0x87,0xff,0x97), MAKE_RGB(0x9a,0xff,0xa6), MAKE_RGB(0xad,0xff,0xb6), MAKE_RGB(0xad,0xff,0xb6),
	/* Green-Blue */
    MAKE_RGB(0x0a,0x41,0x08), MAKE_RGB(0x0d,0x54,0x0a), MAKE_RGB(0x10,0x68,0x0d), MAKE_RGB(0x13,0x7d,0x0f),
    MAKE_RGB(0x16,0x92,0x12), MAKE_RGB(0x19,0xa5,0x14), MAKE_RGB(0x1c,0xb9,0x17), MAKE_RGB(0x1e,0xc9,0x19),
	MAKE_RGB(0x21,0xd9,0x1b), MAKE_RGB(0x47,0xe4,0x2d), MAKE_RGB(0x6e,0xf0,0x40), MAKE_RGB(0x78,0xf7,0x4d),
	MAKE_RGB(0x83,0xff,0x5b), MAKE_RGB(0x9a,0xff,0x7a), MAKE_RGB(0xb2,0xff,0x9a), MAKE_RGB(0xb2,0xff,0x9a),
	/* Green */
    MAKE_RGB(0x04,0x41,0x0b), MAKE_RGB(0x05,0x53,0x0e), MAKE_RGB(0x06,0x66,0x11), MAKE_RGB(0x07,0x77,0x14),
	MAKE_RGB(0x08,0x88,0x17), MAKE_RGB(0x09,0x9b,0x1a), MAKE_RGB(0x0b,0xaf,0x1d), MAKE_RGB(0x48,0xc4,0x1f),
	MAKE_RGB(0x86,0xd9,0x22), MAKE_RGB(0x8f,0xe9,0x24), MAKE_RGB(0x99,0xf9,0x27), MAKE_RGB(0xa8,0xfc,0x41),
	MAKE_RGB(0xb7,0xff,0x5b), MAKE_RGB(0xc9,0xff,0x6e), MAKE_RGB(0xdc,0xff,0x81), MAKE_RGB(0xdc,0xff,0x81),
	/* Yellow-Green */
    MAKE_RGB(0x02,0x35,0x0f), MAKE_RGB(0x07,0x3f,0x15), MAKE_RGB(0x0c,0x4a,0x1c), MAKE_RGB(0x2d,0x5f,0x1e),
	MAKE_RGB(0x4f,0x74,0x20), MAKE_RGB(0x59,0x83,0x24), MAKE_RGB(0x64,0x92,0x28), MAKE_RGB(0x82,0xa1,0x2e),
	MAKE_RGB(0xa1,0xb0,0x34), MAKE_RGB(0xa9,0xc1,0x3a), MAKE_RGB(0xb2,0xd2,0x41), MAKE_RGB(0xc4,0xd9,0x45),
	MAKE_RGB(0xd6,0xe1,0x49), MAKE_RGB(0xe4,0xf0,0x4e), MAKE_RGB(0xf2,0xff,0x53), MAKE_RGB(0xf2,0xff,0x53),
	/* Orange-Green */
    MAKE_RGB(0x26,0x30,0x01), MAKE_RGB(0x24,0x38,0x03), MAKE_RGB(0x23,0x40,0x05), MAKE_RGB(0x51,0x54,0x1b),
	MAKE_RGB(0x80,0x69,0x31), MAKE_RGB(0x97,0x81,0x35), MAKE_RGB(0xaf,0x99,0x3a), MAKE_RGB(0xc2,0xa7,0x3e),
	MAKE_RGB(0xd5,0xb5,0x43), MAKE_RGB(0xdb,0xc0,0x3d), MAKE_RGB(0xe1,0xcb,0x38), MAKE_RGB(0xe2,0xd8,0x36),
	MAKE_RGB(0xe3,0xe5,0x34), MAKE_RGB(0xef,0xf2,0x58), MAKE_RGB(0xfb,0xff,0x7d), MAKE_RGB(0xfb,0xff,0x7d),
	/* Light-Orange */
    MAKE_RGB(0x40,0x1a,0x02), MAKE_RGB(0x58,0x1f,0x05), MAKE_RGB(0x70,0x24,0x08), MAKE_RGB(0x8d,0x3a,0x13),
	MAKE_RGB(0xab,0x51,0x1f), MAKE_RGB(0xb5,0x64,0x27), MAKE_RGB(0xbf,0x77,0x30), MAKE_RGB(0xd0,0x85,0x3a),
	MAKE_RGB(0xe1,0x93,0x44), MAKE_RGB(0xed,0xa0,0x4e), MAKE_RGB(0xf9,0xad,0x58), MAKE_RGB(0xfc,0xb7,0x5c),
	MAKE_RGB(0xff,0xc1,0x60), MAKE_RGB(0xff,0xc6,0x71), MAKE_RGB(0xff,0xcb,0x83), MAKE_RGB(0xff,0xcb,0x83)
};

/* Initialise the palette */
static PALETTE_INIT( atari )
{
	palette_set_colors(machine, 0, atari_palette, ARRAY_LENGTH(atari_palette));
}


static const pokey_interface pokey_config = {
	{ DEVCB_NULL },
	DEVCB_NULL,
	DEVCB_NULL,DEVCB_NULL,
	atari_interrupt_cb
};

static MACHINE_DRIVER_START( a600xl )
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

	MDRV_PALETTE_LENGTH(ARRAY_LENGTH(atari_palette))
	MDRV_PALETTE_INIT(atari)
	MDRV_DEFAULT_LAYOUT(layout_maxaflex)

	MDRV_VIDEO_START(atari)
	MDRV_VIDEO_UPDATE(atari)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("pokey", POKEY, FREQ_17_EXACT)
	MDRV_SOUND_CONFIG(pokey_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("speaker", SPEAKER, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_MACHINE_START( atarixl )
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( maxaflex )
	MDRV_IMPORT_FROM( a600xl )
	MDRV_MACHINE_RESET( supervisor_board )
MACHINE_DRIVER_END

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
