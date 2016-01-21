// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli, Roberto Fresca
/************************************************************************

  Super Draw Poker (c) Stern Electronics 1983

  Driver by Pierpaolo Prazzoli.
  Additional work by Angelo Salese & Roberto Fresca.

*************************************************************************

  Notes:

  - According to some original screenshots floating around the net, this game
    uses a weird coloring scheme for the char text that dynamically changes
    the color of an entire column. For now I can only guess about how it
    truly works.

  - An original snap of these can be seen at:
    http://mamedev.emulab.it/kale/fast/files/A00000211628-007.jpg


*************************************************************************


  Updates:

  - Reworked inputs to match the standard poker inputs names/layout.
  - Hooked the payout switch.
  - Hooked a watchdog circuitry, that seems intended to reset
     the game and/or an external device.
  - Added machine start & reset.
  - All clocks pre defined.
  - Added ay8910 interface as a preliminary attempt to analyze the unknown
     port writes when these ports are set as input.
  - Figured out the following DIP switches:
     Auto Bet (No, Yes).
     Allow Raise (No, Yes).
     Double-Up (No, Yes).
     Minimal Winner Hand (Jacks or Better, Two Pair).
     Deal Speed (Slow, Fast).
     Aces Type (Normal Aces, Number 1).
     Cards Deck Type (English cards, French cards).
     Max Bet (5, 10, 15, 20).
  - Added NVRAM support.
  - Reorganized and cleaned-up the driver.


  To do:

  - Needs schematics to check if the current implementation of the
    "global column coloring" is correct.
  - Check unknown read/writes, too many of them.
  - Check the correct CPU and AY8910 clocks from PCB.
  - A workaround for writes to ay8910 ports when these are set as input.


************************************************************************/


#define MASTER_CLOCK    XTAL_12MHz
#define CPU_CLOCK       MASTER_CLOCK/4  /* guess */
#define SND_CLOCK       MASTER_CLOCK/8  /* guess */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"

class supdrapo_state : public driver_device
{
public:
	supdrapo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_col_line(*this, "col_line"),
		m_videoram(*this, "videoram"),
		m_char_bank(*this, "char_bank") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_col_line;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_char_bank;

	UINT8 m_wdog;

	DECLARE_READ8_MEMBER(rng_r);
	DECLARE_WRITE8_MEMBER(wdog8000_w);
	DECLARE_WRITE8_MEMBER(debug8004_w);
	DECLARE_WRITE8_MEMBER(debug7c00_w);
	DECLARE_WRITE8_MEMBER(coinin_w);
	DECLARE_WRITE8_MEMBER(payout_w);
	DECLARE_WRITE8_MEMBER(ay8910_outputa_w);
	DECLARE_WRITE8_MEMBER(ay8910_outputb_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(supdrapo);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*********************************************************************
                           Video Hardware
**********************************************************************/

void supdrapo_state::video_start()
{
}


UINT32 supdrapo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;
	int count;
	int color;

	count = 0;

	for(y = 0; y < 32; y++)
	{
		for(x = 0; x < 32; x++)
		{
			int tile = m_videoram[count] + m_char_bank[count] * 0x100;
			/* Global Column Coloring, GUESS! */
			color = m_col_line[(x*2) + 1] ? (m_col_line[(x*2) + 1] - 1) & 7 : 0;

			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, tile,color, 0, 0, x*8, y*8);

			count++;
		}
	}

	return 0;
}


