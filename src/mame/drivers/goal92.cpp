// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, David Haywood
/***************************************************************************

    Goal! '92

    driver by Pierpaolo Prazzoli
    and some bits by David Haywood

***************************************************************************/

#include "emu.h"
#include "includes/goal92.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "screen.h"
#include "speaker.h"


WRITE16_MEMBER(goal92_state::goal92_sound_command_w)
{
	if (ACCESSING_BITS_8_15)
	{
		m_soundlatch->write((data >> 8) & 0xff);
		m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

READ16_MEMBER(goal92_state::goal92_inputs_r)
{
	switch(offset)
	{
		case 0:
			return ioport("DSW1")->read();
		case 1:
			return ioport("IN1")->read();
		case 2:
			return ioport("IN2")->read();
		case 3:
			return ioport("IN3")->read();
		case 7:
			return ioport("DSW2")->read();

		default:
			logerror("reading unhandled goal92 inputs %04X %04X @ PC = %04X\n", offset, mem_mask,m_maincpu->pc());
	}

	return 0;
}

void goal92_state::goal92_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x1007ff).ram();
	map(0x100800, 0x100fff).ram().w(FUNC(goal92_state::goal92_background_w)).share("bg_data");
	map(0x101000, 0x1017ff).ram().w(FUNC(goal92_state::goal92_foreground_w)).share("fg_data");
	map(0x101800, 0x101fff).ram(); // it has tiles for clouds, but they aren't used
	map(0x102000, 0x102fff).ram().w(FUNC(goal92_state::goal92_text_w)).share("tx_data");
	map(0x103000, 0x103fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x104000, 0x13ffff).ram();
	map(0x140000, 0x1407ff).ram().share("spriteram");
	map(0x140800, 0x140801).nopw();
	map(0x140802, 0x140803).nopw();
	map(0x180000, 0x18000f).r(FUNC(goal92_state::goal92_inputs_r));
	map(0x180008, 0x180009).w(FUNC(goal92_state::goal92_sound_command_w));
	map(0x18000a, 0x18000b).nopw();
	map(0x180010, 0x180017).writeonly().share("scrollram");
	map(0x18001c, 0x18001d).rw(FUNC(goal92_state::goal92_fg_bank_r), FUNC(goal92_state::goal92_fg_bank_w));
}

/* Sound CPU */

WRITE8_MEMBER(goal92_state::adpcm_control_w)
{
	membank("bank1")->set_entry(data & 0x01);

	m_msm->reset_w(data & 0x08);
}

WRITE8_MEMBER(goal92_state::adpcm_data_w)
{
	m_msm5205next = data;
}

void goal92_state::sound_cpu(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xe000, 0xe000).w(FUNC(goal92_state::adpcm_control_w));
	map(0xe400, 0xe400).w(FUNC(goal92_state::adpcm_data_w));
	map(0xe800, 0xe801).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xec00, 0xec01).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

static INPUT_PORTS_START( goal92 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, "Coin A / Coin C" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, "Coin B / Coin D" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Starting Coin" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "X 2" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_COIN2 )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_COIN4 )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_START3 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_START4 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // unused?

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x0000, "Players / Coin Mode" )
	PORT_DIPSETTING(      0x0000, "4 Players / 1 Coin Slot" )
	PORT_DIPSETTING(      0x0004, "4 Players / 4 Coin Slots" )
	PORT_DIPSETTING(      0x0008, "4 Players / 2 Coin Slots" )
	PORT_DIPSETTING(      0x000c, "2 Players" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xffc0, 0xffc0, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0xffc0, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/* handler called by the 2203 emulator when the internal timers cause an IRQ */
WRITE_LINE_MEMBER(goal92_state::irqhandler)
{
	/* NMI writes to MSM ports *only*! -AS */
	//m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(goal92_state::goal92_adpcm_int)
{
	m_msm->write_data(m_msm5205next);
	m_msm5205next >>= 4;
	m_adpcm_toggle^= 1;

	if (m_adpcm_toggle)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4),RGN_FRAC(0,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4),RGN_FRAC(0,4) },
	{ STEP8(0,1), STEP8(8*8*2,1) },
	{ STEP8(0,8), STEP8(8*8*1,8) },
	16*16
};

#ifdef UNUSED_FUNCTION
static const gfx_layout layout_16x16x4_2 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24,
		512+4,512+0,512+12,512+8,512+20,512+16,512+28,512+24 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,
		256+0*32,256+1*32,256+2*32,256+3*32,256+4*32,256+5*32,256+6*32,256+7*32 },
	1024
};
#endif

static GFXDECODE_START( gfx_goal92 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4,        0*16, 8*16 ) // Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x4,         48*16,   16 ) // Text Layer
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4,        0*16,   16 ) // BG Layer
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4,       16*16,   16 ) // Mid Layer
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4,       32*16,   16 ) // FG Layer
GFXDECODE_END


