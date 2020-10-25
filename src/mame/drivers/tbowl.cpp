// license:BSD-3-Clause
// copyright-holders:David Haywood
/*** Tecmo Bowl (c)1987 Tecmo

driver by David Haywood
wip 20/01/2002

Tecmo Bowl was a popular 4 player American Football game with two screens and
attractive graphics

--- Current Issues

Might be some priority glitches

***/

#include "emu.h"
#include "includes/tbowl.h"

#include "cpu/z80/z80.h"
#include "sound/3812intf.h"

#include "rendlay.h"
#include "screen.h"
#include "speaker.h"


void tbowl_state::coincounter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
}

/*** Banking

note: check this, its borrowed from tecmo.cpp / wc90.cpp at the moment and could well be wrong

***/

void tbowl_state::boardb_bankswitch_w(uint8_t data)
{
	membank("mainbank")->set_entry(data >> 3);
}

void tbowl_state::boardc_bankswitch_w(uint8_t data)
{
	membank("subbank")->set_entry(data >> 3);
}

/*** Memory Structures

    Board B is the main board, reading inputs, and in control of the 2 bg layers & text layer etc.
    Board C is the sub board, main job is the sprites
    Board A is for the sound

***/


/* Board B */

void tbowl_state::_6206B_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).ram().w(FUNC(tbowl_state::bg2videoram_w)).share("bg2videoram");
	map(0xc000, 0xdfff).ram().w(FUNC(tbowl_state::bgvideoram_w)).share("bgvideoram");
	map(0xe000, 0xefff).ram().w(FUNC(tbowl_state::txvideoram_w)).share("txvideoram");
//  map(0xf000, 0xf000).w(FUNC(tbowl_state::unknown_write)); * written during start-up, not again */
	map(0xf000, 0xf7ff).bankr("mainbank");
	map(0xf800, 0xfbff).ram().share("shared_ram"); /* check */
	map(0xfc00, 0xfc00).portr("P1").w(FUNC(tbowl_state::boardb_bankswitch_w));
	map(0xfc01, 0xfc01).portr("P2");
//  map(0xfc01, 0xfc01).w(FUNC(tbowl_state::unknown_write)); /* written during start-up, not again */
	map(0xfc02, 0xfc02).portr("P3");
//  map(0xfc02, 0xfc02).w(FUNC(tbowl_state::unknown_write)); /* written during start-up, not again */
	map(0xfc03, 0xfc03).portr("P4").w(FUNC(tbowl_state::coincounter_w));
//  map(0xfc05, 0xfc05).w(FUNC(tbowl_state::unknown_write)); /* no idea */
//  map(0xfc06, 0xfc06).r(FUNC(tbowl_state::dummy_r));        /* Read During NMI */
	map(0xfc07, 0xfc07).portr("SYSTEM");
	map(0xfc08, 0xfc08).portr("DSW1");
//  map(0xfc08, 0xfc08).w(FUNC(tbowl_state::unknown_write)); /* hardly used .. */
	map(0xfc09, 0xfc09).portr("DSW2");
	map(0xfc0a, 0xfc0a).portr("DSW3");
//  map(0xfc0a, 0xfc0a).w(FUNC(tbowl_state::unknown_write)); /* hardly used .. */
	map(0xfc0d, 0xfc0d).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xfc10, 0xfc10).w(FUNC(tbowl_state::bg2xscroll_lo));
	map(0xfc11, 0xfc11).w(FUNC(tbowl_state::bg2xscroll_hi));
	map(0xfc12, 0xfc12).w(FUNC(tbowl_state::bg2yscroll_lo));
	map(0xfc13, 0xfc13).w(FUNC(tbowl_state::bg2yscroll_hi));
	map(0xfc14, 0xfc14).w(FUNC(tbowl_state::bgxscroll_lo));
	map(0xfc15, 0xfc15).w(FUNC(tbowl_state::bgxscroll_hi));
	map(0xfc16, 0xfc16).w(FUNC(tbowl_state::bgyscroll_lo));
	map(0xfc17, 0xfc17).w(FUNC(tbowl_state::bgyscroll_hi));
}

/* Board C */
void tbowl_state::trigger_nmi(uint8_t data)
{
	/* trigger NMI on 6206B's Cpu? (guess but seems to work..) */
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void tbowl_state::_6206C_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xd7ff).ram();
	map(0xd800, 0xdfff).ram().share("spriteram");
	map(0xe000, 0xefff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette"); // 2x palettes, one for each monitor?
	map(0xf000, 0xf7ff).bankr("subbank");
	map(0xf800, 0xfbff).ram().share("shared_ram");
	map(0xfc00, 0xfc00).w(FUNC(tbowl_state::boardc_bankswitch_w));
	map(0xfc01, 0xfc01).nopw(); /* ? */
	map(0xfc02, 0xfc02).w(FUNC(tbowl_state::trigger_nmi)); /* ? */
	map(0xfc03, 0xfc03).nopw(); /* ? */
	map(0xfc06, 0xfc06).nopw(); /* ? */
}

/* Board A */

void tbowl_state::adpcm_start_w(offs_t offset, uint8_t data)
{
	msm5205_device *adpcm = (offset & 1) ? m_msm2 : m_msm1;
	m_adpcm_pos[offset & 1] = data << 8;
	adpcm->reset_w(0);
}

void tbowl_state::adpcm_end_w(offs_t offset, uint8_t data)
{
	m_adpcm_end[offset & 1] = (data + 1) << 8;
}

void tbowl_state::adpcm_vol_w(offs_t offset, uint8_t data)
{
	msm5205_device *adpcm = (offset & 1) ? m_msm2 : m_msm1;
	adpcm->set_output_gain(ALL_OUTPUTS, (data & 127) / 127.0);
}

