// license:BSD-3-Clause
// copyright-holders:Luca Elia
/******************************************************************************

                            -= Clash Road =-

                    driver by   Luca Elia (l.elia@tin.it)

Main  CPU   :   Z80A (LH0080A)

Video Chips :   ?

Sound CPU   :   Z80A (LH0080A)

Sound Chips :   Custom (Nichibutsu?)

XTAL        :   18.432 MHz

TODO:
- clshroad: erratic gameplay/sound speed.
  Being pretty logical that CPUs runs at master clock / 6 then we also need to
  halve the vblank irq rate so that opponents won't pop up in the middle of the
  screen. Main CPU would also overrun the sound CPU way too much otherwise.
  We also need to hand tune the sound frequencies compared to the other games
  so that it won't cut off BGMs abruptly during playback.
  TL;DR needs verification of all clocks with a PCB;
- firebatl: video (https://tmblr.co/ZgJvzv2E2C_z-) shows transparency for the
  text layer is not correctly emulated, fixed by initializing VRAM to 0xf0?
  (that layer seems unused by this game);
- firebatl: bad sprite colors, most notably player ship (should be way darker);
- firebatl: remove ROM patch;
- firebatl: reads $6000-$6002 and $6100 at POST, and in the range $6100-$61ff
  before every start of gameplay/after player dies.
  Currently 0-filled in ROM loading:
  - $6100 is actually OR-ed with the coinage work RAM buffer setting at $8022;
  - $6124 is shifted right once at PC=0x5df and stored to $82e6, which is later
    checked at PC=0x187 and must be $01 otherwise game goes into an infinite
    loop after dying (without ROM patch);
  - (more ...)

*******************************************************************************/

#include "emu.h"
#include "clshroad.h"
#include "wiping_a.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "speaker.h"

#define MASTER_CLOCK XTAL(18'432'000)

void clshroad_state::machine_start()
{
	save_item(NAME(m_main_irq_mask));
	save_item(NAME(m_sound_irq_mask));
	save_item(NAME(m_color_bank));
}

void clshroad_state::machine_reset()
{
	flip_screen_set(0);
	m_main_irq_mask = m_sound_irq_mask = 0;
	// not initialized by HW, matches grey background on first title screen
	for(int i = 0;i<0x800;i++)
		m_vram_0[i] = 0xf0;
}


uint8_t clshroad_state::input_r(offs_t offset)
{
	return  ((~ioport("P1")->read() & (1 << offset)) ? 1 : 0) |
			((~ioport("P2")->read() & (1 << offset)) ? 2 : 0) |
			((~ioport("DSW1")->read() & (1 << offset)) ? 4 : 0) |
			((~ioport("DSW2")->read() & (1 << offset)) ? 8 : 0) ;
}


// irq/reset controls like in wiping.cpp

WRITE_LINE_MEMBER(clshroad_state::main_irq_mask_w)
{
	m_main_irq_mask = state;
}

WRITE_LINE_MEMBER(clshroad_state::sound_irq_mask_w)
{
	m_sound_irq_mask = state;
}


void clshroad_state::clshroad_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x95ff).ram();
	map(0x9600, 0x97ff).ram().share("share1");
	map(0x9800, 0x9dff).ram();
	map(0x9e00, 0x9fff).ram().share("spriteram");
	map(0xa000, 0xa007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xa100, 0xa107).r(FUNC(clshroad_state::input_r));
	map(0xa800, 0xafff).ram().w(FUNC(clshroad_state::vram_1_w)).share("vram_1"); // Layer 1
	map(0xb000, 0xb001).writeonly().share("vregs"); // Scroll
	map(0xb002, 0xb002).w(FUNC(clshroad_state::color_bank_w));
	map(0xb003, 0xb003).w(FUNC(clshroad_state::video_unk_w));
	map(0xc000, 0xc7ff).ram().w(FUNC(clshroad_state::vram_0_w)).share("vram_0"); // Layer 0
}

void clshroad_state::clshroad_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x7fff).w("custom", FUNC(wiping_sound_device::sound_w));
	map(0x9600, 0x97ff).ram().share("share1");
	map(0xa000, 0xa007).w("mainlatch", FUNC(ls259_device::write_d0));
}



