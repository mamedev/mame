// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Break Thru Doc. Data East (1986)

    driver by Phil Stroffolino

    UNK-1.1    (16Kb)  Code (4000-7FFF)
    UNK-1.2    (32Kb)  Main 6809 (8000-FFFF)
    UNK-1.3    (32Kb)  Mapped (2000-3FFF)
    UNK-1.4    (32Kb)  Mapped (2000-3FFF)

    UNK-1.5    (32Kb)  Sound 6809 (8000-FFFF)

    Background has 4 banks, with 256 16x16x8 tiles in each bank.
    UNK-1.6    (32Kb)  GFX Background
    UNK-1.7    (32Kb)  GFX Background
    UNK-1.8    (32Kb)  GFX Background

    UNK-1.9    (32Kb)  GFX Sprites
    UNK-1.10   (32Kb)  GFX Sprites
    UNK-1.11   (32Kb)  GFX Sprites

    Text has 256 8x8x4 characters.
    UNK-1.12   (8Kb)   GFX Text

    **************************************************************************
    Memory Map for Main CPU by Carlos A. Lozano
    **************************************************************************

    MAIN CPU
    0000-03FF                                   W                   Plane0
    0400-0BFF                                  R/W                  RAM
    0C00-0FFF                                   W                   Plane2(Background)
    1000-10FF                                   W                   Plane1(sprites)
    1100-17FF                                  R/W                  RAM
    1800-180F                                  R/W                  In/Out
    1810-1FFF                                  R/W                  RAM (unmapped?)
    2000-3FFF                                   R                   ROM Mapped(*)
    4000-7FFF                                   R                   ROM(UNK-1.1)
    8000-FFFF                                   R                   ROM(UNK-1.2)

    Interrupts: Reset, NMI, IRQ
    The test routine is at F000

    Sound: YM2203 and YM3526 driven by 6809.  Sound added by Bryan McPhail, 1/4/98.

    2008-07
    Dip locations verified with manual for brkthru (US conv. kit) and darwin

Darwin 4078 PCB layout
f205v

Darwin 4078
Data East, 1986
----------
|----------------------|
| Fully boxed = socket |
|----------------------|


| separation = solder


upper pcb - 3002A
|-------------------------------------------------|
|t                      |-------|                 |
|o          |---------| |YM2203C|                 |
|           |prom28s42| |-------|                 |
|l          |---------| |YM3526                   |C
|o                      |-----|                   |O
|w   |-----|            |epr5 |                   |N
|e   |epr8 |            |-----|                   |N
|r   |-----|                                      |E
|    |epr7 |            |HD68A09P                 |C
|    |-----|                     |-----|          |T
|t   |epr6 |                     |epr4 |          |O
|o   |-----|                     |-----|          |R
|                   |--------|   |epr3 |          |
|l                  |HD6809EP|   |-----|          |
|o                  |--------|   |epr2 |  dip 8x  |
|w                               |-----|  dip 8x  |
|e                               |epr1 |          |
|r                               |-----|          |
|-------------------------------------------------|
Notes:

      Chips:
      HD68A09P : 3G1-UL-HD68A09P Japan (DIP40)
      HD6809EP : 5M1-HD6809EP Japan (DIP40)
       YM2203C : Yamaha YM2203C-5X-18-89-F (DIP40)
        YM3526 : Yamaha YM3526-61-09-75-E (DIP40)

      ROMs:
    1,2,3,5,6,7,8 : Intel IP27256
                4 : M27128Z-N
             prom : TBP28S42N

    Connectors:
               2x flat cable to upper board
               1x NON-JAMMA 22 contacts



lower pcb - 3002B
|-----------------------------------------------------|
|t                   |-----|                          |
|o          |------| |epr11|                          |
|           |pal -a| |-----| |------|                 |
|u          |------| |epr10| |pal -b|                 |
|p                   |-----| |------| |------|        |
|p                   |epr9 |          |pal -c|        |
|e                   |-----|          |------|        |
|r   |-----|                                          |
|    |epr12|                                          |
|    |-----|                                          |
|t                                                    |
|o                                                    |
|                                                     |
|u                                                    |
|p                      12.0000MHz                    |
|p                                                    |
|e                                                    |
|r                                                    |
|-----------------------------------------------------|
Notes:
      ROMs:
          9,10,11 : Intel IP27256
               12 : TMS2764JL
              pal : AmPAL16R4PC

    Connectors:
               2x flat cable to upper board


brkthru, brkthruj and brkthrut have a Self Test Mode not mentioned anywhere
in the manual. It is accessed by holding down both player 1 and player 2 start
buttons while powering up the game. It can be accessed in MAME by holding the
buttons down after the game has started then pressing F3 to reset the game.

***************************************************************************/

#include "emu.h"
#include "includes/brkthru.h"

#include "cpu/m6809/m6809.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"
#include "screen.h"
#include "speaker.h"


#define MASTER_CLOCK        XTAL(12'000'000)


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void brkthru_state::brkthru_1803_w(uint8_t data)
{
	/* bit 0 = NMI enable */
	m_nmi_mask = ~data & 1;

	if(data & 2)
		m_maincpu->set_input_line(0, CLEAR_LINE);

	/* bit 1 = ? maybe IRQ acknowledge */
}

