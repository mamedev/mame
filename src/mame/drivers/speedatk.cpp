// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli
/*****************************************************************************************

Speed Attack! (c) 1984 Seta Kikaku Corp.

driver by Pierpaolo Prazzoli & Angelo Salese, based on early work by David Haywood

TODO:
 - It's possible that there is only one coin chute and not two,needs a real board to know
   more about it.

How to play:
 - A to D selects a card.
 - Turn takes one or more cards into your hand (depends on how many cards you
   putted on the stacks).
 - Left & right puts a card on one of the two stacks.

Notes:
 - According to the text gfx rom, there are also a Taito and a KKK versions out there.

------------------------------------------------------------------------------------------
SPEED ATTACK!
(c)SETA

CPU :Z80 x 1
SOUND   :AY-3-8910 x 1
XTAL    :12MHZ

SETA CUSTOM ?
AC-002 , AC-003

CB1-1   :1C
CB0-2   :1D
CB1-3   :1F
CB0-4   :1H
CB0-5   :7C
CB0-6   :7D
CB0-7   :7E

CB1.BPR :7L TBP18S030
CB2.BPR :6K 82S129

----------------------------------------------------------

DIP SWITCH 8BIT (Default: ALL ON)

SW 1,2 : COIN CREDIT   LL:1-1 HL:1-2 LH:1-5 HH:1-10
SW 3,4 : LEVEL LL:EASY -> LH -> HL -> HH:HARD
SW 5,6 : NOT USE
SW 7   : FLIP SCREEN H:FLIP
SW 8   : TEST MODE H:TEST

   PARTS SIDE | SOLDIER SIDE
  ----------------------------
      GND   | 1|    GND
      GND   | 2|    GND
      +5V   | 3|    +5V
            | 4|
     +12V   | 5|   +12V
  SPEAKER(+)| 6|  SPEAKER(-)
     SYNC   | 7| COIN COUNTER
       B    | 8|  SERVICE
       G    | 9|  COIN SW
       R    |10|
     PD 6   |11|   PS 6 (NOT USE)
     PD 5   |12|   PS 5 (NOT USE)
     PD 4   |13|   PS 4
     PD 3   |14|   PS 3
     PD 1   |15|   PS 1
     PD 2   |16|   PS 2
            |17|
            |18|

PS / PD :  key matrix
*****************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/speedatk.h"

#define MASTER_CLOCK XTAL_12MHz

void speedatk_state::machine_start()
{
	save_item(NAME(m_mux_data));
	save_item(NAME(m_km_status));
	save_item(NAME(m_coin_settings));
	save_item(NAME(m_coin_impulse));
}

UINT8 speedatk_state::iox_key_matrix_calc(UINT8 p_side)
{
	static const char *const keynames[] = { "P1_ROW0", "P1_ROW1", "P2_ROW0", "P2_ROW1" };

	int i, j, t;

	for (i = 0x00 ; i < 0x10 ; i += 8)
	{
		j = (i / 0x08);

		for (t = 0 ; t < 8 ; t ++)
		{
			if (!(ioport(keynames[j+p_side])->read() & ( 1 << t )))
			{
				return (i + t) | (p_side ? 0x20 : 0x00);
			}
		}
	}

	return 0;
}

READ8_MEMBER(speedatk_state::key_matrix_r)
{
	if(m_coin_impulse > 0)
	{
		m_coin_impulse--;
		return 0x80;
	}

	if((ioport("COINS")->read() & 1) || (ioport("COINS")->read() & 2))
	{
		m_coin_impulse = m_coin_settings;
		m_coin_impulse--;
		return 0x80;
	}

	if(m_mux_data != 1 && m_mux_data != 2 && m_mux_data != 4)
		return 0xff; //unknown command

	/* both side checks */
	if(m_mux_data == 1)
	{
		UINT8 p1_side = iox_key_matrix_calc(0);
		UINT8 p2_side = iox_key_matrix_calc(2);

		if(p1_side != 0)
			return p1_side;

		return p2_side;
	}

	/* check individual input side */
	return iox_key_matrix_calc((m_mux_data == 2) ? 0 : 2);
}

WRITE8_MEMBER(speedatk_state::key_matrix_w)
{
	m_mux_data = data;
}

/* Key matrix status,used for coin settings and I don't know what else... */
READ8_MEMBER(speedatk_state::key_matrix_status_r)
{
	/* bit 0: busy flag,active low */
	return (m_km_status & 0xfe) | 1;
}

/*
xxxx ---- command
---- xxxx param
My guess is that the other commands configs the key matrix, it probably needs some tests on the real thing.
1f
3f
41
61
8x coinage setting command
a1
*/
WRITE8_MEMBER(speedatk_state::key_matrix_status_w)
{
	m_km_status = data;
	if((m_km_status & 0xf0) == 0x80) //coinage setting command
		m_coin_settings = m_km_status & 0xf;
}

