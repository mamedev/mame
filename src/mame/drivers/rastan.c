/***************************************************************************

Rastan

driver by Jarek Burczynski


custom ICs
----------
PC040DA x3  video DAC
PC050       coin I/O
PC060HS     main/sub CPU communication
PC080       tilemap generator
PC090       sprite generator


memory map
----------
68000:

The address decoding is done by two PALs (IC11 and IC12) which haven't been
read, so the memory map is inferred by program behaviour

Address                  Dir Data             Name      Description
------------------------ --- ---------------- --------- -----------------------
0000000xxxxxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx ROM0      program ROM
0000001xxxxxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx ROM1      program ROM
0000010xxxxxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx ROM2      program ROM
000100--11xxxxxxxxxxxxxx R/W xxxxxxxxxxxxxxxx WLRAM/WURAM work RAM
001000------xxxxxxxxxxx- R/W xxxxxxxxxxxxxxxx CLCS      palette RAM
0011100-------------000- R   --------xxxxxxxx IN PORT   player 1 inputs
0011100-------------001- R   --------xxxxxxxx IN PORT   player 2 inputs
0011100-------------010- R   --------xxxxxxxx IN PORT   extra inputs
0011100-------------011- R   --------xxxxxxxx IN PORT   coin inputs
0011100-------------100- R   --------xxxxxxxx IN PORT   dip SW1
0011100-------------101- R   --------xxxxxxxx IN PORT   dip SW2
0011100-------------110- R   ---------------- n.c.
0011100-------------111- R   ---------------- n.c.
0011100-----------------   W --------xxx----- OUT8-10   sprite palette bank
0011100-----------------   W -----------x---- n.c.
0011100-----------------   W ------------xxxx           PC050 (coin counters, coin lockout)
0011101----------------- R   ---------------- n.c.
0011101-----------------   W --------------x- M INT     [1]
0011101-----------------   W ---------------x SUB RESET [1]
0011110----------------- R   ---------------- n.c.
0011110-----------------   W ----------------           watchdog reset
0011111---------------x- R/W ------------xxxx SNRD/SNWR PC060HS
0011                                          EXT       [1]
11000xxxxxxxxxxxxxxxxxxx R/W xxxxxxxxxxxxxxxx SCN       PC080 PGA
11010000---xxxxxxxxxxxxx R/W xxxxxxxxxxxxxxxx OBJ       PC090 PGA

[1] Not used, goes to external connector, maybe provision for an MCU?


Z80:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
00xxxxxxxxxxxxxx R   xxxxxxxx           program ROM
01xxxxxxxxxxxxxx R   xxxxxxxx           program ROM (banked)
1000xxxxxxxxxxxx R/W xxxxxxxx SRAM      work RAM
1001-----------x R/W xxxxxxxx YM2151    YM2151 [1]
1010-----------x R/W ----xxxx PC6       PC060HS
1011------------   W xxxxxxxx V-ST-ADRS MSM5205 start address (bits 15-8)
1100------------   W -------- START-VCE MSM5205 start
1101------------   W -------- STOP-VCE  MSM5205 stop
1110------------              n.c.
1111------------              n.c.

[1] Schematics also show a YM3526 that can replace the YM2151


Notes:
- For sound communication, we are using code in audio/taitosnd.c, which
  claims to be for the TC0140SYT chip. That chip is not present in Rastan,
  the communication is handled by PC060HS, which I guess is compatible.

TODO:
- Unknown writes to 0x350008.


Stephh's notes (based on the game M68000 code and some tests) :

1) 'rastan' and 'rastanu'

  - Region stored at 0x05fffe.w
  - Sets :
      * 'rastan'  : region = 0x0001
      * 'rastanu' : region = 0x0000
  - These 2 games are 100% the same, only region differs !
    The US version has the copyright message twice in ROM though
  - Coinage relies on the region (code at 0x05ffa2) :
      * 0x0001 (World) uses TAITO_COINAGE_WORLD
      * other uses TAITO_COINAGE_JAPAN_OLD
  - Game name : "RASTAN"
  - No notice screen
  - In "demo mode", you get a scrolling screen with what the various items do
  - No beginning screen when you start a new game
  - Same "YOU ARE A BRAVE FIGHTER ..." message between levels
  - No message after beating level 6 boss
  - No copyright message on scrolling credits screen
  - Game ends after round 6
  - There was sort of debug address at 0x05ff9e.w in ROM area :
      * bits 0 to 2 determine the level (0 to 5)
      * bits 3 and 5 determine where you start at the level !
          . OFF & OFF : part 1
          . OFF & ON  : part 2
          . ON  & OFF : part 3 (boss)
          . ON  & ON  : IMPOSSIBLE !
      * bit 4 doesn't seem to have any effect
      * bit 6 is the invulnerability flag (stored at 0x10c040.w = $40,A5)
        surprisingly, it doesn't work when you fall in the water
      * bit 7 is the infinite energy flag (stored at 0x10c044.w = $44,A5)
    Be aware that the bits are active low !


2) 'rastanua'

  - Region stored at 0x05fffe.w
  - Sets :
      * 'rastanua' : region = 0xffff
  - Game uses TAITO_COINAGE_JAPAN_OLD
  - Game name : "RASTAN"
  - There was sort of debug address at 0x05fffc.w in ROM area
    See 'rastan' comments to know what the different bits do
  - Same other notes as for 'rastan'


3) 'rastsaga' and 'rastsagaa'

  - Region stored at 0x05fffe.w
  - Sets :
      * 'rastsaga'  : region = 0xffff
      * 'rastsagaa' : region = 0xffff
  - Game uses TAITO_COINAGE_JAPAN_OLD
  - Game name : "RASTAN SAGA"
  - Notice screen
  - In "demo mode", you get no scrolling screen with what the various items do
  - Beginning screen when you start a new game
  - Different messages between levels
  - Message after beating level 6 boss
  - Copyright message on scrolling credits screen
  - There was sort of debug address at 0x05fffc.w in ROM area
    See 'rastan' comments to know what the different bits do
  - Different way to handle coins insertion ? See additional code at 0x039f00
  - Same other notes as for 'rastan'

Note: The 'rastsagaa' set's rom numbers were named as RSxx_37 through RSxx_42
      skipping RSxx_41. It's doubtful that Taito would reuse those numbers in
      2 different sets with different data/code. The names have been corrected
      to known files names where the code matched.
***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "includes/taitoipt.h"
#include "video/taitoic.h"
#include "audio/taitosnd.h"
#include "sound/2151intf.h"
#include "sound/msm5205.h"
#include "includes/rastan.h"

static WRITE8_DEVICE_HANDLER( rastan_bankswitch_w )
{
	memory_set_bank(device->machine(),  "bank1", data & 3);
}


static void rastan_msm5205_vck( device_t *device )
{
	rastan_state *state = device->machine().driver_data<rastan_state>();
	if (state->m_adpcm_data != -1)
	{
		msm5205_data_w(device, state->m_adpcm_data & 0x0f);
		state->m_adpcm_data = -1;
	}
	else
	{
		state->m_adpcm_data = device->machine().region("adpcm")->base()[state->m_adpcm_pos];
		state->m_adpcm_pos = (state->m_adpcm_pos + 1) & 0xffff;
		msm5205_data_w(device, state->m_adpcm_data >> 4);
	}
}

WRITE8_MEMBER(rastan_state::rastan_msm5205_address_w)
{
	m_adpcm_pos = (m_adpcm_pos & 0x00ff) | (data << 8);
}

static WRITE8_DEVICE_HANDLER( rastan_msm5205_start_w )
{
	msm5205_reset_w(device, 0);
}

static WRITE8_DEVICE_HANDLER( rastan_msm5205_stop_w )
{
	rastan_state *state = device->machine().driver_data<rastan_state>();
	msm5205_reset_w(device, 1);
	state->m_adpcm_pos &= 0xff00;
}



static ADDRESS_MAP_START( rastan_map, AS_PROGRAM, 16, rastan_state )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x10c000, 0x10ffff) AM_RAM
	AM_RANGE(0x200000, 0x200fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x350008, 0x350009) AM_WRITENOP	/* 0 only (often) ? */
	AM_RANGE(0x380000, 0x380001) AM_WRITE(rastan_spritectrl_w)	/* sprite palette bank, coin counters & lockout */
	AM_RANGE(0x390000, 0x390001) AM_READ_PORT("P1")
	AM_RANGE(0x390002, 0x390003) AM_READ_PORT("P2")
	AM_RANGE(0x390004, 0x390005) AM_READ_PORT("SPECIAL")
	AM_RANGE(0x390006, 0x390007) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x390008, 0x390009) AM_READ_PORT("DSWA")
	AM_RANGE(0x39000a, 0x39000b) AM_READ_PORT("DSWB")
	AM_RANGE(0x3c0000, 0x3c0001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x3e0000, 0x3e0001) AM_READNOP AM_DEVWRITE8_LEGACY("tc0140syt", tc0140syt_port_w, 0x00ff)
	AM_RANGE(0x3e0002, 0x3e0003) AM_DEVREADWRITE8_LEGACY("tc0140syt", tc0140syt_comm_r, tc0140syt_comm_w, 0x00ff)
	AM_RANGE(0xc00000, 0xc0ffff) AM_DEVREADWRITE_LEGACY("pc080sn", pc080sn_word_r, pc080sn_word_w)
	AM_RANGE(0xc20000, 0xc20003) AM_DEVWRITE_LEGACY("pc080sn", pc080sn_yscroll_word_w)
	AM_RANGE(0xc40000, 0xc40003) AM_DEVWRITE_LEGACY("pc080sn", pc080sn_xscroll_word_w)
	AM_RANGE(0xc50000, 0xc50003) AM_DEVWRITE_LEGACY("pc080sn", pc080sn_ctrl_word_w)
	AM_RANGE(0xd00000, 0xd03fff) AM_DEVREADWRITE_LEGACY("pc090oj", pc090oj_word_r, pc090oj_word_w)	/* sprite ram */
