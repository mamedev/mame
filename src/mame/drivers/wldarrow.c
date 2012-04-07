/******************************************************

 Wild Arrow - Meyco Games 1982

 Preliminary driver by
        Tomasz Slanina
        Pierpaolo Prazzoli

 Wild Arrow (c) 1982 Meyco Games
 CPU: 8080A
 RAM: 411A (x48)
 XTal: 20.0

*******************************************************

To initialize battery RAM, go into testmode (F1 -> F2),
and then press the Reset Counters button.

If a game is not turned off properly, eg. exiting MAME
in mid-game, it may run faulty on the next boot.
Enable the Night Switch to prevent this.


TODO:
- improve inputs and lamp outputs

*******************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/dac.h"
#include "machine/nvram.h"


class wldarrow_state : public driver_device
{
public:
	wldarrow_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram_0;
	UINT8 *m_videoram_1;
	UINT8 *m_videoram_2;
	size_t m_videoram_size;
	DECLARE_WRITE8_MEMBER(lights_1_w);
	DECLARE_WRITE8_MEMBER(lights_2_w);
	DECLARE_WRITE8_MEMBER(counter_w);
};


#define NUM_PENS	(8)


/*************************************
 *
 *  Video system
 *
 *************************************/

static void get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
	}
}


static SCREEN_UPDATE_RGB32( wldarrow )
{
	wldarrow_state *state = screen.machine().driver_data<wldarrow_state>();
	pen_t pens[NUM_PENS];
	offs_t offs;

	get_pens(pens);

	for (offs = 0; offs < state->m_videoram_size; offs++)
	{
		int i;

		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data0 = state->m_videoram_0[offs];
		UINT8 data1 = state->m_videoram_1[offs];
		UINT8 data2 = state->m_videoram_2[offs];

		/* weird equations, but it matches every flyer screenshot -
           perhaphs they used a look-up PROM? */
		UINT8 data_r = data0;
		UINT8 data_g = (data2 & ~data0) | (data2 & data1) | (~data2 & ~data1 & data0);
		UINT8 data_b = data0 ^ data1;

		for (i = 0; i < 8; i++)
		{
			UINT8 color = ((data_r >> 5) & 0x04) |
						  ((data_g >> 6) & 0x02) |
						  ((data_b >> 7) & 0x01);

			bitmap.pix32(y, x) = pens[color];

			data_r = data_r << 1;
			data_g = data_g << 1;
			data_b = data_b << 1;

			x = x + 1;
		}
	}

	return 0;
}



/*************************************
 *
 *  Lights and coin counter
 *
 *************************************/

WRITE8_MEMBER(wldarrow_state::lights_1_w)
{
	/* not sure how it is hooked up */
}


WRITE8_MEMBER(wldarrow_state::lights_2_w)
{
	/* not sure how it is hooked up */
}


WRITE8_MEMBER(wldarrow_state::counter_w)
{
	coin_counter_w(machine(), 0, data);
}