void brkthru_state::darwin_0803_w(uint8_t data)
{
	/* bit 0 = NMI enable */
	m_nmi_mask = data & 1;
	logerror("0803 %02X\n",data);

	if(data & 2)
		m_maincpu->set_input_line(0, CLEAR_LINE);


	/* bit 1 = ? maybe IRQ acknowledge */
}

INPUT_CHANGED_MEMBER(brkthru_state::coin_inserted)
{
	/* coin insertion causes an IRQ */
	if (oldval)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void brkthru_state::brkthru_map(address_map &map)
{
	map(0x0000, 0x03ff).ram().w(FUNC(brkthru_state::brkthru_fgram_w)).share("fg_videoram");
	map(0x0400, 0x0bff).ram();
	map(0x0c00, 0x0fff).ram().w(FUNC(brkthru_state::brkthru_bgram_w)).share("videoram");
	map(0x1000, 0x10ff).ram().share("spriteram");
	map(0x1100, 0x17ff).ram();
	map(0x1800, 0x1800).portr("P1");
	map(0x1801, 0x1801).portr("P2");
	map(0x1802, 0x1802).portr("DSW1");
	map(0x1803, 0x1803).portr("DSW2_COIN");
	map(0x1800, 0x1801).w(FUNC(brkthru_state::brkthru_1800_w));   /* bg scroll and color, ROM bank selection, flip screen */
	map(0x1802, 0x1802).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x1803, 0x1803).w(FUNC(brkthru_state::brkthru_1803_w));   /* NMI enable, + ? */
	map(0x2000, 0x3fff).bankr("bank1");
	map(0x4000, 0xffff).rom();
}

/* same as brktrhu, but xor 0x1000 below 8k */
void brkthru_state::darwin_map(address_map &map)
{
	map(0x1000, 0x13ff).ram().w(FUNC(brkthru_state::brkthru_fgram_w)).share("fg_videoram");
	map(0x1400, 0x1bff).ram();
	map(0x1c00, 0x1fff).ram().w(FUNC(brkthru_state::brkthru_bgram_w)).share("videoram");
	map(0x0000, 0x00ff).ram().share("spriteram");
	map(0x0100, 0x01ff).nopw(); /*tidyup, nothing really here?*/
	map(0x0800, 0x0800).portr("P1");
	map(0x0801, 0x0801).portr("P2");
	map(0x0802, 0x0802).portr("DSW1");
	map(0x0803, 0x0803).portr("DSW2_COIN");
	map(0x0800, 0x0801).w(FUNC(brkthru_state::brkthru_1800_w));     /* bg scroll and color, ROM bank selection, flip screen */
	map(0x0802, 0x0802).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0803, 0x0803).w(FUNC(brkthru_state::darwin_0803_w));     /* NMI enable, + ? */
	map(0x2000, 0x3fff).bankr("bank1");
	map(0x4000, 0xffff).rom();
}


