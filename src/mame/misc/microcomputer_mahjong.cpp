// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class microcomputer_mahjong_state : public driver_device
{
public:
	microcomputer_mahjong_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_palette(*this, "palette"),
		m_beeper(*this, "beeper"),
		m_fgram(*this, "fgram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_controls(*this, "IN%u", 0U)
	{ }

	void mimahjng(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<palette_device> m_palette;
	required_device<beep_device> m_beeper;
	required_shared_ptr<uint8_t> m_fgram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_ioport_array<4> m_controls;

	tilemap_t *m_fg_tilemap = nullptr;

	uint8_t key_read();
	void key_select_w(uint8_t data);

	void sound_write(uint8_t data);

	void fgram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map);

	uint8_t m_key_select;
};

TILE_GET_INFO_MEMBER(microcomputer_mahjong_state::get_fg_tile_info)
{
	tileinfo.set(0,m_fgram[tile_index],0,0);
}

void microcomputer_mahjong_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(microcomputer_mahjong_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void microcomputer_mahjong_state::fgram_w(offs_t offset, uint8_t data)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

uint32_t microcomputer_mahjong_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint8_t microcomputer_mahjong_state::key_read()
{
	uint8_t ret = 0xff;

	if (!(m_key_select & 0x08)) ret &= m_controls[0]->read();
	if (!(m_key_select & 0x04)) ret &= m_controls[1]->read();
	if (!(m_key_select & 0x02)) ret &= m_controls[2]->read();
	if (!(m_key_select & 0x01)) ret &= m_controls[3]->read();

	return ret;
}

void microcomputer_mahjong_state::sound_write(uint8_t data)
{
	m_beeper->set_state(data & 1);
}

void microcomputer_mahjong_state::key_select_w(uint8_t data)
{
	logerror("%s: key_select_w %02x\n", machine().describe_context(), data);
	m_key_select = data;
}

void microcomputer_mahjong_state::prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x5000, 0x53ff).ram();
	map(0x6000, 0x63ff).ram().w(FUNC(microcomputer_mahjong_state::fgram_w)).share(m_fgram);
	map(0x7001, 0x7001).r(FUNC(microcomputer_mahjong_state::key_read));
	map(0x7002, 0x7002).w(FUNC(microcomputer_mahjong_state::key_select_w));
	map(0x7004, 0x7004).w(FUNC(microcomputer_mahjong_state::sound_write));
}

static INPUT_PORTS_START( mimahjng )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static GFXDECODE_START( gfx_mimahjng )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x1, 0, 16 )
GFXDECODE_END

void microcomputer_mahjong_state::machine_start()
{
	save_item(NAME(m_key_select));
}

void microcomputer_mahjong_state::machine_reset()
{
	m_key_select = 0x00;
}

void microcomputer_mahjong_state::mimahjng(machine_config &config)
{
	Z80(config, m_maincpu, 11059200 / 4); // ? MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &microcomputer_mahjong_state::prg_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0,224-1, 0, 224-1);
	screen.set_screen_update(FUNC(microcomputer_mahjong_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_mimahjng);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 8000).add_route(ALL_OUTPUTS, "mono", 1.00);
}

ROM_START( mimahjng )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "ms-1", 0x0000, 0x1000, CRC(a1607ac8) SHA1(4e0d7a0482c7619ef25b12a7e02f5d03bea8ce6f) )
	ROM_LOAD( "ms-2", 0x1000, 0x1000, CRC(cb1cab68) SHA1(88dc4808126528a269edf742062fa12e902be324) )
	ROM_LOAD( "ms-3", 0x2000, 0x1000, CRC(2fdd4f55) SHA1(a9246239144c41fd38bd42015552b5afab40e55a) )
	ROM_LOAD( "ms-4", 0x3000, 0x1000, CRC(cc550e36) SHA1(d66750ce6ddf6e4db4e5bd46a639494d8335a590) )

	ROM_REGION( 0x800, "tiles", 0 )
	ROM_LOAD( "ms-a", 0x000, 0x800, CRC(d1dfe5c1) SHA1(5042b89555867db418f4aeef6b520619d8f533f2) )
ROM_END

} // anonymous namespace

CONS( 198?, mimahjng,  0,    0,       mimahjng, mimahjng,  microcomputer_mahjong_state, empty_init, "<unknown>", "Microcomputer Mahjong", MACHINE_NOT_WORKING )


