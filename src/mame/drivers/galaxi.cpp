// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

Galaxi (C)2000 B.R.L.

driver by Luca Elia

Hardware info (29/07/2008 f205v):

Chips:
    1x missing main CPU (u1)(from the socket I would say it's a 68000)
    1x A40MX04-PL84 (u29)
    1x AD-65 (equivalent to M6295) (u9)(sound)
    1x MC1458P (u10)(sound)
    1x TDA2003 (u8)(sound)
    1x oscillator 10.000MHz (QZ1)
    1x oscillator 16.000000 (QZ2)
ROMs:
    1x AT27C020 (1)
    2x M27C4001 (2,3)
    2x AT49F010 (4,5)
    2x DS1230Y (non volatile SRAM)
Notes:
    1x 28x2 edge connector
    1x trimmer (volume)

- This hardware is almost identical to that in magic10.c


[31/08/2008] (Roberto Fresca)

- Added Magic Joker.
- Fixed the 3rd background offset to Galaxi.
- Remapped inputs to match the standard poker games.

[12/09/2008] (Roberto Fresca)

- Added lamps support to magjoker & galaxi.


***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"
#include "galaxi.lh"

class galaxi_state : public driver_device
{
public:
	galaxi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg1_ram(*this, "bg1_ram"),
		m_bg2_ram(*this, "bg2_ram"),
		m_bg3_ram(*this, "bg3_ram"),
		m_bg4_ram(*this, "bg4_ram"),
		m_fg_ram(*this, "fg_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_bg1_ram;
	required_shared_ptr<UINT16> m_bg2_ram;
	required_shared_ptr<UINT16> m_bg3_ram;
	required_shared_ptr<UINT16> m_bg4_ram;
	required_shared_ptr<UINT16> m_fg_ram;
//  UINT16 *  m_nvram;        // currently this uses generic nvram handling

	/* video-related */
	tilemap_t   *m_bg1_tmap;
	tilemap_t   *m_bg2_tmap;
	tilemap_t   *m_bg3_tmap;
	tilemap_t   *m_bg4_tmap;
	tilemap_t   *m_fg_tmap;

	/* misc */
	int       m_hopper;
	int       m_ticket;
	UINT16    m_out[3];
	DECLARE_WRITE16_MEMBER(galaxi_bg1_w);
	DECLARE_WRITE16_MEMBER(galaxi_bg2_w);
	DECLARE_WRITE16_MEMBER(galaxi_bg3_w);
	DECLARE_WRITE16_MEMBER(galaxi_bg4_w);
	DECLARE_WRITE16_MEMBER(galaxi_fg_w);
	DECLARE_WRITE16_MEMBER(galaxi_500000_w);
	DECLARE_WRITE16_MEMBER(galaxi_500002_w);
	DECLARE_WRITE16_MEMBER(galaxi_500004_w);
	DECLARE_CUSTOM_INPUT_MEMBER(ticket_r);
	DECLARE_CUSTOM_INPUT_MEMBER(hopper_r);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_bg3_tile_info);
	TILE_GET_INFO_MEMBER(get_bg4_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_galaxi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void show_out(  );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};


/***************************************************************************
                                Video Hardware
***************************************************************************/

TILE_GET_INFO_MEMBER(galaxi_state::get_bg1_tile_info)
{
	UINT16 code = m_bg1_ram[tile_index];
	SET_TILE_INFO_MEMBER(0, code, 0x10 + (code >> 12), 0);
}

TILE_GET_INFO_MEMBER(galaxi_state::get_bg2_tile_info)
{
	UINT16 code = m_bg2_ram[tile_index];
	SET_TILE_INFO_MEMBER(0, code, 0x10 + (code >> 12), 0);
}

TILE_GET_INFO_MEMBER(galaxi_state::get_bg3_tile_info)
{
	UINT16 code = m_bg3_ram[tile_index];
	SET_TILE_INFO_MEMBER(0, code, (code >> 12), 0);
}

TILE_GET_INFO_MEMBER(galaxi_state::get_bg4_tile_info)
{
	UINT16 code = m_bg4_ram[tile_index];
	SET_TILE_INFO_MEMBER(0, code, (code >> 12), 0);
}

TILE_GET_INFO_MEMBER(galaxi_state::get_fg_tile_info)
{
	UINT16 code = m_fg_ram[tile_index];
	SET_TILE_INFO_MEMBER(1, code, 0x20 + (code >> 12), 0);
}

WRITE16_MEMBER(galaxi_state::galaxi_bg1_w)
{
	COMBINE_DATA(&m_bg1_ram[offset]);
	m_bg1_tmap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(galaxi_state::galaxi_bg2_w)
{
	COMBINE_DATA(&m_bg2_ram[offset]);
	m_bg2_tmap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(galaxi_state::galaxi_bg3_w)
{
	COMBINE_DATA(&m_bg3_ram[offset]);
	m_bg3_tmap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(galaxi_state::galaxi_bg4_w)
{
	COMBINE_DATA(&m_bg4_ram[offset]);
	m_bg4_tmap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(galaxi_state::galaxi_fg_w)
{
	COMBINE_DATA(&m_fg_ram[offset]);
	m_fg_tmap->mark_tile_dirty(offset);
}

void galaxi_state::video_start()
{
	m_bg1_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(galaxi_state::get_bg1_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 0x20, 0x10);
	m_bg2_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(galaxi_state::get_bg2_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 0x20, 0x10);
	m_bg3_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(galaxi_state::get_bg3_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 0x20, 0x10);
	m_bg4_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(galaxi_state::get_bg4_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 0x20, 0x10);

	m_fg_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(galaxi_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 0x40, 0x20);

	m_bg1_tmap->set_transparent_pen(0);
	m_bg2_tmap->set_transparent_pen(0);
	m_bg3_tmap->set_transparent_pen(0);
	m_bg4_tmap->set_transparent_pen(0);

	m_fg_tmap->set_transparent_pen(0);

	m_bg3_tmap->set_scrolldx(-8, 0);
}

UINT32 galaxi_state::screen_update_galaxi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_R))  // remapped due to inputs changes.
	{
		int msk = 0;
		if (machine().input().code_pressed(KEYCODE_T))  msk |= 1;
		if (machine().input().code_pressed(KEYCODE_Y))  msk |= 2;
		if (machine().input().code_pressed(KEYCODE_U))  msk |= 4;
		if (machine().input().code_pressed(KEYCODE_I))  msk |= 8;
		if (machine().input().code_pressed(KEYCODE_O))  msk |= 16;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	if (layers_ctrl & 1)    m_bg1_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	else                bitmap.fill(m_palette->black_pen(), cliprect);
	if (layers_ctrl & 2)    m_bg2_tmap->draw(screen, bitmap, cliprect, 0, 0);
	if (layers_ctrl & 4)    m_bg3_tmap->draw(screen, bitmap, cliprect, 0, 0);
	if (layers_ctrl & 8)    m_bg4_tmap->draw(screen, bitmap, cliprect, 0, 0);

	if (layers_ctrl & 16)   m_fg_tmap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/***************************************************************************
                            Memory Maps
***************************************************************************/

void galaxi_state::show_out(  )
{
//  popmessage("%04x %04x %04x", m_out[0], m_out[1], m_out[2]);
}

WRITE16_MEMBER(galaxi_state::galaxi_500000_w)
{
	COMBINE_DATA(&m_out[0]);
	show_out();
}

WRITE16_MEMBER(galaxi_state::galaxi_500002_w)
{
	COMBINE_DATA(&m_out[1]);
	show_out();
}

WRITE16_MEMBER(galaxi_state::galaxi_500004_w)
{
	if (ACCESSING_BITS_0_7)
	{
	/*
	    - Lbits -
	    7654 3210
	    =========
	    ---- ---x  Hold1 lamp.
	    ---- --x-  Hold2 lamp.
	    ---- -x--  Hold3 lamp.
	    ---- x---  Hold4 lamp.
	    ---x ----  Hold5 lamp.
	    --x- ----  Start lamp.
	    -x-- ----  Payout.

	*/
		output().set_lamp_value(1, (data & 1));           /* Lamp 1 - HOLD 1 */
		output().set_lamp_value(2, (data >> 1) & 1);      /* Lamp 2 - HOLD 2 */
		output().set_lamp_value(3, (data >> 2) & 1);      /* Lamp 3 - HOLD 3 */
		output().set_lamp_value(4, (data >> 3) & 1);      /* Lamp 4 - HOLD 4 */
		output().set_lamp_value(5, (data >> 4) & 1);      /* Lamp 5 - HOLD 5 */
		output().set_lamp_value(6, (data >> 5) & 1);      /* Lamp 6 - START  */
	}
	if (ACCESSING_BITS_8_15)
	{
		m_ticket = data & 0x0100;
		m_hopper = data & 0x1000;
		machine().bookkeeping().coin_counter_w(0, data & 0x2000);    // coins
	}

	COMBINE_DATA(&m_out[2]);
	show_out();
}

CUSTOM_INPUT_MEMBER(galaxi_state::ticket_r)
{
	return m_ticket && !(m_screen->frame_number() % 10);
}

CUSTOM_INPUT_MEMBER(galaxi_state::hopper_r)
{
	return m_hopper && !(m_screen->frame_number() % 10);
}


static ADDRESS_MAP_START( galaxi_map, AS_PROGRAM, 16, galaxi_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM

	AM_RANGE(0x100000, 0x1003ff) AM_RAM_WRITE(galaxi_bg1_w) AM_SHARE("bg1_ram")
	AM_RANGE(0x100400, 0x1007ff) AM_RAM_WRITE(galaxi_bg2_w) AM_SHARE("bg2_ram")
	AM_RANGE(0x100800, 0x100bff) AM_RAM_WRITE(galaxi_bg3_w) AM_SHARE("bg3_ram")
	AM_RANGE(0x100c00, 0x100fff) AM_RAM_WRITE(galaxi_bg4_w) AM_SHARE("bg4_ram")

	AM_RANGE(0x101000, 0x101fff) AM_RAM_WRITE(galaxi_fg_w ) AM_SHARE("fg_ram")
	AM_RANGE(0x102000, 0x1047ff) AM_READNOP // unknown

	AM_RANGE(0x300000, 0x3007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("INPUTS")
	AM_RANGE(0x500000, 0x500001) AM_WRITE(galaxi_500000_w)
	AM_RANGE(0x500002, 0x500003) AM_WRITE(galaxi_500002_w)
	AM_RANGE(0x500004, 0x500005) AM_WRITE(galaxi_500004_w)

	AM_RANGE(0x700000, 0x700001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)

	AM_RANGE(0x600000, 0x607fff) AM_RAM AM_SHARE("nvram")   // 2x DS1230Y (non volatile SRAM)
ADDRESS_MAP_END

/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( galaxi )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxi_state,hopper_r, (void *)nullptr )   // hopper sensor
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(5)   // coin a
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(5)   // coin b (token)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_COIN3 )   // pin 25LC
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxi_state,ticket_r, (void *)nullptr )  // ticket sensor
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL ) // hopper out (pin 14LS)
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_HIGH )    // test
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SPECIAL ) // (pin 26LC)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL ) // (pin 15LS)
INPUT_PORTS_END

static INPUT_PORTS_START( magjoker )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL )PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxi_state,hopper_r, (void *)nullptr )   // hopper sensor
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(5)   // coin a
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(5)   // coin b (token)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Hopper Refill") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxi_state,ticket_r, (void *)nullptr )  // ticket sensor
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL ) // hopper out (pin 14LS)
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_HIGH )    // test
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT )   // (pin 26LC)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL ) // (pin 15LS)
INPUT_PORTS_END