void brkthru_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2001).w("ym2", FUNC(ym3526_device::write));
	map(0x4000, 0x4000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x6000, 0x6001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x8000, 0xffff).rom();
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( brkthru )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")   /* used only by the self test */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, "Enemy Vehicles" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x20, 0x20, "Enemy Bullets" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x40, 0x00, "Control Panel" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "1 Player" )
	PORT_DIPSETTING(    0x00, "2 Players" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2_COIN")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "99 (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "20000 Points Only" )
	PORT_DIPSETTING(    0x04, "10000/20000 Points" )
	PORT_DIPSETTING(    0x0c, "20000/30000 Points" )
	PORT_DIPSETTING(    0x08, "20000/40000 Points" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5") /* Manual says ALWAYS OFF */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	/* According to the manual, bit 5 should control Flip Screen */
//  PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:6")
//  PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* SW2:7,8 ALWAYS OFF according to the manual  */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, brkthru_state,coin_inserted, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, brkthru_state,coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, brkthru_state,coin_inserted, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( brkthruj )
	PORT_INCLUDE( brkthru )

	PORT_MODIFY("DSW2_COIN")
	PORT_SERVICE_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
INPUT_PORTS_END

static INPUT_PORTS_START( darwin )
	PORT_INCLUDE( brkthru )

	PORT_MODIFY("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )   /* Manual says must be OFF */
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )   /* Manual says must be OFF */

	PORT_MODIFY("DSW2_COIN")    /* modified by Shingo Suzuki 1999/11/02 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "20k, 50k and every 50k" )
	PORT_DIPSETTING(    0x00, "30k, 80k and every 80k" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	/* According to the manual, bit 5 should control Flip Screen */
//  PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:6")
//  PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* SW2:5,7,8 ALWAYS OFF according to the manual  */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, brkthru_state,coin_inserted, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, brkthru_state,coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, brkthru_state,coin_inserted, 0)
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	256,    /* 256 characters */
	3,  /* 3 bits per pixel */
	{ 512*8*8+4, 0, 4 },    /* plane offset */
	{ 256*8*8+0, 256*8*8+1, 256*8*8+2, 256*8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout tilelayout1 =
{
	16,16,  /* 16*16 tiles */
	128,    /* 128 tiles */
	3,  /* 3 bits per pixel */
	{ 0x4000*8+4, 0, 4 },   /* plane offset */
	{ 0, 1, 2, 3, 1024*8*8+0, 1024*8*8+1, 1024*8*8+2, 1024*8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+1024*8*8+0, 16*8+1024*8*8+1, 16*8+1024*8*8+2, 16*8+1024*8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every tile takes 32 consecutive bytes */
};

static const gfx_layout tilelayout2 =
{
	16,16,  /* 16*16 tiles */
	128,    /* 128 tiles */
	3,  /* 3 bits per pixel */
	{ 0x3000*8+0, 0, 4 },   /* plane offset */
	{ 0, 1, 2, 3, 1024*8*8+0, 1024*8*8+1, 1024*8*8+2, 1024*8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+1024*8*8+0, 16*8+1024*8*8+1, 16*8+1024*8*8+2, 16*8+1024*8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every tile takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	1024,   /* 1024 sprites */
	3,  /* 3 bits per pixel */
	{ 2*1024*32*8, 1024*32*8, 0 },  /* plane offset */
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( gfx_brkthru )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,   0x00,  1 )  /* use colors 0x00-0x07 */
	GFXDECODE_ENTRY( "gfx2", 0x00000, tilelayout1,  0x80, 16 )  /* use colors 0x80-0xff */
	GFXDECODE_ENTRY( "gfx2", 0x01000, tilelayout2,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, tilelayout1,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x09000, tilelayout2,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x10000, tilelayout1,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x11000, tilelayout2,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x18000, tilelayout1,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x19000, tilelayout2,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x00000, spritelayout, 0x40,  8 )  /* use colors 0x40-0x7f */
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void brkthru_state::machine_start()
{
	save_item(NAME(m_bgscroll));
	save_item(NAME(m_bgbasecolor));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_nmi_mask));
}

void brkthru_state::machine_reset()
{
	m_bgscroll = 0;
	m_bgbasecolor = 0;
	m_flipscreen = 0;
	m_nmi_mask = 0;
}

WRITE_LINE_MEMBER(brkthru_state::vblank_irq)
{
	if (state && m_nmi_mask)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void brkthru_state::brkthru(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, MASTER_CLOCK/8);         /* 1.5 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &brkthru_state::brkthru_map);

	MC6809(config, m_audiocpu, MASTER_CLOCK/2);         /* 1.5 MHz ? */
	m_audiocpu->set_addrmap(AS_PROGRAM, &brkthru_state::sound_map);

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_brkthru);
	PALETTE(config, m_palette, FUNC(brkthru_state::brkthru_palette), 256);

	/* not sure; assuming to be the same as darwin */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK/2, 384, 8, 248, 272, 8, 248);
	screen.set_screen_update(FUNC(brkthru_state::screen_update_brkthru));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(brkthru_state::vblank_irq));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", MASTER_CLOCK/8));
	ym1.add_route(0, "mono", 0.10);
	ym1.add_route(1, "mono", 0.10);
	ym1.add_route(2, "mono", 0.10);
	ym1.add_route(3, "mono", 0.50);

	ym3526_device &ym2(YM3526(config, "ym2", MASTER_CLOCK/4));
	ym2.irq_handler().set_inputline(m_audiocpu, M6809_IRQ_LINE);
	ym2.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void brkthru_state::darwin(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, MASTER_CLOCK/8);         /* 1.5 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &brkthru_state::darwin_map);

	MC6809(config, m_audiocpu, MASTER_CLOCK/2);         /* 1.5 MHz ? */
	m_audiocpu->set_addrmap(AS_PROGRAM, &brkthru_state::sound_map);

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_brkthru);

	PALETTE(config, m_palette, FUNC(brkthru_state::brkthru_palette), 256);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK/2, 384, 8, 248, 272, 8, 248);
	/* frames per second, vblank duration
	    Horizontal video frequency:
	        HSync = Dot Clock / Horizontal Frame Length
	              = Xtal /2   / (HDisplay + HBlank)
	              = 12MHz/2   / (240 + 144)
	              = 15.625kHz
	    Vertical Video frequency:
	        VSync = HSync / Vertical Frame Length
	              = HSync / (VDisplay + VBlank)
	              = 15.625kHz / (240 + 32)
	              = 57.444855Hz
	    tuned by Shingo SUZUKI(VSyncMAME Project) 2000/10/19 */
	screen.set_screen_update(FUNC(brkthru_state::screen_update_brkthru));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(brkthru_state::vblank_irq));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", MASTER_CLOCK/8));
	ym1.add_route(0, "mono", 0.10);
	ym1.add_route(1, "mono", 0.10);
	ym1.add_route(2, "mono", 0.10);
	ym1.add_route(3, "mono", 0.50);

	ym3526_device &ym2(YM3526(config, "ym2", MASTER_CLOCK/4));
	ym2.irq_handler().set_inputline(m_audiocpu, M6809_IRQ_LINE);
	ym2.add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( brkthru )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for main CPU + 64k for banked ROMs */
	ROM_LOAD( "brkthru.1",    0x04000, 0x4000, CRC(cfb4265f) SHA1(4cd748fa06fd2727de1694196912d605672d4883) )
	ROM_LOAD( "brkthru.2",    0x08000, 0x8000, CRC(fa8246d9) SHA1(d6da03b2a3d8a83411191351ee110b89352a3ead) )
	ROM_LOAD( "brkthru.4",    0x10000, 0x8000, CRC(8cabf252) SHA1(45e8847b2e6b278989f67e0b27b827a9b3b92581) )
	ROM_LOAD( "brkthru.3",    0x18000, 0x8000, CRC(2f2c40c2) SHA1(fcb78941453520a3a07f272127dae7c2cc1999ea) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "brkthru.12",   0x00000, 0x2000, CRC(58c0b29b) SHA1(9dc075f8afae7e8fe164a9fe325e9948cdc7e4bb) )   /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	/* background */
	/* we do a lot of scatter loading here, to place the data in a format */
	/* which can be decoded by MAME's standard functions */
	ROM_LOAD( "brkthru.7",    0x00000, 0x4000, CRC(920cc56a) SHA1(c75806691073f1f3bd54dcaca4c14155ecf4471d) )   /* bitplanes 1,2 for bank 1,2 */
	ROM_CONTINUE(             0x08000, 0x4000 )             /* bitplanes 1,2 for bank 3,4 */
	ROM_LOAD( "brkthru.6",    0x10000, 0x4000, CRC(fd3cee40) SHA1(3308b96bb69e0fa6dffbdff296273fafa16d5e70) )   /* bitplanes 1,2 for bank 5,6 */
	ROM_CONTINUE(             0x18000, 0x4000 )             /* bitplanes 1,2 for bank 7,8 */
	ROM_LOAD( "brkthru.8",    0x04000, 0x1000, CRC(f67ee64e) SHA1(75634bd481ae44b8aa02acb4f9b4d7ff973a4c71) )   /* bitplane 3 for bank 1,2 */
	ROM_CONTINUE(             0x06000, 0x1000 )
	ROM_CONTINUE(             0x0c000, 0x1000 )             /* bitplane 3 for bank 3,4 */
	ROM_CONTINUE(             0x0e000, 0x1000 )
	ROM_CONTINUE(             0x14000, 0x1000 )             /* bitplane 3 for bank 5,6 */
	ROM_CONTINUE(             0x16000, 0x1000 )
	ROM_CONTINUE(             0x1c000, 0x1000 )             /* bitplane 3 for bank 7,8 */
	ROM_CONTINUE(             0x1e000, 0x1000 )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "brkthru.9",    0x00000, 0x8000, CRC(f54e50a7) SHA1(eccf4d859c26944271ec6586644b4730a72851fd) )   /* sprites */
	ROM_LOAD( "brkthru.10",   0x08000, 0x8000, CRC(fd156945) SHA1(a0575a4164217e63317886176ab7e59d255fc771) )
	ROM_LOAD( "brkthru.11",   0x10000, 0x8000, CRC(c152a99b) SHA1(f96133aa01219eda357b9e906bd9577dbfe359c0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "brkthru.13",   0x0000, 0x0100, CRC(aae44269) SHA1(7c66aeb93577104109d264ee8b848254256c81eb) ) /* red and green component */
	ROM_LOAD( "brkthru.14",   0x0100, 0x0100, CRC(f2d4822a) SHA1(f535e91b87ff01f2a73662856fd3f72907ca62e9) ) /* blue component */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "brkthru.5",    0x8000, 0x8000, CRC(c309435f) SHA1(82914004c2b169a7c31aa49af83a699ebbc7b33f) )
