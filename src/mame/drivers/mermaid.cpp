// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Mermaid
    Sanritsu

    driver by Zsolt Vasvari

    Games supported:
        * Mermaid
        * Rougien

    Known issues:
        * stars playfield colors and scrolling is wrong in Rougien;
        * Dunno where the "alien whistle" sample is supposed to play in Rougien;
        * Mermaid has a ROM for sample playback, identify and hook it up;

Yachtsman
Esco/Sanritsu, 1982

PCB Layout
----------


Lower PCB

C1-0111
|--------------------------------------------------------------|
|                                                              |
|                                col_a.96               RV1    |
|                                                              |
|                                                       RV2    |
|                    2016        col_b.95 HD10124 HD10124      |
|                                                              |
|2114                                                          |
|2114    merv_2.26   2114   mera-0.79 HD10124 HD10124 6116     |
|                                                              |
|                    2114   mera-2.78                    6116  |
|                                                              |
|                           merb-0.77                          |
|                                                              |
|                           merb-2.76                          |
|                                                              |
|                                                              |
|                                 24.576MHz HD10124 HD10124    |
|                                                              |
|                                                   HD10124    |
|                                                              |
|--------------------------------------------------------------|



Top Board

C2-0149
|------------------------------------------|
|Z80     mer-10.24   555    RV4 DSW1(8)    |
|                                          |
|        mer-9.23                          |
|                                          |
|        mer-8.22                          |
|                                          |-|
|   2114 mer-7.21           AY-3-8910        |
|                                            |
|   2114 mer-6.20                            |
|                                            |
|   2114 mer-5.19                            |
|                                            |
|   2114 mer-4.18  mervce.39                 |
|                                            |
|        mer-3.17           AY-3-8910        |
|                                          |-|
|        mer-2.16           RV3            |
|                                          |
|        mer-1.15                          |
|------------------------------------------|

Notes:
         RV1/2 : Potentiometers
         RV3/4 : Potentiometers
      Z80 Clock: 3.072MHz
          Vsync: 60Hz
          HSync: 15.24kHz



Stephh's notes (based on the games Z80 code and some tests) :

1) 'mermaid'

  - Player 2 uses 2nd set of inputs ONLY when "Cabinet" Dip Switch is set to "Cokctail".
  - Continue Play is determined via DSW bit 3. When it's ON, insert a coin when
    the message is displayed on the screen. Beware that there is no visible timer !

2) 'yachtmn'

  - Player 2 uses 2nd set of inputs ONLY when "Cabinet" Dip Switch is set to "Cokctail".
  - Continue Play is always possible provided that you insert a coin when the
    message (much shorter than in 'mermaid') is displayed on the screen (there is
    also no timer). Then when you press START, you need to press START again when
    "YES" is displayed on screen (display switches 3 times between "YES" and "NO").
  - Setting BOTH DSW bits 2 and 3 to ON gives you infinite credits and lives.
    This isn't "Free Play" though as you still need to have one credit first.
    This is done by setting "Bonus Life" Dip Switch to "None".

3) 'rougien'

  - Player 2 ALWAYS uses 2nd set of inputs regardless of "Cabinet" Dip Switch.
  - Continue Play is always possible provided that you insert a coin when the
    message is displayed on the screen (there is a 6 "seconds" timer to do so).
  - Setting BOTH DSW bits 2 and 3 to ON gives you infinite credits and lives.
    This isn't "Free Play" though as you still need to have one credit first.
    This is done by setting "Bonus Life" Dip Switch to "70k" and "Difficulty"
    Dip Switch to "Hard".

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/msm5205.h"
#include "includes/mermaid.h"

/* Read/Write Handlers */

WRITE8_MEMBER(mermaid_state::mermaid_ay8910_write_port_w)
{
	if (m_ay8910_enable[0]) m_ay1->data_w(space, offset, data);
	if (m_ay8910_enable[1]) m_ay2->data_w(space, offset, data);
}

WRITE8_MEMBER(mermaid_state::mermaid_ay8910_control_port_w)
{
	if (m_ay8910_enable[0]) m_ay1->address_w(space, offset, data);
	if (m_ay8910_enable[1]) m_ay2->address_w(space, offset, data);
}


WRITE8_MEMBER(mermaid_state::nmi_mask_w)
{
	m_nmi_mask = data & 1;
}

