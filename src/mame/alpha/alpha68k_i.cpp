// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail, Stephane Humbert, Angelo Salese
/**************************************************************************************************

    SNK/Alpha 68000 I board based games

    derived from alpha68k.cpp

    TODO:
    - Both POST screens are X offset by a large margin,
      i.e. paddlema draws a middle line there, which isn't shown on real HW reference instead.
    - paddlema: ranking screen is unreadable on 9th/10th positions during attract,
      maybe the underlying background is supposed to be disabled somehow?

===================================================================================================

The Next Space:
 Mainboard A8004-1
  MC68000P10 @ 9MHz
  Z80A @ 4MHz
  YM3812 @ 4MHz + YM3014 DAC
  24MHz, 18MHz & 4MHz OSCs
  8 switch Dipswitch x 2
  Custom SNK CLK chip
  Custom SNK I/O chip x 2
 Many PCBs feature a A8004-2 daughtercard with 4 smaller mask ROMs, labeled
  NS 5, NS 6, NS 7 & NS 8 instead of the single larger mask ROM

===================================================================================================

Paddle Mania
 Mainboard 68K-96-I
  MC68000-8 @ 6MHz
  Z80A @ 4MHz
  YM3812 @ 4MHz + YM3014 DAC
  24MHz OSC
  8 switch Dipswitch x 2

**************************************************************************************************/

#include "emu.h"
#include "alpha68k.h"

/*
 *
 * Video Section
 *
 */

// TODO: document, also awfully similar to the II board but with proms
void alpha68k_I_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int c, int d)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	for (int offs = 0; offs < 0x400; offs += 0x20)
	{
		int mx = m_spriteram[offs + c];
		int my = (m_yshift - (mx >> 8)) & 0xff;
		mx &= 0xff;

		for (int i = 0; i < 0x20; i++)
		{
			const u16 data = m_spriteram[offs + d + i];
			const u16 tile = data & 0x3fff;
			const bool fy = data & 0x4000;
			const u8 color = m_color_proms[tile << 1 | data >> 15];

			gfx->transpen(bitmap,cliprect, tile, color, 0, fy, mx, my, 0);

			my = (my + 8) & 0xff;
		}
	}
}

u32 alpha68k_I_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	// This appears to be correct priority
	draw_sprites(bitmap, cliprect, 2, 0x0800);
	draw_sprites(bitmap, cliprect, 3, 0x0c00);
	draw_sprites(bitmap, cliprect, 1, 0x0400);
	return 0;
}


/*
 *
 * Read/Write handler overrides
 *
 */

void thenextspace_state::tnextspc_soundlatch_w(u8 data)
{
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void thenextspace_state::tnextspc_coin_counters_w(offs_t offset, u16 data)
{
	machine().bookkeeping().coin_counter_w(offset, data & 0x01);
}

void thenextspace_state::tnextspc_unknown_w(offs_t offset, u16 data)
{
	logerror("tnextspc_unknown_w : PC = %04x - offset = %04x - data = %04x\n", m_maincpu->pc(), offset, data);
	if (offset == 0)
		m_flipscreen = data & 0x100;
}

// TODO: check me
u16 thenextspace_state::sound_cpu_r() { return 1; }


/*
 *
 * Address Maps
 *
 */

void paddlemania_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                    // main program
	map(0x080000, 0x083fff).ram();                    // work RAM
	map(0x100000, 0x103fff).ram().share("spriteram"); // video RAM
	map(0x180000, 0x180001).portr("IN3").nopw();      // LSB: DSW0, MSB: watchdog(?)
	map(0x180008, 0x180009).portr("IN4");             // LSB: DSW1
	map(0x300000, 0x300001).portr("IN0");             // joy1, joy2
	map(0x340000, 0x340001).portr("IN1");             // coin, start, service
	map(0x380000, 0x380001).portr("IN2");
	map(0x380001, 0x380001).w(m_soundlatch, FUNC(generic_latch_8_device::write)); // LSB: sound latch write and RST38 trigger, joy3, joy4
}