ROM_END

ROM_START( brkthruj )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for main CPU + 64k for banked ROMs */
	ROM_LOAD( "1",            0x04000, 0x4000, CRC(09bd60ee) SHA1(9591a4c89bb69d5615a5d6b29c47e6b17350c007) )
	ROM_LOAD( "2",            0x08000, 0x8000, CRC(f2b2cd1c) SHA1(dafccc74310876bc1c88de7f3c86f93ed8a0eb62) )
	ROM_LOAD( "4",            0x10000, 0x8000, CRC(b42b3359) SHA1(c1da550e0f7cc52721802c7c0f2770ef0087e28b) )
	ROM_LOAD( "brkthru.3",    0x18000, 0x8000, CRC(2f2c40c2) SHA1(fcb78941453520a3a07f272127dae7c2cc1999ea) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "12",   0x00000, 0x2000, CRC(3d9a7003) SHA1(2e5de982eb75ac75312fb29bb4cb2ed12ec0fd56) )   /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	/* background */
	/* we do a lot of scatter loading here, to place the data in a format */
	/* which can be decoded by MAME's standard functions */
	ROM_LOAD( "brkthru.7",    0x00000, 0x4000, CRC(920cc56a) SHA1(c75806691073f1f3bd54dcaca4c14155ecf4471d) )   /* bitplanes 1,2 for bank 1,2 */
	ROM_CONTINUE(             0x08000, 0x4000 )             /* bitplanes 1,2 for bank 3,4 */
	ROM_LOAD( "6",            0x10000, 0x4000, CRC(cb47b395) SHA1(bf5459d696e863644f13c8b0786b8f45caf6ceb6) )   /* bitplanes 1,2 for bank 5,6 */
	ROM_CONTINUE(             0x18000, 0x4000 )             /* bitplanes 1,2 for bank 7,8 */
	ROM_LOAD( "8",            0x04000, 0x1000, CRC(5e5a2cd7) SHA1(f1782d67b924b4b89bcb6602e970c28fbeaab522) )   /* bitplane 3 for bank 1,2 */
	ROM_CONTINUE(             0x06000, 0x1000 )
	ROM_CONTINUE(             0x0c000, 0x1000 )             /* bitplane 3 for bank 3,4 */
	ROM_CONTINUE(             0x0e000, 0x1000 )
	ROM_CONTINUE(             0x14000, 0x1000 )             /* bitplane 3 for bank 5,6 */
	ROM_CONTINUE(             0x16000, 0x1000 )
	ROM_CONTINUE(             0x1c000, 0x1000 )             /* bitplane 3 for bank 7,8 */
	ROM_CONTINUE(             0x1e000, 0x1000 )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "brkthru.9",    0x00000, 0x8000, CRC(f54e50a7) SHA1(eccf4d859c26944271ec6586644b4730a72851fd) )   /* sprites */
	ROM_LOAD( "brkthru.10",   0x08000, 0x8000, CRC(fd156945) SHA1(a0575a4164217e63317886176ab7e59d255fc771) )
	ROM_LOAD( "brkthru.11",   0x10000, 0x8000, CRC(c152a99b) SHA1(f96133aa01219eda357b9e906bd9577dbfe359c0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "brkthru.13",   0x0000, 0x0100, CRC(aae44269) SHA1(7c66aeb93577104109d264ee8b848254256c81eb) ) /* red and green component */
	ROM_LOAD( "brkthru.14",   0x0100, 0x0100, CRC(f2d4822a) SHA1(f535e91b87ff01f2a73662856fd3f72907ca62e9) ) /* blue component */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "brkthru.5",    0x8000, 0x8000, CRC(c309435f) SHA1(82914004c2b169a7c31aa49af83a699ebbc7b33f) )