/* Memory Map */

static ADDRESS_MAP_START( mermaid_map, AS_PROGRAM, 8, mermaid_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xcbff) AM_RAM_WRITE(mermaid_videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(mermaid_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd800, 0xd81f) AM_RAM_WRITE(mermaid_bg_scroll_w) AM_SHARE("bg_scrollram")
	AM_RANGE(0xd840, 0xd85f) AM_RAM_WRITE(mermaid_fg_scroll_w) AM_SHARE("fg_scrollram")
	AM_RANGE(0xd880, 0xd8bf) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xdc00, 0xdfff) AM_RAM_WRITE(mermaid_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("DSW")
	AM_RANGE(0xe000, 0xe001) AM_RAM AM_SHARE("ay8910_enable")
	AM_RANGE(0xe002, 0xe004) AM_WRITENOP // ???
	AM_RANGE(0xe005, 0xe005) AM_WRITE(mermaid_flip_screen_x_w)
	AM_RANGE(0xe006, 0xe006) AM_WRITE(mermaid_flip_screen_y_w)
	AM_RANGE(0xe007, 0xe007) AM_WRITE(nmi_mask_w)
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("P1") AM_WRITENOP // ???
	AM_RANGE(0xe801, 0xe801) AM_WRITENOP    // ???
	AM_RANGE(0xe802, 0xe802) AM_WRITENOP    // ???
	AM_RANGE(0xe803, 0xe803) AM_WRITENOP    // ???
	AM_RANGE(0xe804, 0xe804) AM_WRITE(rougien_gfxbankswitch1_w)
	AM_RANGE(0xe805, 0xe805) AM_WRITE(rougien_gfxbankswitch2_w)
	AM_RANGE(0xe807, 0xe807) AM_WRITENOP    // ???
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("P2")
	AM_RANGE(0xf800, 0xf800) AM_READ(mermaid_collision_r)
	AM_RANGE(0xf802, 0xf802) AM_WRITENOP    // ???
	AM_RANGE(0xf806, 0xf806) AM_WRITE(mermaid_ay8910_write_port_w)
	AM_RANGE(0xf807, 0xf807) AM_WRITE(mermaid_ay8910_control_port_w)
ADDRESS_MAP_END

WRITE8_MEMBER(mermaid_state::rougien_sample_rom_lo_w)
{
	m_adpcm_rom_sel = (data & 1) | (m_adpcm_rom_sel & 2);
}

WRITE8_MEMBER(mermaid_state::rougien_sample_rom_hi_w)
{
	m_adpcm_rom_sel = ((data & 1)<<1) | (m_adpcm_rom_sel & 1);
}

WRITE8_MEMBER(mermaid_state::rougien_sample_playback_w)
{
	if((m_adpcm_play_reg & 1) && ((data & 1) == 0))
	{
		m_adpcm_pos = m_adpcm_rom_sel*0x1000;
		m_adpcm_end = m_adpcm_pos+0x1000;
		m_adpcm_idle = 0;
		m_adpcm->reset_w(0);
	}

	m_adpcm_play_reg = data & 1;
}

static ADDRESS_MAP_START( rougien_map, AS_PROGRAM, 8, mermaid_state )
	AM_RANGE(0xe002, 0xe002) AM_WRITE(rougien_sample_playback_w)
	AM_RANGE(0xe802, 0xe802) AM_WRITE(rougien_sample_rom_hi_w)
	AM_RANGE(0xe803, 0xe803) AM_WRITE(rougien_sample_rom_lo_w)
	AM_IMPORT_FROM( mermaid_map )
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( mermaid )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20k" )
	PORT_DIPSETTING(    0x04, "30k" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )
INPUT_PORTS_END

static INPUT_PORTS_START( yachtmn )
	PORT_INCLUDE( mermaid )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       /* see notes */
	PORT_DIPSETTING(    0x00, "10k" )
	PORT_DIPSETTING(    0x04, "20k" )
	PORT_DIPSETTING(    0x08, "30k" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
INPUT_PORTS_END


/* I know I could have used PORT_INCLUDE macro, but it's easier to understand ports this way - see notes */
static INPUT_PORTS_START( rougien )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) )       /* see notes */
	PORT_DIPSETTING(    0x00, "50k" )
	PORT_DIPSETTING(    0x04, "70k" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )       /* see notes */
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )
INPUT_PORTS_END