void tbowl_state::adpcm_int( msm5205_device *device, int num )
{
	if (m_adpcm_pos[num] >= m_adpcm_end[num] ||
				m_adpcm_pos[num] >= memregion("adpcm")->bytes()/2)
		device->reset_w(1);
	else if (m_adpcm_data[num] != -1)
	{
		device->data_w(m_adpcm_data[num] & 0x0f);
		m_adpcm_data[num] = -1;
	}
	else
	{
		uint8_t *ROM = memregion("adpcm")->base() + 0x10000 * num;

		m_adpcm_data[num] = ROM[m_adpcm_pos[num]++];
		device->data_w(m_adpcm_data[num] >> 4);
	}
}

WRITE_LINE_MEMBER(tbowl_state::adpcm_int_1)
{
	adpcm_int(m_msm1, 0);
}

WRITE_LINE_MEMBER(tbowl_state::adpcm_int_2)
{
	adpcm_int(m_msm2, 1);
}

void tbowl_state::_6206A_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xd000, 0xd001).w("ym1", FUNC(ym3812_device::write));
	map(0xd800, 0xd801).w("ym2", FUNC(ym3812_device::write));
	map(0xe000, 0xe001).w(FUNC(tbowl_state::adpcm_end_w));
	map(0xe002, 0xe003).w(FUNC(tbowl_state::adpcm_start_w));
	map(0xe004, 0xe005).w(FUNC(tbowl_state::adpcm_vol_w));
	map(0xe006, 0xe006).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
	map(0xe007, 0xe007).nopw(); // sound watchdog
	map(0xe010, 0xe010).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

/*** Input Ports

Haze's notes :

There are controls for 4 players, each player has 4 directions and 2 buttons as
well as coin and start.  The service switch acts as inserting a coin for all
4 players, the dipswitches are listed in the manual


Steph's notes (2002.02.12) :

I haven't found any manual, so my notes only rely on the Z80 code ...

  -- Inputs --

According to the Z80 code, here is the list of controls for each player :

  - NO START button (you need to press BUTTON1n to start a game for player n)
  - 4 or 8 directions (I can't tell for the moment, so I've chosen 8 directions)
  - 2 buttons (I can't tell for the moment what they do)

There are also 1 coin slot and a "Service" button for each player.

1 "credit" will mean <<time defined by the "Player Time" Dip Switch>>.

COINn adds one COIN for player n. When the number of coins fit the "Coinage"
Dip Switch, 1 "credit" will be added.

SERVICEn adds 1 "credit" for player n.

There is also a GENERAL "Service" switch that adds 1 "credit" for ALL players.
I've mapped it to the F1 key. If you have a better key and/or a better
description for it, feel free to change it.

  -- Dip Switches --

According to the Z80 code, what is called "Difficulty" Dip Switch (DSW 1
bits 0 and 1) doesn't seem to be tested. BTW, where did you find such info ?

I haven't been able to determine yet the effect(s) of DSW3 bits 2 and 3.
Could it be the "Difficulty" you mentioned that is HERE instead of DSW1
bits 0 and 1 ? I'll try to have another look when the sprites stuff is finished.

***/


#define TBOWL_PLAYER_INPUT(_n_) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) PORT_8WAY \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(_n_) PORT_8WAY \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(_n_) PORT_8WAY \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(_n_) PORT_8WAY \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE##_n_ ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START##_n_ )


