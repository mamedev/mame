// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/****************************************************************************

Wall Crash by Midcoin (c) 1984


Driver by Jarek Burczynski
2002.12.23




     DIPSW-8     AY-3-8912                               DIPSW-4
                                                         DIPSW-4

                        74s288


                                               WAC1   WAC2   WAC3
                                               (2532) (2532) (2532)
12.288MHz

   +------------+        2114  2114  2114  2114
   + EPOXY WITH +                                +-------+
   + LS08       +      WAC05  WAC1/52   EMPTY    + SMALL +
   +LS240, LS245+      (2764) (2764)    SOCKET   + EPOXY +
   + Z80        +                                +-------+
   +------------+

The bigger Epoxy brick contains three standard 74LSxxx chips and is used as
DATA lines decoder for all READS from addresses in range: 0..0x7fff.
The pinout (of the whole brick) is 1:1 Z80 and it can be replaced with
a plain Z80, given that decoded ROMS are put in place of WAC05 and WAC1/52.

The smaller Epoxy contains:
 5 chips (names sanded off...): 20 pins, 8 pins, 14 pins, 16 pins, 16 pins,
 1 resistor: 120 Ohm
 1 probably resistor: measured: 1000 Ohm
 1 diode: standard 1N4148 (info from HIGHWAYMAN)
 4 capacitors: 3 same: blue ones probably 10n , 1 smaller 1.3n (measured by HIGHWAYMAN)
It's mapped as ROM at 0x6000-0x7fff but is NOT accessed by the CPU.
It's also not needed for emulation.


Thanks to Dox for donating PCB.
Thanks to HIGHWAYMAN for providing info on how to get to these epoxies
(heat gun) and for info (very close one) on decoding.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/resnet.h"
#include "sound/ay8910.h"


class wallc_state : public driver_device
{
public:
	wallc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT8> m_videoram;

	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE8_MEMBER(wallc_videoram_w);
	DECLARE_WRITE8_MEMBER(wallc_coin_counter_w);
	DECLARE_DRIVER_INIT(wallc);
	DECLARE_DRIVER_INIT(wallca);
	DECLARE_DRIVER_INIT(sidam);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(wallc);
	UINT32 screen_update_wallc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Wall Crash has one 32 bytes palette PROM, connected to the RGB output this
  way:

  bit 6 -- 330 ohm resistor --+-- 330 ohm pulldown resistor -- RED
  bit 5 -- 220 ohm resistor --/

  bit 4 -- NC

  bit 3 -- 330 ohm resistor --+-- 330 ohm pulldown resistor -- GREEN
  bit 2 -- 220 ohm resistor --/

  bit 1 -- 330 ohm resistor --+--+-- 330 ohm pulldown resistor -- BLUE
  bit 0 -- 220 ohm resistor --/  |
                                 |
  bit 7 -+- diode(~655 Ohm)------/
         \------220 ohm pullup (+5V) resistor


***************************************************************************/

PALETTE_INIT_MEMBER(wallc_state, wallc)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	static const int resistances_rg[2] = { 330, 220 };
	static const int resistances_b[3] = { 655, 330, 220 };
	double weights_r[2], weights_g[2], weights_b[3];

	compute_resistor_weights(0, 255,    -1.0,
			2,  resistances_rg, weights_r,  330,    0,
			2,  resistances_rg, weights_g,  330,    0,
			3,  resistances_b,  weights_b,  330,    655+220);

	for (i = 0;i < palette.entries();i++)
	{
		int bit0,bit1,bit7,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 5) & 0x01;
		bit1 = (color_prom[i] >> 6) & 0x01;
		r = combine_2_weights(weights_r, bit1, bit0);

		/* green component */
		bit0 = (color_prom[i] >> 2) & 0x01;
		bit1 = (color_prom[i] >> 3) & 0x01;
		g = combine_2_weights(weights_g, bit1, bit0);

