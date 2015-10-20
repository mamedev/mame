// license:BSD-3-Clause
// copyright-holders:Paul Hampson
/**********************************************************************************************************************
 Championship VBall
 Driver by Paul "TBBle" Hampson

 TODO:
 Needs to be tilemapped. The background layer and sprite layer are identical to spdodgeb, except for the
  back-switched graphics roms and the size of the palette banks.

 03/28/03 - Additions by Steve Ellenoff
 ---------------------------------------

 -Corrected background tiles (tiles are really 512x512 not 256x256 as previously setup)
 -Converted rendering to tilemap system
 -Implemented Scroll Y registers
 -Implemented X Line Scrolling (only seems to be used for displaying Hawaii and Airfield Map Screen)
 -Adjusted visible screen size to match more closely the real game
 -Added support for cocktail mode/flip screen
 -Confirmed the US version uses the oki6295 and does not display the story in attract mode like the JP version
 -Confirmed the Background graphics are contained in that unusual looking dip package on the US board


 Remaining Issues:
 -1) IRQ & NMI code is totally guessed, and needs to be solved properly

Measurements from Guru (someone needs to rewrite INTERRUPT_GEN() in video/vball.c):
6502 /IRQ = 1.720kHz
6202 /NMI = 58 Hz
VBlank = 58Hz


 -2) X Line Scrolling doesn't work 100% when Flip Screen Dip is set
 -3) 2 Player Version - Dips for difficulty don't seem to work or just need more testing

 -4) 2 Player Version - sound ROM is different and the adpcm chip is addressed differently
                        Changed it to use a rom that was dumped from original PCB (readme below),
                        this makes the non-working ROM not used - i don't know where it come from.



  U.S. Championship V'Ball (Japan)
  Technos, 1988

  PCB Layout
  ----------


  TA-0025-P1-02 (M6100357A BEACH VOLLEY 880050B04)
  |---------------------------------------------------------------------|
  |          YM3014  M6295             25J1-0.47   YM2151   3.579545MHz |
  |                1.056MHz  25J0-0.78   Z80       6116                 |
  |                                                                     |
  |                                                                     |
  |                                                                     |
  |                                                                     |
  |    6502 25J2-2-5.124 6116                                           |
  |                                                                     |
  |                    2016                                     12MHz   |
  |J                                                                    |
  |A                                             2016  2016             |
  |M                                                                    |
  |M                                                                    |
  |A                                                                    |
  |  DSW1                              6264     25J4-0.35  25J3-0.5     |
  |  DSW2                                                               |
  |       25J6-0.144                                                    |
  |       25J5-0.143 2016                                               |
  |                       -------------------                           |
  |25J7-0.160             |                 |                           |
  |                       | TOSHIBA  0615   |                           |
  |                  2016 |                 |                           |
  |                       | T5324   TRJ-101 |                           |
  |                       |                 |                           |
  |-----------------------|-----------------|---------------------------|


  Notes:
        6502 clock: 2.000MHz
         Z80 clock: 3.579545MHz
      YM2151 clock: 3.579545MHz
       M6295 clock: 1.056MHz, sample rate = 8kHz (i.e. 1056000/132)


  *********************************************************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/vball.h"

#define MAIN_CLOCK      XTAL_12MHz
#define CPU_CLOCK           MAIN_CLOCK / 6
#define PIXEL_CLOCK     MAIN_CLOCK / 2

/* Based on ddragon driver */
inline int vball_state::scanline_to_vcount(int scanline)
{
	int vcount = scanline + 8;
	if (vcount < 0x100)
		return vcount;
	else
		return (vcount - 0x18) | 0x100;
}

TIMER_DEVICE_CALLBACK_MEMBER(vball_state::vball_scanline)
{
	int scanline = param;
	int screen_height = m_screen->height();
	int vcount_old = scanline_to_vcount((scanline == 0) ? screen_height - 1 : scanline - 1);
	int vcount = scanline_to_vcount(scanline);

	/* Update to the current point */
	if (scanline > 0)
	{
		m_screen->update_partial(scanline - 1);
	}

	/* IRQ fires every on every 8th scanline */
	if (!(vcount_old & 8) && (vcount & 8))
	{
		m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	}

	/* NMI fires on scanline 248 (VBL) and is latched */
	if (vcount == 0xf8)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}

	/* Save the scroll x register value */
	if (scanline < 256)
	{
		m_scrollx[255 - scanline] = (m_scrollx_hi + m_scrollx_lo + 4);
	}
}

