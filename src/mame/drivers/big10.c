/***************************************************************************

  BIG 10
  ------

  Driver by Angelo Salese, Roberto Fresca & Tomasz Slanina.


****************************************************************************

  Dumper Notes:

  Possibly some kind of gambling game.

  Z80A
  XTAL is 21.?727
  YM2149
  8-position DSW x1
  RAM 6264 x1
  RAM 41464 x4
  unknown SDIP64 chip with welded heatsink! Might be a video chip or MCU?


****************************************************************************

  Dev Notes...

  - Guessed and hooked the Yamaha VDP (SDIP64 IC). Same VDP used on MSX systems.
  - Added v9938 stuff, interrupts, video start, machine reset, input ports,
    DIP switch, ym2149 interface, pre-defined main Xtal and derivatives for
    z80 and ym2149.
  - Added NVRAM, defined half of DIP switches bank (coinage & main game rate).
    Added inputs for coins A, B & C, payout, reset, and service mode.
  - Reorganized the driver.

****************************************************************************

  How to Play:

  - This is actually a Keno game (slightly modified Raffle/Bingo/Tombola game).
  - First off, select the bet amount with the BET button.
  - Then choose between "SELECT 10" button (pseudo-random) or user-defined
    numbers,by pressing the desired number with the numpad then "select"
    (enters the decimals first then the units, if three or more buttons
    are pressed the older pressed buttons are discarded, i.e. press 1234
    then SELECT, 1 and 2 are discarded).
  - Press "CANCEL ALL" to redo the numbering scheme.
  - Once that you are happy with it, press START to begin the extraction of
    winning numbers.
  - If you get at least 2-4 numbers out of 20 extracted numbers, you win a
    prize and you are entitled to do a big/small (double up) sub-game.

***************************************************************************/


#define MASTER_CLOCK		XTAL_21_4772MHz		/* Dumper notes poorly refers to a 21.?727 Xtal. */


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/v9938.h"
#include "machine/nvram.h"


class big10_state : public driver_device
{
public:
	big10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_mux_data;
};


#define VDP_MEM             0x40000


/***************************************
*      Interrupt handling & Video      *
***************************************/

static void big10_vdp_interrupt(running_machine &machine, int i)
{
	cputag_set_input_line (machine, "maincpu", 0, (i ? ASSERT_LINE : CLEAR_LINE));
}

static TIMER_DEVICE_CALLBACK( big10_interrupt )
{
	v9938_interrupt(timer.machine(), 0);
}


static VIDEO_START( big10 )
{
	VIDEO_START_CALL(generic_bitmapped);
	v9938_init (machine, 0, *machine.primary_screen, machine.generic.tmpbitmap, MODEL_V9938, VDP_MEM, big10_vdp_interrupt);
	v9938_reset(0);
}


/*************************************
*           Machine Reset            *
*************************************/

static MACHINE_RESET(big10)
{
	v9938_reset(0);
}


/****************************************
*  Input Ports Demux & Common Routines  *
****************************************/


static WRITE8_DEVICE_HANDLER( mux_w )
{
	big10_state *state = device->machine().driver_data<big10_state>();
	state->m_mux_data = ~data;
}

static READ8_HANDLER( mux_r )
{
	big10_state *state = space->machine().driver_data<big10_state>();
	switch(state->m_mux_data)
	{
		case 1: return input_port_read(space->machine(), "IN1");
		case 2: return input_port_read(space->machine(), "IN2");
		case 4: return input_port_read(space->machine(), "IN3");
	}

	return state->m_mux_data;
}