void thenextspace_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x070000, 0x073fff).ram();
	map(0x0a0000, 0x0a3fff).ram().share("spriteram");
	map(0x0d0000, 0x0d0001).nopw(); // unknown write port (0)
	map(0x0e0000, 0x0e0001).portr("P1");
	map(0x0e0002, 0x0e0003).portr("P2");
	map(0x0e0004, 0x0e0005).portr("SYSTEM");
	map(0x0e0006, 0x0e0007).nopw(); // unknown write port (0)
	map(0x0e0008, 0x0e0009).portr("DSW1");
	map(0x0e000a, 0x0e000b).portr("DSW2");
	map(0x0e000e, 0x0e000f).nopw(); // unknown write port (0)
	map(0x0e0018, 0x0e0019).r(FUNC(thenextspace_state::sound_cpu_r));
	map(0x0f0000, 0x0f0001).w(FUNC(thenextspace_state::tnextspc_unknown_w));
	map(0x0f0002, 0x0f0005).w(FUNC(thenextspace_state::tnextspc_coin_counters_w));
	map(0x0f0009, 0x0f0009).w(FUNC(thenextspace_state::tnextspc_soundlatch_w));
}

void paddlemania_state::sound_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xe000, 0xe000).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::clear_w));
	map(0xe800, 0xe800).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0xec00, 0xec00).w("ymsnd", FUNC(ym3812_device::data_w));
	map(0xf000, 0xf7ff).ram();
	map(0xfc00, 0xfc00).ram(); // unknown port
}

void thenextspace_state::sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::clear_w));
}

void thenextspace_state::sound_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0x20, 0x20).w("ymsnd", FUNC(ym3812_device::data_w));
	map(0x3b, 0x3b).nopr(); // unknown read port
	map(0x3d, 0x3d).nopr(); // unknown read port
	map(0x7b, 0x7b).nopr(); // unknown read port
}


/*
 *
 * Gfx layout Defs
 *
 */

// TODO: merge with base driver somehow
static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,4) },
	{ STEP4(8*4*4+3,-1), STEP4(3,-1) },
	{ STEP8(0,4*4) },
	32*8
};

static GFXDECODE_START( gfx_alpha68k_I )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0, 64 )
GFXDECODE_END


/*
 *
 * Input Defs
 *
 */