WRITE8_MEMBER(vball_state::irq_ack_w)
{
	if (offset == 0)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	else
		m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}


/* bit 0 = bank switch
   bit 1 = ?
   bit 2 = ?
   bit 3 = ?
   bit 4 = ?
   bit 5 = graphics tile offset
   bit 6 = scroll y hi
   bit 7 = ?
*/
WRITE8_MEMBER(vball_state::bankswitch_w)
{
	membank("mainbank")->set_entry(data & 1);

	if (m_gfxset != ((data  & 0x20) ^ 0x20))
	{
		m_gfxset = (data  & 0x20) ^ 0x20;
			m_bg_tilemap->mark_all_dirty();
	}
	m_scrolly_hi = (data & 0x40) << 2;
}

/* The sound system comes all but verbatim from Double Dragon */
WRITE8_MEMBER(vball_state::cpu_sound_command_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


/* bit 0 = flip screen
   bit 1 = scrollx hi
   bit 2 = bg prom bank
   bit 3 = bg prom bank
   bit 4 = bg prom bank
   bit 5 = sp prom bank
   bit 6 = sp prom bank
   bit 7 = sp prom bank
*/
WRITE8_MEMBER(vball_state::scrollx_hi_w)
{
	flip_screen_set(~data&1);
	m_scrollx_hi = (data & 0x02) << 7;
	bgprombank_w((data >> 2) & 0x07);
	spprombank_w((data >> 5) & 0x07);
	//logerror("%04x: scrollx_hi = %d\n", space.device().safe_pcbase(), m_scrollx_hi);
}

WRITE8_MEMBER(vball_state::scrollx_lo_w)
{
	m_scrollx_lo = data;
	//logerror("%04x: scrollx_lo =%d\n", space.device().safe_pcbase(), m_scrollx_lo);
}


//Cheaters note: Scores are stored in ram @ 0x57-0x58 (though the space is used for other things between matches)
static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, vball_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x08ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x1000, 0x1000) AM_READ_PORT("P1")
	AM_RANGE(0x1001, 0x1001) AM_READ_PORT("P2")
	AM_RANGE(0x1002, 0x1002) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1003, 0x1003) AM_READ_PORT("DSW1")
	AM_RANGE(0x1004, 0x1004) AM_READ_PORT("DSW2")
	AM_RANGE(0x1005, 0x1005) AM_READ_PORT("P3")
	AM_RANGE(0x1006, 0x1006) AM_READ_PORT("P4")
	AM_RANGE(0x1008, 0x1008) AM_WRITE(scrollx_hi_w)
	AM_RANGE(0x1009, 0x1009) AM_WRITE(bankswitch_w)
	AM_RANGE(0x100a, 0x100b) AM_WRITE(irq_ack_w)  /* is there a scanline counter here? */
	AM_RANGE(0x100c, 0x100c) AM_WRITE(scrollx_lo_w)
	AM_RANGE(0x100d, 0x100d) AM_WRITE(cpu_sound_command_w)
	AM_RANGE(0x100e, 0x100e) AM_WRITEONLY AM_SHARE("scrolly_lo")
	AM_RANGE(0x2000, 0x2fff) AM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x3000, 0x3fff) AM_WRITE(attrib_w) AM_SHARE("attribram")
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("mainbank")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, vball_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8801) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x9800, 0x9803) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xA000, 0xA000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( vball )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2") /* Verified against Taito's US Vball manual */
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c, 0x00, "Single Player Game Time")    PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "1:15")
	PORT_DIPSETTING(    0x04, "1:30")
	PORT_DIPSETTING(    0x0c, "1:45")
	PORT_DIPSETTING(    0x08, "2:00")
	PORT_DIPNAME( 0x30, 0x00, "Start Buttons (4-player)")   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, "Button A")
	PORT_DIPSETTING(    0x10, "Button B")
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x40, 0x40, "PL 1&4 (4-player)")      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Rotate 90")
	PORT_DIPNAME( 0x80, 0x00, "Player Mode")        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "2 Players")
	PORT_DIPSETTING(    0x00, "4 Players")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END

