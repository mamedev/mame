// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Master Boy, Z80 hardware version

this is Gaelco's first game, although there should be a 1986 release too, likely on the same hardware

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"
#include "machine/nvram.h"

class mastboyo_state : public driver_device
{
public:
	mastboyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_fgram2(*this, "fgram2"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_bank1(*this, "bank1"),
		m_questionrom(*this, "questions"),
		m_paletterom(*this, "palette")
	{ }

	void mastboyo(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	tilemap_t *m_fg_tilemap;

	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_fgram2;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_memory_bank m_bank1;
	required_region_ptr<uint8_t> m_questionrom;
	required_region_ptr<uint8_t> m_paletterom;

	void mastboyo_map(address_map &map);
	void mastboyo_portmap(address_map &map);

	DECLARE_WRITE8_MEMBER(fgram_w);
	DECLARE_WRITE8_MEMBER(fgram2_w);
	DECLARE_WRITE8_MEMBER(rombank_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update_mastboyo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(mastboyo);
};

TILE_GET_INFO_MEMBER(mastboyo_state::get_fg_tile_info)
{
	int code = m_fgram[tile_index];
	int attr = m_fgram2[tile_index];
	SET_TILE_INFO_MEMBER(0,
			code,
			attr&0xf,
			0);
}

void mastboyo_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(mastboyo_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

WRITE8_MEMBER(mastboyo_state::fgram_w)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mastboyo_state::fgram2_w)
{
	m_fgram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mastboyo_state::rombank_w)
{
	m_bank1->set_entry(data&0xf);
}

uint32_t mastboyo_state::screen_update_mastboyo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void mastboyo_state::mastboyo_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("aysnd", FUNC(ym2149_device::address_data_w));
	map(0x00, 0x00).r("aysnd", FUNC(ym2149_device::data_r));
}


void mastboyo_state::mastboyo_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram().share("nvram");
	map(0x5000, 0x53ff).ram().w(this, FUNC(mastboyo_state::fgram_w)).share("fgram");
	map(0x5400, 0x57ff).ram().w(this, FUNC(mastboyo_state::fgram2_w)).share("fgram2");
	map(0x6000, 0x6000).w(this, FUNC(mastboyo_state::rombank_w));
//  map(0x7000, 0x7000).portr("UNK"); // possible watchdog? or IRQ ack?
	map(0x8000, 0xffff).bankr("bank1");
}


static INPUT_PORTS_START( mastboyo )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, "Disabled" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	// note, Player 2 buttons must be used when entering name, and can be used for answering questions even in single player mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Red / Enter Initial")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Green / Delete Initial")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Red / <<")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Green / >>")
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 24, 28, 0, 4, 8, 12, 16, 20,}, // note, slightly strange order
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( gfx_mastboyo )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

void mastboyo_state::machine_start()
{
	m_bank1->configure_entries(0, 0x10, &m_questionrom[0x00000], 0x08000);
	m_bank1->set_entry(1);
}

PALETTE_INIT_MEMBER(mastboyo_state, mastboyo)
{
	for (int i = 0; i < palette.entries(); i++)
	{
		const int pal = (m_paletterom[i]&0x0f) | ((m_paletterom[palette.entries() + i]&0x0f) << 4);
		const int green = (pal & 0x07);
		const int red = (pal & 0x38) >> 3;
		const int blue = (pal & 0xc0) >> 6;

		palette.set_pen_color(i, pal3bit(red), pal3bit(green), pal2bit(blue));
	}
}

MACHINE_CONFIG_START(mastboyo_state::mastboyo)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, 20_MHz_XTAL/6)
	MCFG_DEVICE_PROGRAM_MAP(mastboyo_map)
	MCFG_DEVICE_IO_MAP(mastboyo_portmap)
	MCFG_DEVICE_PERIODIC_INT_DRIVER(mastboyo_state, irq0_line_hold, 256.244f) // not sure, INT0 pin was measured at 256.244Hz

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 2*8, 256-2*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(mastboyo_state, screen_update_mastboyo)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("gfxdecode", GFXDECODE, "palette", gfx_mastboyo)

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(mastboyo_state, mastboyo)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("aysnd", AY8910, 20_MHz_XTAL/4)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN0")) // DSW
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("IN1")) // player inputs
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_CONFIG_END

ROM_START( mastboyo )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "masterboy-1987-27128-ic14.bin", 0x00000, 0x04000, CRC(d05a22eb) SHA1(528eb27622d28d098ceefc6a6fb43bb1a527444d) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "masterboy-1987-27128-mbfij-ic36.bin", 0x00000, 0x04000, CRC(aa2a174d) SHA1(a27eae023453877131646594100b575af7a3c473) )

	ROM_REGION( 0x80000, "questions", ROMREGION_ERASEFF )
	/* IC6 - IC9 empty */
	ROM_LOAD( "masterboy-1987-27c512-ic10.bin", 0x40000, 0x10000, CRC(66da2826) SHA1(ef86efbfa251fc7cf62fe99b26551203fd599294) )
	ROM_LOAD( "masterboy-1987-27256-ic11.bin",  0x50000, 0x08000, CRC(40b07eeb) SHA1(93f62e0a2a330f7ff1041eaa7cba3ef42121b1a8) )
	ROM_LOAD( "masterboy-1987-27c512-ic12.bin", 0x60000, 0x10000, CRC(b2819e38) SHA1(e4dd036747221926d0f9f86e00e28f578decc942) )
	ROM_LOAD( "masterboy-1987-27c512-ic13.bin", 0x70000, 0x10000, CRC(71df82e9) SHA1(a0d75ec1181094e39d2246805e4dac5a621daa1a) )

	ROM_REGION( 0x100, "proms", 0 ) // timing or memory mapping?
	ROM_LOAD( "masterboy-1987-82s129-d-ic23.bin", 0x000, 0x100, CRC(d5fd2dfd) SHA1(66e3afa9e73507db0647d125c0be992b27d08adc) )

	ROM_REGION( 0x200, "palette", 0 )
	ROM_LOAD( "masterboy-1987-82s129-h-ic39.bin", 0x100, 0x100, CRC(8e965fc3) SHA1(b52c8e505438937c7a5d3e1393d54f0ad0425e78) )
	ROM_LOAD( "masterboy-1987-82s129-l-ic40.bin", 0x000, 0x100, CRC(4d061216) SHA1(1abf9320da75a3fd23c6bdbcc4088d18e133c4e5) )
ROM_END

GAME( 1987, mastboyo, 0, mastboyo, mastboyo, mastboyo_state, empty_init, ROT0, "Gaelco (Covielsa license)", "Master Boy (1987, Z80 hardware)", MACHINE_SUPPORTS_SAVE )