// TODO: deduplicate these
#ifndef ALPHA68K_PLAYER_INPUT_LSB
#define ALPHA68K_PLAYER_INPUT_LSB( player, button3, start, active ) \
	PORT_BIT( 0x0001, active, IPT_JOYSTICK_UP    ) PORT_PLAYER(player) \
	PORT_BIT( 0x0002, active, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(player) \
	PORT_BIT( 0x0004, active, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(player) \
	PORT_BIT( 0x0008, active, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) \
	PORT_BIT( 0x0010, active, IPT_BUTTON1        ) PORT_PLAYER(player) \
	PORT_BIT( 0x0020, active, IPT_BUTTON2        ) PORT_PLAYER(player) \
	PORT_BIT( 0x0040, active, button3            ) PORT_PLAYER(player) \
	PORT_BIT( 0x0080, active, start )
#endif

#ifndef ALPHA68K_PLAYER_INPUT_MSB
#define ALPHA68K_PLAYER_INPUT_MSB( player, button3, start, active ) \
	PORT_BIT( 0x0100, active, IPT_JOYSTICK_UP    ) PORT_PLAYER(player) \
	PORT_BIT( 0x0200, active, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(player) \
	PORT_BIT( 0x0400, active, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(player) \
	PORT_BIT( 0x0800, active, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) \
	PORT_BIT( 0x1000, active, IPT_BUTTON1        ) PORT_PLAYER(player) \
	PORT_BIT( 0x2000, active, IPT_BUTTON2        ) PORT_PLAYER(player) \
	PORT_BIT( 0x4000, active, button3            ) PORT_PLAYER(player) \
	PORT_BIT( 0x8000, active, start )
#endif

static INPUT_PORTS_START( paddlema )
	PORT_START("IN0") // (bottom players)
	ALPHA68K_PLAYER_INPUT_LSB( 1, IPT_UNKNOWN, IPT_UNKNOWN, IP_ACTIVE_LOW )
	ALPHA68K_PLAYER_INPUT_MSB( 2, IPT_UNKNOWN, IPT_UNKNOWN, IP_ACTIVE_LOW )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Button A (Start)")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Button B (Start)")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE2 ) // "Test" ?
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2") // (top players)
	ALPHA68K_PLAYER_INPUT_LSB( 3, IPT_UNKNOWN, IPT_UNKNOWN, IP_ACTIVE_LOW )
	ALPHA68K_PLAYER_INPUT_MSB( 4, IPT_UNKNOWN, IPT_UNKNOWN, IP_ACTIVE_LOW )

	PORT_START("IN3") // DSW0
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )            PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )            PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Game_Time ) )         PORT_DIPLOCATION("SW1:4,3") // See notes for Game Time / Match Type combos
	PORT_DIPSETTING(    0x00, "Default Time" )
	PORT_DIPSETTING(    0x20, "+10 Seconds" )
	PORT_DIPSETTING(    0x10, "+20 Seconds" )
	PORT_DIPSETTING(    0x30, "+30 Seconds" )
	PORT_DIPNAME( 0xc0, 0x40, "Match Type" )                 PORT_DIPLOCATION("SW1:2,1") // Styles are for Upright/Table & Single/Dual controls????
	PORT_DIPSETTING(    0x80, "A to B" )                     // Manual shows "Upright Sytle B"
	PORT_DIPSETTING(    0x00, "A to C" )                     // Manual shows "Upright Sytle A"
	PORT_DIPSETTING(    0x40, "A to E" )                     // Manual shows "Table Sytle C"
//  PORT_DIPSETTING(    0xc0, "A to B" )                     // Manual shows "Table Sytle D"

	PORT_START("IN4") // DSW1
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_HIGH, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x01, "SW2:7" )             // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x01, "SW2:6" )             // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x01, "SW2:5" )             // Listed as "Unused"
	PORT_DIPNAME( 0x30, 0x00, "Game Mode" )                  PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x20, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x00, "Demo Sounds On" )
	PORT_DIPSETTING(    0x10, "Win Match Against CPU (Cheat)")
	PORT_DIPSETTING(    0x30, "Freeze" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Language ) )          PORT_DIPLOCATION("SW2:2") // Manual shows "Off" for this dipswitch
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )    PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tnextspc )
	PORT_START("P1")
	ALPHA68K_PLAYER_INPUT_LSB( 1, IPT_UNKNOWN, IPT_START1, IP_ACTIVE_LOW )

	PORT_START("P2")
	ALPHA68K_PLAYER_INPUT_LSB( 2, IPT_UNKNOWN, IPT_START2, IP_ACTIVE_LOW )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW1:2" )             // Listed as "Unused"
	PORT_DIPNAME( 0x04, 0x04, "Additional Bonus Life" )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "2nd Extend ONLY" )
	PORT_DIPSETTING(    0x00, "Every Extend" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )             // Listed as "Unused"
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )           PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "A 1C/1C B 1C/2C" )
	PORT_DIPSETTING(    0x20, "A 2C/1C B 1C/3C" )
	PORT_DIPSETTING(    0x10, "A 3C/1C B 1C/5C" )
	PORT_DIPSETTING(    0x00, "A 4C/1C B 1C/6C" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) )             PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )       PORT_DIPLOCATION("SW2:3") PORT_CONDITION("DSW2",0x08,EQUALS,0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Game Mode" )                  PORT_DIPLOCATION("SW2:3") PORT_CONDITION("DSW2",0x08,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPSETTING(    0x04, "Infinite Lives (Cheat)")
	PORT_DIPNAME( 0x08, 0x08, "SW2:3 Demo Sound/Game Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, "Game Mode" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "100000 200000" )
	PORT_DIPSETTING(    0x20, "150000 300000" )
	PORT_DIPSETTING(    0x10, "300000 500000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


/*
 *
 * Machine Config
 *
 */

void alpha68k_I_state::base_config(machine_config &config)
{
	MCFG_MACHINE_START_OVERRIDE(alpha68k_I_state,common)
	MCFG_MACHINE_RESET_OVERRIDE(alpha68k_I_state,common)

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 24_MHz_XTAL/6)); // 4MHz (24MHz/6) verified
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 1.0);
}