ROM_END

// Tecfri PCB with Data East license (DE-0230-2 / DE-0231-2).
ROM_START( brkthrut )
	ROM_REGION( 0x20000, "maincpu", 0 )     // 64k for main CPU + 64k for banked ROMs
	ROM_LOAD( "5_de-0230-2_27128.f9",  0x04000, 0x4000, CRC(158e660a) SHA1(608082e8b49d3c5595c25be8c19b80402310406a) )
	ROM_LOAD( "6_de-0230-2_27256.f11", 0x08000, 0x8000, CRC(62dbe49e) SHA1(f0510cf0144d75c208f0ced342d3a726325a70d4) )
	ROM_LOAD( "8_de-0230-2_27256.f13", 0x10000, 0x8000, CRC(8cabf252) SHA1(45e8847b2e6b278989f67e0b27b827a9b3b92581) ) // Same as parent
	ROM_LOAD( "7_de-0230-2_27256.f12", 0x18000, 0x8000, CRC(2f2c40c2) SHA1(fcb78941453520a3a07f272127dae7c2cc1999ea) ) // Same as parent

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "9_de-0231-2_2764.c8", 0x00000, 0x2000, CRC(58c0b29b) SHA1(9dc075f8afae7e8fe164a9fe325e9948cdc7e4bb) ) // Characters, same as parent

	ROM_REGION( 0x20000, "gfx2", 0 )
	// Background
	// We do a lot of scatter loading here, to place the data in a format
	// which can be decoded by MAME's standard functions
	ROM_LOAD( "2_de-0230-2_27256.a6", 0x00000, 0x4000, CRC(920cc56a) SHA1(c75806691073f1f3bd54dcaca4c14155ecf4471d) ) // Bitplanes 1,2 for bank 1,2, same as parent
	ROM_CONTINUE(             0x08000, 0x4000 )             // Bitplanes 1,2 for bank 3,4
	ROM_LOAD( "1_de-0230-2_27256.a5", 0x10000, 0x4000, CRC(fd3cee40) SHA1(3308b96bb69e0fa6dffbdff296273fafa16d5e70) ) // Bitplanes 1,2 for bank 5,6, same as parent
	ROM_CONTINUE(             0x18000, 0x4000 )             // Bitplanes 1,2 for bank 7,8
	ROM_LOAD( "3_de-0230-2_27256.a8", 0x04000, 0x1000, CRC(f67ee64e) SHA1(75634bd481ae44b8aa02acb4f9b4d7ff973a4c71) ) // Bitplane 3 for bank 1,2, same as parent
	ROM_CONTINUE(             0x06000, 0x1000 )
	ROM_CONTINUE(             0x0c000, 0x1000 )             // Bitplane 3 for bank 3,4
	ROM_CONTINUE(             0x0e000, 0x1000 )
	ROM_CONTINUE(             0x14000, 0x1000 )             // Bitplane 3 for bank 5,6
	ROM_CONTINUE(             0x16000, 0x1000 )
	ROM_CONTINUE(             0x1c000, 0x1000 )             // Bitplane 3 for bank 7,8
	ROM_CONTINUE(             0x1e000, 0x1000 )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "10_de-0231-2_27156.h2", 0x00000, 0x8000, CRC(f54e50a7) SHA1(eccf4d859c26944271ec6586644b4730a72851fd) ) // Sprites, same as parent
	ROM_LOAD( "11_de-0231-2_27156.h4", 0x08000, 0x8000, CRC(fd156945) SHA1(a0575a4164217e63317886176ab7e59d255fc771) ) // Same as parent
	ROM_LOAD( "12_de-0231-2_27156.h5", 0x10000, 0x8000, CRC(c152a99b) SHA1(f96133aa01219eda357b9e906bd9577dbfe359c0) ) // Same as parent

	ROM_REGION( 0x0300, "proms", 0 ) // Truly dumped on the Tecfri PCB.
	/*
	   The original 82s129 bipolar PROM for R/G was replaced with one 82s147
	   that has twice the size. Seems that A05 line was disconnected, and the
	   rest of bigger addressing lines were displaced covering the hole...
	*/
	ROM_LOAD( "6309.c2", 0x0000, 0x0040, CRC(cd9709be) SHA1(1d9c451c771a7b38680e2179aa22289ea7cb2720) ) // Red and Green component (82S147N)
	ROM_CONTINUE(        0x0020, 0x0040 )
	ROM_CONTINUE(        0x0040, 0x0040 )
	ROM_CONTINUE(        0x0060, 0x0040 )
	ROM_CONTINUE(        0x0080, 0x0040 )
	ROM_CONTINUE(        0x00a0, 0x0040 )
	ROM_CONTINUE(        0x00c0, 0x0040 )
	ROM_CONTINUE(        0x00e0, 0x0040 )

	ROM_LOAD( "6301.c1", 0x0100, 0x0100, CRC(f2d4822a) SHA1(f535e91b87ff01f2a73662856fd3f72907ca62e9) ) // Blue component (82S129N), same as parent

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "4_de-0230-2_27256.d6", 0x8000, 0x8000, CRC(c309435f) SHA1(82914004c2b169a7c31aa49af83a699ebbc7b33f) ) // Same as parent
ROM_END

