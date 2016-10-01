// license:BSD-3-Clause
// copyright-holders:MetalliC
/*************************************************************************

    Istrebiteli preliminary driver by MetalliC

**************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"

#define I8080_TAG   "maincpu"

class istrebiteli_state : public driver_device
{
public:
	istrebiteli_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, I8080_TAG)
		, m_ppi0(*this, "ppi0")
		, m_ppi1(*this, "ppi1")
		, m_tileram(*this, "tileram")
		, m_gfxdecode(*this, "gfxdecode")
	{
	}

	DECLARE_PALETTE_INIT(istrebiteli);
	DECLARE_READ8_MEMBER(ppi0_r);
	DECLARE_WRITE8_MEMBER(ppi0_w);
	DECLARE_READ8_MEMBER(ppi1_r);
	DECLARE_WRITE8_MEMBER(ppi1_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(spr0_ctrl_w);
	DECLARE_WRITE8_MEMBER(spr1_ctrl_w);
	DECLARE_WRITE8_MEMBER(spr_xy_w);
	DECLARE_CUSTOM_INPUT_MEMBER(coin_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inc);

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi0;
	required_device<i8255_device> m_ppi1;
	required_shared_ptr<UINT8> m_tileram;
	required_device<gfxdecode_device> m_gfxdecode;

	virtual void machine_start() override { }
	virtual void machine_reset() override;
	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_tile_info);
	tilemap_t *m_tilemap;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	UINT8 m_spr0_ctrl;
	UINT8 m_spr1_ctrl;
	UINT8 m_spr_xy[8];
	UINT8 coin_count;
};

void istrebiteli_state::machine_reset()
{
	m_spr0_ctrl = m_spr1_ctrl = 0;
	coin_count = 0;
	memset(m_spr_xy, 0, sizeof(m_spr_xy));
}

static const rgb_t istreb_palette[4] = {
	rgb_t(0x00, 0x00, 0x00),
	rgb_t(0x00, 0x00, 0xff),
	rgb_t(0xff, 0xff, 0xff),
	rgb_t(0x00, 0x00, 0xff)
};

PALETTE_INIT_MEMBER(istrebiteli_state,istrebiteli)
{
	palette.set_pen_colors(0, istreb_palette, ARRAY_LENGTH(istreb_palette));
}

TILE_GET_INFO_MEMBER(istrebiteli_state::get_tile_info)
{
	SET_TILE_INFO_MEMBER(0, m_tileram[tile_index], 0, 0);
}

void istrebiteli_state::video_start()
{
	UINT8 *gfx = memregion("sprite")->base();
	UINT8 temp[64];

	for (int offs = 0; offs < 0x200; offs += 0x40)
	{
		memset(&temp[0], 0, 64);
		for (int spnum = 0; spnum < 8; spnum++)
			for (int dot = 0; dot < 64; dot++)
				temp[(dot >> 3) + spnum * 8] |= ((gfx[offs + dot] >> spnum) & 1) << (dot & 7);
		memcpy(&gfx[offs], &temp[0], 64);
	}

	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(istrebiteli_state::get_tile_info), this), TILEMAP_SCAN_ROWS,
		16, 8, 1, 16);
}

UINT32 istrebiteli_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(1);

	m_tilemap->mark_all_dirty();
	rectangle myrect;
	myrect.set_size(16, 16*8);
	m_tilemap->draw(screen, bitmap, myrect, 0, 0);

	int spritecode;

	spritecode = (m_spr0_ctrl & 0x1f) + ((m_spr0_ctrl & 0x80) >> 2);
	m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, spritecode, 0, 0, 0, m_spr_xy[6], m_spr_xy[7] ^ 0xff, 1);
	spritecode = (m_spr1_ctrl & 0x1f) + ((m_spr1_ctrl & 0x80) >> 2);
	m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,	spritecode, 0, 0, 0, m_spr_xy[4], m_spr_xy[5] ^ 0xff, 1);

	m_gfxdecode->gfx(3)->transpen(bitmap, cliprect, (m_spr0_ctrl & 0x40) ? 5:7, 0, 0, 0, m_spr_xy[2], m_spr_xy[3] ^ 0xff, 1);
	m_gfxdecode->gfx(4)->transpen(bitmap, cliprect, (m_spr1_ctrl & 0x40) ? 5:7, 0, 0, 0, m_spr_xy[0], m_spr_xy[1] ^ 0xff, 1);

	return 0;
}

READ8_MEMBER(istrebiteli_state::ppi0_r)
{
	return m_ppi0->read(space, offset ^ 3) ^ 0xff;
}
WRITE8_MEMBER(istrebiteli_state::ppi0_w)
{
	m_ppi0->write(space, offset ^ 3, data ^ 0xff);
}
READ8_MEMBER(istrebiteli_state::ppi1_r)
{
	return m_ppi1->read(space, offset ^ 3) ^ 0xff;
}
WRITE8_MEMBER(istrebiteli_state::ppi1_w)
{
	m_ppi1->write(space, offset ^ 3, data ^ 0xff);
}

WRITE8_MEMBER(istrebiteli_state::sound_w)
{
	if (data & 1)
		coin_count = 0;
	// bits 1-7 sound control, TODO
}

WRITE8_MEMBER(istrebiteli_state::spr0_ctrl_w)
{
	m_spr0_ctrl = data;
}

WRITE8_MEMBER(istrebiteli_state::spr1_ctrl_w)
{
	m_spr1_ctrl = data;
}

WRITE8_MEMBER(istrebiteli_state::spr_xy_w)
{
	m_spr_xy[offset] = data;
}

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, istrebiteli_state)
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0x13ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, istrebiteli_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xb0, 0xbf) AM_RAM AM_SHARE("tileram")
	AM_RANGE(0xc0, 0xc3) AM_READWRITE(ppi0_r, ppi0_w)
	AM_RANGE(0xc4, 0xc7) AM_READWRITE(ppi1_r, ppi1_w)
	AM_RANGE(0xc8, 0xcf) AM_WRITE(spr_xy_w)
ADDRESS_MAP_END

CUSTOM_INPUT_MEMBER(istrebiteli_state::coin_r)
{
	return coin_count;
}

INPUT_CHANGED_MEMBER(istrebiteli_state::coin_inc)
{
	if (oldval == 0 && newval == 1)
		++coin_count;
}

static INPUT_PORTS_START( istreb )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // sprite collision flag
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // sprite collision flag
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START2)
	PORT_BIT(0x3c, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, istrebiteli_state, coin_r, nullptr)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_HBLANK("screen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_VBLANK("screen")

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, istrebiteli_state,coin_inc, nullptr)
INPUT_PORTS_END

static const gfx_layout char_layout =
{
	16,8,
	32,
	1,
	{ 0 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	{ 0,1,2,3,4,5,6,7 },
	16*8
};

static const gfx_layout sprite_layout =
{
	8,8,
	64,
	1,
	{ 0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 7,6,5,4,3,2,1,0 },
	8*8
};

static const gfx_layout projectile_layout =
{
	16,16,
	8,
	1,
	{ 0 },
	{ 15*8,14*8,13*8,12*8,11*8,10*8,9*8,8*8,7*8,6*8,5*8,4*8,3*8,2*8,1*8,0*8 },
	{ 15*128,14*128,13*128,12*128,11*128,10*128,9*128,8*128,7*128,6*128,5*128,4*128,3*128,2*128,1*128,0*128 },
	1
};

static GFXDECODE_START( istrebiteli )
	GFXDECODE_ENTRY( "chars", 0x0000, char_layout, 0, 2 )
	GFXDECODE_ENTRY( "sprite", 0x0000, sprite_layout, 2, 2 )
	GFXDECODE_ENTRY( "sprite", 0x0000, sprite_layout, 0, 2 )
	GFXDECODE_ENTRY( "sprite", 0x0200, projectile_layout, 2, 2 )
	GFXDECODE_ENTRY( "sprite", 0x0200, projectile_layout, 0, 2 )
GFXDECODE_END

static MACHINE_CONFIG_START( istreb, istrebiteli_state)
	/* basic machine hardware */
	MCFG_CPU_ADD(I8080_TAG, I8080, 8000000 / 4)		// KR580VM80A
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_DEVICE_ADD("ppi0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN0"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(istrebiteli_state, sound_w))

	MCFG_DEVICE_ADD("ppi1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(istrebiteli_state, spr0_ctrl_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(istrebiteli_state, spr1_ctrl_w))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(4000000, 256, 0, 256, 312, 0, 192)
	MCFG_SCREEN_UPDATE_DRIVER(istrebiteli_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", istrebiteli)
	MCFG_PALETTE_ADD("palette", 4)
	MCFG_PALETTE_INIT_OWNER(istrebiteli_state, istrebiteli)
MACHINE_CONFIG_END

ROM_START( istreb )
	ROM_REGION( 0x1000, I8080_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "main.bin", 0x000, 0xa00, CRC(ae67c41c) SHA1(1f7807d486cd4d161ee49be991b81db7dc9b0f3b) )	// actually 5x 512B ROMs, TODO split

	ROM_REGION(0x1000, "chars", ROMREGION_ERASEFF)
	ROM_LOAD("003-g8.bin", 0x000, 0x200, CRC(5cd7ad47) SHA1(2142711c8a3640b7aa258a2059cfb0f14297a5ac) )

	ROM_REGION(0x1000, "sprite", 0)
	ROM_LOAD("001-g4.bin",  0x000, 0x200, CRC(ca3c531b) SHA1(8295167895d51e626b6d5946b565d5e8b8466ac0) )
	ROM_LOAD("001-g9.bin",  0x000, 0x200, CRC(ca3c531b) SHA1(8295167895d51e626b6d5946b565d5e8b8466ac0) )
	ROM_LOAD("001-a11.bin", 0x200, 0x100, CRC(4e05b7dd) SHA1(335e975ae9e8f775c1ac07f60420680ad878c3ae) )
	ROM_LOAD("001-b11.bin", 0x200, 0x100, CRC(4e05b7dd) SHA1(335e975ae9e8f775c1ac07f60420680ad878c3ae) )

	ROM_REGION(0x200, "soundrom", 0)
	ROM_LOAD("003-w3.bin", 0x000, 0x200, CRC(54eb4893) SHA1(c7a4724045c645ab728074ed7fef1882d9776005) )
ROM_END

GAME( 198?, istreb,  0,        istreb,  istreb,  driver_device,  0, ROT90, "Terminal", "Istrebiteli", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
