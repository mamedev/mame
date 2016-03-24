// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

    Atari Mini Golf (prototype) driver

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"

class mgolf_state : public driver_device
{
public:
	enum
	{
		TIMER_INTERRUPT
	};

	mgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_video_ram(*this, "video_ram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<UINT8> m_video_ram;

	/* video-related */
	tilemap_t* m_bg_tilemap;

	/* misc */
	UINT8 m_prev;
	UINT8 m_mask;
	attotime m_time_pushed;
	attotime m_time_released;
	emu_timer *m_interrupt_timer;

	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(wram_r);
	DECLARE_READ8_MEMBER(dial_r);
	DECLARE_READ8_MEMBER(misc_r);
	DECLARE_WRITE8_MEMBER(wram_w);

	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(mgolf);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(interrupt_callback);

	void update_plunger(  );
	double calc_plunger_pos();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


TILE_GET_INFO_MEMBER(mgolf_state::get_tile_info)
{
	UINT8 code = m_video_ram[tile_index];

	SET_TILE_INFO_MEMBER(0, code, code >> 7, 0);
}


WRITE8_MEMBER(mgolf_state::vram_w)
{
	m_video_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void mgolf_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mgolf_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


UINT32 mgolf_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	/* draw playfield */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw sprites */
	for (i = 0; i < 2; i++)
	{
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			m_video_ram[0x399 + 4 * i],
			i,
			0, 0,
			m_video_ram[0x390 + 2 * i] - 7,
			m_video_ram[0x398 + 4 * i] - 16, 0);

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			m_video_ram[0x39b + 4 * i],
			i,
			0, 0,
			m_video_ram[0x390 + 2 * i] - 15,
			m_video_ram[0x39a + 4 * i] - 16, 0);
	}
	return 0;
}


void mgolf_state::update_plunger(  )
{
	UINT8 val = ioport("BUTTON")->read();

	if (m_prev != val)
	{
		if (val == 0)
		{
			m_time_released = machine().time();

			if (!m_mask)
				m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		}
		else
			m_time_pushed = machine().time();

		m_prev = val;
	}
}


void mgolf_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_INTERRUPT:
		interrupt_callback(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in mgolf_state::device_timer");
	}
}