ROM_START( brkthrubl ) // 3002A + 3002B PCBs. Everything's the same as the original but the main CPU ROMs, which differ quite a lot (possibly bootlegged from a different revision?)
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "4",    0x04000, 0x4000, CRC(0f21b4c5) SHA1(e5ef67006394d475571196dcdc181d28a0831bdf) )
	ROM_LOAD( "3",    0x08000, 0x8000, CRC(51c7c378) SHA1(b779df719b89bb41e342c472b38e20d8fc71ca6d) )
	ROM_LOAD( "1",    0x10000, 0x8000, CRC(209484c2) SHA1(da7311768b675b833066a7403fd507969d74699e) )
	ROM_LOAD( "2",    0x18000, 0x8000, CRC(2f2c40c2) SHA1(fcb78941453520a3a07f272127dae7c2cc1999ea) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "12",   0x00000, 0x2000, CRC(58c0b29b) SHA1(9dc075f8afae7e8fe164a9fe325e9948cdc7e4bb) )   // characters

	ROM_REGION( 0x20000, "gfx2", 0 ) // background
	ROM_LOAD( "7",    0x00000, 0x4000, CRC(920cc56a) SHA1(c75806691073f1f3bd54dcaca4c14155ecf4471d) )   // bitplanes 1,2 for bank 1,2
	ROM_CONTINUE(             0x08000, 0x4000 )             // bitplanes 1,2 for bank 3,4
	ROM_LOAD( "6",    0x10000, 0x4000, CRC(fd3cee40) SHA1(3308b96bb69e0fa6dffbdff296273fafa16d5e70) )   // bitplanes 1,2 for bank 5,6
	ROM_CONTINUE(             0x18000, 0x4000 )             // bitplanes 1,2 for bank 7,8
	ROM_LOAD( "8",    0x04000, 0x1000, CRC(f67ee64e) SHA1(75634bd481ae44b8aa02acb4f9b4d7ff973a4c71) )   // bitplane 3 for bank 1,2
	ROM_CONTINUE(             0x06000, 0x1000 )
	ROM_CONTINUE(             0x0c000, 0x1000 )             // bitplane 3 for bank 3,4
	ROM_CONTINUE(             0x0e000, 0x1000 )
	ROM_CONTINUE(             0x14000, 0x1000 )             // bitplane 3 for bank 5,6
	ROM_CONTINUE(             0x16000, 0x1000 )
	ROM_CONTINUE(             0x1c000, 0x1000 )             // bitplane 3 for bank 7,8
	ROM_CONTINUE(             0x1e000, 0x1000 )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "9",    0x00000, 0x8000, CRC(f54e50a7) SHA1(eccf4d859c26944271ec6586644b4730a72851fd) )   // sprites
	ROM_LOAD( "10",   0x08000, 0x8000, CRC(fd156945) SHA1(a0575a4164217e63317886176ab7e59d255fc771) )
	ROM_LOAD( "11",   0x10000, 0x8000, CRC(c152a99b) SHA1(f96133aa01219eda357b9e906bd9577dbfe359c0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "brkthru.13",   0x0000, 0x0100, CRC(aae44269) SHA1(7c66aeb93577104109d264ee8b848254256c81eb) ) // red and green component
	ROM_LOAD( "brkthru.14",   0x0100, 0x0100, CRC(f2d4822a) SHA1(f535e91b87ff01f2a73662856fd3f72907ca62e9) ) // blue component

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5",    0x8000, 0x8000, CRC(c309435f) SHA1(82914004c2b169a7c31aa49af83a699ebbc7b33f) )
ROM_END

/* bootleg, changed prg rom fails test, probably just the japanese version modified to have english title */
ROM_START( forcebrk )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for main CPU + 64k for banked ROMs */
	ROM_LOAD( "1",            0x04000, 0x4000, CRC(09bd60ee) SHA1(9591a4c89bb69d5615a5d6b29c47e6b17350c007) )
	ROM_LOAD( "2",            0x08000, 0x8000, CRC(f2b2cd1c) SHA1(dafccc74310876bc1c88de7f3c86f93ed8a0eb62) )
	ROM_LOAD( "forcebrk4",    0x10000, 0x8000, CRC(b4838c19) SHA1(b32f183ee042872a6eb6689aab219108d37829e4) )
	ROM_LOAD( "brkthru.3",    0x18000, 0x8000, CRC(2f2c40c2) SHA1(fcb78941453520a3a07f272127dae7c2cc1999ea) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "12",           0x00000, 0x2000, CRC(3d9a7003) SHA1(2e5de982eb75ac75312fb29bb4cb2ed12ec0fd56) )   /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	/* background */
	/* we do a lot of scatter loading here, to place the data in a format */
	/* which can be decoded by MAME's standard functions */
	ROM_LOAD( "brkthru.7",    0x00000, 0x4000, CRC(920cc56a) SHA1(c75806691073f1f3bd54dcaca4c14155ecf4471d) )   /* bitplanes 1,2 for bank 1,2 */
	ROM_CONTINUE(             0x08000, 0x4000 )             /* bitplanes 1,2 for bank 3,4 */
	ROM_LOAD( "forcebrk6",    0x10000, 0x4000, CRC(08bca16a) SHA1(d5dcf5cf68a5090f467c076abb1b9cf0baffe272) )   /* bitplanes 1,2 for bank 5,6 */
	ROM_CONTINUE(             0x18000, 0x4000 )             /* bitplanes 1,2 for bank 7,8 */
	ROM_LOAD( "forcebrk8",    0x04000, 0x1000, CRC(a3a1131e) SHA1(e0b73c8b2c8ea6b31418bc642830875c5985f800) )   /* bitplane 3 for bank 1,2 */
	ROM_CONTINUE(             0x06000, 0x1000 )
	ROM_CONTINUE(             0x0c000, 0x1000 )             /* bitplane 3 for bank 3,4 */
	ROM_CONTINUE(             0x0e000, 0x1000 )
	ROM_CONTINUE(             0x14000, 0x1000 )             /* bitplane 3 for bank 5,6 */
	ROM_CONTINUE(             0x16000, 0x1000 )
	ROM_CONTINUE(             0x1c000, 0x1000 )             /* bitplane 3 for bank 7,8 */
	ROM_CONTINUE(             0x1e000, 0x1000 )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "brkthru.9",    0x00000, 0x8000, CRC(f54e50a7) SHA1(eccf4d859c26944271ec6586644b4730a72851fd) )   /* sprites */
	ROM_LOAD( "brkthru.10",   0x08000, 0x8000, CRC(fd156945) SHA1(a0575a4164217e63317886176ab7e59d255fc771) )
	ROM_LOAD( "brkthru.11",   0x10000, 0x8000, CRC(c152a99b) SHA1(f96133aa01219eda357b9e906bd9577dbfe359c0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "brkthru.13",   0x0000, 0x0100, CRC(aae44269) SHA1(7c66aeb93577104109d264ee8b848254256c81eb) ) /* red and green component */
	ROM_LOAD( "brkthru.14",   0x0100, 0x0100, CRC(f2d4822a) SHA1(f535e91b87ff01f2a73662856fd3f72907ca62e9) ) /* blue component */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "brkthru.5",    0x8000, 0x8000, CRC(c309435f) SHA1(82914004c2b169a7c31aa49af83a699ebbc7b33f) )
