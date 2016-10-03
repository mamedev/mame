// license:BSD-3-Clause
// copyright-holders:MetalliC
/*************************************************************************

    Istrebiteli driver by MetalliC

    TODO:
      sound emulation
      check sprite priorities
      accurate sprite collision

	how to play:
      insert one or more coins, each coin gives 2 minutes of play time, then press 1 or 2 player game start
	  hit enemy 15 or more times to get bonus game
	
	test mode:
	  insert 12 or more coins then press 2 player start

**************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"

#define I8080_TAG   "maincpu"

/////////////////////////////////////////////////////////////

class istrebiteli_sound_device : public device_t,
	public device_sound_interface
{
public:
	istrebiteli_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void sound_w(UINT8 data);
protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
private:
	// internal state
	sound_stream *m_channel;
	UINT8 *m_rom;
	int m_rom_cnt;
	int m_rom_incr;
	int m_sample_num;
	bool m_cnt_reset;
	bool m_rom_out_en;
	UINT8 m_prev_data;
};

extern const device_type ISTREBITELI_SOUND;

//////////////////////////////////////////////////////////////

const device_type ISTREBITELI_SOUND = &device_creator<istrebiteli_sound_device>;

istrebiteli_sound_device::istrebiteli_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ISTREBITELI_SOUND, "Istrebiteli Sound", tag, owner, clock, "istrebiteli_sound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_channel(nullptr),
		m_rom(nullptr),
		m_rom_cnt(0),
		m_rom_incr(0),
		m_sample_num(0),
		m_cnt_reset(true),
		m_rom_out_en(false),
		m_prev_data(0)
{
}

void istrebiteli_sound_device::device_start()
{
	m_channel = stream_alloc(0, 1, clock() / 2);
	m_rom = machine().root_device().memregion("soundrom")->base();
}

void istrebiteli_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *sample = outputs[0];

	while (samples-- > 0)
	{
		int smpl = 0;
		if (m_rom_out_en)
			smpl = (m_rom[m_rom_cnt] >> m_sample_num) & 1;

		// below is huge guess
		if ((m_prev_data & 0x40) == 0)				// b6 noice enable ?
			smpl &= rand() & 1;	
		smpl *= (m_prev_data & 0x80) ? 1000 : 4000; // b7 volume ?

		*sample++ = smpl;
		m_rom_cnt = (m_rom_cnt + m_rom_incr) & 0x1ff;
	}
}

void istrebiteli_sound_device::sound_w(UINT8 data)
{
	m_cnt_reset = (data & 2) ? true : false;
	m_sample_num = (data >> 2) & 7;
	m_rom_out_en = (data & 0x20) ? false : true;

	if (m_cnt_reset)
	{
		m_rom_cnt = 0;
		m_rom_incr = 0;
	}
	else
		m_rom_incr = 1;
//	if (m_prev_data != data)
//		printf("sound %02X rescnt %d sample %d outen %d b6 %d b7 %d\n", data, (data >> 1) & 1, (data >> 2) & 7, (data >> 5) & 1, (data >> 6) & 1, (data >> 7) & 1);
	m_prev_data = data;
}

//////////////////////////////////////////////////////////////

class istrebiteli_state : public driver_device
{
public:
	istrebiteli_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, I8080_TAG)
		, m_ppi0(*this, "ppi0")
		, m_ppi1(*this, "ppi1")
		, m_gfxdecode(*this, "gfxdecode")
		, m_sound_dev(*this, "custom")
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
	DECLARE_WRITE8_MEMBER(tileram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(collision_r);
	DECLARE_CUSTOM_INPUT_MEMBER(coin_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inc);

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi0;
	required_device<i8255_device> m_ppi1;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<istrebiteli_sound_device> m_sound_dev;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_tile_info);
	tilemap_t *m_tilemap;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	UINT8 m_spr0_ctrl;
	UINT8 m_spr1_ctrl;
	UINT8 coin_count;
	UINT8 m_spr_xy[8];
	UINT8 m_tileram[16];
};

void istrebiteli_state::machine_start()
{
	save_item(NAME(m_spr0_ctrl));
	save_item(NAME(m_spr1_ctrl));
	save_item(NAME(coin_count));
	save_item(NAME(m_spr_xy));
	save_item(NAME(m_tileram));
}

void istrebiteli_state::machine_reset()
{
	m_spr0_ctrl = m_spr1_ctrl = 0;
	coin_count = 0;
	memset(m_spr_xy, 0, sizeof(m_spr_xy));
	memset(m_tileram, 0, sizeof(m_tileram));
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
	SET_TILE_INFO_MEMBER(0, m_tileram[tile_index] & 0x1f, 0, 0);
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

	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(istrebiteli_state::get_tile_info), this), TILEMAP_SCAN_ROWS_FLIP_X,
		8, 16, 16, 1);
	m_tilemap->set_scrolldx(96, 96);
}

UINT32 istrebiteli_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(1);

	rectangle rect = cliprect;
	rect.offset(32, 64);
	rect.set_size(16*8, 16);
	m_tilemap->draw(screen, bitmap, rect, 0, 0);

	int spritecode;

	spritecode = (m_spr0_ctrl & 0x1f) + ((m_spr0_ctrl & 0x80) >> 2);
	m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, spritecode, 0, 0, 0, m_spr_xy[7], m_spr_xy[6], 1);
	spritecode = (m_spr1_ctrl & 0x1f) + ((m_spr1_ctrl & 0x80) >> 2);
	m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,	spritecode, 0, 0, 0, m_spr_xy[5], m_spr_xy[4], 1);

	m_gfxdecode->gfx(3)->transpen(bitmap, cliprect, (m_spr0_ctrl & 0x40) ? 5:7, 0, 0, 0, m_spr_xy[3], m_spr_xy[2], 1);
	m_gfxdecode->gfx(3)->transpen(bitmap, cliprect, (m_spr1_ctrl & 0x40) ? 4:6, 0, 0, 0, m_spr_xy[1], m_spr_xy[0], 1);

	return 0;
}

WRITE8_MEMBER(istrebiteli_state::tileram_w)
{
	m_tileram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
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
	machine().bookkeeping().coin_lockout_w(0, data & 1);
	if (data & 1)
		coin_count = 0;
	m_sound_dev->sound_w(data);
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
	AM_RANGE(0xb0, 0xbf) AM_WRITE(tileram_w)
	AM_RANGE(0xc0, 0xc3) AM_READWRITE(ppi0_r, ppi0_w)
	AM_RANGE(0xc4, 0xc7) AM_READWRITE(ppi1_r, ppi1_w)
	AM_RANGE(0xc8, 0xcf) AM_WRITE(spr_xy_w)
ADDRESS_MAP_END

CUSTOM_INPUT_MEMBER(istrebiteli_state::collision_r)
{
	// piece of HACK
	// real hardware does per-pixel sprite collision detection
	int id = *(int*)&param * 2;

	int sx = m_spr_xy[5 + id];
	int sy = m_spr_xy[4 + id];
	int px = m_spr_xy[3 - id] + 3;
	int py = m_spr_xy[2 - id] + 3;

	if (sx < 56)
		return 0;

	if (px >= sx && px < (sx + 8) && py >= sy && py < (sy + 8))
		return 1;

	return 0;
}

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
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, istrebiteli_state, collision_r, 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, istrebiteli_state, collision_r, 1)
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
	8,16,
	32,
	1,
	{ 0 },
	{ 7,6,5,4,3,2,1,0 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	8*16
};

static const gfx_layout sprite_layout =
{
	8,8,
	64,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 7*8,6*8,5*8,4*8,3*8,2*8,1*8,0*8 },
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
	GFXDECODE_ENTRY( "sprite", 0x0200, projectile_layout, 0, 2 )
GFXDECODE_END

static MACHINE_CONFIG_START( istreb, istrebiteli_state)
	/* basic machine hardware */
	MCFG_CPU_ADD(I8080_TAG, I8080, XTAL_8MHz / 4)		// KR580VM80A
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
	MCFG_SCREEN_RAW_PARAMS(XTAL_8MHz / 2, 256, 64, 256, 312, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(istrebiteli_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", istrebiteli)
	MCFG_PALETTE_ADD("palette", 4)
	MCFG_PALETTE_INIT_OWNER(istrebiteli_state, istrebiteli)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("custom", ISTREBITELI_SOUND, XTAL_8MHz / 2 / 256)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

ROM_START( istreb )
	ROM_REGION( 0x1000, I8080_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "main.bin", 0x000, 0xa00, CRC(ae67c41c) SHA1(1f7807d486cd4d161ee49be991b81db7dc9b0f3b) )	// actually 5x 512B ROMs, TODO split

	ROM_REGION( 0x200, "chars", 0 )
	ROM_LOAD( "003-g8.bin", 0x000, 0x200, CRC(5cd7ad47) SHA1(2142711c8a3640b7aa258a2059cfb0f14297a5ac) )

	ROM_REGION( 0x1000, "sprite", 0 )
	ROM_LOAD( "001-g4.bin",  0x000, 0x200, CRC(ca3c531b) SHA1(8295167895d51e626b6d5946b565d5e8b8466ac0) )
	ROM_LOAD( "001-g9.bin",  0x000, 0x200, CRC(ca3c531b) SHA1(8295167895d51e626b6d5946b565d5e8b8466ac0) )
	ROM_LOAD( "001-a11.bin", 0x200, 0x100, CRC(4e05b7dd) SHA1(335e975ae9e8f775c1ac07f60420680ad878c3ae) )
	ROM_LOAD( "001-b11.bin", 0x200, 0x100, CRC(4e05b7dd) SHA1(335e975ae9e8f775c1ac07f60420680ad878c3ae) )

	ROM_REGION(0x200, "soundrom", 0)
	ROM_LOAD( "003-w3.bin", 0x000, 0x200, CRC(54eb4893) SHA1(c7a4724045c645ab728074ed7fef1882d9776005) )
ROM_END

GAME( 198?, istreb,  0,        istreb,  istreb,  driver_device,  0, ROT0, "Terminal", "Istrebiteli", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