		/* blue component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit7 = (color_prom[i] >> 7) & 0x01;
		b = combine_3_weights(weights_b, bit7, bit1, bit0);

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}

WRITE8_MEMBER(wallc_state::wallc_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(wallc_state::get_bg_tile_info)
{
	SET_TILE_INFO_MEMBER(0, m_videoram[tile_index] + 0x100, 1, 0);
}

void wallc_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wallc_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS_FLIP_Y,   8, 8, 32, 32);
}

UINT32 wallc_state::screen_update_wallc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

WRITE8_MEMBER(wallc_state::wallc_coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(0,data & 2);
}

static ADDRESS_MAP_START( wallc_map, AS_PROGRAM, 8, wallc_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(wallc_videoram_w) AM_MIRROR(0xc00) AM_SHARE("videoram")   /* 2114, 2114 */
	AM_RANGE(0xa000, 0xa3ff) AM_RAM     /* 2114, 2114 */

	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW1")
	AM_RANGE(0xb200, 0xb200) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xb400, 0xb400) AM_READ_PORT("DIAL")
	AM_RANGE(0xb600, 0xb600) AM_READ_PORT("DSW2")

	AM_RANGE(0xb000, 0xb000) AM_WRITENOP
	AM_RANGE(0xb100, 0xb100) AM_WRITE(wallc_coin_counter_w)
	AM_RANGE(0xb200, 0xb200) AM_WRITENOP
	AM_RANGE(0xb500, 0xb500) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0xb600, 0xb600) AM_DEVWRITE("aysnd", ay8910_device, data_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( wallc )
	PORT_START("SYSTEM")    /* b200 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )    //Right curve button; select current playfield in test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    //not used ?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )   //service?? plays loud,high-pitched sound
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )    //Left curve button; browse playfields in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )  //ok
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )  //ok
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )  //ok
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) //ok

	PORT_START("DIAL")      /* b400 - player position 8 bit analog input - value read is used as position of the player directly - what type of input is that ? DIAL ?*/
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(3) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("DSW1")      /* b000 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life) )        PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "100K/200K/400K/800K" )
	PORT_DIPSETTING(    0x08, "80K/160K/320K/640K" )
	PORT_DIPSETTING(    0x04, "60K/120K/240K/480K" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, "Curve Effect" )          PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "More" )
	PORT_DIPNAME( 0x60, 0x60, "Timer Speed" )           PORT_DIPLOCATION("SW3:2,3")
	PORT_DIPSETTING(    0x60, "Slow" )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, "Fast" )
	PORT_DIPSETTING(    0x00, "Super Fast" )
	PORT_DIPNAME( 0x80, 0x00, "Service" )               PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x80, "Free Play With Level Select" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )

	PORT_START("DSW2")      /* b600 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x30, 0x00, "Coin C" )                PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" ) /* Shown as "Unused" in the manual */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" ) /* Shown as "Unused" in the manual */
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,3),
	3,  /* 3 bits per pixel */
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) }, /* the bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( wallc )
	GFXDECODE_ENTRY( "gfx1", 0     , charlayout, 0, 4 )
GFXDECODE_END

DRIVER_INIT_MEMBER(wallc_state,wallc)
{
	UINT8 c;
	UINT32 i;

	UINT8 *ROM = memregion("maincpu")->base();

	for (i=0; i<0x2000*2; i++)
	{
		c = ROM[ i ] ^ 0x55 ^ 0xff; /* NOTE: this can be shortened but now it fully reflects what the bigger module really does */
		c = BITSWAP8(c, 4,2,6,0,7,1,3,5); /* also swapped inside of the bigger module */
		ROM[ i ] = c;
	}
}

DRIVER_INIT_MEMBER(wallc_state,wallca)
{
	UINT8 c;
	UINT32 i;

	UINT8 *ROM = memregion("maincpu")->base();

	for (i=0; i<0x4000; i++)
	{
		if(i & 0x100)
		{
			c = ROM[ i ] ^ 0x4a;
			c = BITSWAP8(c, 4,7,1,3,2,0,5,6);
		}
		else
		{
			c = ROM[ i ] ^ 0xa5;
			c = BITSWAP8(c, 0,2,3,6,1,5,7,4);
		}

		ROM[ i ] = c;
	}
}