static INPUT_PORTS_START (vball2pj)
	PORT_INCLUDE( vball )

	/* The 2-player roms have the game-time in the difficulty spot, and I've assumed vice-versa. (VS the instructions scanned in Naz's dump) */

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, "Single Player Game Time")    PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1:30")
	PORT_DIPSETTING(    0x01, "1:45")
	PORT_DIPSETTING(    0x03, "2:00")
	PORT_DIPSETTING(    0x02, "2:15")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4") /* Difficulty order needs to be verified */
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Very_Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" ) /* Dips 5 through 8 are used for 4 player mode, not supported in 2 player set */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_MODIFY("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) /* Used in 4 player mode, not supported in 2 player set */

	PORT_MODIFY("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) /* Used in 4 player mode, not supported in 2 player set */
INPUT_PORTS_END

void vball_state::machine_start()
{
	membank("mainbank")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x4000);
}


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ 0*8*8+1, 0*8*8+0, 1*8*8+1, 1*8*8+0, 2*8*8+1, 2*8*8+0, 3*8*8+1, 3*8*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 16*8+3, 16*8+2, 16*8+1, 16*8+0,
			32*8+3, 32*8+2, 32*8+1, 32*8+0, 48*8+3, 48*8+2, 48*8+1, 48*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	64*8
};


static GFXDECODE_START( vb )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 8 )  /* 8x8 chars */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 128, 8 )  /* 16x16 sprites */
GFXDECODE_END


