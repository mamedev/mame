// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

 quickpick5.cpp: Konami "Quick Pick 5" medal game

 Quick Pick 5
 (c) 199? Konami

 Driver by R. Belmont

 Rundown of PCB:
  Main CPU:  Z80

 Konami Custom chips:
  051649 (SCC1 sound)
  053252 (timing/interrupt controller?)
  053244 (sprites)
  053245 (sprites)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/k051649.h"
#include "sound/okim6295.h"
#include "video/k053244_k053245.h"
#include "video/konami_helper.h"
#include "screen.h"
#include "speaker.h"

class quickpick5_state : public driver_device
{
public:
	quickpick5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_k053245(*this, "k053245"),
		m_gfxdecode(*this, "gfxdecode"),
		m_vram(*this, "vram"),
		m_oki(*this, "oki")
	{ }

	uint32_t screen_update_quickpick5(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	K05324X_CB_MEMBER(sprite_callback);
	TILE_GET_INFO_MEMBER(ttl_get_tile_info);

	READ8_MEMBER(control_r) { return m_control; }

	READ8_MEMBER(inp_magic_r) { return 0xff; }

	WRITE8_MEMBER(control_w)
	{
		printf("%02x to control\n", data);
		membank("bank1")->set_entry(data&0x1);
		//m_k053245->set_rmrd_line((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
		if (((m_control & 0x60) != 0x60) && ((data & 0x60) == 0x60))
		{
			m_ttlrom_offset = 0;
		}
		m_control = data;
	}

	DECLARE_READ8_MEMBER(vram_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<k05324x_device> m_k053245;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_vram;
	required_device<okim6295_device> m_oki;

	int         m_ttl_gfx_index;
	tilemap_t   *m_ttl_tilemap;
	uint8_t     m_control;
	int         m_ttlrom_offset;
};

READ8_MEMBER(quickpick5_state::vram_r)
{
	if ((m_control & 0x60) == 0x60)
	{
		uint8_t *ROM = memregion("ttl")->base();
		return ROM[m_ttlrom_offset++];
	}

	return m_vram[offset];
}

void quickpick5_state::video_start()
{
	static const gfx_layout charlayout =
	{
		8, 8,   // 8x8
		4096,   // # of tiles
		4,      // 4bpp
		{ 0, 1, 2, 3 }, // plane offsets
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 }, // X offsets
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 }, // Y offsets
		8*8*4
	};

	int gfx_index;

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (m_gfxdecode->gfx(gfx_index) == nullptr)
			break;

	assert(gfx_index != MAX_GFX_ELEMENTS);

	// decode the ttl layer's gfx
	m_gfxdecode->set_gfx(gfx_index, std::make_unique<gfx_element>(m_palette, charlayout, memregion("ttl")->base(), 0, m_palette->entries() / 16, 0));
	m_ttl_gfx_index = gfx_index;

	m_ttl_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(quickpick5_state::ttl_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_ttl_tilemap->set_transparent_pen(0);

	m_ttlrom_offset = 0;
}

TILE_GET_INFO_MEMBER(quickpick5_state::ttl_get_tile_info)
{
	uint8_t *lvram = &m_vram[0];
	int attr, code;

	attr = lvram[BYTE_XOR_LE((tile_index<<1)+1)];
	code = lvram[BYTE_XOR_LE((tile_index<<1))] | ((attr & 0xf) << 8);
	attr >>= 4;

	SET_TILE_INFO_MEMBER(m_ttl_gfx_index, code, attr, 0);
}

uint32_t quickpick5_state::screen_update_quickpick5(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);

	m_k053245->sprites_draw(bitmap, cliprect, screen.priority());