static INPUT_PORTS_START( clshroad )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5") // Damage when falling
	PORT_DIPSETTING(    0x18, DEF_STR( Normal )  )  // 8
	PORT_DIPSETTING(    0x10, DEF_STR( Hard )    )  // A
	PORT_DIPSETTING(    0x08, DEF_STR( Harder )  )  // C
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )  // E
	PORT_DIPNAME( 0x20, 0x20, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )        /* Listed as "Unused" */

	/* DSW2 is listed as "Unused" */
	PORT_START("DSW2")
/*
first bit OFF is:   0           0   <- value
                    1           1
                    2           2
                    3           3
                    4           4
                    5           5
                    6           6
                    else        FF

But the values seems unused then.
*/
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */
INPUT_PORTS_END

static INPUT_PORTS_START( firebatl )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	// TODO: unconventional default/structure, may or may not be modified by contents of $6000-$7fff
	// read at $8304
	PORT_DIPNAME( 0x7f, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x07, "4" )
	PORT_DIPSETTING(    0x0f, "5" )
	PORT_DIPSETTING(    0x1f, "6" )
	PORT_DIPSETTING(    0x3f, "7" )
	PORT_DIPSETTING(    0x7f, "255" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "10K 30K+" )
	PORT_DIPSETTING(    0x00, "20K 30K+" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END


static const gfx_layout layout_8x8x2 =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,8*2) },
	8*8*2
};

static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2) + 0, RGN_FRAC(1,2) + 4, 0, 4 },
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,8*2) },
	8*8*2
};

static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2) + 0, RGN_FRAC(1,2) + 4, 0, 4 },
	{ STEP4(0,1), STEP4(8,1), STEP4(8*8*2+0,1), STEP4(8*8*2+8,1) },
	{ STEP8(0,8*2), STEP8(8*8*2*2,8*2) },
	16*16*2
};

static GFXDECODE_START( gfx_firebatl )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4,   0, 16 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4,   0, 16 ) // [1] Layer 0
	GFXDECODE_ENTRY( "gfx3", 0, layout_8x8x2,   512, 64 ) // [2] Layer 1
GFXDECODE_END

static GFXDECODE_START( gfx_clshroad )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4,   0, 16 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4,   0, 16 ) // [1] Layer 0
	GFXDECODE_ENTRY( "gfx3", 0, layout_8x8x4,     0, 16 ) // [2] Layer 1
GFXDECODE_END