ROM_END

ROM_START( darwin )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for main CPU + 64k for banked ROMs */
	ROM_LOAD( "darw_04.rom",  0x04000, 0x4000, CRC(0eabf21c) SHA1(ccad6b30fe9361e8a21b8aaf8116aa85f9e6bb19) )
	ROM_LOAD( "darw_05.rom",  0x08000, 0x8000, CRC(e771f864) SHA1(8ba9f97c6abf035ceaf9f5505495708506f1b0c5) )
	ROM_LOAD( "darw_07.rom",  0x10000, 0x8000, CRC(97ac052c) SHA1(8baa117472d46b99e5946f095b869de9b5c48f9a) )
	ROM_LOAD( "darw_06.rom",  0x18000, 0x8000, CRC(2a9fb208) SHA1(f04a5502600e49e2494a87ec65a44a2843441d37) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "darw_09.rom",  0x00000, 0x2000, CRC(067b4cf5) SHA1(fc752bb72e4850b71565afd1df0cbb4f732f131c) )   /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	/* background */
	/* we do a lot of scatter loading here, to place the data in a format */
	/* which can be decoded by MAME's standard functions */
	ROM_LOAD( "darw_03.rom",  0x00000, 0x4000, CRC(57d0350d) SHA1(6f904047485e669afb5f4b590818743111f010c6) )   /* bitplanes 1,2 for bank 1,2 */
	ROM_CONTINUE(             0x08000, 0x4000 )             /* bitplanes 1,2 for bank 3,4 */
	ROM_LOAD( "darw_02.rom",  0x10000, 0x4000, CRC(559a71ab) SHA1(a28de25e89e0d68332f4095b988827a9cb72c675) )   /* bitplanes 1,2 for bank 5,6 */
	ROM_CONTINUE(             0x18000, 0x4000 )             /* bitplanes 1,2 for bank 7,8 */
	ROM_LOAD( "darw_01.rom",  0x04000, 0x1000, CRC(15a16973) SHA1(5eb978a32be88176936e5d37b6ec18820d9720d8) )   /* bitplane 3 for bank 1,2 */
	ROM_CONTINUE(             0x06000, 0x1000 )
	ROM_CONTINUE(             0x0c000, 0x1000 )             /* bitplane 3 for bank 3,4 */
	ROM_CONTINUE(             0x0e000, 0x1000 )
	ROM_CONTINUE(             0x14000, 0x1000 )             /* bitplane 3 for bank 5,6 */
	ROM_CONTINUE(             0x16000, 0x1000 )
	ROM_CONTINUE(             0x1c000, 0x1000 )             /* bitplane 3 for bank 7,8 */
	ROM_CONTINUE(             0x1e000, 0x1000 )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "darw_10.rom",  0x00000, 0x8000, CRC(487a014c) SHA1(c9543df8115088b02019e76a6473ecc5f645a836) )   /* sprites */
	ROM_LOAD( "darw_11.rom",  0x08000, 0x8000, CRC(548ce2d1) SHA1(3b1757c70346ab4ee19ec85e7ae5137f8ccf446f) )
	ROM_LOAD( "darw_12.rom",  0x10000, 0x8000, CRC(faba5fef) SHA1(848da4d4888f0218b737f1dc9b62944f68349a43) )

	// A PCB has been found with the first PROM substituted with a TBP28S42 (4b56a744) SHA1(5fdc336d90c8a289c146c66f241dd217fc11bf35), see brkthrut ROM loading for how they did it.
	// With that in mind, there's a one byte difference at 0x55 (0xf0 instead of 0x70). It is unknown if it's bitrot or if it's intended.
	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "df.12",   0x0000, 0x0100, CRC(89b952ef) SHA1(77dc4020a2e25f81fae1182d58993cf09d13af00) ) /* red and green component */
	ROM_LOAD( "df.13",   0x0100, 0x0100, CRC(d595e91d) SHA1(5e9793f6602455c79afdc855cd13183a7f48ab1e) ) /* blue component */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "darw_08.rom",  0x8000, 0x8000, CRC(6b580d58) SHA1(a70aebc6b4a291b4adddbb41d092b2682fc2d421) )

	ROM_REGION( 0x600, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "1-pal16r4pc.bin",  0x000, 0x104, CRC(c859298c) SHA1(7db617fec6eecaf3f6043d7446bc1786bbc2b08c) )
	ROM_LOAD( "2-pal16r4pc.bin",  0x200, 0x104, CRC(226629c3) SHA1(fd5704dfbb91a46665050b27b15bd22527a46a6e) )
	ROM_LOAD( "3-pal16r4pc.bin",  0x400, 0x104, CRC(b3e980a0) SHA1(b1dbf01621d1053e641570fcac6618562d0721b4) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void brkthru_state::init_brkthru()
{
	uint8_t *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 8, &ROM[0x10000], 0x2000);
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, brkthru,   0,       brkthru, brkthru,  brkthru_state, init_brkthru, ROT0,   "Data East USA",                          "Break Thru (US)",             MACHINE_SUPPORTS_SAVE )
GAME( 1986, brkthruj,  brkthru, brkthru, brkthruj, brkthru_state, init_brkthru, ROT0,   "Data East Corporation",                  "Kyohkoh-Toppa (Japan)",       MACHINE_SUPPORTS_SAVE )
GAME( 1986, brkthrut,  brkthru, brkthru, brkthruj, brkthru_state, init_brkthru, ROT0,   "Data East Corporation (Tecfri license)", "Break Thru (Tecfri license)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, forcebrk,  brkthru, brkthru, brkthruj, brkthru_state, init_brkthru, ROT0,   "bootleg",                                "Force Break (bootleg)",       MACHINE_SUPPORTS_SAVE )
GAME( 1986, brkthrubl, brkthru, brkthru, brkthruj, brkthru_state, init_brkthru, ROT0,   "bootleg",                                "Break Thru (bootleg)",        MACHINE_SUPPORTS_SAVE )
GAME( 1986, darwin,    0,       darwin,  darwin,   brkthru_state, init_brkthru, ROT270, "Data East Corporation",                  "Darwin 4078 (Japan)",         MACHINE_SUPPORTS_SAVE )