static ADDRESS_MAP_START( speedatk_mem, AS_PROGRAM, 8, speedatk_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8000) AM_READWRITE(key_matrix_r,key_matrix_w)
	AM_RANGE(0x8001, 0x8001) AM_READWRITE(key_matrix_status_r,key_matrix_status_w)
	AM_RANGE(0x8800, 0x8fff) AM_RAM
	AM_RANGE(0xa000, 0xa3ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xb000, 0xb3ff) AM_RAM AM_SHARE("colorram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( speedatk_io, AS_IO, 8, speedatk_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_WRITE(m6845_w) //h46505 address / data routing
	AM_RANGE(0x24, 0x24) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x40, 0x40) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	//what's 60-6f for? Seems used only in attract mode and read back when a 2p play ends ...
ADDRESS_MAP_END

static INPUT_PORTS_START( speedatk )
	PORT_START("DSW")
	PORT_SERVICE( 0x01, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("P1_ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 C") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 D") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Left") PORT_CODE(KEYCODE_A)

	PORT_START("P1_ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Right") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Turn") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 C")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 D")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Left")

	PORT_START("P2_ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Turn")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
INPUT_PORTS_END

static const gfx_layout charlayout_1bpp =
{
	8,8,
	RGN_FRAC(1,1),
	3,
	{ 0, 0, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout charlayout_3bpp =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( speedatk )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_1bpp,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout_3bpp,   0, 32 )
GFXDECODE_END

WRITE8_MEMBER(speedatk_state::output_w)
{
	m_flip_scr = data & 0x80;

	if((data & 0x7f) != 0x7f)
		logerror("%02x\n",data);
}

static MACHINE_CONFIG_START( speedatk, speedatk_state )

	MCFG_CPU_ADD("maincpu", Z80,MASTER_CLOCK/2) //divider is unknown
	MCFG_CPU_PROGRAM_MAP(speedatk_mem)
	MCFG_CPU_IO_MAP(speedatk_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", speedatk_state,  irq0_line_hold)

	MCFG_WATCHDOG_VBLANK_INIT(8) // timing is unknown

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(speedatk_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MC6845_ADD("crtc", H46505, "screen", MASTER_CLOCK/16)   /* hand tuned to get ~60 fps */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", speedatk)
	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INDIRECT_ENTRIES(16)
	MCFG_PALETTE_INIT_OWNER(speedatk_state, speedatk)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK/4) //divider is unknown
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW"))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(speedatk_state, output_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

ROM_START( speedatk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cb1-1",        0x0000, 0x2000, CRC(df988e05) SHA1(0ec91c5f2e1adf952a4fe7aede591e763773a75b) )
	ROM_LOAD( "cb0-2",        0x2000, 0x2000, CRC(be949154) SHA1(8a594a7ebdc8456290919163f7ea4ccb0d1f4edb) )
	ROM_LOAD( "cb1-3",        0x4000, 0x2000, CRC(741a5949) SHA1(7f7bebd4fb73fef9aa28549d100f632c442ac9b3) )
	ROM_LOAD( "cb0-4",        0x6000, 0x2000, CRC(53a9c0c8) SHA1(cd0fd94411dabf09828c1f629891158c40794127) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cb0-7",        0x0000, 0x2000, CRC(a86007b5) SHA1(8e5cab76c37a8d53e1355000cd1a0a85ffae0e8c) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "cb0-5",        0x0000, 0x2000, CRC(47a966e7) SHA1(fdaa0f88656afc431bae367679ce6298fa962e0f) )
	ROM_LOAD( "cb0-6",        0x2000, 0x2000, CRC(cc1da937) SHA1(1697bb008bfa5c33a282bd470ac39c324eea7509) )
	ROM_COPY( "gfx2",         nullptr, 0x4000, 0x1000 ) /* Fill the blank space with cards gfx */
	ROM_COPY( "gfx1",         0x1000, 0x5000, 0x1000 ) /* Gfx from cb0-7 */

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "cb1.bpr",      0x0000, 0x0020, CRC(a0176c23) SHA1(133fb9eef8a6595cac2dcd7edce4789899a59e84) ) /* color PROM */
	ROM_LOAD( "cb2.bpr",      0x0020, 0x0100, CRC(a604cf96) SHA1(a4ef6e77dcd3abe4c27e8e636222a5ee711a51f5) ) /* lookup table */
ROM_END

GAME( 1984, speedatk, 0, speedatk, speedatk, driver_device, 0, ROT0, "Seta Kikaku Corp.", "Speed Attack! (Japan)", MACHINE_SUPPORTS_SAVE )