/* Graphics Layouts */

static const gfx_layout background_charlayout =
{
	8, 8,    /* 8*8 chars */
	RGN_FRAC(1,1),    /* 256 characters */
	1,      /* 1 bit per pixel */
	{ 0 },  /* single bitplane */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout foreground_charlayout =
{
	8, 8,    /* 8*8 chars */
	RGN_FRAC(1,2),   /* 1024 characters */
	2,      /* 2 bits per pixel */
	{ 0, RGN_FRAC(1,2) },  /* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16, 16, /* 16*16 sprites */
	RGN_FRAC(1,2),  /* 256 sprites */
	2,      /* 2 bits per pixel */
	{ 0, RGN_FRAC(1,2) },   /* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

/* Graphics Decode Information */

static GFXDECODE_START( mermaid )
	GFXDECODE_ENTRY( "gfx1", 0, foreground_charlayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,              0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, background_charlayout,  4*16, 2  )
GFXDECODE_END

/* Sound Interface */

/* Machine Driver */

void mermaid_state::machine_start()
{
	save_item(NAME(m_coll_bit0));
	save_item(NAME(m_coll_bit1));
	save_item(NAME(m_coll_bit2));
	save_item(NAME(m_coll_bit3));
	save_item(NAME(m_coll_bit6));
	save_item(NAME(m_rougien_gfxbank1));
	save_item(NAME(m_rougien_gfxbank2));

	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_end));
	save_item(NAME(m_adpcm_idle));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_adpcm_trigger));
	save_item(NAME(m_adpcm_rom_sel));
	save_item(NAME(m_adpcm_play_reg));
}

void mermaid_state::machine_reset()
{
	m_coll_bit0 = 0;
	m_coll_bit1 = 0;
	m_coll_bit2 = 0;
	m_coll_bit3 = 0;
	m_coll_bit6 = 0;
	m_rougien_gfxbank1 = 0;
	m_rougien_gfxbank2 = 0;

	m_adpcm_idle = 1;
	m_adpcm_rom_sel = 0;
	m_adpcm_play_reg = 0;
}

/* Similar to Jantotsu, apparently the HW has three ports that controls what kind of sample should be played. Every sample size is 0x1000. */
WRITE_LINE_MEMBER(mermaid_state::rougien_adpcm_int)
{
//  popmessage("%08x",m_adpcm_pos);

	if (m_adpcm_pos >= m_adpcm_end || m_adpcm_idle)
	{
		//m_adpcm_idle = 1;
		m_adpcm->reset_w(1);
		m_adpcm_trigger = 0;
	}
	else
	{
		UINT8 *ROM = memregion("adpcm")->base();

		m_adpcm_data = ((m_adpcm_trigger ? (ROM[m_adpcm_pos] & 0x0f) : (ROM[m_adpcm_pos] & 0xf0) >> 4));
		m_adpcm->data_w(m_adpcm_data & 0xf);
		m_adpcm_trigger ^= 1;
		if (m_adpcm_trigger == 0)
		{
			m_adpcm_pos++;
			//if ((ROM[m_adpcm_pos] & 0xff) == 0x70)
			//  m_adpcm_idle = 1;
		}
	}
}