/*************************************
 *
 *  Audio system - this is a guess
 *
 *  The software repeatedly writes to
 *  either f000/f0ff or f00f/f0f0,
 *  so I suspect that A3/A4 have
 *  meaning.
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( wldarrow_dac_1_w )
{
	dac_data_w(device, 0x00);
}


static WRITE8_DEVICE_HANDLER( wldarrow_dac_2_w )
{
	dac_data_w(device, 0x55);
}


static WRITE8_DEVICE_HANDLER( wldarrow_dac_3_w )
{
	dac_data_w(device, 0xaa);
}


static WRITE8_DEVICE_HANDLER( wldarrow_dac_4_w )
{
	dac_data_w(device, 0xff);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( wldarrow_map, AS_PROGRAM, 8, wldarrow_state )
	AM_RANGE(0x0000, 0x37ff) AM_ROM
	AM_RANGE(0x3800, 0x3800) AM_READ_PORT("IN0")
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_BASE(m_videoram_0) AM_SIZE(m_videoram_size)
	AM_RANGE(0x6000, 0x7fff) AM_RAM AM_BASE(m_videoram_1)
	AM_RANGE(0x8000, 0x9fff) AM_RAM AM_BASE(m_videoram_2)
	AM_RANGE(0xcd00, 0xcdff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("BITSW") AM_DEVWRITE_LEGACY("dac", wldarrow_dac_1_w)
	AM_RANGE(0xf004, 0xf004) AM_READ_PORT("IN1") AM_WRITE(lights_1_w)
	AM_RANGE(0xf006, 0xf006) AM_READ_PORT("IN2") AM_WRITE(lights_2_w)
	AM_RANGE(0xf008, 0xf008) AM_WRITE(counter_w)
	AM_RANGE(0xf00f, 0xf00f) AM_DEVWRITE_LEGACY("dac", wldarrow_dac_2_w)
	AM_RANGE(0xf0f0, 0xf0f0) AM_DEVWRITE_LEGACY("dac", wldarrow_dac_3_w)
	AM_RANGE(0xf0ff, 0xf0ff) AM_DEVWRITE_LEGACY("dac", wldarrow_dac_4_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( wldarrow )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_DIPNAME( 0x04, 0x00, "Monitor" )
	PORT_DIPSETTING(    0x00, "Color" )
	PORT_DIPSETTING(    0x04, "B/W" )
	PORT_DIPNAME( 0x08, 0x08, "0-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "0-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "0-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "0-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "0-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BITSW")
	PORT_DIPNAME( 0x01, 0x00, "Bit Switch 1" )		PORT_DIPLOCATION("BITSWITCH:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bit Switch 2" )		PORT_DIPLOCATION("BITSWITCH:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bit Switch 3" )		PORT_DIPLOCATION("BITSWITCH:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bit Switch 4" )		PORT_DIPLOCATION("BITSWITCH:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) )	PORT_DIPLOCATION("BITSWITCH:5")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x20, 0x00, "Bit Switch 6" )		PORT_DIPLOCATION("BITSWITCH:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bit Switch 7" )		PORT_DIPLOCATION("BITSWITCH:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Bit Switch 8" )		PORT_DIPLOCATION("BITSWITCH:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Stop Reel 3") /* Skill Stop only? */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Stop Reel 2") /* Skill Stop only? */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stop Reel 1") /* Skill Stop only? */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Spin")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset Counters")
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, "Night Switch" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END