/**************************************
*             Memory Map              *
**************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(mux_r)			/* present in test mode */
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SYSTEM")	/* coins and service */
	AM_RANGE(0x98, 0x98) AM_WRITE(v9938_0_vram_w) AM_READ(v9938_0_vram_r)
	AM_RANGE(0x99, 0x99) AM_WRITE(v9938_0_command_w) AM_READ(v9938_0_status_r)
	AM_RANGE(0x9a, 0x9a) AM_WRITE(v9938_0_palette_w)
	AM_RANGE(0x9b, 0x9b) AM_WRITE(v9938_0_register_w)
	AM_RANGE(0xa0, 0xa1) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0xa2, 0xa2) AM_DEVREAD("aysnd", ay8910_r) /* Dip-Switches routes here. */
ADDRESS_MAP_END


/**************************************
*            Input Ports              *
**************************************/

static INPUT_PORTS_START( big10 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_TOGGLE	/* Service Mode */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_W) PORT_NAME("Payout")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_IMPULSE(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 )  PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Number 0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON8 )  PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Number 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 )  PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Number 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Number 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Number 4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Number 5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Number 6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Number 7")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_8_PAD)     PORT_NAME("Number 8")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_CODE(KEYCODE_9_PAD)     PORT_NAME("Number 9")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )  PORT_CODE(KEYCODE_F)         PORT_NAME("Flip Flop")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )    PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Select")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )    PORT_CODE(KEYCODE_Z)         PORT_NAME("Select 10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )    PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Cancel All")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )  PORT_CODE(KEYCODE_2)         PORT_NAME("Start")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_CODE(KEYCODE_1)         PORT_NAME("Bet")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_3) PORT_NAME("Double Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_4) PORT_NAME("Take Score")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_A) PORT_NAME("Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_S) PORT_NAME("Small")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Main Game Rate" )			PORT_DIPLOCATION("DSW1:4,3")
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x10, "70%" )
	PORT_DIPSETTING(    0x20, "80%" )
	PORT_DIPSETTING(    0x30, "90%" )
	PORT_DIPNAME( 0xC0, 0xc0, "Coinage (A=1; B=5; C=10)" )	PORT_DIPLOCATION("DSW1:2,1")
	PORT_DIPSETTING(    0x00, "x1" )
	PORT_DIPSETTING(    0x40, "x2" )
	PORT_DIPSETTING(    0x80, "x5" )
	PORT_DIPSETTING(    0xC0, "x10" )

	/* Unconnected, probably missing from the board */
	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/**********************************
*       AY-3-8910 Interface       *
**********************************/

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW2"),
	DEVCB_INPUT_PORT("DSW1"),
	DEVCB_HANDLER(mux_w),
	DEVCB_NULL
};


/**************************************
*           Machine Driver            *
**************************************/

static MACHINE_CONFIG_START( big10, big10_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/6)	/* guess */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io)
	MCFG_TIMER_ADD_SCANLINE("scantimer", big10_interrupt, "screen", 0, 1)

	MCFG_MACHINE_RESET(big10)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512 + 32, (212 + 28) * 2)
	MCFG_SCREEN_VISIBLE_AREA(0, 512 + 32 - 1, 0, (212 + 28) * 2 - 1)
	MCFG_SCREEN_UPDATE(generic_bitmapped)

	MCFG_PALETTE_LENGTH(512)
	MCFG_PALETTE_INIT(v9938)
	MCFG_VIDEO_START(big10)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK/12)	/* guess */
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/**************************************
*              ROM Load               *
**************************************/

ROM_START( big10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1", 0x0000, 0x4000, CRC(03e50455) SHA1(36834d35d037303e8b9e4ce950d22f11a52e9388) )
	ROM_LOAD( "2", 0x4000, 0x4000, CRC(b4626a5f) SHA1(a9b3b9575c657748a7f0b60ec2c7411dad0c83c1) )
	ROM_LOAD( "3", 0x8000, 0x4000, CRC(8d15da74) SHA1(0e114de6fcf79beac800575bfb739e6a6bf35660) )
ROM_END


/**************************************
*           Game Driver(s)            *
**************************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT   ROT    COMPANY    FULLNAME   FLAGS  */
GAME( 198?, big10,    0,      big10,    big10,    0,     ROT0, "<unknown>", "Big 10",   0 )