ADDRESS_MAP_END


static ADDRESS_MAP_START( rastan_s_map, AS_PROGRAM, 8, rastan_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE_LEGACY("tc0140syt", tc0140syt_slave_port_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREADWRITE_LEGACY("tc0140syt", tc0140syt_slave_comm_r, tc0140syt_slave_comm_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(rastan_msm5205_address_w)
	AM_RANGE(0xc000, 0xc000) AM_DEVWRITE_LEGACY("msm", rastan_msm5205_start_w)
	AM_RANGE(0xd000, 0xd000) AM_DEVWRITE_LEGACY("msm", rastan_msm5205_stop_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( rastan )
	PORT_START("P1")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("P2")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL )                // from PC050 (coin A gets locked if 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )                // from PC050 (coin B gets locked if 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )                // from PC050 (above 2 bits not checked when 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* 0x390008 -> 0x10c018 ($18,A5) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )	/* Normally Demo Sound, but not used */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	TAITO_COINAGE_WORLD_LOC(SW1)

	/* 0x39000a -> 0x10c01c ($1c,A5) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:3,4")           /* table at 0x059f2e */
	PORT_DIPSETTING(    0x0c, "100k 200k 400k 600k 800k" )
	PORT_DIPSETTING(    0x08, "150k 300k 600k 900k 1200k" )
	PORT_DIPSETTING(    0x04, "200k 400k 800k 1200k 1600k" )
	PORT_DIPSETTING(    0x00, "250k 500k 1000k 1500k 2000k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( rastsaga )
	PORT_INCLUDE( rastan )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END



static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, RGN_FRAC(1,2)+0 ,RGN_FRAC(1,2)+4, 8+0, 8+4, RGN_FRAC(1,2)+8+0, RGN_FRAC(1,2)+8+4 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, RGN_FRAC(1,2)+0 ,RGN_FRAC(1,2)+4, 8+0, 8+4, RGN_FRAC(1,2)+8+0, RGN_FRAC(1,2)+8+4,
			16+0, 16+4, RGN_FRAC(1,2)+16+0, RGN_FRAC(1,2)+16+4, 24+0, 24+4, RGN_FRAC(1,2)+24+0, RGN_FRAC(1,2)+24+4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*16
};

static GFXDECODE_START( rastan )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   0, 0x80 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 0x80 )
GFXDECODE_END



/* handler called by the YM2151 emulator when the internal timers cause an IRQ */
static void irqhandler( device_t *device, int irq )
{
	rastan_state *state = device->machine().driver_data<rastan_state>();
	device_set_input_line(state->m_audiocpu, 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2151_interface ym2151_config =
{
	irqhandler,
	rastan_bankswitch_w
};

static const msm5205_interface msm5205_config =
{
	rastan_msm5205_vck,	/* VCK function */
	MSM5205_S48_4B		/* 8 kHz */
};

static MACHINE_START( rastan )
{
	rastan_state *state = machine.driver_data<rastan_state>();
	UINT8 *ROM = machine.region("audiocpu")->base();

	memory_configure_bank(machine, "bank1", 0, 1, &ROM[0x00000], 0x4000);
	memory_configure_bank(machine, "bank1", 1, 3, &ROM[0x10000], 0x4000);

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("audiocpu");
	state->m_pc080sn = machine.device("pc080sn");
	state->m_pc090oj = machine.device("pc090oj");

	state->save_item(NAME(state->m_sprite_ctrl));
	state->save_item(NAME(state->m_sprites_flipscreen));

	state->save_item(NAME(state->m_adpcm_pos));
	state->save_item(NAME(state->m_adpcm_data));
}

static MACHINE_RESET( rastan )
{
	rastan_state *state = machine.driver_data<rastan_state>();

	state->m_sprite_ctrl = 0;
	state->m_sprites_flipscreen = 0;
	state->m_adpcm_pos = 0;
	state->m_adpcm_data = -1;
}


static const pc080sn_interface rastan_pc080sn_intf =
{
	0,	 /* gfxnum */
	0, 0, 0, 0	/* x_offset, y_offset, y_invert, dblwidth */
};

static const pc090oj_interface rastan_pc090oj_intf =
{
	1, 0, 0, 0
};

static const tc0140syt_interface rastan_tc0140syt_intf =
{
	"maincpu", "audiocpu"
};

static MACHINE_CONFIG_START( rastan, rastan_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)	/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(rastan_map)
	MCFG_CPU_VBLANK_INT("screen", irq5_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_16MHz/4)	/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(rastan_s_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))	/* 10 CPU slices per frame - enough for the sound CPU to read all commands */

	MCFG_MACHINE_START(rastan)
	MCFG_MACHINE_RESET(rastan)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_STATIC(rastan)

	MCFG_GFXDECODE(rastan)
	MCFG_PALETTE_LENGTH(8192)

	MCFG_PC080SN_ADD("pc080sn", rastan_pc080sn_intf)
	MCFG_PC090OJ_ADD("pc090oj", rastan_pc090oj_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_16MHz/4)	/* verified on pcb */
	MCFG_SOUND_CONFIG(ym2151_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)

	MCFG_SOUND_ADD("msm", MSM5205, XTAL_384kHz)	/* verified on pcb */
	MCFG_SOUND_CONFIG(msm5205_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	MCFG_TC0140SYT_ADD("tc0140syt", rastan_tc0140syt_intf)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

/* Byte changes in Rev 1 world sets:

 ROM       0x5203  0x520B
-------------------------
B04-43-1    0x00    0x00
B04-43      0x01    0x03
*/
ROM_START( rastan )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "b04-38.19",  0x00000, 0x10000, CRC(1c91dbb1) SHA1(17fc55e8546cc0b847aebd67fb4570a1e9f128f3) )
	ROM_LOAD16_BYTE( "b04-37.7",   0x00001, 0x10000, CRC(ecf20bdd) SHA1(92e46b1edef40a19be17091c09daba598d77bca8) )
	ROM_LOAD16_BYTE( "b04-40.20",  0x20000, 0x10000, CRC(0930d4b3) SHA1(c269b3856040ed9409de99cca48f22a2f355fc4c) )
	ROM_LOAD16_BYTE( "b04-39.8",   0x20001, 0x10000, CRC(d95ade5e) SHA1(f47557dcfa9d3137e2a3838e45858fc21471cc91) )
	ROM_LOAD16_BYTE( "b04-42.21",  0x40000, 0x10000, CRC(1857a7cb) SHA1(7d967d04ade648c6ddb19aad9e184b6e272856da) )
	ROM_LOAD16_BYTE( "b04-43-1.9", 0x40001, 0x10000, CRC(ca4702ff) SHA1(0f8c2d7d332c4e35884c48d87ba9fd26924d1692) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x4000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )
	ROM_CONTINUE(          0x10000, 0xc000 )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD( "b04-03.39", 0x20000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD( "b04-02.67", 0x40000, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD( "b04-04.66", 0x60000, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD( "b04-07.14", 0x20000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD( "b04-06.28", 0x40000, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD( "b04-08.27", 0x60000, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )	/* MSM5205 samples */
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

ROM_START( rastana )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "b04-38.19", 0x00000, 0x10000, CRC(1c91dbb1) SHA1(17fc55e8546cc0b847aebd67fb4570a1e9f128f3) )
	ROM_LOAD16_BYTE( "b04-37.7",  0x00001, 0x10000, CRC(ecf20bdd) SHA1(92e46b1edef40a19be17091c09daba598d77bca8) )
	ROM_LOAD16_BYTE( "b04-40.20", 0x20000, 0x10000, CRC(0930d4b3) SHA1(c269b3856040ed9409de99cca48f22a2f355fc4c) )
	ROM_LOAD16_BYTE( "b04-39.8",  0x20001, 0x10000, CRC(d95ade5e) SHA1(f47557dcfa9d3137e2a3838e45858fc21471cc91) )
	ROM_LOAD16_BYTE( "b04-42.21", 0x40000, 0x10000, CRC(1857a7cb) SHA1(7d967d04ade648c6ddb19aad9e184b6e272856da) )
	ROM_LOAD16_BYTE( "b04-43.9",  0x40001, 0x10000, CRC(c34b9152) SHA1(6ed9247ad455bc3b71d78b541591b269969830cb) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x4000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )
	ROM_CONTINUE(          0x10000, 0xc000 )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD( "b04-03.39", 0x20000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD( "b04-02.67", 0x40000, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD( "b04-04.66", 0x60000, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD( "b04-07.14", 0x20000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD( "b04-06.28", 0x40000, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD( "b04-08.27", 0x60000, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )	/* 64k for the samples */
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) ) /* samples are 4bit ADPCM */
ROM_END

/* Byte changes in Rev 1 US sets:

 ROM       0x5203  0x520B
-------------------------
B04-41-1    0x00    0x00 <-- Same changes as seen with the World set's B04-43-1.9 & B04-43.9
B04-41      0x01    0x03
*/
ROM_START( rastanu ) /* This US set is based on newer code */
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "b04-38.19",  0x00000, 0x10000, CRC(1c91dbb1) SHA1(17fc55e8546cc0b847aebd67fb4570a1e9f128f3) )
	ROM_LOAD16_BYTE( "b04-37.7",   0x00001, 0x10000, CRC(ecf20bdd) SHA1(92e46b1edef40a19be17091c09daba598d77bca8) )
	ROM_LOAD16_BYTE( "b04-45.20",  0x20000, 0x10000, CRC(362812dd) SHA1(f7df037ef423d931ca780b34813d4e9e4db67054) )
	ROM_LOAD16_BYTE( "b04-44.8",   0x20001, 0x10000, CRC(51cc5508) SHA1(2bd266351a4d1b94c8c3a489e4d267695d93db5e) )
	ROM_LOAD16_BYTE( "b04-42.21",  0x40000, 0x10000, CRC(1857a7cb) SHA1(7d967d04ade648c6ddb19aad9e184b6e272856da) )
	ROM_LOAD16_BYTE( "b04-41-1.9", 0x40001, 0x10000, CRC(bd403269) SHA1(14aee828d5efb65370a5e453c8fd1c7b3e718074) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x4000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )
	ROM_CONTINUE(          0x10000, 0xc000 )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD( "b04-03.39", 0x20000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD( "b04-02.67", 0x40000, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD( "b04-04.66", 0x60000, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD( "b04-07.14", 0x20000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD( "b04-06.28", 0x40000, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD( "b04-08.27", 0x60000, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )	/* MSM5205 samples */
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

ROM_START( rastanua ) /* This US set is based on newer code */
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "b04-38.19", 0x00000, 0x10000, CRC(1c91dbb1) SHA1(17fc55e8546cc0b847aebd67fb4570a1e9f128f3) )
	ROM_LOAD16_BYTE( "b04-37.7",  0x00001, 0x10000, CRC(ecf20bdd) SHA1(92e46b1edef40a19be17091c09daba598d77bca8) )
	ROM_LOAD16_BYTE( "b04-45.20", 0x20000, 0x10000, CRC(362812dd) SHA1(f7df037ef423d931ca780b34813d4e9e4db67054) )
	ROM_LOAD16_BYTE( "b04-44.8",  0x20001, 0x10000, CRC(51cc5508) SHA1(2bd266351a4d1b94c8c3a489e4d267695d93db5e) )
	ROM_LOAD16_BYTE( "b04-42.21", 0x40000, 0x10000, CRC(1857a7cb) SHA1(7d967d04ade648c6ddb19aad9e184b6e272856da) )
	ROM_LOAD16_BYTE( "b04-41.9",  0x40001, 0x10000, CRC(b44ca1c4) SHA1(11f1ccc35b6b24aaf253c7994014f08007aba76b) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x4000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )
	ROM_CONTINUE(          0x10000, 0xc000 )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD( "b04-03.39", 0x20000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD( "b04-02.67", 0x40000, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD( "b04-04.66", 0x60000, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD( "b04-07.14", 0x20000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD( "b04-06.28", 0x40000, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD( "b04-08.27", 0x60000, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )	/* MSM5205 samples */
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

/* Byte changes between US & Japan sets for the B04-14/B04-21 & B04-14/B04-13 combo:

 ROM     0x01CB  0x02E2
-----------------------
B04-21    0x1C    0x1C
B04-13    0x44    0x44

These US & Japan sets didn't use a region code byte (see above) so the changes are likely
pointers to the copyright string.
*/
ROM_START( rastanub ) /* This US set is based on the earlier code */
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "b04-14.19", 0x00000, 0x10000, CRC(a38ac909) SHA1(66d792fee03c6bd87d15060b9d5cae74137c5ebd) ) /* This is the correct verfied ROM number */
	ROM_LOAD16_BYTE( "b04-21.7",  0x00001, 0x10000, CRC(7c8dde9a) SHA1(0cfc3b4f3bc7b940a6c07267ac95e4aae25801ea) ) /* 2 bytes differ from the Japan set */
	ROM_LOAD16_BYTE( "b04-23.20", 0x20000, 0x10000, CRC(254b3dce) SHA1(5126cd5268abaa78dfdcd2ca70621c093c79be67) )
	ROM_LOAD16_BYTE( "b04-22.8",  0x20001, 0x10000, CRC(98e8edcf) SHA1(cc89ef36da6d21192efc19c3bbb215b1635b7ef3) )
	ROM_LOAD16_BYTE( "b04-25.21", 0x40000, 0x10000, CRC(d1e5adee) SHA1(eafc275a0023aecb2efaff14cd890915fa162624) )
	ROM_LOAD16_BYTE( "b04-24.9",  0x40001, 0x10000, CRC(a3dcc106) SHA1(3a8854530b08864a1f7f46c427e49ceec8297806) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x4000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )
	ROM_CONTINUE(          0x10000, 0xc000 )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD( "b04-03.39", 0x20000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD( "b04-02.67", 0x40000, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD( "b04-04.66", 0x60000, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD( "b04-07.14", 0x20000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD( "b04-06.28", 0x40000, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD( "b04-08.27", 0x60000, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )	/* MSM5205 samples */
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

ROM_START( rastsaga )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "b04-14.19",   0x00000, 0x10000, CRC(a38ac909) SHA1(66d792fee03c6bd87d15060b9d5cae74137c5ebd) )
	ROM_LOAD16_BYTE( "b04-13.7",    0x00001, 0x10000, CRC(bad60872) SHA1(e020f79b3ac3d2abccfcd5d135d2dc49e1335c7d) )
	ROM_LOAD16_BYTE( "b04-16-1.20", 0x20000, 0x10000, CRC(00b59e60) SHA1(545ab3eb9ef25c532dda5a9eec087665ba0cecc1) )
	ROM_LOAD16_BYTE( "b04-15-1.8",  0x20001, 0x10000, CRC(ff9e018a) SHA1(37048eecec799f29564517fae9525ef0e3d9d9e5) )
	ROM_LOAD16_BYTE( "b04-18-1.21", 0x40000, 0x10000, CRC(b626c439) SHA1(976e820edc4ba107c5b579edaaee1e354e85fb67) )
	ROM_LOAD16_BYTE( "b04-17-1.9",  0x40001, 0x10000, CRC(c928a516) SHA1(fe87fdf2d1b7ba93e1986460eb6af648b58f42e4) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x4000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )
	ROM_CONTINUE(          0x10000, 0xc000 )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD( "b04-03.39", 0x20000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD( "b04-02.67", 0x40000, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD( "b04-04.66", 0x60000, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD( "b04-07.14", 0x20000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD( "b04-06.28", 0x40000, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD( "b04-08.27", 0x60000, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )	/* MSM5205 samples */
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

ROM_START( rastsagaa )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "b04-14.19",   0x00000, 0x10000, CRC(a38ac909) SHA1(66d792fee03c6bd87d15060b9d5cae74137c5ebd) ) /* Dumped as "RS19_38.BIN", corrected to B04-14 */
	ROM_LOAD16_BYTE( "b04-13.7",    0x00001, 0x10000, CRC(bad60872) SHA1(e020f79b3ac3d2abccfcd5d135d2dc49e1335c7d) ) /* Dumped as "RS07_37.BIN", corrected to B04-13 */
	ROM_LOAD16_BYTE( "b04-16.20",   0x20000, 0x10000, CRC(6bcf70dc) SHA1(3e369548ac01981c503150b44c2747e6c2cec12a) ) /* Dumped as "RS20_40.BIN", but is likely B04-16 - Need to verify */
	ROM_LOAD16_BYTE( "b04-15.8",    0x20001, 0x10000, CRC(8838ecc5) SHA1(42b43ab77969bbacdf178fbe73a0a27652ccb297) ) /* Dumped as "RS08_39.BIN", but is likely B04-15 - Need to verify */
	ROM_LOAD16_BYTE( "b04-18-1.21", 0x40000, 0x10000, CRC(b626c439) SHA1(976e820edc4ba107c5b579edaaee1e354e85fb67) ) /* Dumped as "RS21_42.BIN", corrected to B04-18-1 */
	ROM_LOAD16_BYTE( "b04-17-1.9",  0x40001, 0x10000, CRC(c928a516) SHA1(fe87fdf2d1b7ba93e1986460eb6af648b58f42e4) ) /* Dumped as "RS09_43.BIN", corrected to B04-17-1 */

	ROM_REGION( 0x1c000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x4000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )
	ROM_CONTINUE(          0x10000, 0xc000 )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD( "b04-03.39", 0x20000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD( "b04-02.67", 0x40000, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD( "b04-04.66", 0x60000, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD( "b04-07.14", 0x20000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD( "b04-06.28", 0x40000, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD( "b04-08.27", 0x60000, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )	/* MSM5205 samples */
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

/* Newer revised code base */
GAME( 1987, rastan,    0,      rastan, rastan,   0, ROT0, "Taito Corporation Japan",   "Rastan (World Rev 1)", GAME_SUPPORTS_SAVE )
GAME( 1987, rastana,   rastan, rastan, rastsaga, 0, ROT0, "Taito Corporation Japan",   "Rastan (World)", GAME_SUPPORTS_SAVE )
GAME( 1987, rastanu,   rastan, rastan, rastsaga, 0, ROT0, "Taito America Corporation", "Rastan (US Rev 1)", GAME_SUPPORTS_SAVE )
GAME( 1987, rastanua,  rastan, rastan, rastsaga, 0, ROT0, "Taito America Corporation", "Rastan (US)", GAME_SUPPORTS_SAVE )

/* Based on earliest code base */
GAME( 1987, rastanub,  rastan, rastan, rastsaga, 0, ROT0, "Taito America Corporation", "Rastan (US, Earlier code base)", GAME_SUPPORTS_SAVE )
GAME( 1987, rastsaga,  rastan, rastan, rastsaga, 0, ROT0, "Taito Corporation",         "Rastan Saga (Japan Rev 1)", GAME_SUPPORTS_SAVE )
GAME( 1987, rastsagaa, rastan, rastan, rastsaga, 0, ROT0, "Taito Corporation",         "Rastan Saga (Japan)", GAME_SUPPORTS_SAVE )