/***************************************************************************
                               Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x8x4 =
{
	8, 8,
	0x1000, // 0x1000 tiles are accessible
	4,
	{ STEP4(0,1) },
	{ STEP4(4*4,4), STEP4(0,4) },
	{ STEP8(0,4*8) },
	8*8*4
};

static const gfx_layout layout_16x16x4 =
{
	16, 16,
	0x1000, // 0x1000 tiles are accessible
	4,
	{ STEP4(0,1) },
	{ STEP4(4*4,4), STEP4(0,4), STEP4(4*4+8*16*4,4), STEP4(0+8*16*4,4) },
	{ STEP16(0,4*8) },
	16*16*4
};

static GFXDECODE_START( galaxi )
	GFXDECODE_ENTRY( "gfx1", 0x00000, layout_16x16x4, 0, 0x400/0x10 )
	GFXDECODE_ENTRY( "gfx1", 0x80000, layout_8x8x4,   0, 0x400/0x10 )
GFXDECODE_END


/***************************************************************************
                              Machine Drivers
***************************************************************************/

void galaxi_state::machine_start()
{
	save_item(NAME(m_hopper));
	save_item(NAME(m_ticket));
	save_item(NAME(m_out));
}

void galaxi_state::machine_reset()
{
	m_hopper = 0;
	m_ticket = 0;
	m_out[0] = 0;
	m_out[1] = 0;
	m_out[2] = 0;
}