INTERRUPT_GEN_MEMBER(mermaid_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( mermaid, mermaid_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)   // ???
	MCFG_CPU_PROGRAM_MAP(mermaid_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mermaid_state,  vblank_irq)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(mermaid_state, screen_update_mermaid)
	MCFG_SCREEN_VBLANK_DRIVER(mermaid_state, screen_eof_mermaid)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mermaid)
	MCFG_PALETTE_ADD("palette", 4*16+2*2)
	MCFG_PALETTE_INDIRECT_ENTRIES(64+1)
	MCFG_PALETTE_INIT_OWNER(mermaid_state, mermaid)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( rougien, mermaid )

	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(rougien_map)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(mermaid_state,rougien)

	MCFG_SOUND_ADD("adpcm", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(mermaid_state, rougien_adpcm_int))  /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S96_4B)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( mermaid )
	ROM_REGION( 0x10000, "maincpu", 0 )       // 64k for code
	ROM_LOAD( "g960_32.15", 0x0000, 0x1000, CRC(8311f090) SHA1(c59485a712cf1cd384f03874c693b58e972fe4da) )
	ROM_LOAD( "g960_33.16", 0x1000, 0x1000, CRC(9f274fc4) SHA1(4098e98c9d95f7e621de061925374154a23c5d35) )
	ROM_LOAD( "g960_34.17", 0x2000, 0x1000, CRC(5f910179) SHA1(bcf1e24b7584d18f9e85a8b4aec6f03bb1034150) )
	ROM_LOAD( "g960_35.18", 0x3000, 0x1000, CRC(db1868a1) SHA1(f5bb0b9895c5e2facc5ae9db9f1bed44e14d308a) )
	ROM_LOAD( "g960_36.19", 0x4000, 0x1000, CRC(178a3567) SHA1(993479d9fadf1c4d3f44ce030f2d6197ecfceb9d) )
	ROM_LOAD( "g960_37.20", 0x5000, 0x1000, CRC(7d602527) SHA1(1a888bd1829b9f12dd820c49785bea6bc8edab04) )
	ROM_LOAD( "g960_38.21", 0x6000, 0x1000, CRC(bf9f623c) SHA1(48d3aebb01c01c51acaccd1a4582ab21e6ed1104) )
	ROM_LOAD( "g960_39.22", 0x7000, 0x1000, CRC(df0db390) SHA1(b466cf1abbf0703d6fbacc86c65d254ef310ba27) )
	ROM_LOAD( "g960_40.23", 0x8000, 0x1000, CRC(fb7aba3f) SHA1(fe6903c11363ed4c34b29226df58e833150cc525) )
	ROM_LOAD( "g960_41.24", 0x9000, 0x1000, CRC(d022981d) SHA1(ab1659a933af4d49daeacd70072f6c1197181c20) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "g960_45.77", 0x0000, 0x1000, CRC(1f6b735e) SHA1(dd7ea4ef674f0495a87fc1929ea14852e8d8d338) )
	ROM_LOAD( "g960_44.76", 0x1000, 0x1000, CRC(fd76074e) SHA1(673a214fc41b923191b4136c0cf39fc5efa970ba) )
	ROM_LOAD( "g960_47.79", 0x2000, 0x1000, CRC(3b7d4ad0) SHA1(722483989c611b6396538dd3b357589262f366e3) )
	ROM_LOAD( "g960_46.78", 0x3000, 0x1000, CRC(50c117cd) SHA1(45b4055497c785218e2aaaffa86d732912555821) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "g960_43.26", 0x0000, 0x1000, CRC(6f077417) SHA1(f2c20e03427a2f5a113c6a4cf95875b77a0ec418) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "col_a.96",       0x0000, 0x0020, CRC(ef87bcd6) SHA1(00a5888ad028fabeb7369eed33be5cd49b6b7bb0) )
	ROM_LOAD( "col_b.95",       0x0020, 0x0020, CRC(ca48abdd) SHA1(a864612c2c33acddfa9993ed10a1d63d2e3f145d) )

	ROM_REGION( 0x1000, "adpcm", 0 )    // unknown, ADPCM?
	ROM_LOAD( "g960_42.39", 0x0000, 0x1000, CRC(287840bb) SHA1(9a1836f39f328b0c9672976d95a9ece45bb9e89f) )
ROM_END

