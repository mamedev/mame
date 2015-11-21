// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

    Atari Drag Race Driver

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "includes/dragrace.h"
#include "sound/discrete.h"

#include "dragrace.lh"


TIMER_DEVICE_CALLBACK_MEMBER(dragrace_state::dragrace_frame_callback)
{
	static const char *const portnames[] = { "P1", "P2" };

	for (int i = 0; i < 2; i++)
	{
		switch (ioport(portnames[i])->read())
		{
			case 0x01: m_gear[i] = 1; break;
			case 0x02: m_gear[i] = 2; break;
			case 0x04: m_gear[i] = 3; break;
			case 0x08: m_gear[i] = 4; break;
			case 0x10: m_gear[i] = 0; break;
		}
	}
	output_set_value("P1gear", m_gear[0]);
	output_set_value("P2gear", m_gear[1]);

	/* watchdog is disabled during service mode */
	machine().watchdog_enable(ioport("IN0")->read() & 0x20);
}


void dragrace_state::dragrace_update_misc_flags( address_space &space )
{
	/* 0x0900 = set 3SPEED1         0x00000001
	 * 0x0901 = set 4SPEED1         0x00000002
	 * 0x0902 = set 5SPEED1         0x00000004
	 * 0x0903 = set 6SPEED1         0x00000008
	 * 0x0904 = set 7SPEED1         0x00000010
	 * 0x0905 = set EXPLOSION1      0x00000020
	 * 0x0906 = set SCREECH1        0x00000040
	 * 0x0920 - 0x0927 = clear 0x0900 - 0x0907

	 * 0x0909 = set KLEXPL1         0x00000200
	 * 0x090b = set MOTOR1          0x00000800
	 * 0x090c = set ATTRACT         0x00001000
	 * 0x090d = set LOTONE          0x00002000
	 * 0x090f = set Player 1 Start Lamp 0x00008000
	 * 0x0928 - 0x092f = clear 0x0908 - 0x090f

	 * 0x0910 = set 3SPEED2         0x00010000
	 * 0x0911 = set 4SPEED2         0x00020000
	 * 0x0912 = set 5SPEED2         0x00040000
	 * 0x0913 = set 6SPEED2         0x00080000
	 * 0x0914 = set 7SPEED2         0x00100000
	 * 0x0915 = set EXPLOSION2      0x00200000
	 * 0x0916 = set SCREECH2        0x00400000
	 * 0x0930 = clear 0x0910 - 0x0917

	 * 0x0919 = set KLEXPL2         0x02000000
	 * 0x091b = set MOTOR2          0x08000000
	 * 0x091d = set HITONE          0x20000000
	 * 0x091f = set Player 2 Start Lamp 0x80000000
	 * 0x0938 = clear 0x0918 - 0x091f
	 */
	set_led_status(machine(), 0, m_misc_flags & 0x00008000);
	set_led_status(machine(), 1, m_misc_flags & 0x80000000);

	m_discrete->write(space, DRAGRACE_MOTOR1_DATA,  ~m_misc_flags & 0x0000001f);       // Speed1 data*
	m_discrete->write(space, DRAGRACE_EXPLODE1_EN, (m_misc_flags & 0x00000020) ? 1: 0);    // Explosion1 enable
	m_discrete->write(space, DRAGRACE_SCREECH1_EN, (m_misc_flags & 0x00000040) ? 1: 0);    // Screech1 enable
	m_discrete->write(space, DRAGRACE_KLEXPL1_EN, (m_misc_flags & 0x00000200) ? 1: 0); // KLEXPL1 enable
	m_discrete->write(space, DRAGRACE_MOTOR1_EN, (m_misc_flags & 0x00000800) ? 1: 0);  // Motor1 enable

	m_discrete->write(space, DRAGRACE_MOTOR2_DATA, (~m_misc_flags & 0x001f0000) >> 0x10);  // Speed2 data*
	m_discrete->write(space, DRAGRACE_EXPLODE2_EN, (m_misc_flags & 0x00200000) ? 1: 0);    // Explosion2 enable
	m_discrete->write(space, DRAGRACE_SCREECH2_EN, (m_misc_flags & 0x00400000) ? 1: 0);    // Screech2 enable
	m_discrete->write(space, DRAGRACE_KLEXPL2_EN, (m_misc_flags & 0x02000000) ? 1: 0); // KLEXPL2 enable
	m_discrete->write(space, DRAGRACE_MOTOR2_EN, (m_misc_flags & 0x08000000) ? 1: 0);  // Motor2 enable

	m_discrete->write(space, DRAGRACE_ATTRACT_EN, (m_misc_flags & 0x00001000) ? 1: 0); // Attract enable
	m_discrete->write(space, DRAGRACE_LOTONE_EN, (m_misc_flags & 0x00002000) ? 1: 0);  // LoTone enable
	m_discrete->write(space, DRAGRACE_HITONE_EN, (m_misc_flags & 0x20000000) ? 1: 0);  // HiTone enable

	// the tachometers are driven from the same frequency generator that creates the engine sound
	output_set_value("tachometer", ~m_misc_flags & 0x0000001f);
	output_set_value("tachometer2", (~m_misc_flags & 0x001f0000) >> 0x10);
}