static MACHINE_CONFIG_START( galaxi, galaxi_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz) // ?
	MCFG_CPU_PROGRAM_MAP(galaxi_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaxi_state,  irq4_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(16*5, 512-16*2-1, 16*1, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(galaxi_state, screen_update_galaxi)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", galaxi)
	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_16MHz/16, OKIM6295_PIN7_LOW)  // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( magjoker, galaxi )

	/* basic machine hardware */

	/* sound hardware */
	MCFG_SOUND_MODIFY("oki")

	/* ADPCM samples are recorded with extremely low volume */
	MCFG_SOUND_ROUTES_RESET()
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 4.0)
MACHINE_CONFIG_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( galaxi )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "5.u48", 0x00000, 0x20000, CRC(53d86ed0) SHA1(d04ad4c79b0ae46d3d5820b16481ea95c1370e6d) )
	ROM_LOAD16_BYTE( "4.u47", 0x00001, 0x20000, CRC(ddd67683) SHA1(68f8969949e1db90a765c1f31cb8957eef505d5f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "3.u34", 0x00000, 0x80000, CRC(4a59ad63) SHA1(34fc1a948fc205f8c55a8e99d143bbdf4d1b220f) )
	ROM_LOAD16_BYTE( "2.u33", 0x00001, 0x80000, CRC(a8b29a97) SHA1(835c6885d5adf0e7600810ad9fcda88c22077495) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "1.u38", 0x00000, 0x40000, CRC(50e289db) SHA1(43c576c014f4c3d22bfa4c932e161d7558d483f6) )
ROM_END

ROM_START( magjoker )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "25.u48", 0x00000, 0x20000, CRC(505bdef2) SHA1(9c2a525f2eb3cc39bdd6219bad7c5a1a8bc0b274) )
	ROM_LOAD16_BYTE( "24.u47", 0x00001, 0x20000, CRC(380fd0cd) SHA1(bcd6d23e41e249c7e587b253958eec180440639a) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "23.u34", 0x00000, 0x80000, CRC(952b7c84) SHA1(a28e1b79444331837ffc07c8d3c16c1d9a3c974c) )
	ROM_LOAD16_BYTE( "22.u33", 0x00001, 0x80000, CRC(41866733) SHA1(257d77f89fcf1e8f36fb6a8fcb8ad48b1127e457) )

	ROM_REGION( 0x40000, "oki", 0 ) /* 4-bit ADPCM mono @ 6 kHz.*/
	ROM_LOAD( "21.u38", 0x00000, 0x40000, CRC(199baf33) SHA1(006708d955481fe1ae44555d27896d18e1ff8440) )
ROM_END


/***************************************************************************
                               Game Drivers
***************************************************************************/

/*     YEAR  NAME      PARENT  MACHINE   INPUT     INIT  ROT    COMPANY   FULLNAME                       FLAGS                   LAYOUT  */
GAMEL( 2000, galaxi,   0,      galaxi,   galaxi, driver_device,   0,    ROT0,  "B.R.L.", "Galaxi (v2.0)",               MACHINE_SUPPORTS_SAVE,     layout_galaxi )
GAMEL( 2000, magjoker, 0,      magjoker, magjoker, driver_device, 0,    ROT0,  "B.R.L.", "Magic Joker (v1.25.10.2000)", MACHINE_SUPPORTS_SAVE,     layout_galaxi )