void alpha68k_I_state::video_config(machine_config &config, int yshift)
{
	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(alpha68k_I_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_alpha68k_I);
	PALETTE(config, m_palette, FUNC(alpha68k_I_state::palette_init), 1024, 256);

	// TODO: add video device here
	m_yshift = yshift;
}

void paddlemania_state::paddlema(machine_config &config)
{
	base_config(config);

	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/4); // 6MHz (24MHz/4) verified
	m_maincpu->set_addrmap(AS_PROGRAM, &paddlemania_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(paddlemania_state::irq1_line_hold)); // VBL

	Z80(config, m_audiocpu, 24_MHz_XTAL/6); // 4MHz (24MHz/6) verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &paddlemania_state::sound_map);

	video_config(config, 0);

	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0, HOLD_LINE);
}

void thenextspace_state::tnextspc(machine_config &config)
{
	base_config(config);

	// basic machine hardware
	M68000(config, m_maincpu, 18_MHz_XTAL/2); // 9MHz (18MHz/2) verified
	m_maincpu->set_addrmap(AS_PROGRAM, &thenextspace_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(thenextspace_state::irq1_line_hold)); // VBL

	Z80(config, m_audiocpu, 4_MHz_XTAL); // 4MHz verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &thenextspace_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &thenextspace_state::sound_iomap);

	video_config(config, 1);
}


/*
 *
 * ROM definitions
 *
 */