static INPUT_PORTS_START( tbowl )
	PORT_START("P1")    /* 0xfc00 */
	TBOWL_PLAYER_INPUT(1)

	PORT_START("P2")    /* 0xfc01 */
	TBOWL_PLAYER_INPUT(2)

	PORT_START("P3")    /* 0xfc02 */
	TBOWL_PLAYER_INPUT(3)

	PORT_START("P4")    /* 0xfc03 */
	TBOWL_PLAYER_INPUT(4)

	PORT_START("SYSTEM")    /* 0xfc07 -> 0x80f9 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Service (General)") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  /* 0xfc08 -> 0xffb4 */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING (   0x00, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING (   0x01, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING (   0x02, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING (   0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING (   0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING (   0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING (   0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (   0x07, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xf8, 0xb8, "Time (Players)" )        PORT_DIPLOCATION("SW1:4,5,6,7,8")
	PORT_DIPSETTING (   0x00, "7:00" )
	PORT_DIPSETTING (   0x08, "6:00" )
	PORT_DIPSETTING (   0x10, "5:00" )
	PORT_DIPSETTING (   0x18, "4:30" )
	PORT_DIPSETTING (   0x20, "3:40" )
	PORT_DIPSETTING (   0x28, "3:20" )
	PORT_DIPSETTING (   0x30, "3:00" )
	PORT_DIPSETTING (   0x38, "2:50" )
	PORT_DIPSETTING (   0x40, "2:40" )
	PORT_DIPSETTING (   0x48, "2:30" )
	PORT_DIPSETTING (   0x50, "2:20" )
	PORT_DIPSETTING (   0x58, "2:10" )
	PORT_DIPSETTING (   0x60, "2:00" )
	PORT_DIPSETTING (   0x68, "1:55" )
	PORT_DIPSETTING (   0x70, "1:50" )
	PORT_DIPSETTING (   0x78, "1:45" )
	PORT_DIPSETTING (   0x80, "1:40" )
	PORT_DIPSETTING (   0x88, "1:35" )
	PORT_DIPSETTING (   0x90, "1:25" )
	PORT_DIPSETTING (   0x98, "1:20" )
	PORT_DIPSETTING (   0xa0, "1:15" )
	PORT_DIPSETTING (   0xa8, "1:10" )
	PORT_DIPSETTING (   0xb0, "1:05" )
	PORT_DIPSETTING (   0xb8, "1:00" )
	PORT_DIPSETTING (   0xc0, "0:55" )
	PORT_DIPSETTING (   0xc8, "0:50" )
	PORT_DIPSETTING (   0xd0, "0:45" )
	PORT_DIPSETTING (   0xd8, "0:40" )
	PORT_DIPSETTING (   0xe0, "0:35" )
	PORT_DIPSETTING (   0xe8, "0:30" )
	PORT_DIPSETTING (   0xf0, "0:25" )
//  PORT_DIPSETTING (   0xf8, "1:00" )

	PORT_START("DSW2")  /* 0xfc09 -> 0xffb5 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2") // To be checked again
	PORT_DIPSETTING (   0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING (   0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING (   0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING (   0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Extra Time (Players)" )  PORT_DIPLOCATION("SW2:3,4") // For multiple "credits"
	PORT_DIPSETTING (   0x00, "0:30" )          /* manual shows 0:10 */
	PORT_DIPSETTING (   0x04, "0:20" )          /* manual shows 0:05 */
	PORT_DIPSETTING (   0x08, "0:10" )          /* manual shows 0:02 */
	PORT_DIPSETTING (   0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, "Timer Speed" )       PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING (   0x00, "Slowest" )           /* manual shows 1 Count = 60/60 Second - was 56/60 */
	PORT_DIPSETTING (   0x10, "Slow" )          /* manual shows 1 Count = 54/60 Second - was 51/60 */
	PORT_DIPSETTING (   0x30, DEF_STR( Normal ) )       /* manual shows 1 Count = 50/60 Second - was 47/60 */
	PORT_DIPSETTING (   0x20, "Fast" )          /* manual shows 1 Count = 45/60 Second - was 42/60 */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7") // Check code at 0x0393
	PORT_DIPSETTING (   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Hi-Score Reset" )        PORT_DIPLOCATION("SW2:8") // Only if P1 buttons 1 and 2 are pressed during P.O.S.T. !
	PORT_DIPSETTING (   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x80, DEF_STR( On ) )

	PORT_START("DSW3")  /* 0xfc0a -> 0xffb6 */
	PORT_DIPNAME( 0x03, 0x03, "Time (Quarter)" )        PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING (   0x00, "8:00" )
	PORT_DIPSETTING (   0x01, "5:00" )
	PORT_DIPSETTING (   0x03, "4:00" )
	PORT_DIPSETTING (   0x02, "3:00" )
	PORT_DIPNAME( 0x0c, 0x08, "Bonus Frequency" )       PORT_DIPLOCATION("SW3:3,4") // Check code at 0x6e16 (0x6e37 for tbowlj), each step is + 0x12
	PORT_DIPSETTING (   0x00, "Most" )          /* Value in 0x8126.w = 0x54f3 (0x5414 for tbowlj) */
	PORT_DIPSETTING (   0x04, "More" )          /* Value in 0x8126.w = 0x54e1 (0x5402 for tbowlj) */
	PORT_DIPSETTING (   0x08, DEF_STR( Normal ) )       /* Value in 0x8126.w = 0x54cf (0x54f0 for tbowlj), manual shows this is Least, but values is > least */
	PORT_DIPSETTING (   0x0c, "Least" )             /* Value in 0x8126.w = 0x54bd (0x54de for tbowlj), manual shows this is Normal, but value is least */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( tbowlj ) /* "Quarter Time" Dip Switch for "3:00" and "4:00" are inverted */
	PORT_INCLUDE( tbowl )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Time (Quarter)" )        PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING (   0x00, "8:00" )
	PORT_DIPSETTING (   0x01, "5:00" )
	PORT_DIPSETTING (   0x02, "4:00" )
	PORT_DIPSETTING (   0x03, "3:00" )
INPUT_PORTS_END


/*** Graphic Decodes

***/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout bgtilelayout =
{
	16,16,  /* tile size */
	RGN_FRAC(1,1),  /* number of tiles */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4, 32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32},
	128*8   /* offset to next tile */
};

static const gfx_layout sprite8layout =
{
	8,8,    /* tile size */
	RGN_FRAC(1,1),  /* number of tiles */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32    /* offset to next tile */
};

static GFXDECODE_START( gfx_tbowl )
	GFXDECODE_ENTRY( "characters", 0, charlayout,   256, 16 )
	GFXDECODE_ENTRY( "bg_tiles", 0, bgtilelayout, 768, 16 )
	GFXDECODE_ENTRY( "bg_tiles", 0, bgtilelayout, 512, 16 )
	GFXDECODE_ENTRY( "sprites", 0, sprite8layout, 0,   16 )

GFXDECODE_END


/*** Machine Driver

there are 3 boards, each with a cpu, boards b and c contain
NEC D70008AC-8's which is just a Z80, board a (the sound board)
has an actual Z80 chip

Sound Hardware should be 2 YM3812's + 2 MSM5205's

The game is displayed on 2 monitors

***/

void tbowl_state::machine_start()
{
	membank("mainbank")->configure_entries(0, 32, memregion("maincpu")->base() + 0x10000, 0x800);
	membank("subbank")->configure_entries(0, 32, memregion("sub")->base() + 0x10000, 0x800);

	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_end));
	save_item(NAME(m_adpcm_data));

}

void tbowl_state::machine_reset()
{
	m_adpcm_pos[0] = m_adpcm_pos[1] = 0;
	m_adpcm_end[0] = m_adpcm_end[1] = 0;
	m_adpcm_data[0] = m_adpcm_data[1] = -1;
	m_soundlatch->acknowledge_w();
}

void tbowl_state::tbowl(machine_config &config)
{
	/* CPU on Board '6206B' */
	Z80(config, m_maincpu, 8000000); /* NEC D70008AC-8 (Z80 Clone) */
	m_maincpu->set_addrmap(AS_PROGRAM, &tbowl_state::_6206B_map);
	m_maincpu->set_vblank_int("lscreen", FUNC(tbowl_state::irq0_line_hold));

	/* CPU on Board '6206C' */
	z80_device &sub(Z80(config, "sub", 8000000)); /* NEC D70008AC-8 (Z80 Clone) */
	sub.set_addrmap(AS_PROGRAM, &tbowl_state::_6206C_map);
	sub.set_vblank_int("lscreen", FUNC(tbowl_state::irq0_line_hold));

	/* CPU on Board '6206A' */
	Z80(config, m_audiocpu, 4000000); /* Actual Z80 */
	m_audiocpu->set_addrmap(AS_PROGRAM, &tbowl_state::_6206A_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tbowl);
	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 1024*2).set_endianness(ENDIANNESS_BIG);
	config.set_default_layout(layout_dualhsxs);

	TECMO_SPRITE(config, m_sprgen, 0);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(32*8, 32*8);
	lscreen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	lscreen.set_screen_update(FUNC(tbowl_state::screen_update_left));
	lscreen.set_palette(m_palette);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(32*8, 32*8);
	rscreen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	rscreen.set_screen_update(FUNC(tbowl_state::screen_update_right));
	rscreen.set_palette(m_palette);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);

	ym3812_device &ym1(YM3812(config, "ym1", 4000000));
	ym1.irq_handler().set_inputline(m_audiocpu, 0);
	ym1.add_route(ALL_OUTPUTS, "mono", 0.80);

	ym3812_device &ym2(YM3812(config, "ym2", 4000000));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);

	/* something for the samples? */
	MSM5205(config, m_msm1, 384000);
	m_msm1->vck_legacy_callback().set(FUNC(tbowl_state::adpcm_int_1));    /* interrupt function */
	m_msm1->set_prescaler_selector(msm5205_device::S48_4B); /* 8KHz */
	m_msm1->add_route(ALL_OUTPUTS, "mono", 0.50);

	MSM5205(config, m_msm2, 384000);
	m_msm2->vck_legacy_callback().set(FUNC(tbowl_state::adpcm_int_2));    /* interrupt function */
	m_msm2->set_prescaler_selector(msm5205_device::S48_4B); /* 8KHz */
	m_msm2->add_route(ALL_OUTPUTS, "mono", 0.50);
}


/* Board Layout from readme.txt

6206A
+-----------------------------------+
|                                   |
|                                   |
|                                   |
|       Z80                         |
|                                   |
|       1                           |
|                                   |
| 3                                 |
|                                   |
| 2                                 |
+-----------------------------------+

6206B
+-----------------------------------+
|                                   |
|                                   |
|                                   |
|   10          6                   |
|   11          7                   |
|   12          8                   |
|   13          9                   |
|                                   |
|                                   |
|                   NEC D70008AC-8  |
|                       4           |
|                       5           |
| 14                                |
| 15                                |
|                                   |
+-----------------------------------+

6206C
+-----------------------------------+
|                                   |
|                                   |
|                                   |
|   D70008AC-8                      |
|   24                              |
|   25                              |
|                                   |
|                   20  16          |
|                   21  17          |
|                   22  18          |
|                   23  19          |
+-----------------------------------+

*/

/*** Rom Loading ***

we currently have two dumps, one appears to be a world/us version, the
other is clearly a Japan version as it displays a regional warning

there is also a bad dump which for reference has the following roms
different to the world dump

    "24.rom" 0x10000 0x39a2d923 (code)
    "25.rom" 0x10000 0x9a0a9cd6 (code / data)

    "21.rom" 0x10000 0x93651858 (gfx)
    "22.rom" 0x10000 0xee7561d9 (gfx)
    "23.rom" 0x10000 0x46b3c186 (gfx)

this fails its rom check so I assume its corrupt

***/


ROM_START( tbowl )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* NEC D70008AC-8 (Z80 Clone) */
	ROM_LOAD( "4.b11",       0x00000, 0x08000, CRC(db8a4f5d) SHA1(730dee040c18ed8736c07a7de0b986f667b0f2f5) )
	ROM_LOAD( "6206b-5.b13", 0x10000, 0x10000, CRC(133c5c11) SHA1(7d4e76db3505ccf033d0d9b8d21feaf09b76dcc4) )

	ROM_REGION( 0x20000, "sub", 0 ) /* NEC D70008AC-8 (Z80 Clone) */
	ROM_LOAD( "6206c-24.h5",   0x00000, 0x10000, CRC(040c8138) SHA1(f6fea192bf2ef0a3f0876133c761488184f54f50) )
	ROM_LOAD( "6206c-25.h7",   0x10000, 0x10000, CRC(92c3cef5) SHA1(75883663b309bf46be544114c6e9086ab222300d) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 */
	ROM_LOAD( "6206a-1.f11",   0x00000, 0x08000, CRC(4370207b) SHA1(2c929b571c86d35e646870644751e86bd16b5e22) )

	ROM_REGION( 0x10000, "characters", 0 ) /* 8x8 Characters inc. Alphanumerics */
	ROM_LOAD16_BYTE( "14.13l",      0x00000, 0x08000, CRC(f9cf60b9) SHA1(0a79ed29f82ac7bd08062f922f79e439c194f30a) )
	ROM_LOAD16_BYTE( "15.15l",      0x00001, 0x08000, CRC(a23f6c53) SHA1(0bb64894a27f41d74117ec492aafd52bc5b16ca4) )

	ROM_REGION( 0x80000, "bg_tiles", 0 ) /* BG GFX */
	ROM_LOAD16_BYTE( "6206b-8.e1",     0x40001, 0x10000, CRC(b9615ffa) SHA1(813896387291f5325ed7e4058347fe35c0d7b839) )
	ROM_LOAD16_BYTE( "6206b-8.e4",     0x40000, 0x10000, CRC(6389c719) SHA1(8043907d6f5b37228c09f05bbf12b4b9bb9bc130) )
	ROM_LOAD16_BYTE( "6206b-7.e2",     0x00001, 0x10000, CRC(d139c397) SHA1(4093220e6bddb95d0af445944bead7a064b64c39) )
	ROM_LOAD16_BYTE( "6206b-9.e6",     0x00000, 0x10000, CRC(975ded4c) SHA1(4045ee12f43dd23dadf6f9d0f7b25d04f9fda3d8) )
	ROM_LOAD16_BYTE( "6206b-10.l1",    0x60001, 0x10000, CRC(9b4fa82e) SHA1(88df18985a04c6653a71db07fbbe0ce0670fe540) )
	ROM_LOAD16_BYTE( "6206b-12.l4",    0x60000, 0x10000, CRC(7d0030f6) SHA1(24f0eca87ce38b974b9f359dd5f12f3be1ae7ff1) )
	ROM_LOAD16_BYTE( "6206b-11.l2",    0x20001, 0x10000, CRC(06bf07bb) SHA1(9f12a39b8832bff2ffd84b7e6c1ddb2855ff924b) )
	ROM_LOAD16_BYTE( "6206b-13.l6",    0x20000, 0x10000, CRC(4ad72c16) SHA1(554474987349b5b11e181ee8a2d1308777b030c1) )

	ROM_REGION( 0x80000, "sprites", 0 ) /* SPR GFX */
	ROM_LOAD16_BYTE( "6206c-16.b11",    0x60001, 0x10000, CRC(1a2fb925) SHA1(bc96ee87372826d5bee2b4d2aefde4c47b9ee80a) )
	ROM_LOAD16_BYTE( "6206c-20.d11",    0x60000, 0x10000, CRC(70bb38a3) SHA1(5145b246f7720dd0359b97be35aa027af07cb6da) )
	ROM_LOAD16_BYTE( "6206c-17.b13",    0x40001, 0x10000, CRC(de16bc10) SHA1(88e2452c7caf44cd541c27fc56c99703f3330bd7) )
	ROM_LOAD16_BYTE( "6206c-21.d13",    0x40000, 0x10000, CRC(41b2a910) SHA1(98bf0fc9728240f35385ab0370bb47108f2d2bc2) )
	ROM_LOAD16_BYTE( "6206c-18.b14",    0x20001, 0x10000, CRC(0684e188) SHA1(3d3c71c915cff62021baa17df37d0a68847d57cf) )
	ROM_LOAD16_BYTE( "6206c-22.d14",    0x20000, 0x10000, CRC(cf660ebc) SHA1(3ca9577a36708c44a1bc9238faf14dbab1a0c3ca) )
	ROM_LOAD16_BYTE( "6206c-19.b16",    0x00001, 0x10000, CRC(71795604) SHA1(57ef4f14dfe1829d5dddeba81bf2f7354d971d27) )
	ROM_LOAD16_BYTE( "6206c-23.d16",    0x00000, 0x10000, CRC(97fba168) SHA1(107de19614d57453a37462e1a4d499d14633d50b) )

	ROM_REGION( 0x20000, "adpcm", 0 )
	ROM_LOAD( "6206a-3.l18",    0x00000, 0x10000, CRC(3aa24744) SHA1(06de3f9a2431777218cc67f59230fddbfa01cf2d) )
	ROM_LOAD( "6206a-2.l16",    0x10000, 0x10000, CRC(1e9e5936) SHA1(60370d1de28b1c5ffeff7843702aaddb19ff1f58) )
ROM_END


ROM_START( tbowla )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* NEC D70008AC-8 (Z80 Clone) */
	ROM_LOAD( "6206b-4.b11",    0x00000, 0x08000, CRC(8c4260b1) SHA1(1559849b00c6ba818a5dae4e96ccc3bf58e6243f) )
	ROM_LOAD( "6206b-5.b13",    0x10000, 0x10000, CRC(133c5c11) SHA1(7d4e76db3505ccf033d0d9b8d21feaf09b76dcc4) )

	ROM_REGION( 0x20000, "sub", 0 ) /* NEC D70008AC-8 (Z80 Clone) */
	ROM_LOAD( "6206c-24.h5",   0x00000, 0x10000, CRC(040c8138) SHA1(f6fea192bf2ef0a3f0876133c761488184f54f50) )
	ROM_LOAD( "6206c-25.h7",   0x10000, 0x10000, CRC(92c3cef5) SHA1(75883663b309bf46be544114c6e9086ab222300d) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 */
	ROM_LOAD( "6206a-1.f11",   0x00000, 0x08000, CRC(4370207b) SHA1(2c929b571c86d35e646870644751e86bd16b5e22) )

	ROM_REGION( 0x10000, "characters", 0 ) /* 8x8 Characters inc. Alphanumerics */
	ROM_LOAD16_BYTE( "6206b-14.l13",      0x00000, 0x08000, CRC(cf99d0bf) SHA1(d1f23e23c2ebd26e2ffe8b23a02d86e4d32c6f11) )
	ROM_LOAD16_BYTE( "6206b-15.l14",      0x00001, 0x08000, CRC(d69248cf) SHA1(4dad6a3fdc36b2fe625df0a43fd9e82d1dfd2af6) )

	ROM_REGION( 0x80000, "bg_tiles", 0 ) /* BG GFX */
	ROM_LOAD16_BYTE( "6206b-8.e1",     0x40001, 0x10000, CRC(b9615ffa) SHA1(813896387291f5325ed7e4058347fe35c0d7b839) )
	ROM_LOAD16_BYTE( "6206b-8.e4",     0x40000, 0x10000, CRC(6389c719) SHA1(8043907d6f5b37228c09f05bbf12b4b9bb9bc130) )
	ROM_LOAD16_BYTE( "6206b-7.e2",     0x00001, 0x10000, CRC(d139c397) SHA1(4093220e6bddb95d0af445944bead7a064b64c39) )
	ROM_LOAD16_BYTE( "6206b-9.e6",     0x00000, 0x10000, CRC(975ded4c) SHA1(4045ee12f43dd23dadf6f9d0f7b25d04f9fda3d8) )
	ROM_LOAD16_BYTE( "6206b-10.l1",    0x60001, 0x10000, CRC(9b4fa82e) SHA1(88df18985a04c6653a71db07fbbe0ce0670fe540) )
	ROM_LOAD16_BYTE( "6206b-12.l4",    0x60000, 0x10000, CRC(7d0030f6) SHA1(24f0eca87ce38b974b9f359dd5f12f3be1ae7ff1) )
	ROM_LOAD16_BYTE( "6206b-11.l2",    0x20001, 0x10000, CRC(06bf07bb) SHA1(9f12a39b8832bff2ffd84b7e6c1ddb2855ff924b) )
	ROM_LOAD16_BYTE( "6206b-13.l6",    0x20000, 0x10000, CRC(4ad72c16) SHA1(554474987349b5b11e181ee8a2d1308777b030c1) )

	ROM_REGION( 0x80000, "sprites", 0 ) /* SPR GFX */
	ROM_LOAD16_BYTE( "6206c-16.b11",    0x60001, 0x10000, CRC(1a2fb925) SHA1(bc96ee87372826d5bee2b4d2aefde4c47b9ee80a) )
	ROM_LOAD16_BYTE( "6206c-20.d11",    0x60000, 0x10000, CRC(70bb38a3) SHA1(5145b246f7720dd0359b97be35aa027af07cb6da) )
	ROM_LOAD16_BYTE( "6206c-17.b13",    0x40001, 0x10000, CRC(de16bc10) SHA1(88e2452c7caf44cd541c27fc56c99703f3330bd7) )
	ROM_LOAD16_BYTE( "6206c-21.d13",    0x40000, 0x10000, CRC(41b2a910) SHA1(98bf0fc9728240f35385ab0370bb47108f2d2bc2) )
	ROM_LOAD16_BYTE( "6206c-18.b14",    0x20001, 0x10000, CRC(0684e188) SHA1(3d3c71c915cff62021baa17df37d0a68847d57cf) )
	ROM_LOAD16_BYTE( "6206c-22.d14",    0x20000, 0x10000, CRC(cf660ebc) SHA1(3ca9577a36708c44a1bc9238faf14dbab1a0c3ca) )
	ROM_LOAD16_BYTE( "6206c-19.b16",    0x00001, 0x10000, CRC(71795604) SHA1(57ef4f14dfe1829d5dddeba81bf2f7354d971d27) )
	ROM_LOAD16_BYTE( "6206c-23.d16",    0x00000, 0x10000, CRC(97fba168) SHA1(107de19614d57453a37462e1a4d499d14633d50b) )

	ROM_REGION( 0x20000, "adpcm", 0 )
	ROM_LOAD( "6206a-3.l18",    0x00000, 0x10000, CRC(3aa24744) SHA1(06de3f9a2431777218cc67f59230fddbfa01cf2d) )
	ROM_LOAD( "6206a-2.l16",    0x10000, 0x10000, CRC(1e9e5936) SHA1(60370d1de28b1c5ffeff7843702aaddb19ff1f58) )
ROM_END

ROM_START( tbowlp )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* NEC D70008AC-8 (Z80 Clone) */
	// same as 'tbowl'
	ROM_LOAD( "4.b11",               0x00000, 0x08000, CRC(db8a4f5d) SHA1(730dee040c18ed8736c07a7de0b986f667b0f2f5) )
	ROM_LOAD( "main_data_10-25.b13", 0x10000, 0x10000, CRC(133c5c11) SHA1(7d4e76db3505ccf033d0d9b8d21feaf09b76dcc4) )

	ROM_REGION( 0x20000, "sub", 0 ) /* NEC D70008AC-8 (Z80 Clone) */
	// different to other sets
	ROM_LOAD( "sub_pro_10-29.h5",   0x00000, 0x10000, CRC(1933a3f0) SHA1(e19b3d7ad3cf6ccfc7b51240608f0edb95a50b5a) )
	ROM_LOAD( "sub_data_10-25.h7",  0x10000, 0x10000, CRC(7277c852) SHA1(0b9e607159f54cf59727299c82cfc01dd90c8eb3) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 */
	// this rom is quite strange, maybe damaged / badly programmed? areas which should be a 0x00 0x00 end up being
	// a 0x00 / 0xff alternating pattern, and there are some odd sounds at times.  It does however read consistently
	// and is a different revision of the code to the other sets, so it might be correct and we can't just replace it
	// with a rom from another set.
	ROM_LOAD( "6206_sound_10-25.f11",   0x00000, 0x08000, CRC(2158472d) SHA1(bc47f4d59505fec6a5c2b924cbe8fc6d6cd4609e) )

	ROM_REGION( 0x10000, "characters", 0 ) /* 8x8 Characters inc. Alphanumerics */
	ROM_LOAD16_BYTE( "14.13l",      0x00000, 0x08000, CRC(f9cf60b9) SHA1(0a79ed29f82ac7bd08062f922f79e439c194f30a) )
	ROM_LOAD16_BYTE( "15.15l",      0x00001, 0x08000, CRC(a23f6c53) SHA1(0bb64894a27f41d74117ec492aafd52bc5b16ca4) )

	ROM_REGION( 0x80000, "bg_tiles", 0 ) /* BG GFX */
	ROM_LOAD16_BYTE( "6206b-8.e1",     0x40001, 0x10000, CRC(b9615ffa) SHA1(813896387291f5325ed7e4058347fe35c0d7b839) )
	ROM_LOAD16_BYTE( "6206b-8.e4",     0x40000, 0x10000, CRC(6389c719) SHA1(8043907d6f5b37228c09f05bbf12b4b9bb9bc130) )
	ROM_LOAD16_BYTE( "6206b-7.e2",     0x00001, 0x10000, CRC(d139c397) SHA1(4093220e6bddb95d0af445944bead7a064b64c39) )
	ROM_LOAD16_BYTE( "6206b-9.e6",     0x00000, 0x10000, CRC(975ded4c) SHA1(4045ee12f43dd23dadf6f9d0f7b25d04f9fda3d8) )
	ROM_LOAD16_BYTE( "6206b-10.l1",    0x60001, 0x10000, CRC(9b4fa82e) SHA1(88df18985a04c6653a71db07fbbe0ce0670fe540) )
	ROM_LOAD16_BYTE( "6206b-12.l4",    0x60000, 0x10000, CRC(7d0030f6) SHA1(24f0eca87ce38b974b9f359dd5f12f3be1ae7ff1) )
	ROM_LOAD16_BYTE( "6206b-11.l2",    0x20001, 0x10000, CRC(06bf07bb) SHA1(9f12a39b8832bff2ffd84b7e6c1ddb2855ff924b) )
	ROM_LOAD16_BYTE( "6206b-13.l6",    0x20000, 0x10000, CRC(4ad72c16) SHA1(554474987349b5b11e181ee8a2d1308777b030c1) )

	ROM_REGION( 0x80000, "sprites", 0 ) /* SPR GFX */
	// todo: check how these differ
	ROM_LOAD16_BYTE( "sp_7_10-16.b11",  0x60001, 0x10000, CRC(807af46a) SHA1(c7b2ce489b129de16e1081595c255b85ea2b684a) )
	ROM_LOAD16_BYTE( "sp_6_10-16.d11",  0x60000, 0x10000, CRC(3c5654a9) SHA1(44f8d251c9f5d8c2c0aaaf23426c16d3fedaa0c0) )
	ROM_LOAD16_BYTE( "6206c-17.b13",    0x40001, 0x10000, CRC(de16bc10) SHA1(88e2452c7caf44cd541c27fc56c99703f3330bd7) )
	ROM_LOAD16_BYTE( "6206c-21.d13",    0x40000, 0x10000, CRC(41b2a910) SHA1(98bf0fc9728240f35385ab0370bb47108f2d2bc2) )
	ROM_LOAD16_BYTE( "6206c-18.b14",    0x20001, 0x10000, CRC(0684e188) SHA1(3d3c71c915cff62021baa17df37d0a68847d57cf) )
	ROM_LOAD16_BYTE( "6206c-22.d14",    0x20000, 0x10000, CRC(cf660ebc) SHA1(3ca9577a36708c44a1bc9238faf14dbab1a0c3ca) )
	ROM_LOAD16_BYTE( "6206c-19.b16",    0x00001, 0x10000, CRC(71795604) SHA1(57ef4f14dfe1829d5dddeba81bf2f7354d971d27) )
	ROM_LOAD16_BYTE( "6206c-23.d16",    0x00000, 0x10000, CRC(97fba168) SHA1(107de19614d57453a37462e1a4d499d14633d50b) )

	ROM_REGION( 0x20000, "adpcm", 0 )
	ROM_LOAD( "6206a-3.l18",    0x00000, 0x10000, CRC(3aa24744) SHA1(06de3f9a2431777218cc67f59230fddbfa01cf2d) )
	ROM_LOAD( "6206a-2.l16",    0x10000, 0x10000, CRC(1e9e5936) SHA1(60370d1de28b1c5ffeff7843702aaddb19ff1f58) )
ROM_END

ROM_START( tbowlj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* NEC D70008AC-8 (Z80 Clone) */
	ROM_LOAD( "6206b.4",    0x00000, 0x08000, CRC(7ed3eff7) SHA1(4a17f2838e9bbed8b1638783c62d07d1074e2b35) )
	ROM_LOAD( "6206b.5",    0x10000, 0x10000, CRC(133c5c11) SHA1(7d4e76db3505ccf033d0d9b8d21feaf09b76dcc4) )

	ROM_REGION( 0x20000, "sub", 0 ) /* NEC D70008AC-8 (Z80 Clone) */
	ROM_LOAD( "6206c-24.h5",   0x00000, 0x10000, CRC(040c8138) SHA1(f6fea192bf2ef0a3f0876133c761488184f54f50) )
	ROM_LOAD( "6206c-25.h7",   0x10000, 0x10000, CRC(92c3cef5) SHA1(75883663b309bf46be544114c6e9086ab222300d) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 */
	ROM_LOAD( "6206a-1.f11",   0x00000, 0x08000, CRC(4370207b) SHA1(2c929b571c86d35e646870644751e86bd16b5e22) )

	ROM_REGION( 0x10000, "characters", 0 ) /* 8x8 Characters inc. Alphanumerics */
	ROM_LOAD16_BYTE( "6206b-14.l13",      0x00000, 0x08000, CRC(cf99d0bf) SHA1(d1f23e23c2ebd26e2ffe8b23a02d86e4d32c6f11) )
	ROM_LOAD16_BYTE( "6206b-15.l14",      0x00001, 0x08000, CRC(d69248cf) SHA1(4dad6a3fdc36b2fe625df0a43fd9e82d1dfd2af6) )

	ROM_REGION( 0x80000, "bg_tiles", 0 ) /* BG GFX */
	ROM_LOAD16_BYTE( "6206b-8.e1",     0x40001, 0x10000, CRC(b9615ffa) SHA1(813896387291f5325ed7e4058347fe35c0d7b839) )
	ROM_LOAD16_BYTE( "6206b-8.e4",     0x40000, 0x10000, CRC(6389c719) SHA1(8043907d6f5b37228c09f05bbf12b4b9bb9bc130) )
	ROM_LOAD16_BYTE( "6206b-7.e2",     0x00001, 0x10000, CRC(d139c397) SHA1(4093220e6bddb95d0af445944bead7a064b64c39) )
	ROM_LOAD16_BYTE( "6206b-9.e6",     0x00000, 0x10000, CRC(975ded4c) SHA1(4045ee12f43dd23dadf6f9d0f7b25d04f9fda3d8) )
	ROM_LOAD16_BYTE( "6206b-10.l1",    0x60001, 0x10000, CRC(9b4fa82e) SHA1(88df18985a04c6653a71db07fbbe0ce0670fe540) )
	ROM_LOAD16_BYTE( "6206b-12.l4",    0x60000, 0x10000, CRC(7d0030f6) SHA1(24f0eca87ce38b974b9f359dd5f12f3be1ae7ff1) )
	ROM_LOAD16_BYTE( "6206b-11.l2",    0x20001, 0x10000, CRC(06bf07bb) SHA1(9f12a39b8832bff2ffd84b7e6c1ddb2855ff924b) )
	ROM_LOAD16_BYTE( "6206b-13.l6",    0x20000, 0x10000, CRC(4ad72c16) SHA1(554474987349b5b11e181ee8a2d1308777b030c1) )

	ROM_REGION( 0x80000, "sprites", 0 ) /* SPR GFX */
	ROM_LOAD16_BYTE( "6206c-16.b11",    0x60001, 0x10000, CRC(1a2fb925) SHA1(bc96ee87372826d5bee2b4d2aefde4c47b9ee80a) )
	ROM_LOAD16_BYTE( "6206c-20.d11",    0x60000, 0x10000, CRC(70bb38a3) SHA1(5145b246f7720dd0359b97be35aa027af07cb6da) )
	ROM_LOAD16_BYTE( "6206c-17.b13",    0x40001, 0x10000, CRC(de16bc10) SHA1(88e2452c7caf44cd541c27fc56c99703f3330bd7) )
	ROM_LOAD16_BYTE( "6206c-21.d13",    0x40000, 0x10000, CRC(41b2a910) SHA1(98bf0fc9728240f35385ab0370bb47108f2d2bc2) )
	ROM_LOAD16_BYTE( "6206c-18.b14",    0x20001, 0x10000, CRC(0684e188) SHA1(3d3c71c915cff62021baa17df37d0a68847d57cf) )
	ROM_LOAD16_BYTE( "6206c-22.d14",    0x20000, 0x10000, CRC(cf660ebc) SHA1(3ca9577a36708c44a1bc9238faf14dbab1a0c3ca) )
	ROM_LOAD16_BYTE( "6206c-19.b16",    0x00001, 0x10000, CRC(71795604) SHA1(57ef4f14dfe1829d5dddeba81bf2f7354d971d27) )
	ROM_LOAD16_BYTE( "6206c-23.d16",    0x00000, 0x10000, CRC(97fba168) SHA1(107de19614d57453a37462e1a4d499d14633d50b) )

	ROM_REGION( 0x20000, "adpcm", 0 )
	ROM_LOAD( "6206a-3.l18",    0x00000, 0x10000, CRC(3aa24744) SHA1(06de3f9a2431777218cc67f59230fddbfa01cf2d) )
	ROM_LOAD( "6206a-2.l16",    0x10000, 0x10000, CRC(1e9e5936) SHA1(60370d1de28b1c5ffeff7843702aaddb19ff1f58) )
ROM_END

GAME( 1987, tbowl,    0,        tbowl,    tbowl,  tbowl_state, empty_init, ROT0, "Tecmo", "Tecmo Bowl (World, set 1)",      MACHINE_SUPPORTS_SAVE )
GAME( 1987, tbowla,   tbowl,    tbowl,    tbowl,  tbowl_state, empty_init, ROT0, "Tecmo", "Tecmo Bowl (World, set 2)",      MACHINE_SUPPORTS_SAVE )
GAME( 1987, tbowlp,   tbowl,    tbowl,    tbowl,  tbowl_state, empty_init, ROT0, "Tecmo", "Tecmo Bowl (World, prototype?)", MACHINE_SUPPORTS_SAVE ) // or early version, handwritten labels
GAME( 1987, tbowlj,   tbowl,    tbowl,    tbowlj, tbowl_state, empty_init, ROT0, "Tecmo", "Tecmo Bowl (Japan)",             MACHINE_SUPPORTS_SAVE )