static MACHINE_CONFIG_START( wallc, wallc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 12288000 / 4)  /* 3.072 MHz ? */
	MCFG_CPU_PROGRAM_MAP(wallc_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", wallc_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(wallc_state, screen_update_wallc)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", wallc)
	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(wallc_state, wallc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 12288000 / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wallc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wac05.h7",   0x0000, 0x2000, CRC(ab6e472e) SHA1(a387fec24fb899df349a35d1d3a91e897b074712) )
	ROM_LOAD( "wac1-52.h6", 0x2000, 0x2000, CRC(988eaa6d) SHA1(d5e5dbee6e7e0488fdecfb864198c686cbd5d59c) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "wc1.e3",     0x0000, 0x1000, CRC(ca5c4b53) SHA1(5d2e14fe81cca4ec7dbe0c98eaa26890fca28e58) )
	ROM_LOAD( "wc2.e2",     0x1000, 0x1000, CRC(b7f52a59) SHA1(737e7616d7295762057fbdb69d65c8c1edc773dc) )
	ROM_LOAD( "wc3.e1",     0x2000, 0x1000, CRC(f6854b3a) SHA1(bc1e7f785c338c1afa4ab61c07c61397b3de0b01) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.c2",  0x0000, 0x0020, CRC(83e3e293) SHA1(a98c5e63b688de8d175adb6539e0cdc668f313fd) )
ROM_END

/* this set uses a different encryption, but the decrypted code is the same */
ROM_START( wallca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom4.rom",     0x0000, 0x2000, CRC(ce43af1b) SHA1(c05419cb4aa57c6187b469573a3787d9123c4a05) )
	ROM_LOAD( "rom5.rom",     0x2000, 0x2000, CRC(b789a705) SHA1(2b62b14d1a3ad5eff5b8d502d7891e58379ee820) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "rom3.rom",     0x0800, 0x0800, CRC(6634db73) SHA1(fe6104f974495a250e0cd14c0745eec8e44b8d3a) )
	ROM_LOAD( "rom2.rom",     0x1800, 0x0800, CRC(79f49c2c) SHA1(485fdba5ebdb4c01306f3ef26c992a513aa6b5dc) )
	ROM_LOAD( "rom1.rom",     0x2800, 0x0800, CRC(3884fd4f) SHA1(47254c8828128ac48fc15f05b52fe4d42d4919e7) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.c2",  0x0000, 0x0020, CRC(83e3e293) SHA1(a98c5e63b688de8d175adb6539e0cdc668f313fd) )
ROM_END

ROM_START( brkblast )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fadesa-r0.6m",     0x0000, 0x4000, CRC(4e96ca15) SHA1(87f1a3538712aa3d6c3713b845679dd42a4ba5a4) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "rom3.rom",     0x0800, 0x0800, CRC(6634db73) SHA1(fe6104f974495a250e0cd14c0745eec8e44b8d3a) )
	ROM_LOAD( "rom2.rom",     0x1800, 0x0800, CRC(79f49c2c) SHA1(485fdba5ebdb4c01306f3ef26c992a513aa6b5dc) )
	ROM_LOAD( "rom1.rom",     0x2800, 0x0800, CRC(3884fd4f) SHA1(47254c8828128ac48fc15f05b52fe4d42d4919e7) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.c2",  0x0000, 0x0020, CRC(83e3e293) SHA1(a98c5e63b688de8d175adb6539e0cdc668f313fd) )
ROM_END



/*

It use a epoxy brick like wallc
Inside the brick there are:
- 74245
- 74368
- Pal16r4

74368 is a tristate not, it's used to:
-nagate D0 that goes to the CPU if A15 is low
-nagate D1 that goes to the CPU if A15 is low
-nagate D2 that goes to the CPU if A15 is low
-nagate D3 that goes to the CPU if A15 is low

-negate cpu clk to feed the pal clk ALWAYS
-negate A15 to feed 74245 /EN ALWAYS


The 74245 let pass the data unmodifyed if A15 is high (like wallc)

If A15 is low a Pal16r4 kick in
this chip can modify D2,D3,D4,D5,D6,D7

D0 and D1 are negated from outside to real Z80
D2 and D3 are negated after begin modified by the Pal

Pal input
A1
A3
A6
A15 (Output enable, not in equation)

D2
D3
D4
D5  (2 times)
D7  (2 times)

Pal output
D2 (via not to cpu)
D3 (via not to cpu)
D4 to cpu
D5 to cpu
D6 to cpu
D7 to cpu

*/

ROM_START( sidampkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11600.0",     0x0000, 0x1000, CRC(88cac4d2) SHA1(a369da3dc80671eeff549077cf2ce860d5f4ea35) )
	ROM_LOAD( "11600.1",     0x1000, 0x1000, CRC(96cca320) SHA1(85326f7126c8250a22f35f6eed138051a9ab35cb) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "11605.b",     0x0800, 0x0800, CRC(a7800f8a) SHA1(3955e0f71ced6fd759f52d12c0b39ab6aab31ca4) )
	ROM_LOAD( "11605.g",     0x1800, 0x0800, CRC(b7bebf1e) SHA1(764536989ba4c4c143a61d4453c3bba547bc630a) )
	ROM_LOAD( "11605.r",     0x2800, 0x0800, CRC(4d645b8d) SHA1(d4f8d11c4ef796cf66ebf2e6b8a11247d630951a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "11607-74.288",  0x0000, 0x0020, CRC(e14bf545) SHA1(5e8c5a9ea6e4842f27a47c1d7224ed294bbaa40b) )
ROM_END

DRIVER_INIT_MEMBER(wallc_state,sidam)
{
	UINT8 c;
	UINT32 i;

	UINT8 *ROM = memregion("maincpu")->base();
	int count = 0;

	for (i=0; i<0x2000; i++)
	{
		switch (i & 0x4a)  // A1, A3, A6
		{
			case 0x00:
				logerror("%02x ", ROM[i]);
				count++;
				break;
			case 0x02:
				break;
			case 0x08:
				break;
			case 0x0a:
				break;
			case 0x40:
				break;
			case 0x42:
				break;
			case 0x48:
				break;
			case 0x4a:
				break;
		}



		if (count==16)
		{
			count = 0;
			logerror("\n");
		}

		c = ROM[ i ] ^ 0x0f;
		ROM[ i ] = c;
	}


}

GAME( 1984, wallc,  0,      wallc,  wallc, wallc_state, wallc,  ROT0, "Midcoin", "Wall Crash (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, wallca, wallc,  wallc,  wallc, wallc_state, wallca, ROT0, "Midcoin", "Wall Crash (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, brkblast,wallc, wallc,  wallc, wallc_state, wallca, ROT0, "bootleg (Fadesa)", "Brick Blast (bootleg of Wall Crash)", MACHINE_SUPPORTS_SAVE ) // Spanish bootleg board, Fadesa stickers / text on various components

GAME( 1984, sidampkr,0,     wallc,  wallc, wallc_state, sidam,  ROT270, "Sidam", "unknown Sidam Poker", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