TIMER_CALLBACK_MEMBER(mgolf_state::interrupt_callback)
{
	int scanline = param;

	update_plunger();

	generic_pulse_irq_line(*m_maincpu, 0, 1);

	scanline = scanline + 32;

	if (scanline >= 262)
		scanline = 16;

	m_interrupt_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


double mgolf_state::calc_plunger_pos()
{
	return (machine().time().as_double() - m_time_released.as_double()) * (m_time_released.as_double() - m_time_pushed.as_double() + 0.2);
}


READ8_MEMBER(mgolf_state::wram_r)
{
	return m_video_ram[0x380 + offset];
}


READ8_MEMBER(mgolf_state::dial_r)
{
	UINT8 val = ioport("41")->read();

	if ((ioport("DIAL")->read() + 0x00) & 0x20)
	{
		val |= 0x01;
	}
	if ((ioport("DIAL")->read() + 0x10) & 0x20)
	{
		val |= 0x02;
	}

	return val;
}


READ8_MEMBER(mgolf_state::misc_r)
{
	double plunger = calc_plunger_pos(); /* see Video Pinball */

	UINT8 val = ioport("61")->read();

	if (plunger >= 0.000 && plunger <= 0.001)
	{
		val &= ~0x20;   /* PLUNGER1 */
	}
	if (plunger >= 0.006 && plunger <= 0.007)
	{
		val &= ~0x40;   /* PLUNGER2 */
	}

	return val;
}


WRITE8_MEMBER(mgolf_state::wram_w)
{
	m_video_ram[0x380 + offset] = data;
}



static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 8, mgolf_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)

	AM_RANGE(0x0040, 0x0040) AM_READ_PORT("40")
	AM_RANGE(0x0041, 0x0041) AM_READ(dial_r)
	AM_RANGE(0x0060, 0x0060) AM_READ_PORT("60")
	AM_RANGE(0x0061, 0x0061) AM_READ(misc_r)
	AM_RANGE(0x0080, 0x00ff) AM_READ(wram_r)
	AM_RANGE(0x0180, 0x01ff) AM_READ(wram_r)
	AM_RANGE(0x0800, 0x0bff) AM_READONLY

	AM_RANGE(0x0000, 0x0009) AM_WRITENOP
	AM_RANGE(0x0024, 0x0024) AM_WRITENOP
	AM_RANGE(0x0028, 0x0028) AM_WRITENOP
	AM_RANGE(0x0042, 0x0042) AM_WRITENOP
	AM_RANGE(0x0044, 0x0044) AM_WRITENOP /* watchdog? */
	AM_RANGE(0x0046, 0x0046) AM_WRITENOP
	AM_RANGE(0x0060, 0x0060) AM_WRITENOP
	AM_RANGE(0x0061, 0x0061) AM_WRITENOP
	AM_RANGE(0x006a, 0x006a) AM_WRITENOP
	AM_RANGE(0x006c, 0x006c) AM_WRITENOP
	AM_RANGE(0x006d, 0x006d) AM_WRITENOP
	AM_RANGE(0x0080, 0x00ff) AM_WRITE(wram_w)
	AM_RANGE(0x0180, 0x01ff) AM_WRITE(wram_w)
	AM_RANGE(0x0800, 0x0bff) AM_WRITE(vram_w) AM_SHARE("video_ram")

	AM_RANGE(0x2000, 0x3fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( mgolf )

	PORT_START("40")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x10, DEF_STR( French ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x30, DEF_STR( German ) )
	PORT_DIPNAME( 0xc0, 0x40, "Shots per Coin" )
	PORT_DIPSETTING(    0x00, "25" )
	PORT_DIPSETTING(    0x40, "30" )
	PORT_DIPSETTING(    0x80, "35" )
	PORT_DIPSETTING(    0xc0, "40" )

	PORT_START("41")
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* DIAL A */
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* DIAL B */
	PORT_BIT ( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT ( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("60")
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START("61")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Course Select") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL ) /* PLUNGER 1 */
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) /* PLUNGER 2 */
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(25)

	PORT_START("BUTTON")
	PORT_BIT ( 0xff, IP_ACTIVE_HIGH, IPT_BUTTON1 )

INPUT_PORTS_END


PALETTE_INIT_MEMBER(mgolf_state, mgolf)
{
	palette.set_pen_color(0, rgb_t(0x80, 0x80, 0x80));
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(2, rgb_t(0x80, 0x80, 0x80));
	palette.set_pen_color(3, rgb_t(0xff, 0xff, 0xff));
}

static const gfx_layout tile_layout =
{
	8, 8,
	128,
	1,
	{ 0 },
	{
		0, 1, 2, 3, 4, 5, 6, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
	},
	0x40
};


static const gfx_layout sprite_layout =
{
	8, 16,
	16,
	1,
	{ 0 },
	{
		7, 6, 5, 4, 3, 2, 1, 0,
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x80
};


static GFXDECODE_START( mgolf )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout, 0, 2 )
GFXDECODE_END


void mgolf_state::machine_start()
{
	m_interrupt_timer = timer_alloc(TIMER_INTERRUPT);

	save_item(NAME(m_prev));
	save_item(NAME(m_mask));
	save_item(NAME(m_time_pushed));
	save_item(NAME(m_time_released));
}

void mgolf_state::machine_reset()
{
	m_interrupt_timer->adjust(m_screen->time_until_pos(16), 16);

	m_mask = 0;
	m_prev = 0;
}


static MACHINE_CONFIG_START( mgolf, mgolf_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 12096000 / 16) /* ? */
	MCFG_CPU_PROGRAM_MAP(cpu_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 0, 223)
	MCFG_SCREEN_UPDATE_DRIVER(mgolf_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mgolf)
	MCFG_PALETTE_ADD("palette", 4)
	MCFG_PALETTE_INIT_OWNER(mgolf_state, mgolf)

	/* sound hardware */
MACHINE_CONFIG_END


ROM_START( mgolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "33496-01.e1", 0x2000, 0x0800, CRC(9ea98f39) SHA1(f5685554c2088032d3e8b9e8066bb3e8274c2425) )
	ROM_LOAD_NIB_HIGH( "33497-01.j1", 0x2000, 0x0800, CRC(0f34962b) SHA1(f71c4a008905bc87cb2ce4971fea357ed7d5d28a) )
	ROM_LOAD_NIB_LOW ( "33498-01.c2", 0x2800, 0x0800, CRC(413b616e) SHA1(dec9d9a86159a378ae79986d7fbc6f326b48c969) )
	ROM_LOAD_NIB_HIGH( "33499-01.k1", 0x2800, 0x0800, CRC(e4566326) SHA1(bc838f1bb82c865ec4357b3274ff3306336a4601) )
	ROM_LOAD_NIB_LOW ( "33500-01.e2", 0x3000, 0x0800, CRC(50bb1eb6) SHA1(6973d4817d4819fb2ada88f96f19d8248228d01f) )
	ROM_LOAD_NIB_HIGH( "33501-01.m2", 0x3000, 0x0800, CRC(a66a6ff2) SHA1(aa58349451e31b9ab28136a424e83dfc796af205) )
	ROM_LOAD_NIB_LOW ( "33502-01.j2", 0x3800, 0x0800, CRC(2177b041) SHA1(c842f8764e28c377e35458f1ae972a3c0278df45) )
	ROM_LOAD_NIB_HIGH( "33503-01.k2", 0x3800, 0x0800, CRC(db6ccbf6) SHA1(84f7b8bf37b487a386f700fb35c15a0c6e5254a4) )

	ROM_REGION( 0x0400, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "33524-01.h8", 0x0000, 0x0200, CRC(bd0e3bb3) SHA1(d833bf777118800c84fdae3d52c856375e05bc26) )
	ROM_LOAD( "33525-01.f8", 0x0200, 0x0200, CRC(7b2bac96) SHA1(2d2580b66b56de2837ccb3b60d0f24a03d018fbd) )

	ROM_REGION( 0x0100, "gfx2", 0 ) /* sprites */
	ROM_LOAD_NIB_LOW ( "33526-01.f5", 0x0000, 0x0100, CRC(feee59ad) SHA1(6a7a3e043d7db2c2711029fcd49e1e2ff4cfde78) )
	ROM_LOAD_NIB_HIGH( "33527-01.e5", 0x0000, 0x0100, CRC(d482bdf2) SHA1(59251980bb7c6b02dcd75c46e32c9bf9d8c5e8c1) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "33756-01.m7", 0x0000, 0x0200, CRC(4cec9bf3) SHA1(6dd49f045fb53ae9f412639117b107faa93dfd99) )
ROM_END


GAME( 1978, mgolf, 0, mgolf, mgolf, driver_device, 0, ROT270, "Atari", "Atari Mini Golf (prototype)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