static MACHINE_CONFIG_START( vball, vball_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, CPU_CLOCK)   /* 2 MHz - measured by guru but it makes the game far far too slow ?! */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", vball_state, vball_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, 3579545)  /* 3.579545 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 384, 0, 256, 272, 8, 248)   /* based on ddragon driver */
	MCFG_SCREEN_UPDATE_DRIVER(vball_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", vb)
	MCFG_PALETTE_ADD("palette", 256)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", 3579545)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.60)

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( vball ) /* US version */
	ROM_REGION( 0x18000, "maincpu", 0 ) /* Main CPU */
	ROM_LOAD( "25a2-4.124",   0x10000, 0x08000, CRC(06d0c013) SHA1(e818ae0ffb32bcf97da2651a9b8efbd4859b2f4c) )/* Bankswitched */
	ROM_CONTINUE(         0x08000, 0x08000 ) /* Static code  */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* region#2: music CPU, 64kb */
	ROM_LOAD( "25j1-0.47",    0x00000, 0x8000,  CRC(10ca79ad) SHA1(aad4a09d6745ca0b5665cb00ff7a4e08ea434068) )

	/* the original has the image data stored in a special ceramic embedded package made by Toshiba
	with part number 'TOSHIBA TRJ-101' (which has been dumped using a custom made adapter)
	there are a few bytes different between the bootleg and the original (the original is correct though!) */
	ROM_REGION(0x80000, "gfx1", 0 ) /* fg tiles */
	ROM_LOAD( "trj-101.96",   0x00000, 0x80000, CRC(f343eee4) SHA1(1ce95285631f7ec91fe3f6c3d62b13f565d3816a) )

	ROM_REGION(0x40000, "gfx2", 0 ) /* sprites */
	ROM_LOAD( "25j4-0.35",    0x00000, 0x20000, CRC(877826d8) SHA1(fd77298f9343051f66259dad9127f40afb95f385) ) /* 0,1,2,3 */
	ROM_LOAD( "25j3-0.5",     0x20000, 0x20000, CRC(c6afb4fa) SHA1(6d7c966300ce5fb2094476b393434486965d62b4) ) /* 0,1,2,3 */

	ROM_REGION(0x40000, "oki", 0 ) /* Sound region#1: adpcm */
	ROM_LOAD( "25j0-0.78",    0x00000, 0x20000, CRC(8e04bdbf) SHA1(baafc5033c9442b83cb332c2c453c13117b31a3b) )

	ROM_REGION(0x1000, "proms", 0 ) /* color PROMs */
	ROM_LOAD_NIB_LOW ( "25j5-0.144",   0x0000,  0x00800, CRC(a317240f) SHA1(bd57ad516f7a8ff774276fd26b02dd34659d41ad) )
	ROM_LOAD_NIB_HIGH( "25j6-0.143",   0x0000,  0x00800, CRC(1ff70b4f) SHA1(a469baa0dda844ba307c09ddefb23f239cfe7b5f) )
	ROM_LOAD(          "25j7-0.160",   0x0800,  0x00800, CRC(2ffb68b3) SHA1(d560fdcd5e5c79d37e5b5bde22fbaf662fe89252) )
ROM_END

ROM_START( vball2pj ) /* Japan version */
	ROM_REGION( 0x18000, "maincpu", 0 ) /* Main CPU */
	ROM_LOAD( "25j2-2-5.124", 0x10000, 0x08000,  CRC(432509c4) SHA1(6de50e21d279f4ac9674bc91990ba9535e80908c) )/* Bankswitched */
	ROM_CONTINUE(         0x08000, 0x08000 ) /* Static code  */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* region#2: music CPU, 64kb */
	ROM_LOAD( "25j1-0.47",    0x00000, 0x8000,  CRC(10ca79ad) SHA1(aad4a09d6745ca0b5665cb00ff7a4e08ea434068) )

	/* the original has the image data stored in a special ceramic embedded package made by Toshiba
	with part number 'TOSHIBA TRJ-101' (which has been dumped using a custom made adapter)
	there are a few bytes different between the bootleg and the original (the original is correct though!) */
	ROM_REGION(0x80000, "gfx1", 0 ) /* fg tiles */
	ROM_LOAD( "trj-101.96",   0x00000, 0x80000, CRC(f343eee4) SHA1(1ce95285631f7ec91fe3f6c3d62b13f565d3816a) )

	ROM_REGION(0x40000, "gfx2", 0 ) /* sprites */
	ROM_LOAD( "25j4-0.35",    0x00000, 0x20000, CRC(877826d8) SHA1(fd77298f9343051f66259dad9127f40afb95f385) ) /* 0,1,2,3 */
	ROM_LOAD( "25j3-0.5",     0x20000, 0x20000, CRC(c6afb4fa) SHA1(6d7c966300ce5fb2094476b393434486965d62b4) ) /* 0,1,2,3 */

	ROM_REGION(0x40000, "oki", 0 ) /* Sound region#1: adpcm */
	ROM_LOAD( "25j0-0.78",    0x00000, 0x20000, CRC(8e04bdbf) SHA1(baafc5033c9442b83cb332c2c453c13117b31a3b) )

	ROM_REGION(0x1000, "proms", 0 ) /* color PROMs */
	ROM_LOAD_NIB_LOW ( "25j5-0.144",   0x0000,  0x00800, CRC(a317240f) SHA1(bd57ad516f7a8ff774276fd26b02dd34659d41ad) )
	ROM_LOAD_NIB_HIGH( "25j6-0.143",   0x0000,  0x00800, CRC(1ff70b4f) SHA1(a469baa0dda844ba307c09ddefb23f239cfe7b5f) )
	ROM_LOAD(          "25j7-0.160",   0x0800,  0x00800, CRC(2ffb68b3) SHA1(d560fdcd5e5c79d37e5b5bde22fbaf662fe89252) )
ROM_END

ROM_START( vballb ) /* bootleg */
	ROM_REGION( 0x18000, "maincpu", 0 ) /* Main CPU: 64k for code */
	ROM_LOAD( "vball.124",    0x10000, 0x08000, CRC(be04c2b5) SHA1(40fed4ae272719e940f1796ef35420ab451ab7b6) )/* Bankswitched */
	ROM_CONTINUE(         0x08000, 0x08000 ) /* Static code  */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* region#2: music CPU, 64kb */
	ROM_LOAD( "25j1-0.47",    0x00000, 0x8000,  CRC(10ca79ad) SHA1(aad4a09d6745ca0b5665cb00ff7a4e08ea434068) )

	/* The bootlegs used standard roms on a daughter card that plugs into the socket for the TOSHIBA TRJ-101 dip rom */
	ROM_REGION(0x80000, "gfx1", 0 )  /* fg tiles */
	ROM_LOAD( "13", 0x00000, 0x10000, CRC(f26df8e1) SHA1(72186c1430d07c7fd9211245b539f05a0660bebe) ) /* 0,1,2,3 */
	ROM_LOAD( "14", 0x10000, 0x10000, CRC(c9798d0e) SHA1(ec156f6c7ecccaa216ce8076f75ad7627ee90945) ) /* 0,1,2,3 */
	ROM_LOAD( "15", 0x20000, 0x10000, CRC(68e69c4b) SHA1(9870674c91cab7215ad8ed40eb82facdee478fde) ) /* 0,1,2,3 */
	ROM_LOAD( "16", 0x30000, 0x10000, CRC(936457ba) SHA1(1662bbd777fcd33a298d192a3f06681809b9d049) ) /* 0,1,2,3 */
	ROM_LOAD( "9",  0x40000, 0x10000, CRC(42874924) SHA1(a75eed7934e089f035000b7f35f6ba8dd96f1e98) ) /* 0,1,2,3 */
	ROM_LOAD( "10", 0x50000, 0x10000, CRC(6cc676ee) SHA1(6e8c590946211baa9266b19b871f252829057696) ) /* 0,1,2,3 */
	ROM_LOAD( "11", 0x60000, 0x10000, CRC(4754b303) SHA1(8630f077b542590ef1340a2f0a6b94086ff91c40) ) /* 0,1,2,3 */
	ROM_LOAD( "12", 0x70000, 0x10000, CRC(21294a84) SHA1(b36ea9ddf6879443d3104241997fa0f916856528) ) /* 0,1,2,3 */

	ROM_REGION(0x40000, "gfx2", 0 ) /* sprites */
	ROM_LOAD( "vball.35",     0x00000, 0x20000, CRC(877826d8) SHA1(fd77298f9343051f66259dad9127f40afb95f385) ) /* 0,1,2,3 == 25j4-0.35 */
	ROM_LOAD( "vball.5",      0x20000, 0x20000, CRC(c6afb4fa) SHA1(6d7c966300ce5fb2094476b393434486965d62b4) ) /* 0,1,2,3 == 25j3-0.5  */

	ROM_REGION(0x40000, "oki", 0 ) /* Sound region#1: adpcm */
	ROM_LOAD( "vball.78a",    0x00000, 0x10000, CRC(f3e63b76) SHA1(da54d1d7d7d55b73e49991e4363bc6f46e0f70eb) ) /* == 1st half of 25j0-0.78 */
	ROM_LOAD( "vball.78b",    0x10000, 0x10000, CRC(7ad9d338) SHA1(3e3c270fa69bda93b03f07a54145eb5e211ec8ba) ) /* == 2nd half of 25j0-0.78 */

	ROM_REGION(0x1000, "proms", 0 ) /* color PROMs */
	ROM_LOAD_NIB_LOW ( "25j5-0.144",   0x0000,  0x00800, CRC(a317240f) SHA1(bd57ad516f7a8ff774276fd26b02dd34659d41ad) )
	ROM_LOAD_NIB_HIGH( "25j6-0.143",   0x0000,  0x00800, CRC(1ff70b4f) SHA1(a469baa0dda844ba307c09ddefb23f239cfe7b5f) )
	ROM_LOAD(          "25j7-0.160",   0x0800,  0x00800, CRC(2ffb68b3) SHA1(d560fdcd5e5c79d37e5b5bde22fbaf662fe89252) )
ROM_END

ROM_START( vball2pjb ) /* bootleg of the Japan set with unmoddified program rom */
	ROM_REGION( 0x18000, "maincpu", 0 ) /* Main CPU: 64k for code */
	ROM_LOAD( "1.124", 0x10000, 0x08000, CRC(432509c4) SHA1(6de50e21d279f4ac9674bc91990ba9535e80908c) )/* Bankswitched, == 25j2-2-5.124 from vball2pj */
	ROM_CONTINUE(      0x08000, 0x08000 ) /* Static code  */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU, 64kb */
	ROM_LOAD( "4.ic47", 0x00000, 0x8000,  CRC(534dfbd9) SHA1(d0cb37caf94fa85da4ebdfe15e7a78109084bf91) )

	/* The bootlegs used standard roms on a daughter card that plugs into the socket for the TOSHIBA TRJ-101 dip rom */
	ROM_REGION(0x80000, "gfx1", 0 )  /* fg tiles */
	ROM_LOAD( "13", 0x00000, 0x10000, CRC(f26df8e1) SHA1(72186c1430d07c7fd9211245b539f05a0660bebe) ) /* 0,1,2,3 */
	ROM_LOAD( "14", 0x10000, 0x10000, CRC(c9798d0e) SHA1(ec156f6c7ecccaa216ce8076f75ad7627ee90945) ) /* 0,1,2,3 */
	ROM_LOAD( "15", 0x20000, 0x10000, CRC(68e69c4b) SHA1(9870674c91cab7215ad8ed40eb82facdee478fde) ) /* 0,1,2,3 */
	ROM_LOAD( "16", 0x30000, 0x10000, CRC(936457ba) SHA1(1662bbd777fcd33a298d192a3f06681809b9d049) ) /* 0,1,2,3 */
	ROM_LOAD( "9",  0x40000, 0x10000, CRC(42874924) SHA1(a75eed7934e089f035000b7f35f6ba8dd96f1e98) ) /* 0,1,2,3 */
	ROM_LOAD( "10", 0x50000, 0x10000, CRC(6cc676ee) SHA1(6e8c590946211baa9266b19b871f252829057696) ) /* 0,1,2,3 */
	ROM_LOAD( "11", 0x60000, 0x10000, CRC(4754b303) SHA1(8630f077b542590ef1340a2f0a6b94086ff91c40) ) /* 0,1,2,3 */
	ROM_LOAD( "12", 0x70000, 0x10000, CRC(21294a84) SHA1(b36ea9ddf6879443d3104241997fa0f916856528) ) /* 0,1,2,3 */

	ROM_REGION(0x40000, "gfx2", 0 ) /* sprites */
	ROM_LOAD( "8", 0x00000, 0x10000, CRC(b18d083c) SHA1(8c7a39b8a9c79a13682a4f283470801c3cbb748c) ) /* == 1st half of 25j4-0.35 */
	ROM_LOAD( "7", 0x10000, 0x10000, CRC(79a35321) SHA1(0953730b1baa9bda4b2eb703258476423e5448f5) ) /* == 2nd half of 25j4-0.35 */
	ROM_LOAD( "6", 0x20000, 0x10000, CRC(49c6aad7) SHA1(6c026ddd97a5dfd138fb65781504f192c11ee6aa) ) /* == 1st half of 25j3-0.5  */
	ROM_LOAD( "5", 0x30000, 0x10000, CRC(9bb95651) SHA1(ec8a481cc7f0d6e469489db7c51103446910ae80) ) /* == 2nd half of 25j3-0.5  */

	ROM_REGION(0x40000, "oki", 0 ) /* Sound region#1: adpcm */
	ROM_LOAD( "vball.78a", 0x00000, 0x10000, CRC(f3e63b76) SHA1(da54d1d7d7d55b73e49991e4363bc6f46e0f70eb) ) /* == 1st half of 25j0-0.78    (ROM type 27512) */
	ROM_LOAD( "3.ic79",    0x10000, 0x08000, CRC(d77349ba) SHA1(5ef25636056607fae7a5463957487b53da0dd310) ) /* == 3rd quarter of 25j0-0.78 (ROM type 27256) */

	ROM_REGION(0x1000, "proms", 0 ) /* color PROMs */
	ROM_LOAD_NIB_LOW ( "25j5-0.144",   0x0000,  0x00800, CRC(a317240f) SHA1(bd57ad516f7a8ff774276fd26b02dd34659d41ad) )
	ROM_LOAD_NIB_HIGH( "25j6-0.143",   0x0000,  0x00800, CRC(1ff70b4f) SHA1(a469baa0dda844ba307c09ddefb23f239cfe7b5f) )
	ROM_LOAD(          "25j7-0.160",   0x0800,  0x00800, CRC(2ffb68b3) SHA1(d560fdcd5e5c79d37e5b5bde22fbaf662fe89252) )
ROM_END


GAME( 1988, vball,    0,     vball,    vball, driver_device,    0, ROT0, "Technos Japan", "U.S. Championship V'ball (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, vball2pj, vball, vball,    vball2pj, driver_device, 0, ROT0, "Technos Japan", "U.S. Championship V'ball (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, vballb,   vball, vball,    vball, driver_device,    0, ROT0, "bootleg", "U.S. Championship V'ball (bootleg of US set)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, vball2pjb,vball, vball,    vball, driver_device,    0, ROT0, "bootleg", "U.S. Championship V'ball (bootleg of Japan set)", MACHINE_SUPPORTS_SAVE )