/*Maybe bit 2 & 3 of the second color prom are intensity bits? */
PALETTE_INIT_MEMBER(supdrapo_state, supdrapo)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int bit0, bit1, bit2 , r, g, b;
	int i;

	for (i = 0; i < 0x100; ++i)
	{
		bit0 = 0;//(color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 0) & 0x01;
		bit2 = (color_prom[0] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;//(color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[0] >> 2) & 0x01;
		bit2 = (color_prom[0] >> 3) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;//(color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0x100] >> 0) & 0x01;
		bit2 = (color_prom[0x100] >> 1) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}


/*********************************************************************
                            R/W Handlers
**********************************************************************/

READ8_MEMBER(supdrapo_state::rng_r)
{
	return machine().rand();
}

WRITE8_MEMBER(supdrapo_state::wdog8000_w)
{
/*  Kind of state watchdog alternating 0x00 & 0x01 writes.
    Used when exit the test mode (writes 2 consecutive 0's).
    Seems to be intended to reset some external device.

  Watchdog: 01
  Watchdog: 00
  Watchdog: 01
  Watchdog: 00
  Watchdog: 01
  Watchdog: 00
  Watchdog: 00 ---> Exit from test mode.

  Debug3: 00
  Watchdog: 00
  Watchdog: 00

  Debug3: 00
  Watchdog: 01
  Watchdog: 00
  Watchdog: 01
  Watchdog: 00
  Watchdog: 01
  Watchdog: 00

*/


	if (m_wdog == data)
	{
		watchdog_reset_w(space, 0, 0);  /* Reset */
	}

	m_wdog = data;
//  logerror("Watchdog: %02X\n", data);
}


WRITE8_MEMBER(supdrapo_state::debug8004_w)
{
/*  Writes 0x00 each time the machine is initialized */

	logerror("debug8004: %02X\n", data);
//  popmessage("written : %02X", data);
}

WRITE8_MEMBER(supdrapo_state::debug7c00_w)
{
/*  This one write 0's constantly when the input test mode is running */
	logerror("debug7c00: %02X\n", data);
}


/*********************************************************************
                         Coin I/O Counters
**********************************************************************/

WRITE8_MEMBER(supdrapo_state::coinin_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x01);  /* Coin In */
}

WRITE8_MEMBER(supdrapo_state::payout_w)
{
	machine().bookkeeping().coin_counter_w(1, data & 0x01);  /* Payout */
}


/*********************************************************************
                        Machine Start & Reset
**********************************************************************/

void supdrapo_state::machine_start()
{
	save_item(NAME(m_wdog));
}


void supdrapo_state::machine_reset()
{
	m_wdog = 1;
}


/*********************************************************************
                              Memory Map
**********************************************************************/

static ADDRESS_MAP_START( sdpoker_mem, AS_PROGRAM, 8, supdrapo_state )
	AM_RANGE(0x0000, 0x4fff) AM_ROM
	AM_RANGE(0x5000, 0x50ff) AM_RAM AM_SHARE("col_line")
	AM_RANGE(0x57ff, 0x57ff) AM_RAM AM_SHARE("col_line")
	AM_RANGE(0x5800, 0x58ff) AM_RAM AM_SHARE("col_line")
	AM_RANGE(0x6000, 0x67ff) AM_RAM //work ram
	AM_RANGE(0x6800, 0x6bff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x6c00, 0x6fff) AM_RAM AM_SHARE("char_bank")
	AM_RANGE(0x7000, 0x7bff) AM_RAM //$7600 seems watchdog
	AM_RANGE(0x7c00, 0x7c00) AM_WRITE(debug7c00_w)
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("IN4") AM_WRITE(wdog8000_w)
	AM_RANGE(0x8001, 0x8001) AM_READ_PORT("IN0")
	AM_RANGE(0x8002, 0x8002) AM_READ_PORT("IN1") AM_WRITE(payout_w)
	AM_RANGE(0x8003, 0x8003) AM_READ_PORT("IN2") AM_WRITE(coinin_w)
	AM_RANGE(0x8004, 0x8004) AM_READ_PORT("IN3") AM_WRITE(debug8004_w)
	AM_RANGE(0x8005, 0x8005) AM_READ_PORT("SW1")
	AM_RANGE(0x8006, 0x8006) AM_READ_PORT("SW2")
	AM_RANGE(0x9000, 0x90ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x9400, 0x9400) AM_READ(rng_r)
	AM_RANGE(0x9800, 0x9801) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
ADDRESS_MAP_END


/*********************************************************************
                             Input Ports
**********************************************************************/