// Several bootleg boards with identical ROMs have been found.
ROM_START( paddlema )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "padlem.6g",  0x00000, 0x10000, CRC(c227a6e8) SHA1(9c98be6e82a0dd76fd5b786601456b060407c57f) )
	ROM_LOAD16_BYTE( "padlem.3g",  0x00001, 0x10000, CRC(f11a21aa) SHA1(6eda9ff99f2aa8832fff1e2a054c5ffb6dae7ae3) )
	ROM_LOAD16_BYTE( "padlem.6h",  0x20000, 0x10000, CRC(8897555f) SHA1(7d30aa56a727700a6e02af92b065ed982a39ccc2) )
	ROM_LOAD16_BYTE( "padlem.3h",  0x20001, 0x10000, CRC(f0fe9b9d) SHA1(2e7a80dc25c549e57b7698052f53562a9a608205) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "padlem.18c", 0x000000, 0x10000, CRC(9269778d) SHA1(bdc9100827f2e018db943d9f7d81b7936c155bf0) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "padlem.9m",       0x00001, 0x10000, CRC(4ee4970d) SHA1(d57d9178129236dfb3a18688e8544e5e555ce559) )
	ROM_LOAD16_BYTE( "padlem.16m",      0x00000, 0x10000, CRC(0984fb4d) SHA1(6bc529db93fad277f286e4a380812c40c7f42301) )
	ROM_LOAD16_BYTE( "padlem.9n",       0x20001, 0x10000, CRC(a1756f15) SHA1(1220075e34c482e38eead9ea5e63b53b822e87de) )
	ROM_LOAD16_BYTE( "padlem.16n",      0x20000, 0x10000, CRC(4249e047) SHA1(9f35b316b5de65f8b1878fca283c9d534bb8ae25) )
	ROM_LOAD16_BYTE( "padlem.6m",       0x40001, 0x10000, CRC(3f47910c) SHA1(429d425dc57fbd868bc39c3d799bbaebcf313cc0) )
	ROM_LOAD16_BYTE( "padlem.13m",      0x40000, 0x10000, CRC(fd9dbc27) SHA1(c01f512afef7686c64cc0766c235084cc8e2f5fc) )
	ROM_LOAD16_BYTE( "padlem.6n",       0x60001, 0x10000, CRC(fe337655) SHA1(ac04124642b245d6a530c72d0dea1b1585b5cebd) )
	ROM_LOAD16_BYTE( "padlem.13n",      0x60000, 0x10000, CRC(1d460486) SHA1(4ade817a036447e7e6d4fe56fa2c5712f198c625) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "padlem.a",        0x0000,  0x0100,  CRC(cae6bcd6) SHA1(da3b3bdcdc7fefae80b0ef8365565bbe5ff0d5d2) ) // R
	ROM_LOAD( "padlem.b",        0x0100,  0x0100,  CRC(b6df8dcb) SHA1(318ca20fab6608aa2956ec3bb82e8ae77c250d51) ) // G
	ROM_LOAD( "padlem.c",        0x0200,  0x0100,  CRC(39ca9b86) SHA1(8b8d7aae85830e69366e86f8b6cccfb8140cd526) ) // B

	ROM_REGION( 0x800, "clut_proms", 0 )
	ROM_LOAD( "padlem.17j",      0x0000,  0x0400,  CRC(86170069) SHA1(8e2ad7afa50453e9a2dc89386ce02d10e7c89fbc) ) // Clut low nibble
	ROM_LOAD( "padlem.16j",      0x0400,  0x0400,  CRC(8da58e2c) SHA1(6012715a2d3ba4cf8bc5a8250e7f28cb59913092) ) // Clut high nibble

	ROM_REGION( 0x8000, "color_proms", 0 )
	ROM_LOAD( "padlem.18n",      0x0000,  0x8000, CRC(488df971) SHA1(fe1436ddc63ffb37fcc9e57aeb923c8c96fd6ac3) ) // Colour lookup
	// The dump with the following hashes has a bad bit 3 at 0x7de6: CRC(06506200) SHA1(d43337e5611cb0d3432942539ccf04bff2bdd345)
ROM_END

ROM_START( tnextspc )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ns_4.4", 0x00000, 0x20000, CRC(4617cba3) SHA1(615a1e67fc1c76d2be004b19a965f423b8daaf5c) ) // Silkscreened "4" @ 14L
	ROM_LOAD16_BYTE( "ns_3.3", 0x00001, 0x20000, CRC(a6c47fef) SHA1(b7e4a0fffd5c44ed0b138c1ad04c3b6644ec463b) ) // Silkscreened "3" @ 11L

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "ns_1.1", 0x000000, 0x10000, CRC(fc26853c) SHA1(0118b048046a6125bba20dec081b936486eb1597) ) // Silkscreened "1" @ 18B

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD16_WORD_SWAP( "ns_5678.bin", 0x000000, 0x80000, CRC(22756451) SHA1(ce1d58a75ef4b09feb6fd9b3dd2de48b986070c0) ) // single mask ROM for gfx

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "2.p2", 0x0000,  0x0100,  CRC(1f388d48) SHA1(5e7dc37b4e177483f4fc65b801dca8ef132ac282) ) // R
	ROM_LOAD( "3.p3", 0x0100,  0x0100,  CRC(0254533a) SHA1(d0ec0d03ed78482cd9344661eab3305640e85682) ) // G
	ROM_LOAD( "1.p1", 0x0200,  0x0100,  CRC(488fd0e9) SHA1(cde18e9ca0b320ded821bea537c88424b02e8910) ) // B

	ROM_REGION( 0x800, "clut_proms", 0 )
	ROM_LOAD( "5.p5", 0x0000,  0x0400,  CRC(9c8527bf) SHA1(6b52ab37ea6c07a4814ed33deadd59d137b5fd4d) ) // Clut high nibble
	ROM_LOAD( "4.p4", 0x0400,  0x0400,  CRC(cc9ff769) SHA1(e9de0371fd8bae7f08924891d78799ace97902b1) ) // Clut low nibble

	ROM_REGION( 0x8000, "color_proms", 0 )
	ROM_LOAD( "ns_2.bin", 0x0000,  0x8000,  CRC(05771d48) SHA1(9e9376b1449679f554eabf8cea023714dd1ed487) ) // Colour lookup
