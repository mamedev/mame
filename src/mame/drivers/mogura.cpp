// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Mogura Desse */

#include "emu.h"
#include "includes/konamipt.h"

#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "speaker.h"


class mogura_state : public driver_device
{
public:
	mogura_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ldac(*this, "ldac"),
		m_rdac(*this, "rdac"),
		m_gfxram(*this, "gfxram"),
		m_tileram(*this, "tileram"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<dac_byte_interface> m_ldac;
	required_device<dac_byte_interface> m_rdac;
	required_shared_ptr<uint8_t> m_gfxram;
	required_shared_ptr<uint8_t> m_tileram;
	required_device<gfxdecode_device> m_gfxdecode;

	tilemap_t *m_tilemap;
	DECLARE_WRITE8_MEMBER(mogura_tileram_w);
	DECLARE_WRITE8_MEMBER(mogura_dac_w);
	DECLARE_WRITE8_MEMBER(mogura_gfxram_w);
	TILE_GET_INFO_MEMBER(get_mogura_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(mogura);
	uint32_t screen_update_mogura(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mogura(machine_config &config);
	void mogura_io_map(address_map &map);
	void mogura_map(address_map &map);
};


PALETTE_INIT_MEMBER(mogura_state, mogura)
{
	const uint8_t *color_prom = memregion("proms")->base();
	int i, j;

	j = 0;
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(j, rgb_t(r, g, b));
		j += 4;
		if (j > 31) j -= 31;
	}
}


TILE_GET_INFO_MEMBER(mogura_state::get_mogura_tile_info)
{
	int code = m_tileram[tile_index];
	int attr = m_tileram[tile_index + 0x800];

	SET_TILE_INFO_MEMBER(0,
			code,
			(attr >> 1) & 7,
			0);
}


void mogura_state::video_start()
{
	m_gfxdecode->gfx(0)->set_source(m_gfxram);
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(mogura_state::get_mogura_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

uint32_t mogura_state::screen_update_mogura(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle &visarea = screen.visible_area();

	/* tilemap layout is a bit strange ... */
	rectangle clip = visarea;
	clip.max_x = 256 - 1;
	m_tilemap->set_scrollx(0, 256);
	m_tilemap->draw(screen, bitmap, clip, 0, 0);

	clip.min_x = 256;
	clip.max_x = 512 - 1;
	m_tilemap->set_scrollx(0, -128);
	m_tilemap->draw(screen, bitmap, clip, 0, 0);

	return 0;
}

WRITE8_MEMBER(mogura_state::mogura_tileram_w)
{
	m_tileram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE8_MEMBER(mogura_state::mogura_dac_w)
{
	m_ldac->write(data >> 4);
	m_rdac->write(data & 15);
}


WRITE8_MEMBER(mogura_state::mogura_gfxram_w)
{
	m_gfxram[offset] = data ;

	m_gfxdecode->gfx(0)->mark_dirty(offset / 16);
}


ADDRESS_MAP_START(mogura_state::mogura_map)
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM // main ram
	AM_RANGE(0xe000, 0xefff) AM_RAM_WRITE(mogura_gfxram_w) AM_SHARE("gfxram") // ram based characters
	AM_RANGE(0xf000, 0xffff) AM_RAM_WRITE(mogura_tileram_w) AM_SHARE("tileram") // tilemap
ADDRESS_MAP_END

ADDRESS_MAP_START(mogura_state::mogura_io_map)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITENOP    // ??
	AM_RANGE(0x08, 0x08) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("P1")
	AM_RANGE(0x0d, 0x0d) AM_READ_PORT("P2")
	AM_RANGE(0x0e, 0x0e) AM_READ_PORT("P3")
	AM_RANGE(0x0f, 0x0f) AM_READ_PORT("P4")
	AM_RANGE(0x10, 0x10) AM_READ_PORT("SERVICE")
	AM_RANGE(0x14, 0x14) AM_WRITE(mogura_dac_w) /* 4 bit DAC x 2. MSB = left, LSB = right */
ADDRESS_MAP_END

static INPUT_PORTS_START( mogura )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )

	PORT_START("P1")
	KONAMI8_B123_START(1)

	PORT_START("P2")
	KONAMI8_B123_START(2)

	PORT_START("P3")
	KONAMI8_B123_START(3)

	PORT_START("P4")
	KONAMI8_B123_START(4)

	PORT_START("SERVICE")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout tiles8x8_layout =
{
	8,8,
	0x1000*8/(16*8),
	2,
	{ 0, 1 },
	{ 0, 2, 4, 6, 8, 10, 12, 14 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( mogura )
	GFXDECODE_ENTRY( nullptr, 0, tiles8x8_layout, 0, 8 )
GFXDECODE_END

void mogura_state::machine_start()
{
}

MACHINE_CONFIG_START(mogura_state::mogura)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,3000000)         /* 3 MHz */
	MCFG_CPU_PROGRAM_MAP(mogura_map)
	MCFG_CPU_IO_MAP(mogura_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mogura_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60) // ?
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(mogura_state, screen_update_mogura)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mogura)
	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(mogura_state, mogura)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ldac", DAC_4BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25) // unknown DAC
	MCFG_SOUND_ADD("rdac", DAC_4BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "ldac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "ldac", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "rdac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "rdac", -1.0, DAC_VREF_NEG_INPUT)
	MACHINE_CONFIG_END


ROM_START( mogura )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gx141.5n", 0x00000, 0x08000, CRC(98e6120d) SHA1(45cdb2d78224a7c44fff8cd3487f33c57669a06c)  )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "gx141.7j", 0x00, 0x20,  CRC(b21c5d5f) SHA1(6913c840dd69a7d4687f4c4cbe3ff12300f62bc2) )
ROM_END

GAME( 1991, mogura, 0, mogura, mogura, mogura_state, 0, ROT0, "Konami", "Mogura Desse (Japan)", MACHINE_SUPPORTS_SAVE )