WRITE8_MEMBER(dragrace_state::dragrace_misc_w)
{
	/* Set/clear individual bit */
	UINT32 mask = 1 << offset;
	if (data & 0x01)
		m_misc_flags |= mask;
	else
		m_misc_flags &= (~mask);
	logerror("Set   %#6x, Mask=%#10x, Flag=%#10x, Data=%x\n", 0x0900 + offset, mask, m_misc_flags, data & 0x01);
	dragrace_update_misc_flags(space);
}

WRITE8_MEMBER(dragrace_state::dragrace_misc_clear_w)
{
	/* Clear 8 bits */
	UINT32 mask = 0xff << (((offset >> 3) & 0x03) * 8);
	m_misc_flags &= (~mask);
	logerror("Clear %#6x, Mask=%#10x, Flag=%#10x, Data=%x\n", 0x0920 + offset, mask, m_misc_flags, data & 0x01);
	dragrace_update_misc_flags(space);
}

READ8_MEMBER(dragrace_state::dragrace_input_r)
{
	int val = ioport("IN2")->read();
	static const char *const portnames[] = { "IN0", "IN1" };

	UINT8 maskA = 1 << (offset % 8);
	UINT8 maskB = 1 << (offset / 8);

	for (int i = 0; i < 2; i++)
	{
		int in = ioport(portnames[i])->read();

		if (m_gear[i] != 0)
			in &= ~(1 << m_gear[i]);

		if (in & maskA)
			val |= 1 << i;
	}

	return (val & maskB) ? 0xff : 0x7f;
}


READ8_MEMBER(dragrace_state::dragrace_steering_r)
{
	int bitA[2];
	int bitB[2];
	static const char *const dialnames[] = { "DIAL1", "DIAL2" };

	for (int i = 0; i < 2; i++)
	{
		int dial = ioport(dialnames[i])->read();

		bitA[i] = ((dial + 1) / 2) & 1;
		bitB[i] = ((dial + 0) / 2) & 1;
	}

	return
		(bitA[0] << 0) | (bitB[0] << 1) |
		(bitA[1] << 2) | (bitB[1] << 3);
}


READ8_MEMBER(dragrace_state::dragrace_scanline_r)
{
	return (m_screen->vpos() ^ 0xf0) | 0x0f;
}


static ADDRESS_MAP_START( dragrace_map, AS_PROGRAM, 8, dragrace_state )
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0x0800, 0x083f) AM_READ(dragrace_input_r)
	AM_RANGE(0x0900, 0x091f) AM_WRITE(dragrace_misc_w)
	AM_RANGE(0x0920, 0x093f) AM_WRITE(dragrace_misc_clear_w)
	AM_RANGE(0x0a00, 0x0aff) AM_WRITEONLY AM_SHARE("playfield_ram")
	AM_RANGE(0x0b00, 0x0bff) AM_WRITEONLY AM_SHARE("position_ram")
	AM_RANGE(0x0c00, 0x0c00) AM_READ(dragrace_steering_r)
	AM_RANGE(0x0d00, 0x0d00) AM_READ(dragrace_scanline_r)
	AM_RANGE(0x0e00, 0x0eff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x1000, 0x1fff) AM_ROM /* program */
	AM_RANGE(0xf800, 0xffff) AM_ROM /* program mirror */
ADDRESS_MAP_END


static INPUT_PORTS_START( dragrace )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 1 Gas") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) /* player 1 gear 1 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) /* player 1 gear 2 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) /* player 1 gear 3 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) /* player 1 gear 4 */
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0xc0, 0x80, "Extended Play" )
	PORT_DIPSETTING( 0x00, "6.9 seconds" )
	PORT_DIPSETTING( 0x80, "5.9 seconds" )
	PORT_DIPSETTING( 0x40, "4.9 seconds" )
	PORT_DIPSETTING( 0xc0, "Never" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 2 Gas") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) /* player 2 gear 1 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) /* player 2 gear 2 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) /* player 2 gear 3 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) /* player 2 gear 4 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x80, "Number Of Heats" )
	PORT_DIPSETTING( 0xc0, "3" )
	PORT_DIPSETTING( 0x80, "4" )
	PORT_DIPSETTING( 0x00, "5" )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) /* IN0 connects here */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) /* IN1 connects here */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING( 0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Free_Play ) )

	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL_V ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL_V ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("P1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 1 Gear 1") PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 1 Gear 2") PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 1 Gear 3") PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 1 Gear 4") PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Player 1 Neutral") PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Player 2 Gear 1") PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Player 2 Gear 2") PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Player 2 Gear 3") PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Player 2 Gear 4") PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Player 2 Neutral") PORT_PLAYER(2)

	PORT_START("MOTOR1")
	PORT_ADJUSTER( 81, "Motor 1 RPM" )

	PORT_START("MOTOR2")
	PORT_ADJUSTER( 85, "Motor 2 RPM" )