ROM_END

ROM_START( tnextspc2 ) // two bootleg PCBs have been found with the same ROMs as this set, the only difference being ns_2.bin being double sized with 1st and 2nd half identical
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ns_4.4", 0x00000, 0x20000, CRC(4617cba3) SHA1(615a1e67fc1c76d2be004b19a965f423b8daaf5c) ) // == b18.ic13
	ROM_LOAD16_BYTE( "ns_3.3", 0x00001, 0x20000, CRC(a6c47fef) SHA1(b7e4a0fffd5c44ed0b138c1ad04c3b6644ec463b) ) // == b17.ic11

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "ns_1.1",    0x000000, 0x10000, CRC(fc26853c) SHA1(0118b048046a6125bba20dec081b936486eb1597) ) // == b1.ic129

	ROM_REGION( 0x080000, "gfx1", 0 ) // EPROMs, graphics are odd/even interleaved
	ROM_LOAD16_BYTE( "b3.ic49",  0x00001, 0x10000, CRC(2bddf94d) SHA1(e064f48d0e3bb089753c1b59c863bb46bfa2bcee) )
	ROM_LOAD16_BYTE( "b7.ic53",  0x00000, 0x10000, CRC(a8b13a9a) SHA1(2f808c17e97a272be14099c53b287e665dd90b14) )
	ROM_LOAD16_BYTE( "b4.ic50",  0x20001, 0x10000, CRC(80c6c841) SHA1(ab0aa4cad6dcadae62f849e53c3c5cd909f77971) )
	ROM_LOAD16_BYTE( "b8.ic54",  0x20000, 0x10000, CRC(bf0762a0) SHA1(2fe32b1bf08dfc78668d7a12a7a67e6b2c0a2c48) )
	ROM_LOAD16_BYTE( "b5.ic51",  0x40001, 0x10000, CRC(e487750b) SHA1(f01d15f6624822dc78ff7e1cd2fdce54568ab5dc) )
	ROM_LOAD16_BYTE( "b9.ic55",  0x40000, 0x10000, CRC(45d730b9) SHA1(cb05942b12589e76afbbbac94cba0e8284bb3deb) )
	ROM_LOAD16_BYTE( "b6.ic52",  0x60001, 0x10000, CRC(0618cf49) SHA1(fad33b316613b82231f8372d0faf8cf1c669ac98) )
	ROM_LOAD16_BYTE( "b10.ic56", 0x60000, 0x10000, CRC(f48819df) SHA1(86d688590379316638ef4c3094c11629931cd69e) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "2.p2", 0x0000,  0x0100,  CRC(1f388d48) SHA1(5e7dc37b4e177483f4fc65b801dca8ef132ac282) ) // R
	ROM_LOAD( "3.p3", 0x0100,  0x0100,  CRC(0254533a) SHA1(d0ec0d03ed78482cd9344661eab3305640e85682) ) // G
	ROM_LOAD( "1.p1", 0x0200,  0x0100,  CRC(488fd0e9) SHA1(cde18e9ca0b320ded821bea537c88424b02e8910) ) // B

	ROM_REGION( 0x800, "clut_proms", 0 )
	ROM_LOAD( "5.p5", 0x0000,  0x0400,  CRC(9c8527bf) SHA1(6b52ab37ea6c07a4814ed33deadd59d137b5fd4d) ) // Clut high nibble
	ROM_LOAD( "4.p4", 0x0400,  0x0400,  CRC(cc9ff769) SHA1(e9de0371fd8bae7f08924891d78799ace97902b1) ) // Clut low nibble

	ROM_REGION( 0x8000, "color_proms", 0 )
	ROM_LOAD( "ns_2.bin", 0x0000,  0x8000,  CRC(05771d48) SHA1(9e9376b1449679f554eabf8cea023714dd1ed487) ) // b2.ic90 - Colour lookup