static INPUT_PORTS_START( supdrapo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("P1 Win/Take") PORT_CODE(KEYCODE_4) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("P1 Cancel") PORT_CODE(KEYCODE_N) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )  PORT_NAME("P1 Deal") PORT_CODE(KEYCODE_2) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("P1 Bet/Play") PORT_CODE(KEYCODE_1) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("Coin 4: 10")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("Coin 3:  5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("Coin 2:  2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin 1:  1")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Hold 5") PORT_CODE(KEYCODE_B) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Hold 4") PORT_CODE(KEYCODE_V) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Hold 3") PORT_CODE(KEYCODE_C) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Hold 2") PORT_CODE(KEYCODE_X) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Hold 1") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("P1 Black") PORT_CODE(KEYCODE_A) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Red") PORT_CODE(KEYCODE_S) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("P1 Double Up") PORT_CODE(KEYCODE_3) PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Win/Take") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Cancel") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Deal") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Bet/Play") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Coin 4: 10")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Coin 3:  5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Coin 2:  2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Coin 1:  1")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 5") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 4") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 3") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 2") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 1") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Black") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Red") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Double Up") PORT_PLAYER(2)

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Flip") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Flip") PORT_PLAYER(2)
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x80, "2 Players" )

	PORT_START("SW1") // Bank 1 @ 8A
	PORT_DIPNAME( 0x0f, 0x02, "Payout Percentage" )     PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, "25%" )
	PORT_DIPSETTING(    0x01, "30%" )
	PORT_DIPSETTING(    0x02, "35%" )
	PORT_DIPSETTING(    0x03, "40%" )
	PORT_DIPSETTING(    0x04, "45%" )
	PORT_DIPSETTING(    0x05, "50%" )
	PORT_DIPSETTING(    0x06, "55%" )
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x08, "65%" )
	PORT_DIPSETTING(    0x09, "70%" )
	PORT_DIPSETTING(    0x0a, "75%" )
	PORT_DIPSETTING(    0x0b, "80%" )
	PORT_DIPSETTING(    0x0c, "85%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0e, "95%" )
	PORT_DIPSETTING(    0x0f, "100%" )
	PORT_DIPNAME( 0x30, 0x10, "Maximum Payout Points" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x10, "200" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x30, "1000" )
	PORT_DIPNAME( 0xc0, 0xc0, "Maximum Bet Points" )    PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "15" )
	PORT_DIPSETTING(    0xc0, "30" )

	PORT_START("SW2") // Bank 2 @ 9A
	PORT_DIPNAME( 0x01, 0x01, "Deal Play Last Amount" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Allow Raise" )           PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Red/Black Double-Up" )   PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Minimum Winning Hand" )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Jacks or Better" )
	PORT_DIPSETTING(    0x00, "Two Pair or Better" )
	PORT_DIPNAME( 0x10, 0x10, "Deal Speed" )            PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x10, "Fast" )
	PORT_DIPNAME( 0x20, 0x20, "Flash Buttons" )         PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( French ) )
	PORT_DIPNAME( 0x80, 0x00, "Cards Deck Type" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, "English Cards" )
	PORT_DIPSETTING(    0x80, "French Cards" )
INPUT_PORTS_END


/*********************************************************************
                      Graphics Decode / Layout
**********************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( supdrapo )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
GFXDECODE_END


/*********************************************************************
                         Sound Interface
**********************************************************************/

WRITE8_MEMBER(supdrapo_state::ay8910_outputa_w)
{
//  popmessage("ay8910_outputa_w %02x",data);
}

WRITE8_MEMBER(supdrapo_state::ay8910_outputb_w)
{
//  popmessage("ay8910_outputb_w %02x",data);
}


/*********************************************************************
                           Machine Driver
**********************************************************************/