static INPUT_PORTS_START( mdrawpkr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_DIPNAME( 0x04, 0x04, "0-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "0-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "0-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "0-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "0-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "0-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BITSW")
	PORT_DIPNAME( 0x01, 0x00, "Bit Switch 1" )		PORT_DIPLOCATION("BITSWITCH:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bit Switch 2" )		PORT_DIPLOCATION("BITSWITCH:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bit Switch 3" )		PORT_DIPLOCATION("BITSWITCH:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bit Switch 4" )		PORT_DIPLOCATION("BITSWITCH:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Bit Switch 5" )		PORT_DIPLOCATION("BITSWITCH:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bit Switch 6" )		PORT_DIPLOCATION("BITSWITCH:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bit Switch 7" )		PORT_DIPLOCATION("BITSWITCH:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Bit Switch 8" )		PORT_DIPLOCATION("BITSWITCH:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "2-2 BET" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset Counters")
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, "Night Switch" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END


static INPUT_PORTS_START( unkmeyco )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_DIPNAME( 0x04, 0x04, "0-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "0-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "0-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "0-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "0-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "0-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BITSW")
	PORT_DIPNAME( 0x01, 0x00, "Bit Switch 1" )		PORT_DIPLOCATION("BITSWITCH:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bit Switch 2" )		PORT_DIPLOCATION("BITSWITCH:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Bit Switch 3" )		PORT_DIPLOCATION("BITSWITCH:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bit Switch 4" )		PORT_DIPLOCATION("BITSWITCH:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Bit Switch 5" )		PORT_DIPLOCATION("BITSWITCH:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bit Switch 6" )		PORT_DIPLOCATION("BITSWITCH:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bit Switch 7" )		PORT_DIPLOCATION("BITSWITCH:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Bit Switch 8" )		PORT_DIPLOCATION("BITSWITCH:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Split")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stand")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hit")
	PORT_DIPNAME( 0x10, 0x10, "2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset Counters")
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, "Night Switch" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Insurance")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Surrender")
	PORT_DIPNAME( 0x04, 0x04, "3-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Bonus")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Double Down")
	PORT_DIPNAME( 0x20, 0x20, "3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "3-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( wldarrow, wldarrow_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, XTAL_20MHz / 10) // divider guessed
	MCFG_CPU_PROGRAM_MAP(wldarrow_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 32*8-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_STATIC(wldarrow)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( wldarrow )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "a1-v48.8k", 0x0000, 0x0800, CRC(05dd8056) SHA1(556ca28d090cbf1855618ba40fc631523bdfadd5) )
	ROM_LOAD( "a2-v48.7k", 0x0800, 0x0800, CRC(37df3acf) SHA1(a7f7f54af533dd8231bb20c526c053dd99e74863) )
	ROM_LOAD( "a3-v48.6k", 0x1000, 0x0800, CRC(1295cee2) SHA1(61b260eb907ee4bbf1460277d09e3205c1f6d8a0) )
	ROM_LOAD( "a4-v48.5k", 0x1800, 0x0800, CRC(5562614e) SHA1(7cb04d76e987944d385d40515396fc27ba00ae83) )
ROM_END


/*

Meyco Games, unknown PCB, 1981

NOTE TO WHOEVER IS MAME DEVELOPER FOR THIS DRIVER:  This is obviously part of a multiple board set.
The other PCB board(s) were unfound.  If/when the game is identified,
could you please let me know what it is?

Dumper notes:

-Board is approximately 16.5"x14.25", with no daughterboards or satellite add-on boards.

-Board has 8 pin connector and 50 pin connector with no keying pins (see diagram)

-Board has an 20.000MHz crystal @ C4 (see diagram below).

-No dip switchs or sound chips on this board.

-Board has 32 uPD411 RAM chips.

-The 40 pin CPU is missing, Guru feels it will be an 8080 CPU.

-Dump consists of:
--6x2516 @ K3, K4, K5, K6, K7, K8.

   K      J     H       G      F             E      D      C       B       A
  |------------------------------------------------------------------------------|
 1|                                                                              |
 2|                                                                              |
 3|ROM.3k                                    DS0026CN                            |
 4|ROM.4k                                           20.000                      *|
 5|ROM.5k                      N8T26AN                                           |
 6|ROM.6k               C      898-1-R 1.5K  N8T26AN                            &|
 7|ROM.7k               P      N8T26AN                                          &|
 8|ROM.8k               U      N8T26AN                                          &|
 9|                                                                             &|
10|                                                                              |
11|                                                                              |
12|uPD411 uPD411 uPD411 uPD411 uPD411        uPD411 uPD411 uPD411                |
13|uPD411 uPD411 uPD411 uPD411 uPD411        uPD411 uPD411 uPD411                |
14|uPD411 uPD411 uPD411 uPD411 uPD411        uPD411 uPD411 uPD411                |
15|uPD411 uPD411 uPD411 uPD411 uPD411        uPD411 uPD411 uPD411                |
  |------------------------------------------------------------------------------|

* = 8 pin connector
& = 50 pin connector (assuming to other half of PCB)

*/

ROM_START( mdrawpkr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "tms2516.k8", 0x0000, 0x0800, CRC(2e5fc31e) SHA1(5ea01298051bc51250f67305ac8a65b0b94c120f) )
	ROM_LOAD( "tms2516.k7", 0x0800, 0x0800, CRC(baaf874e) SHA1(b7bb476ef873102979ad3252d19a26ee3a31d933) )
	ROM_LOAD( "tms2516.k6", 0x1000, 0x0800, CRC(a0e13c41) SHA1(17f78f91dae64c39f1a39a0b99a081af1d3bed47) )
	ROM_LOAD( "tms2516.k5", 0x1800, 0x0800, CRC(530a48fd) SHA1(e539962d19d884794ece2e426423f6b33d54058d) )
	ROM_LOAD( "tms2516.k4", 0x2000, 0x0800, CRC(bb1bd38a) SHA1(90256991eb1d030dd72e7e6f8d1a7cce22340b42) )
	ROM_LOAD( "tms2516.k3", 0x2800, 0x0800, CRC(30904dc8) SHA1(c82276aa0eb8f48d136ad8c15dd309c9b880c294) )
ROM_END

ROM_START( mdrawpkra )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "meyco.k8", 0x0000, 0x0800, CRC(b4f6994d) SHA1(46fb2784b7b333f6755522fd27741a2fc4a4bc99) )
	ROM_LOAD( "meyco.k7", 0x0800, 0x0800, CRC(8830365d) SHA1(044bc92880b78fa2a6ed5e133b484ac7d34c455a) )
	ROM_LOAD( "meyco.k6", 0x1000, 0x0800, CRC(e1d5d38d) SHA1(523029185e2edc0351fa128d6494a5002cb2d7e7) )
	ROM_LOAD( "meyco.k5", 0x1800, 0x0800, CRC(27c8dbcc) SHA1(996732b16c46460400957b3ed7bc36f537258dd7) )
	ROM_LOAD( "meyco.k4", 0x2000, 0x0800, CRC(e3f18769) SHA1(7c98ca3f8b423200eb51ebe432591a98394ef952) )
	ROM_LOAD( "meyco.k3", 0x2800, 0x0800, CRC(72aee07f) SHA1(a6d6086f3a6181d5111d05ae779c3f7b363c7f14) )
ROM_END

ROM_START( unkmeyco )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "k8", 0x0000, 0x0800, CRC(c6d8a2f7) SHA1(1fcd3ad72a29f832ffaf37edcb74e84e21909496) )
	ROM_LOAD( "k7", 0x0800, 0x0800, CRC(6e30e5ae) SHA1(980096adefce4d4d97607b22f56f3acf246986ed) )
	ROM_LOAD( "k6", 0x1000, 0x0800, CRC(7e2cb0e1) SHA1(3ea8f3c051ba730a39404b718d93bcdd762834c3) )
	ROM_LOAD( "k5", 0x1800, 0x0800, CRC(536c83f9) SHA1(0dc9171c14f96bde68dc3b175abe31fa25753b48) )
	ROM_LOAD( "k4", 0x2000, 0x0800, CRC(178b0f95) SHA1(cea5922f5294958c59cacd42e30399fc329426d9) )
	ROM_LOAD( "k3", 0x2800, 0x0800, CRC(9cd6b843) SHA1(fb9c5c5ba96ebb75dc42e7c891d6da2a8a1ea6c1) )
	ROM_LOAD( "k2", 0x3000, 0x0800, CRC(5f82eafa) SHA1(4f5a4dc773ceae9a69ec532166047867db4ddadf) )
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1982, wldarrow,  0,        wldarrow, wldarrow, 0, ROT0, "Meyco Games", "Wild Arrow (Standard V4.8)",                    GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1984, mdrawpkr,  0,        wldarrow, mdrawpkr, 0, ROT0, "Meyco Games", "Draw Poker - Joker's Wild (Standard)",          GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) // year not shown, but it is in mdrawpkra
GAME( 1984, mdrawpkra, mdrawpkr, wldarrow, mdrawpkr, 0, ROT0, "Meyco Games", "Draw Poker - Joker's Wild (02-11)",             GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1983, unkmeyco,  0,        wldarrow, unkmeyco, 0, ROT0, "Meyco Games", "unknown Meyco blackjack game (Standard 00-05)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