INTERRUPT_GEN_MEMBER(clshroad_state::vblank_irq)
{
	if(m_main_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(clshroad_state::half_vblank_irq)
{
	// without this then clshroad runs too fast & BGMs stops playing exactly halfway thru.
	// it also otherwise make opponents to pop up in the middle of the screen
	if (m_screen->frame_number() & 1)
		return;

	if(m_main_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(clshroad_state::sound_timer_irq)
{
	if(m_sound_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

void clshroad_state::firebatl(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK / 6); // μPD780C running at 3.072 MHz? Overruns max frequency of 2.5 MHz ...
	m_maincpu->set_addrmap(AS_PROGRAM, &clshroad_state::clshroad_map);
	m_maincpu->set_vblank_int("screen", FUNC(clshroad_state::vblank_irq));

	Z80(config, m_audiocpu, MASTER_CLOCK / 6); // μPD780C running at 3.072 MHz? Overruns max frequency of 2.5 MHz ...
	m_audiocpu->set_addrmap(AS_PROGRAM, &clshroad_state::clshroad_sound_map);
	m_audiocpu->set_periodic_int(FUNC(clshroad_state::sound_timer_irq), attotime::from_hz(120)); // periodic interrupt, exact frequency unknown

	config.set_maximum_quantum(attotime::from_hz(MASTER_CLOCK / 6 / 512)); // 6000 Hz

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set_inputline(m_audiocpu, INPUT_LINE_RESET).invert();
	mainlatch.q_out_cb<1>().set(FUNC(clshroad_state::main_irq_mask_w));
	mainlatch.q_out_cb<3>().set(FUNC(clshroad_state::sound_irq_mask_w));
	mainlatch.q_out_cb<4>().set(FUNC(clshroad_state::flipscreen_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK / 3, 384, 0, 288, 264, 16, 240); // unknown, single XTAL on PCB & 288x224 suggests 60.606060 Hz like Galaxian HW
	m_screen->set_screen_update(FUNC(clshroad_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_firebatl);
	PALETTE(config, m_palette, FUNC(clshroad_state::firebatl_palette), 512+64*4, 256);

	MCFG_VIDEO_START_OVERRIDE(clshroad_state,firebatl)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	WIPING_CUSTOM(config, "custom", 96000 / 2).add_route(ALL_OUTPUTS, "mono", 1.0); // 48000 Hz?
}

void clshroad_state::clshroad(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK / 6);  // LH0080A running at 3.072 MHz? /5 is too fast, /6 matches wiping.cpp
	m_maincpu->set_addrmap(AS_PROGRAM, &clshroad_state::clshroad_map);
	m_maincpu->set_vblank_int("screen", FUNC(clshroad_state::half_vblank_irq));

	Z80(config, m_audiocpu, MASTER_CLOCK / 6); // LH0080A running at 3.072 MHz?
	m_audiocpu->set_addrmap(AS_PROGRAM, &clshroad_state::clshroad_sound_map);
	// TODO: by logic this should be MASTER_CLOCK / 3 / 65536 = 93.75 Hz, but it quite don't work right.
	m_audiocpu->set_periodic_int(FUNC(clshroad_state::sound_timer_irq), attotime::from_hz(82.75)); // periodic interrupt, exact frequency unknown

	config.set_maximum_quantum(attotime::from_hz(MASTER_CLOCK / 6 / 512)); // 6000 Hz

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set_nop(); // never writes here?
	mainlatch.q_out_cb<1>().set(FUNC(clshroad_state::main_irq_mask_w));
	mainlatch.q_out_cb<3>().set(FUNC(clshroad_state::sound_irq_mask_w));
	mainlatch.q_out_cb<4>().set(FUNC(clshroad_state::flipscreen_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK / 3, 384, 0, 288, 264, 16, 240); // unknown, single XTAL on PCB & 288x224 suggests 60.606060 Hz like Galaxian HW
	m_screen->set_screen_update(FUNC(clshroad_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_clshroad);
	PALETTE(config, m_palette, FUNC(clshroad_state::clshroad_palette), 256);

	MCFG_VIDEO_START_OVERRIDE(clshroad_state,clshroad)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	WIPING_CUSTOM(config, "custom", MASTER_CLOCK / 3 / 256).add_route(ALL_OUTPUTS, "mono", 1.0); // 24000 Hz?
}


/***************************************************************************

 Fire Battle by TAITO/GRAPHIC RESEARCH (1984)

 Location     Device        File ID      Checksum
 ------------------------------------------------
 CPU E8        2764          ROM01         4683   [ main program ]
 CPU D8        2764          ROM02         6903   [ main program ]
 CPU C8        2764          ROM03         767F   [ main program ]
 CPU R6        2764          ROM04         B4AE   [ SND  program ]
 CPU F3        2764          ROM05         E853   [  sound Data  ]
 VID U4        2764          ROM06         F507   [     GFX      ]
 VID S4        2764          ROM07         4D76   [     GFX      ]
 VID P4        2764          ROM08         B79F   [     GFX      ]
 VID N4        2764          ROM09         333B   [     GFX      ]
 VID K4        2764          ROM11         D60D   [     GFX      ]
 VID J4        2764          ROM12         E22A   [     GFX      ]
 VID H4        2764          ROM13         4ABB   [     GFX      ]
 VID F4        2764          ROM14         FCA6   [     GFX      ]
 VID M4        2732          ROM15         D8B9   [     GFX      ]
 CPU N2   TBP18S030      PROM1.BPR         1614
 CPU K2    TBP24S10      PROM2.BPR         075C
 CPU J4    TBP24S10      PROM3.BPR         0680
 VID S1    TBP24S10      PROM4.BPR         03C0
 VID P1    TBP24S10      PROM5.BPR         0780
 VID H1    TBP24S10      PROM6.BPR         048B
 VID F1    TBP24S10      PROM7.BPR         0439
 VID E1    TBP24S10      PROM8.BPR         045D
 VID P2    TBP24S10      PROM9.BPR         0A13
 VID N2    TBP24S10     PROM10.BPR         060B
 VID M2    TBP24S10     PROM11.BPR         03B8
 VID H2    TBP24S10     PROM12.BPR         0A90
 VID W8    TBP24S10     PROM13.BPR         0E7E

 Notes:   CPU - Top PCB       (SCO-102A 1983 GRC) *
          VID - Lower PCB     (GRP-109C 1984)     **

 *  2 x NEC D780C
 ** 18.??? MHz clock

***************************************************************************/

ROM_START( firebatl )
	ROM_REGION( 0x8000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "rom01.e8", 0x00000, 0x2000, CRC(10e24ef6) SHA1(b6dae9824eb3cecececbdfdb416a90b1b61ff18d) )
	ROM_LOAD( "rom02.d8", 0x02000, 0x2000, CRC(47f79bee) SHA1(23e64ff69ff5112b0413d12a283ca90cf3642389) )
	ROM_LOAD( "rom03.c8", 0x04000, 0x2000, CRC(693459b9) SHA1(8bba526960f49c9e6c7bca40eb8fbbfc81588660) )
	ROM_FILL(             0x06000, 0x2000, 0x00 ) // Accessed, could this range map to the Z4 device at A8 ?!

	ROM_REGION( 0x2000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "rom04.r6", 0x0000, 0x2000, CRC(5f232d9a) SHA1(d0b9926cb02203f1a1f7fd0d0d7b1fe8eddc6511) )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "rom14.f4", 0x0000, 0x2000, CRC(36a508a7) SHA1(9b2dede4332d2b8e55e7c5f916d8cf370d7e77fc) )
	ROM_LOAD( "rom13.h4", 0x2000, 0x2000, CRC(a2ec508e) SHA1(a6dd7b9729f320ed3a28e0cd8ea7b26c2a639e1a) )
	ROM_LOAD( "rom12.j4", 0x4000, 0x2000, CRC(f80ece92) SHA1(2cc4317b2c58be48dc285bb3a667863e2ca8d5b7) )
	ROM_LOAD( "rom11.k4", 0x6000, 0x2000, CRC(b293e701) SHA1(9dacaa9897d91dc465f2c1907804fed9bfb7207b) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_INVERT ) /* Layer 0 */
	ROM_LOAD( "rom09.n4", 0x0000, 0x2000, CRC(77ea3e39) SHA1(c897664bd4f4b163a557d39d12374dae08a0a0c2) )
	ROM_LOAD( "rom08.p4", 0x2000, 0x2000, CRC(1b7585dd) SHA1(e402c879c5651bf0fa21dcf1ff3c4b7bf690cbaa) )
	ROM_LOAD( "rom07.s4", 0x4000, 0x2000, CRC(e3ec9825) SHA1(ea266683a48e8515d40ed077fd55d15a1859c942) )
	ROM_LOAD( "rom06.u4", 0x6000, 0x2000, CRC(d29fab5f) SHA1(de5f8d57d3dd9090e6c056ff7f1ab0bb59630863) )

	ROM_REGION( 0x1000, "gfx3", 0 )    /* Layer 1 */
	ROM_LOAD( "rom15.m4", 0x0000, 0x1000, CRC(8b5464d6) SHA1(e65acd280c0d9776cb80073241cf260b76ff0ca6) )

	ROM_REGION( 0x0a20, "proms", 0 )
	ROM_LOAD( "prom6.h1",  0x0000, 0x0100, CRC(b117d22c) SHA1(357efed6597757907077a7e5130bfa643d5dd197) ) /* palette red */
	ROM_LOAD( "prom7.f1",  0x0100, 0x0100, CRC(9b6b4f56) SHA1(7fd726a20fce40b8ba4b8ef05fb51a85ad9fd282) ) /* palette green */
	ROM_LOAD( "prom8.e1",  0x0200, 0x0100, CRC(67cb68ae) SHA1(9b54c7e51d8db0d8699723173709f04dd2fdfa77) ) /* palette blue */
	ROM_LOAD( "prom9.p2",  0x0300, 0x0100, CRC(dd015b80) SHA1(ce45577204cfbbe623121c1bd99a190464ae7895) ) /* char lookup table msb */
	ROM_LOAD( "prom10.n2", 0x0400, 0x0100, CRC(71b768c7) SHA1(3d8c106758d279daf8e989d4c1bb72de3419d2d6) ) /* char lookup table lsb */
	ROM_LOAD( "prom4.s1",  0x0500, 0x0100, CRC(06523b81) SHA1(0042c364fd2fabd6b04cb2d59a71a7e6deb90ab3) ) /* unknown */
	ROM_LOAD( "prom5.p1",  0x0600, 0x0100, CRC(75ea8f70) SHA1(1a2c478e7b87fa7f8725a3d1ff06c5c9422dd524) ) /* unknown */
	ROM_LOAD( "prom11.m2", 0x0700, 0x0100, CRC(ba42a582) SHA1(2e8f3dab82a34078b866e9875978e83fef045f86) ) /* unknown */
	ROM_LOAD( "prom12.h2", 0x0800, 0x0100, CRC(f2540c51) SHA1(126f698eb65e54fa16a1abfa5b40b0161cb66254) ) /* unknown */
	ROM_LOAD( "prom13.w8", 0x0900, 0x0100, CRC(4e2a2781) SHA1(7be2e066499ea0af76f6ae926fe87e02f8c36a6f) ) /* unknown */
	ROM_LOAD( "prom1.n2",  0x0a00, 0x0020, CRC(1afc04f0) SHA1(38207cf3e15bac7034ac06469b95708d22b57da4) ) /* timing? (on the cpu board) */

	ROM_REGION( 0x2000, "custom:samples", 0 )
	ROM_LOAD( "rom05.f3", 0x0000, 0x2000, CRC(21544cd6) SHA1(b9644ab3c4393cd2669d2b5b3c80d7a9f1c91ca6) )

	ROM_REGION( 0x0200, "custom:soundproms", 0 )   /* 4bit->8bit sample expansion PROMs */
	ROM_LOAD( "prom3.j4", 0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) ) /* low 4 bits */
	ROM_LOAD( "prom2.k2", 0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) ) /* high 4 bits */
ROM_END

ROM_START( clshroad )
	ROM_REGION( 0x8000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "clashr3.bin", 0x0000, 0x8000, CRC(865c32ae) SHA1(e5cdd2d624fe6dc8bd6bebf2bd1c79d287408c63) )

	ROM_REGION( 0x2000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "clashr2.bin", 0x0000, 0x2000, CRC(e6389ec1) SHA1(6ec94d5e389e9104f40fc48df6f15674415851c0) )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "clashr5.bin", 0x0000, 0x4000, CRC(094858b8) SHA1(a19f79cb665bbb1e25a94e9dd09a9e99f553afe8) )
	ROM_LOAD( "clashr6.bin", 0x4000, 0x4000, CRC(daa1daf3) SHA1(cc24c97c9950adc0041f68832774e40c87d1d4b2) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_INVERT ) /* Layer 0 */
	ROM_LOAD( "clashr8.bin", 0x0000, 0x4000, CRC(cbb66719) SHA1(2497575f84a956bc2b9e4c3f2c71ae42d036355e) )
	ROM_LOAD( "clashr9.bin", 0x4000, 0x4000, CRC(c15e8eed) SHA1(3b1e7fa014d176a01d5f9214051b0c8cc5556684) )

	ROM_REGION( 0x4000, "gfx3", ROMREGION_INVERT)  /* Layer 1 */
	ROM_LOAD( "clashr4.bin", 0x0000, 0x2000, CRC(664201d9) SHA1(4eb85306f0c9683d0e0cf787f6389df8fe4a3d9d) )
	ROM_LOAD( "clashr7.bin", 0x2000, 0x2000, CRC(97973030) SHA1(cca7a9d2751add7f6dd9bac83f7f63ece8021dbc) )

	ROM_REGION( 0x0b40, "proms", 0 )
	ROM_LOAD( "82s129.6", 0x0000, 0x0100, CRC(38f443da) SHA1(a015217508b18eb3f1987cd5b53f31608b13de08) )    /* r */
	ROM_LOAD( "82s129.7", 0x0100, 0x0100, CRC(977fab0c) SHA1(78e7b4f1e9891d2d9cf1e1ec0c4f59a311cef1c5) )    /* g */
	ROM_LOAD( "82s129.8", 0x0200, 0x0100, CRC(ae7ae54d) SHA1(d7d4682e437f2f7adb7fceb813437c06f27f2711) )    /* b */
	/* all other proms that firebatl has are missing */
	ROM_LOAD( "clashrd.a2",  0x0900, 0x0100, CRC(4e2a2781) SHA1(7be2e066499ea0af76f6ae926fe87e02f8c36a6f) ) /* unknown */
	ROM_LOAD( "clashrd.g4",  0x0a00, 0x0020, CRC(1afc04f0) SHA1(38207cf3e15bac7034ac06469b95708d22b57da4) ) /* timing? */
	ROM_LOAD( "clashrd.b11", 0x0a20, 0x0020, CRC(d453f2c5) SHA1(7fdc5bf59bad9e8f00e970565ff6f6b3773541db) ) /* unknown (possibly bad dump) */
	ROM_LOAD( "clashrd.g10", 0x0a40, 0x0100, CRC(73afefd0) SHA1(d14c5490c5b174d54043bfdf5c6fb675e67492e7) ) /* unknown (possibly bad dump) */

	ROM_REGION( 0x2000, "custom:samples", 0 )
	ROM_LOAD( "clashr1.bin", 0x0000, 0x2000, CRC(0d0a8068) SHA1(529878d0c5f078590e07ec0fffc27b212843c0ad) )

	ROM_REGION( 0x0200, "custom:soundproms", 0 )   /* 4bit->8bit sample expansion PROMs */
	ROM_LOAD( "clashrd.g8",  0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) ) /* low 4 bits */
	ROM_LOAD( "clashrd.g7",  0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) ) /* high 4 bits */
ROM_END

ROM_START( clshroads )
	ROM_REGION( 0x8000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "cr-3",  0x0000, 0x8000, CRC(23559df2) SHA1(41a08a4fbad3da1898226e2ca1956a9f7c8f94b0) )

	ROM_REGION( 0x2000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "clashr2.bin", 0x0000, 0x2000, CRC(e6389ec1) SHA1(6ec94d5e389e9104f40fc48df6f15674415851c0) )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "cr-12", 0x0000, 0x2000, CRC(e5aa4c46) SHA1(c0ed717e263aca2d0ec8a078f0aa3377357b9e3d) )
	ROM_LOAD( "cr-11", 0x2000, 0x2000, CRC(7fc11c7c) SHA1(e798c4abe87fd701f250625ae6545ab00bcfbef5) )
	ROM_LOAD( "cr-10", 0x4000, 0x2000, CRC(6b1293b7) SHA1(3219c03e87be0f53d4556a45d82278fc712f4d0b) )
	ROM_LOAD( "cr-9",  0x6000, 0x2000, CRC(d219722c) SHA1(e68a0883113db43a878a5529fa8deb7816573e35) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_INVERT ) /* Layer 0 */
	ROM_LOAD( "cr-7",  0x0000, 0x2000, CRC(e8aa7ac3) SHA1(12f4f1041001ce6e77e9b0c691663f4a20969eaa) )
	ROM_LOAD( "cr-6",  0x2000, 0x2000, CRC(037be475) SHA1(72a8c1da210239ecac96ea22e54a5d193f32ad83) )
	ROM_LOAD( "cr-5",  0x4000, 0x2000, CRC(a4151734) SHA1(1ccf68270bbfd557cd8cca3f7f36e9e2de7e94e6) )
	ROM_LOAD( "cr-4",  0x6000, 0x2000, CRC(5ef24757) SHA1(4c6a06fdadb0b52f62148642e0416c4f60c8048b) )

	ROM_REGION( 0x4000, "gfx3", ROMREGION_INVERT)  /* Layer 1 */
	ROM_LOAD( "cr-13", 0x0000, 0x2000, CRC(012a6412) SHA1(ae9757e56f896e4158e6af7dc12eb6c2a3755f4d) )
	ROM_LOAD( "cr-8",  0x2000, 0x2000, CRC(3c2b816c) SHA1(3df3d9f49475ccfbb0792d98c3d12c8fd15034bc) )

	ROM_REGION( 0x0b40, "proms", 0 )
	ROM_LOAD( "82s129.6", 0x0000, 0x0100, CRC(38f443da) SHA1(a015217508b18eb3f1987cd5b53f31608b13de08) )    /* r */
	ROM_LOAD( "82s129.7", 0x0100, 0x0100, CRC(977fab0c) SHA1(78e7b4f1e9891d2d9cf1e1ec0c4f59a311cef1c5) )    /* g */
	ROM_LOAD( "82s129.8", 0x0200, 0x0100, CRC(ae7ae54d) SHA1(d7d4682e437f2f7adb7fceb813437c06f27f2711) )    /* b */
	/* all other proms that firebatl has are missing */
	ROM_LOAD( "clashrd.a2",  0x0900, 0x0100, CRC(4e2a2781) SHA1(7be2e066499ea0af76f6ae926fe87e02f8c36a6f) ) /* unknown */
	ROM_LOAD( "clashrd.g4",  0x0a00, 0x0020, CRC(1afc04f0) SHA1(38207cf3e15bac7034ac06469b95708d22b57da4) ) /* timing? */
	ROM_LOAD( "clashrd.b11", 0x0a20, 0x0020, CRC(d453f2c5) SHA1(7fdc5bf59bad9e8f00e970565ff6f6b3773541db) ) /* unknown (possibly bad dump) */
	ROM_LOAD( "clashrd.g10", 0x0a40, 0x0100, CRC(73afefd0) SHA1(d14c5490c5b174d54043bfdf5c6fb675e67492e7) ) /* unknown (possibly bad dump) */

	ROM_REGION( 0x2000, "custom:samples", 0 )
	ROM_LOAD( "clashr1.bin", 0x0000, 0x2000, CRC(0d0a8068) SHA1(529878d0c5f078590e07ec0fffc27b212843c0ad) )

	ROM_REGION( 0x0200, "custom:soundproms", 0 )   /* 4bit->8bit sample expansion PROMs */
	ROM_LOAD( "clashrd.g8",  0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) ) /* low 4 bits */
	ROM_LOAD( "clashrd.g7",  0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) ) /* high 4 bits */