	m_ttl_tilemap->mark_all_dirty();
	m_ttl_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

K05324X_CB_MEMBER(quickpick5_state::sprite_callback)
{
	*priority = 0;
}

static ADDRESS_MAP_START( quickpick5_main, AS_PROGRAM, 8, quickpick5_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc000) AM_READWRITE(control_r, control_w)
	AM_RANGE(0xd800, 0xdbff) AM_RAM // stack
	AM_RANGE(0xdc40, 0xdc4f) AM_DEVREADWRITE("k053245", k05324x_device, k053244_r, k053244_w)
	AM_RANGE(0xdcc1, 0xdcc1) AM_READ(inp_magic_r)
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE("vram") AM_READ(vram_r)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_DEVREADWRITE("k053245", k05324x_device, k053245_r, k053245_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( quickpick5 )
INPUT_PORTS_END

void quickpick5_state::machine_start()
{
	membank("bank1")->configure_entries(0, 0x2, memregion("maincpu")->base()+0x8000, 0x4000);
	membank("bank1")->set_entry(0);
}

void quickpick5_state::machine_reset()
{
}

static MACHINE_CONFIG_START( quickpick5 )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_32MHz/4) // z84c0008pec 8mhz part, 32Mhz xtal verified on PCB, divisor unknown
	MCFG_CPU_PROGRAM_MAP(quickpick5_main)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.62)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 33*8)
	MCFG_SCREEN_VISIBLE_AREA(80, 464-1, 24, 264-1)
	MCFG_SCREEN_UPDATE_DRIVER(quickpick5_state, screen_update_quickpick5)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 8192)
	MCFG_PALETTE_ENABLE_SHADOWS()
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("k053245", K053245, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K05324X_OFFSETS(0, 0)
	MCFG_K05324X_CB(quickpick5_state, sprite_callback)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_K051649_ADD("k051649", XTAL_32MHz/16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)

	MCFG_OKIM6295_ADD("oki", XTAL_32MHz/16, PIN7_HIGH)
	MCFG_SOUND_ROUTE(0, "mono", 1.0)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
MACHINE_CONFIG_END

ROM_START( quickp5 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "117.10e.bin",  0x000000, 0x010000, CRC(3645e1a5) SHA1(7d0d98772f3732510e7a58f50a622fcec74087c3) )

	ROM_REGION( 0x40000, "k053245", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "117-a02-7k.bin", 0x000003, 0x010000, CRC(745a1dc9) SHA1(33d876fb70cb802d62f87ad3721740e0961c7bec) )
	ROM_LOAD16_BYTE( "117-a03-7l.bin", 0x000002, 0x010000, CRC(07ec6db7) SHA1(7a94efc5f313fee6b9b63b7d2b6ba1cbf4158900) )
	ROM_LOAD16_BYTE( "117-a04-3l.bin", 0x000001, 0x010000, CRC(08dba5df) SHA1(2174be21c5a7db31ccc20ca0b88e4a94145776a5) )
	ROM_LOAD16_BYTE( "117-a05-3k.bin", 0x000000, 0x010000, CRC(9b2d0501) SHA1(3f1c69ef101153da5ac3335585541006c42e954d) )

	ROM_REGION( 0x80000, "ttl", 0 ) /* TTL text tilemap characters? */
	ROM_LOAD( "117-18e.bin",  0x000000, 0x020000, CRC(10e0d1e2) SHA1(f4ba190814d5e3f3e910c9da24845b6ddb259bff) )

	ROM_REGION( 0x20000, "okim6295", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "117-a01-2e.bin", 0x000000, 0x020000, CRC(3d8fbd01) SHA1(f350da2a4e7bfff9975188a39acf73415bd85b3d) )

	ROM_REGION( 0x80000, "pals", 0 )
	ROM_LOAD( "054590.11g",   0x000000, 0x040000, CRC(0442621c) SHA1(2e79bea4e37028a3c1223fb4e3b3e12ccad2b39b) )
	ROM_LOAD( "054591.12g",   0x040000, 0x040000, CRC(eaa92d8f) SHA1(7a430f11127148f0c035973ce21cfec4cb60ce9d) )

ROM_END

GAME( 1995, quickp5, 0, quickpick5, quickpick5,  quickpick5_state, 0, ROT0, "Konami", "Quick Pick 5", MACHINE_NOT_WORKING)