ROM_START( yachtmn )
	ROM_REGION( 0x10000, "maincpu", 0 )       // 64k for code
	ROM_LOAD( "mer-1.15",   0x0000, 0x1000, CRC(a102b180) SHA1(f1f029797d09d89c98ffc96b1e57f3ab8e89f35a) )
	ROM_LOAD( "mer-2.16",   0x1000, 0x1000, CRC(0f2ba7fc) SHA1(5eac8300eb755f5f3a88776dbc5cf7995d2f3c44) )
	ROM_LOAD( "mer-3.17",   0x2000, 0x1000, CRC(46c22b6b) SHA1(3d6293cf99e9263e986a6046a0f08ee0416a2856) )
	ROM_LOAD( "mer-4.18",   0x3000, 0x1000, CRC(0ec84a12) SHA1(4f2d1509785d659b7e66df0525cbbd3a500370e2) )
	ROM_LOAD( "mer-5.19",   0x4000, 0x1000, CRC(315153d5) SHA1(c3fa4c1e59026e291ddbd448aede159af9827714) )
	ROM_LOAD( "g960_37.20", 0x5000, 0x1000, CRC(7d602527) SHA1(1a888bd1829b9f12dd820c49785bea6bc8edab04) ) // mer-6.20
	ROM_LOAD( "mer-7.21",   0x6000, 0x1000, CRC(20d56a6e) SHA1(b9867f073b38cbf6a98697fe6af6c4cb20d7f54b) )
	ROM_LOAD( "g960_39.22", 0x7000, 0x1000, CRC(df0db390) SHA1(b466cf1abbf0703d6fbacc86c65d254ef310ba27) ) // mer-8.22
	ROM_LOAD( "g960_40.23", 0x8000, 0x1000, CRC(fb7aba3f) SHA1(fe6903c11363ed4c34b29226df58e833150cc525) ) // mer-9.23
	ROM_LOAD( "mer-10.24",  0x9000, 0x1000, CRC(04ca4f8c) SHA1(c7a437fabe3dd6968258f13e688bd6ed8500eb8e) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "g960_45.77", 0x0000, 0x1000, CRC(1f6b735e) SHA1(dd7ea4ef674f0495a87fc1929ea14852e8d8d338) ) // merb-0.77
	ROM_LOAD( "g960_44.76", 0x1000, 0x1000, CRC(fd76074e) SHA1(673a214fc41b923191b4136c0cf39fc5efa970ba) ) // merb-2.76
	ROM_LOAD( "mera-0.79",  0x2000, 0x1000, CRC(6e3e48c4) SHA1(810e140310e668343bc2052e6c9527c090e0aa3c) )
	ROM_LOAD( "g960_46.78", 0x3000, 0x1000, CRC(50c117cd) SHA1(45b4055497c785218e2aaaffa86d732912555821) ) // mera-2.78

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "g960_43.26", 0x0000, 0x1000, CRC(6f077417) SHA1(f2c20e03427a2f5a113c6a4cf95875b77a0ec418) ) // merv_2.26

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "col_a.96",       0x0000, 0x0020, CRC(ef87bcd6) SHA1(00a5888ad028fabeb7369eed33be5cd49b6b7bb0) ) // col_a.96
	ROM_LOAD( "col_b.95",       0x0020, 0x0020, CRC(ca48abdd) SHA1(a864612c2c33acddfa9993ed10a1d63d2e3f145d) ) // col_b.95

	ROM_REGION( 0x1000, "adpcm", 0 )    // unknown, ADPCM?
	ROM_LOAD( "g960_42.39", 0x0000, 0x1000, CRC(287840bb) SHA1(9a1836f39f328b0c9672976d95a9ece45bb9e89f) ) // mervce.39
ROM_END