ROM_END

// this set came from a bootleg board, but I believe it to be original for the following reason:
//  the ONLY difference between this and the parent set is the Wood Place string, however, in the parent
//  set the Wood Place string is padded with several 0x20 (Space) characters to fit the same number of bytes
//  in which the Data East Corporation string fits, suggesting that they always planned to put it there.
ROM_START( clshroadd )
	ROM_REGION( 0x8000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "crdeco-3.bin",  0x0000, 0x8000, CRC(1d54195c) SHA1(4b1d7d333707b5ebd57572742eb74df5abe8a70d) )

	ROM_REGION( 0x2000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "clashr2.bin", 0x0000, 0x2000, CRC(e6389ec1) SHA1(6ec94d5e389e9104f40fc48df6f15674415851c0) )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "clashr5.bin", 0x0000, 0x4000, CRC(094858b8) SHA1(a19f79cb665bbb1e25a94e9dd09a9e99f553afe8) )
	ROM_LOAD( "clashr6.bin", 0x4000, 0x4000, CRC(daa1daf3) SHA1(cc24c97c9950adc0041f68832774e40c87d1d4b2) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_INVERT ) /* Layer 0 */
	ROM_LOAD( "clashr8.bin", 0x0000, 0x4000, CRC(cbb66719) SHA1(2497575f84a956bc2b9e4c3f2c71ae42d036355e) )
	ROM_LOAD( "clashr9.bin", 0x4000, 0x4000, CRC(c15e8eed) SHA1(3b1e7fa014d176a01d5f9214051b0c8cc5556684) )

	ROM_REGION( 0x4000, "gfx3", ROMREGION_INVERT)  /* Layer 1 */
	ROM_LOAD( "clashr4.bin", 0x0000, 0x2000, CRC(664201d9) SHA1(4eb85306f0c9683d0e0cf787f6389df8fe4a3d9d) )
	ROM_LOAD( "clashr7.bin", 0x2000, 0x2000, CRC(97973030) SHA1(cca7a9d2751add7f6dd9bac83f7f63ece8021dbc) )

	ROM_REGION( 0x0b40, "proms", 0 )
	ROM_LOAD( "82s129.6", 0x0000, 0x0100, CRC(38f443da) SHA1(a015217508b18eb3f1987cd5b53f31608b13de08) )    /* r */
	ROM_LOAD( "82s129.7", 0x0100, 0x0100, CRC(977fab0c) SHA1(78e7b4f1e9891d2d9cf1e1ec0c4f59a311cef1c5) )    /* g */
	ROM_LOAD( "82s129.8", 0x0200, 0x0100, CRC(ae7ae54d) SHA1(d7d4682e437f2f7adb7fceb813437c06f27f2711) )    /* b */
	/* all other proms that firebatl has are missing */
	ROM_LOAD( "clashrd.a2",  0x0900, 0x0100, CRC(4e2a2781) SHA1(7be2e066499ea0af76f6ae926fe87e02f8c36a6f) ) /* unknown */
	ROM_LOAD( "clashrd.g4",  0x0a00, 0x0020, CRC(1afc04f0) SHA1(38207cf3e15bac7034ac06469b95708d22b57da4) ) /* timing? */
	ROM_LOAD( "clashrd.b11", 0x0a20, 0x0020, CRC(d453f2c5) SHA1(7fdc5bf59bad9e8f00e970565ff6f6b3773541db) ) /* unknown (possibly bad dump) */
	ROM_LOAD( "clashrd.g10", 0x0a40, 0x0100, CRC(73afefd0) SHA1(d14c5490c5b174d54043bfdf5c6fb675e67492e7) ) /* unknown (possibly bad dump) */

	ROM_REGION( 0x2000, "custom:samples", 0 )
	ROM_LOAD( "clashr1.bin", 0x0000, 0x2000, CRC(0d0a8068) SHA1(529878d0c5f078590e07ec0fffc27b212843c0ad) )

	ROM_REGION( 0x0200, "custom:soundproms", 0 )   /* 4bit->8bit sample expansion PROMs */
	ROM_LOAD( "clashrd.g8",  0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) ) /* low 4 bits */
	ROM_LOAD( "clashrd.g7",  0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) ) /* high 4 bits */
ROM_END

void clshroad_state::init_firebatl()
{
	// cfr. notes at top
	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0x6124] = 0x02;
}

GAME( 1984, firebatl,  0,        firebatl, firebatl, clshroad_state, init_firebatl, ROT90, "Woodplace Inc. (Taito license)",             "Fire Battle",                    MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS | MACHINE_UNEMULATED_PROTECTION ) // developed by Graphic Research
GAME( 1986, clshroad,  0,        clshroad, clshroad, clshroad_state, empty_init,    ROT0,  "Woodplace Inc.",                             "Clash-Road",                     MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_TIMING )
GAME( 1986, clshroads, clshroad, clshroad, clshroad, clshroad_state, empty_init,    ROT0,  "Woodplace Inc. (Status Game Corp. license)", "Clash-Road (Status license)",    MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_TIMING )
GAME( 1986, clshroadd, clshroad, clshroad, clshroad, clshroad_state, empty_init,    ROT0,  "Woodplace Inc. (Data East license)",         "Clash-Road (Data East license)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_TIMING )