static MACHINE_CONFIG_START( supdrapo, supdrapo_state )

	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK) /* guess */
	MCFG_CPU_PROGRAM_MAP(sdpoker_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", supdrapo_state,  irq0_line_hold)


	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(supdrapo_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", supdrapo)
	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(supdrapo_state, supdrapo)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, SND_CLOCK)  /* guess */
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(supdrapo_state, ay8910_outputa_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(supdrapo_state, ay8910_outputb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/*********************************************************************
                            ROMs Load
**********************************************************************/

/*
A2-1C   8910
A2-1D   Z80
A1-1E
A1-1H
A3-1J

        A1-4K
        A1-4L
        A1-4N
        A1-4P           A1-9N (6301)
                        A1-9P
*/
ROM_START( supdrapo )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "a2-1c",  0x0000, 0x1000, CRC(b65af689) SHA1(b45cd15ca8f9c931d83a90f3cdbebf218070b195) )
	ROM_LOAD( "a2-1d",  0x1000, 0x1000, CRC(9ccc4347) SHA1(ea8f4d17aaacc7091ca0a66247b55eb12155c9d7) )
	ROM_LOAD( "a1-1e",  0x2000, 0x1000, CRC(44f2b75d) SHA1(615d0acd3f8a109334f415732b6b4fe7b810d91c) ) //a2-1e
	ROM_LOAD( "a1-1h",  0x3000, 0x1000, CRC(9c1a10ff) SHA1(243dd64f0b29f9bed4cfa8ecb801ddd973d9e3c3) )
	ROM_LOAD( "a3-1j",  0x4000, 0x1000, CRC(71c2bf1c) SHA1(cb98bbf88b8a410075a074ec8619c6e703c6c582) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a1-4p",  0x0000, 0x1000, CRC(5ac096cc) SHA1(60173a83d0e60fd4d0eb40b7b4e80a74ac5fb23d) )
	ROM_LOAD( "a1-4n",  0x1000, 0x1000, CRC(6985fac9) SHA1(c6357fe0f042b67f8559ec9da03106d1ff08dc66) )
	ROM_LOAD( "a1-4l",  0x2000, 0x1000, CRC(534f7b94) SHA1(44b83053827b27b9e45f6fc50d3878984ef5c5cc) )
	ROM_LOAD( "a1-4k",  0x3000, 0x1000, CRC(3d881f5b) SHA1(53d8800a098e4393224de0b82f8e516f73fd6b62) )

	ROM_REGION( 0x00200, "proms", 0 )
	ROM_LOAD( "a1-9n",  0x0000, 0x0100, CRC(e62529e3) SHA1(176f2069b0c06c1d088909e81658652af06c8eec) )
	ROM_LOAD( "a1-9p",  0x0100, 0x0100, CRC(a0547746) SHA1(747c8aef5afa26124fe0763e7f96c4ff6be31863) )
ROM_END

/*
------------------------------------------

  Jeutel (bootleg?)

  1x MOSTEK 8236 / MK3880N-IRL / Z80-CPU
  1x SOUND AY-3-8910
  1x X-TAL 12,000
  2x 8 DIP switches banks.

------------------------------------------
*/
ROM_START( supdrapoa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.c1",  0x0000, 0x1000, CRC(63e2775a) SHA1(742e8db5378631fd93a22d2131f9523ee74c03a5) )
	ROM_LOAD( "1.d1",  0x1000, 0x1000, CRC(aa1578de) SHA1(8f1a33864b2c8e09a25c7603522ebfc7e0757d56) )
	ROM_LOAD( "2.e1",  0x2000, 0x1000, CRC(ffe0415c) SHA1(0233d192814ced0b32abd4b7d2e93431a339732f) )
	ROM_LOAD( "3.h1",  0x3000, 0x1000, CRC(1bae52fa) SHA1(f89d48d67e52d0fca51eb23fee2d5cb94afcf7f4) )
	ROM_LOAD( "4.j1",  0x4000, 0x1000, CRC(7af26f63) SHA1(aeeca69ef1c21acae4283183e4b073ec0d303f4a) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "8.p4",  0x0000, 0x1000, CRC(ef0700c5) SHA1(53f49d99f310fdf675e3b7339bdca1115e4a1935) )
	ROM_LOAD( "7.n4",  0x1000, 0x1000, CRC(3f77031b) SHA1(2d282d39ea568aa44af8b56228b6e096c2713a15) )
	ROM_LOAD( "6.l4",  0x2000, 0x1000, CRC(d70cd50e) SHA1(c3e3dcf79f8a25df5b878ef8734a6d0dc22004ba) )
	ROM_LOAD( "5.k4",  0x3000, 0x1000, CRC(34564917) SHA1(90b49fe8a5371159388839d42913352cf58c60e6) )

	ROM_REGION( 0x00200, "proms", 0 )   /* using the color PROMs from the parent set - no reason to think they differ */
	ROM_LOAD( "a1-9n",  0x0000, 0x0100, CRC(e62529e3) SHA1(176f2069b0c06c1d088909e81658652af06c8eec) )
	ROM_LOAD( "a1-9p",  0x0100, 0x0100, CRC(a0547746) SHA1(747c8aef5afa26124fe0763e7f96c4ff6be31863) )
ROM_END

/*
Poker Relance Gamble
EMU Infos dumper    f205v
manufacturer    Valadon Automation

Technical references

CPUs
QTY     Type        clock       position    function
1x      NEC D780C               2c          8-bit Microprocessor - main
1x      AY-3-8910               2a          Programmable Sound Generator - sound
1x      LM380N                  10b         Audio Amplifier - sound
1x      oscillator  12.000MHz   5b

ROMs
QTY     Type                    position    status
9x      ET2732Q                 0-8         dumped
1x      DM74S287N               9n,9p       dumped

RAMs
QTY     Type                    position
8x      MM2114N-3               1k,1l,1m,1n,2f,3f,3h,3j
1x      MWS5101AEL2             2p

Others

1x 22x2 edge connector
1x 31x2 thin edge connector
1x trimmer (volume)(10a)
2x 8x2 switches DIP (8a,9a)

Notes

At 2l there is an empty space with "batt." handwritten on the PCB
At 1p there is an unmarked DIP20 mil.300 chip.

*/

ROM_START( supdrapob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pok0.1c",    0x0000, 0x1000, CRC(b53f0470) SHA1(79003cc957e22d5bde720b6f4caed5481edd2c7e) )
	ROM_LOAD( "pok1.1d",    0x1000, 0x1000, CRC(9797a42d) SHA1(65446317e6f1a2de53dd10146338fb63bd5b0a99) )
	ROM_LOAD( "pok2.1ef",   0x2000, 0x1000, CRC(2b7a5baa) SHA1(dd86bb35692eabc1482768cf0bc082f3e0bd90fe) )
	ROM_LOAD( "pok3.1h",    0x3000, 0x1000, CRC(9c3ea609) SHA1(612f455515f367b7d59608528d06221665da8876) )
	ROM_LOAD( "pok4.1j",    0x4000, 0x1000, CRC(52025ba3) SHA1(923de6110a3608698a31baf552d4854b1053cc0e) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "pok8.4p",    0x0000, 0x1000, CRC(82b387e1) SHA1(d7b7e4f7b5b8082438444ce1fa585917ae737bcf) )
	ROM_LOAD( "pok7.4n",    0x1000, 0x1000, CRC(6ab0ad02) SHA1(86b22ab3ceb69f981aa32247c93411631c33a6e8) )
	ROM_LOAD( "pok6.4lm",   0x2000, 0x1000, CRC(c8eab65c) SHA1(c4d37fed9675d8bb051e6f97e56f27450a24ddb8) )
	ROM_LOAD( "pok5.4k",    0x3000, 0x1000, CRC(2c0bb656) SHA1(aa2f309afcdefda5e40be0a354d0b3e5548c44bb) )

	ROM_REGION( 0x00200, "proms", 0 )
	ROM_LOAD( "dm74s287n.9n",        0x0000, 0x0100, CRC(e62529e3) SHA1(176f2069b0c06c1d088909e81658652af06c8eec) )
	ROM_LOAD( "dm74s287n.9p",        0x0100, 0x0100, CRC(a0547746) SHA1(747c8aef5afa26124fe0763e7f96c4ff6be31863) )
ROM_END


/*********************************************************************
                           Games Drivers
**********************************************************************/

/*    YEAR  NAME       PARENT    MACHINE   INPUT     STATE          INIT  ROT     COMPANY                                           FULLNAME                     FLAGS... */
GAME( 1983, supdrapo,  0,        supdrapo, supdrapo, driver_device, 0,    ROT90, "Valadon Automation (Stern Electronics license)", "Super Draw Poker (set 1)",   MACHINE_SUPPORTS_SAVE )
GAME( 1983, supdrapoa, supdrapo, supdrapo, supdrapo, driver_device, 0,    ROT90, "Valadon Automation / Jeutel",                    "Super Draw Poker (set 2)",   MACHINE_SUPPORTS_SAVE )
GAME( 1983, supdrapob, supdrapo, supdrapo, supdrapo, driver_device, 0,    ROT90, "bootleg",                                        "Super Draw Poker (bootleg)", MACHINE_SUPPORTS_SAVE )