ROM_START( rougien )
	ROM_REGION( 0x10000, "maincpu", 0 )       // 64k for code
	ROM_LOAD( "rou-00.bin", 0x0000, 0x1000, CRC(14cd1108) SHA1(46657fa4d900936e2a71ad43702d2c43fef09efe) )
	ROM_LOAD( "rou-01.bin", 0x1000, 0x1000, CRC(ee40670d) SHA1(7c31c3b693999bc1ae42b9f2de1a9883d3db535d) )
	ROM_LOAD( "rou-02.bin", 0x2000, 0x1000, CRC(5e528f46) SHA1(6bad10bb72eab423b6478e8d5a41e92f0b72793c) )
	ROM_LOAD( "rou-03.bin", 0x3000, 0x1000, CRC(d235a7e8) SHA1(0c0cbf81d3b379dc5d82c3541dd3b32ccad73046) )
	ROM_LOAD( "rou-04.bin", 0x4000, 0x1000, CRC(7352bb66) SHA1(0bc41faf67d5305258909c9523461cdf8d10d9dc) )
	ROM_LOAD( "rou-05.bin", 0x5000, 0x1000, CRC(af77444d) SHA1(d7c2b59fbaa7be813ed5dcd9d7babc19e0fec9fd) )
	ROM_LOAD( "rou-06.bin", 0x6000, 0x1000, CRC(2c16c857) SHA1(f31189cf73635a1542a18193da85ed898c297768) )
	ROM_LOAD( "rou-07.bin", 0x7000, 0x1000, CRC(fbf32f2e) SHA1(ab8a2bc8ee10e308e887b2bef2b92e1669aab888) )
	ROM_LOAD( "rou-08.bin", 0x8000, 0x1000, CRC(bfac531c) SHA1(63e6bdd1ca2709ae733c84311df5833546f08663) )
	ROM_LOAD( "rou-09.bin", 0x9000, 0x1000, CRC(af854340) SHA1(f2d5e1bb6b25d87ee03c21975f9c9976ae3652b1) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "rou-21.bin", 0x0000, 0x1000, CRC(36e4ba8c) SHA1(6c39de7d983019b280c54e03d4ca0fe2cef4ea90) )
	ROM_LOAD( "rou-20.bin", 0x1000, 0x1000, CRC(c5dc1258) SHA1(20034c77f205684f9c868747988ab391456a2189) )
	ROM_LOAD( "rou-23.bin", 0x2000, 0x1000, CRC(5974c848) SHA1(a3e5408aaee87afadea521115f78686f84832ab9) )
	ROM_LOAD( "rou-22.bin", 0x3000, 0x1000, CRC(35811443) SHA1(3e0ec254a94730664a3d13dd10d87d2040c9c5e6) )
	ROM_LOAD( "rou-25.bin", 0x4000, 0x1000, CRC(706d9864) SHA1(26d7e803670f791938a7e93bf3b68a94525c0458) )
	ROM_LOAD( "rou-24.bin", 0x5000, 0x1000, CRC(56ceb0be) SHA1(a5475ce7d66e9f97da373d3fb694b536a257e78d) )
	ROM_LOAD( "rou-27.bin", 0x6000, 0x1000, CRC(522fa2e0) SHA1(ce20c5e447f27cb147a62c1dd176d9dfa60f4c33) )
	ROM_LOAD( "rou-26.bin", 0x7000, 0x1000, CRC(fbbc6339) SHA1(a4c7035fe61a267b53372a0504e7932e52ac4119) )
	ROM_LOAD( "rou-29.bin", 0x8000, 0x1000, CRC(bf018a7e) SHA1(e608630ead16f01f9b186f622644ce2567f29057) )
	ROM_LOAD( "rou-28.bin", 0x9000, 0x1000, CRC(33f160dc) SHA1(3fc3a31a37cc724c692080edc2e4fd8678e9a8c9) )
	ROM_LOAD( "rou-31.bin", 0xa000, 0x1000, CRC(b2a6f058) SHA1(faba09b6fd80e1e20e79435b60ce89e7110ec98a) )
	ROM_LOAD( "rou-30.bin", 0xb000, 0x1000, CRC(c75be223) SHA1(ca3fa46d9132a31b46e6e29a8b91acf7a380fd74) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "rou-43.bin", 0x0000, 0x1000, CRC(ee4b9de4) SHA1(878a86113435536545353f68864c3a034566c616) )

	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "prom_a.bin", 0x0000, 0x0020, CRC(49f619b9) SHA1(c936aaf79822628a2ffff169d236389bc2eef6a5) )
	ROM_LOAD( "prom_b.bin", 0x0020, 0x0020, CRC(41ad4fc8) SHA1(a9d24586130f00cd350459635de5f4f7629e00b4) )

	ROM_REGION( 0x10000, "adpcm", 0 )   // ADPCM data
	ROM_LOAD( "rou-42.bin", 0x0000, 0x1000, CRC(5ce13444) SHA1(e6da83190b26b094159a3a97deffd31d0d20a061) ) // "rougien" speech
	ROM_LOAD( "rou-41.bin", 0x1000, 0x1000, CRC(59ed0d88) SHA1(7faf6ab01fa3c1c04c38d2ea27b27c47450876de) ) // laugh
	ROM_LOAD( "rou-40.bin", 0x2000, 0x1000, CRC(ab38b942) SHA1(9575f67e002c68d384122e05a12c6c0f21335825) ) // alien whistle
ROM_END


/* Game Drivers */

GAME( 1982, mermaid,  0,        mermaid,  mermaid, driver_device,  0, ROT0, "Sanritsu / Rock-Ola", "Mermaid", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1982, yachtmn,  mermaid,  mermaid,  yachtmn, driver_device,  0, ROT0, "Sanritsu / Esco", "Yachtsman", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1982, rougien,  0,        rougien,  rougien, driver_device,  0, ROT0, "Sanritsu", "Rougien", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