ROM_END

ROM_START( tnextspcj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ns_ver1_j4.4", 0x00000, 0x20000, CRC(5cdf710d) SHA1(c744e532f2f5a248d7b50a2e62cc77a0888d8dff) ) // Silkscreened "4" @ 14L
	ROM_LOAD16_BYTE( "ns_ver1_j3.3", 0x00001, 0x20000, CRC(cd9532d0) SHA1(dbd7ced8f015334f0acb8d760f4d9d0299feef70) ) // Silkscreened "3" @ 11L

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "ns_1.1", 0x000000, 0x10000, CRC(fc26853c) SHA1(0118b048046a6125bba20dec081b936486eb1597) ) // Silkscreened "1" @ 18B

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD16_WORD_SWAP( "ns_5678.bin", 0x000000, 0x80000, CRC(22756451) SHA1(ce1d58a75ef4b09feb6fd9b3dd2de48b986070c0) ) // single mask ROM for gfx

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "2.p2", 0x0000,  0x0100,  CRC(1f388d48) SHA1(5e7dc37b4e177483f4fc65b801dca8ef132ac282) ) // R
	ROM_LOAD( "3.p3", 0x0100,  0x0100,  CRC(0254533a) SHA1(d0ec0d03ed78482cd9344661eab3305640e85682) ) // G
	ROM_LOAD( "1.p1", 0x0200,  0x0100,  CRC(488fd0e9) SHA1(cde18e9ca0b320ded821bea537c88424b02e8910) ) // B

	ROM_REGION( 0x800, "clut_proms", 0 )
	ROM_LOAD( "5.p5", 0x0000,  0x0400,  CRC(9c8527bf) SHA1(6b52ab37ea6c07a4814ed33deadd59d137b5fd4d) ) // Clut high nibble
	ROM_LOAD( "4.p4", 0x0400,  0x0400,  CRC(cc9ff769) SHA1(e9de0371fd8bae7f08924891d78799ace97902b1) ) // Clut low nibble

	ROM_REGION( 0x8000, "color_proms", 0 )
	ROM_LOAD( "ns_2.bin", 0x0000,  0x8000,  CRC(05771d48) SHA1(9e9376b1449679f554eabf8cea023714dd1ed487) ) // Colour lookup
ROM_END

void paddlemania_state::init_paddlema()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0;
	m_game_id = 0;
}

void thenextspace_state::init_tnextspc()
{
	m_invert_controls = 0;
	m_microcontroller_id = 0x890a;
	m_game_id = 0;
}

GAME( 1988, paddlema,  0,        paddlema,       paddlema,  paddlemania_state,  init_paddlema,  ROT90, "SNK",                                        "Paddle Mania", MACHINE_SUPPORTS_SAVE )

GAME( 1989, tnextspc,  0,        tnextspc,       tnextspc,  thenextspace_state, init_tnextspc,  ROT90, "SNK",                                        "The Next Space (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1989, tnextspc2, tnextspc, tnextspc,       tnextspc,  thenextspace_state, init_tnextspc,  ROT90, "SNK",                                        "The Next Space (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1989, tnextspcj, tnextspc, tnextspc,       tnextspc,  thenextspace_state, init_tnextspc,  ROT90, "SNK (Pasadena International Corp. license)", "The Next Space (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