void goal92_state::machine_start()
{
	uint8_t *ROM = memregion("audiocpu")->base();

	membank("bank1")->configure_entries(0, 2, &ROM[0x10000], 0x4000);


	save_item(NAME(m_fg_bank));
	save_item(NAME(m_msm5205next));
	save_item(NAME(m_adpcm_toggle));
}

void goal92_state::machine_reset()
{
	m_fg_bank = 0;
	m_msm5205next = 0;
	m_adpcm_toggle = 0;
}

void goal92_state::goal92(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &goal92_state::goal92_map);
	m_maincpu->set_vblank_int("screen", FUNC(goal92_state::irq6_line_hold)); /* VBL */

	Z80(config, m_audiocpu, 2500000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &goal92_state::sound_cpu);  /* IRQs are triggered by the main CPU */

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1); // black border at bottom is a game bug...
	screen.set_screen_update(FUNC(goal92_state::screen_update_goal92));
	screen.screen_vblank().set(FUNC(goal92_state::screen_vblank_goal92));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_goal92);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 128*16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ym1(YM2203(config, "ym1", 2500000/2));
	ym1.irq_handler().set(FUNC(goal92_state::irqhandler));
	ym1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ym2203_device &ym2(YM2203(config, "ym2", 2500000/2));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.25);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(goal92_state::goal92_adpcm_int));   /* interrupt function */
	m_msm->set_prescaler_selector(msm5205_device::S96_4B);      /* 4KHz 4-bit */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.60);
}

/*

Goal '92 (bootleg of Olympic Soccer '92 on non-Seibu board)


PCB Layout

----------------------------------------------------------
| 400KHz  Z80                 10MHz            68000     |
|   6116   1                                   2   3     |
|  YM2203           6116                    681000 681000|
| 5205              6116                                 |
|                   6116                                 |
| 10MHz             6116                       24.0MHz   |
|                         6264                           |
|                         6264        TPC1020            |
|                                                        |
|     6116                                               |
|                                                        |
|     6116                                               |
|                                                        |
| DSW1  DSW3                                             |
|                                                        |
| DSW2                                                   |
|                             4             8            |
|                   6116      5             9            |
|                             6             10           |
|                   6116      7             11           |
|                                                        |
----------------------------------------------------------

Notes:
Z80 clock: 2.51MHz
68k clock: 12.0MHz
    VSync: 60Hz
    HSync: 15.27kHz

*/

ROM_START( goal92 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code - first 0x20000 bytes are a heavily patched copy of olysoc92a */
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x80000, CRC(db0a6c7c) SHA1(b609db7806b99bc921806d8b3e5e515b4651c375) )
	ROM_LOAD16_BYTE( "3.bin", 0x00001, 0x80000, CRC(e4c45dee) SHA1(542749bd1ff51220a151fe66acdadac83df8f0ee) )

	ROM_REGION( 0x18000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "1.bin",        0x00000, 0x8000, CRC(3d317622) SHA1(ae4e8c5247bc215a2769786cb8639bce2f80db22) )
	ROM_CONTINUE(             0x10000, 0x8000 ) /* banked at 8000-bfff */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "6.bin",        0x000000, 0x040000, CRC(83cadc8f) SHA1(1d3309750347c5d6d661f5cf452235e5a83a7483) )
	ROM_LOAD( "7.bin",        0x040000, 0x040000, CRC(067e10fc) SHA1(9831b8dc9b8efa6f7797b2946ee5be03fb36de7b) )
	ROM_LOAD( "5.bin",        0x080000, 0x040000, CRC(9a390af2) SHA1(8bc46f8cc7823b8caf381866bea016ebfad9d5d3) )
	ROM_LOAD( "4.bin",        0x0c0000, 0x040000, CRC(69b118d5) SHA1(80ab6f03e1254ba47c27299ce11559b244a024ad) )  // sldh

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "11.bin",       0x000000, 0x080000, CRC(5701e626) SHA1(e6915714e9ca90be8fa8ab1bf7fd1f23a83fb82c) )
	ROM_LOAD( "10.bin",       0x080000, 0x080000, CRC(ebb359cc) SHA1(b2f724ef7a91fca0ff0b7d7abe1c37816464b37d) )
	ROM_LOAD( "9.bin",        0x100000, 0x080000, CRC(c9f0dd07) SHA1(d70cdb24b7df521255b5841f01dd9e8344ab7bdb) )
	ROM_LOAD( "8.bin",        0x180000, 0x080000, CRC(aeab3534) SHA1(af91238f412bfcff3a52232278d81276584614a7) )
ROM_END



GAME( 1992, goal92, cupsoc, goal92, goal92, goal92_state, empty_init, ROT0, "bootleg", "Goal! '92", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