INPUT_PORTS_END


static const gfx_layout dragrace_tile_layout1 =
{
	16, 16,   /* width, height */
	0x40,     /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x100      /* increment */
};


static const gfx_layout dragrace_tile_layout2 =
{
	16, 16,   /* width, height */
	0x20,     /* total         */
	2,        /* planes        */
	{         /* plane offsets */
		0x0000, 0x2000
	},
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x100      /* increment */
};


static GFXDECODE_START( dragrace )
	GFXDECODE_ENTRY( "gfx1", 0, dragrace_tile_layout1, 0, 4 )
	GFXDECODE_ENTRY( "gfx2", 0, dragrace_tile_layout2, 8, 2 )
GFXDECODE_END


PALETTE_INIT_MEMBER(dragrace_state, dragrace)
{
	palette.set_pen_color(0, rgb_t(0xFF, 0xFF, 0xFF));   /* 2 color tiles */
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(2, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(3, rgb_t(0xFF, 0xFF, 0xFF));
	palette.set_pen_color(4, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(5, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(6, rgb_t(0xFF, 0xFF, 0xFF));
	palette.set_pen_color(7, rgb_t(0xFF, 0xFF, 0xFF));
	palette.set_pen_color(8, rgb_t(0xFF, 0xFF, 0xFF));   /* 4 color tiles */
	palette.set_pen_color(9, rgb_t(0xB0, 0xB0, 0xB0));
	palette.set_pen_color(10,rgb_t(0x5F, 0x5F, 0x5F));
	palette.set_pen_color(11,rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(12,rgb_t(0xFF, 0xFF, 0xFF));
	palette.set_pen_color(13,rgb_t(0x5F, 0x5F, 0x5F));
	palette.set_pen_color(14,rgb_t(0xB0, 0xB0, 0xB0));
	palette.set_pen_color(15,rgb_t(0x00, 0x00, 0x00));
}


void dragrace_state::machine_start()
{
	save_item(NAME(m_misc_flags));
	save_item(NAME(m_gear));
}

void dragrace_state::machine_reset()
{
	m_misc_flags = 0;
	m_gear[0] = 0;
	m_gear[1] = 0;
}

static MACHINE_CONFIG_START( dragrace, dragrace_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_12_096MHz / 12)
	MCFG_CPU_PROGRAM_MAP(dragrace_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(dragrace_state, irq0_line_hold,  4*60)
	MCFG_WATCHDOG_VBLANK_INIT(8)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("frame_timer", dragrace_state, dragrace_frame_callback, attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(dragrace_state, screen_update_dragrace)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dragrace)
	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(dragrace_state, dragrace)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(dragrace)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


ROM_START( dragrace )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8513.c1", 0x1000, 0x0800, CRC(543bbb30) SHA1(646a41d1124c8365f07a93de38af007895d7d263) )
	ROM_LOAD( "8514.a1", 0x1800, 0x0800, CRC(ad218690) SHA1(08ba5f4fa4c75d8dad1a7162888d44b3349cbbe4) )
	ROM_RELOAD(          0xf800, 0x0800 )

	ROM_REGION( 0x800, "gfx1", 0 )   /* 2 color tiles */
	ROM_LOAD( "8519dr.j0", 0x000, 0x200, CRC(aa221ba0) SHA1(450acbf349d77a790a25f3e303c31b38cc426a38) )
	ROM_LOAD( "8521dr.k0", 0x200, 0x200, CRC(0cb33f12) SHA1(d50cb55391aec03e064eecad1624d50d4c30ccab) )
	ROM_LOAD( "8520dr.r0", 0x400, 0x200, CRC(ee1ae6a7) SHA1(83491095260c8b7c616ff17ec1e888d05620f166) )

	ROM_REGION( 0x800, "gfx2", 0 )   /* 4 color tiles */
	ROM_LOAD( "8515dr.e0", 0x000, 0x200, CRC(9510a59e) SHA1(aea0782b919279efe55a07007bd55a16f7f59239) )
	ROM_LOAD( "8517dr.h0", 0x200, 0x200, CRC(8b5bff1f) SHA1(fdcd719c66bff7c4b9f3d56d1e635259dd8add61) )
	ROM_LOAD( "8516dr.l0", 0x400, 0x200, CRC(d1e74af1) SHA1(f55a3bfd7d152ac9af128697f55c9a0c417779f5) )
	ROM_LOAD( "8518dr.n0", 0x600, 0x200, CRC(b1369028) SHA1(598a8779982d532c9f34345e793a79fcb29cac62) )

	ROM_REGION( 0x100, "sync", 0 )  /* sync prom located at L8, it's a 82s129 */
	ROM_LOAD( "l8.bin", 0x000, 0x100, CRC(3610b453) SHA1(9e33ee04f22a9174c29fafb8e71781fa330a7a08) )
ROM_END


GAMEL(1977, dragrace, 0, dragrace, dragrace, driver_device, 0, 0, "Atari (Kee Games)", "Drag Race", MACHINE_SUPPORTS_SAVE, layout_dragrace )
