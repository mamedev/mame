// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli
/**************************************************************************************************

Puzzle Time (Prototype)
Elettronica Video-Games S.R.L, 199?

dump and info provided by Yoshi

To initialize the eeprom keep Service button pressed at boot.

TODO:
- Text tilemap blinking could be a bit slower / faster
- Brightness effect could be a bit darker / lighter
- Ticket test in service mode keeps outputting tickets once enabled, and flips statuses between
  "IN SEDE" (online) and "FUORI SEDE" (offline). Is it intended behaviour? Verify in-game.

**************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class pzletime_state : public driver_device
{
public:
	pzletime_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
		, m_eeprom(*this, "eeprom")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette%u", 0U)
		, m_ticket(*this, "ticket")

		, m_video_regs(*this, "video_regs")
		, m_tilemap_regs(*this, "tilemap_regs")
		, m_bg_videoram(*this, "bg_videoram")
		, m_mid_videoram(*this, "mid_videoram")
		, m_txt_videoram(*this, "txt_videoram")
		, m_spriteram(*this, "spriteram")
	{ }

	void pzletime(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device_array<palette_device, 2> m_palette;
	required_device<ticket_dispenser_device> m_ticket;

	required_shared_ptr<uint16_t> m_video_regs;
	required_shared_ptr<uint16_t> m_tilemap_regs;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_mid_videoram;
	required_shared_ptr<uint16_t> m_txt_videoram;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_mid_tilemap = nullptr;
	tilemap_t *m_txt_tilemap = nullptr;

	void main_map(address_map &map) ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void mid_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void txt_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void video_regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void oki_bank_w(uint16_t data);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_txt_tile_info);
	void palette_init(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


TILE_GET_INFO_MEMBER(pzletime_state::get_mid_tile_info)
{
	int tileno = m_mid_videoram[tile_index] & 0x0fff;
	int colour = m_mid_videoram[tile_index] & 0xf000;
	colour = colour >> 12;
	tileinfo.set(2, tileno, colour, 0);
}

TILE_GET_INFO_MEMBER(pzletime_state::get_txt_tile_info)
{
	int tileno = m_txt_videoram[tile_index] & 0x0fff;
	int colour = m_txt_videoram[tile_index] & 0xf000;
	colour = colour >> 12;

	tileinfo.set(0, tileno, colour, 0);

	tileinfo.category = BIT(colour, 3);
}

void pzletime_state::video_start()
{
	m_mid_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pzletime_state::get_mid_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 64, 16);
	m_txt_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pzletime_state::get_txt_tile_info)), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);

	m_mid_tilemap->set_transparent_pen(0);
	m_txt_tilemap->set_transparent_pen(0);
}

uint32_t pzletime_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette[0]->pen(0), cliprect);

	// TODO: [3] bits 0-2 may actually control tilemap enable, per layer
	m_txt_tilemap->set_scrolly(0, m_tilemap_regs[0] - 3);
	m_txt_tilemap->set_scrollx(0, m_tilemap_regs[1]);

	m_mid_tilemap->set_scrolly(0, m_tilemap_regs[2] - 3);
	m_mid_tilemap->set_scrollx(0, m_tilemap_regs[3] - 7);

	if (m_video_regs[2] & 1)
	{
		int count = 0;

		for (int y = 255; y >= 0; y--)
		{
			for (int x = 0; x < 512; x++)
			{
				const uint16_t pen_dot = m_bg_videoram[count];
				const int res_y = (y - 18) & 0xff;
				const int res_x = (x - 32) & 0x1ff;
				if (pen_dot & 0x8000 && cliprect.contains(res_x, res_y))
					bitmap.pix(res_y, res_x) = m_palette[1]->pen(pen_dot & 0x7fff);

				count++;
			}
		}
	}

	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	uint16_t const *const spriteram = m_spriteram;
	for (int offs = 0; offs < 0x2000 / 2; offs += 4)
	{
		if(spriteram[offs + 0] == 8)
			break;

		int spr_offs = spriteram[offs + 3] & 0x0fff;
		int sy = 0x200 - (spriteram[offs + 0] & 0x1ff) - 35;
		int sx = (spriteram[offs + 1] & 0x1ff) - 30;
		int colour = (spriteram[offs + 0] & 0xf000) >> 12;

		// is spriteram[offs + 0] & 0x200 flipy? it's always set

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, spr_offs, colour, 0, 1, sx, sy, 0);
	}

	m_txt_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	// TODO: unconfirmed blink ratio
	if ((screen.frame_number() & 8) != 0)
		m_txt_tilemap->draw(screen, bitmap, cliprect, 1, 0);

	return 0;
}

void pzletime_state::mid_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mid_videoram[offset]);
	m_mid_tilemap->mark_tile_dirty(offset);
}

void pzletime_state::txt_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_txt_videoram[offset]);
	m_txt_tilemap->mark_tile_dirty(offset);
}

void pzletime_state::video_regs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_regs[offset]);

	if (offset < 2 && m_video_regs[offset] > 0)
	{
		double contrast = 32768.0 / static_cast<double>(m_video_regs[offset]);

		for (unsigned i = 0; i < m_palette[offset]->entries(); i++)
			m_palette[offset]->set_pen_contrast(i, contrast);
	}
}

void pzletime_state::oki_bank_w(uint16_t data)
{
	m_oki->set_rom_bank(data & 0x3);
}

void pzletime_state::main_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x700000, 0x700005).ram().w(FUNC(pzletime_state::video_regs_w)).share("video_regs");
	map(0x800001, 0x800001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x900000, 0x9005ff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette0");
	map(0xa00000, 0xa00007).ram().share("tilemap_regs");
	map(0xb00000, 0xb3ffff).ram().share("bg_videoram");
	map(0xc00000, 0xc00fff).ram().w(FUNC(pzletime_state::mid_videoram_w)).share("mid_videoram");
	map(0xc01000, 0xc01fff).ram().w(FUNC(pzletime_state::txt_videoram_w)).share("txt_videoram");
	map(0xd00000, 0xd01fff).ram().share("spriteram");
	map(0xe00000, 0xe00001).portr("INPUT").portw("EEPROMOUT");
	map(0xe00002, 0xe00003).portr("SYSTEM").portw("TICKETOUT");
	map(0xe00004, 0xe00005).w(FUNC(pzletime_state::oki_bank_w));
	map(0xf00000, 0xf0ffff).ram();
}


static INPUT_PORTS_START( pzletime )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	// NOTE: was (m_ticket && !(m_screen->frame_number() % 128));
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("EEPROMOUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)

	PORT_START("TICKETOUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, motor_w)
INPUT_PORTS_END


static GFXDECODE_START( gfx_pzletime )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_lsb,               0x100, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_col_2x2_group_packed_lsb, 0x200, 0x10 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x4_col_2x2_group_packed_lsb, 0x000, 0x10 )
GFXDECODE_END

void pzletime_state::machine_start()
{
}

void pzletime_state::machine_reset()
{
}

void pzletime_state::pzletime(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pzletime_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(pzletime_state::irq4_line_hold));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 48*8-1, 0*8, 28*8-1);
	m_screen->set_screen_update(FUNC(pzletime_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette[0], gfx_pzletime);

	PALETTE(config, m_palette[0]).set_format(palette_device::xRGB_555, 768);

	PALETTE(config, m_palette[1], palette_device::RGB_555);

	EEPROM_93C46_16BIT(config, "eeprom");
	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(2000));

	SPEAKER(config, "mono").front_center();
	OKIM6295(config, m_oki, 937500, okim6295_device::PIN7_HIGH); //freq & pin7 taken from stlforce
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pzletime )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "5.bin", 0x000000, 0x80000, CRC(78b027dc) SHA1(6719908a075ecf0666bb817ac8a31056a7f315c6) )
	ROM_LOAD16_BYTE( "1.bin", 0x000001, 0x80000, CRC(0a69cbc7) SHA1(bae8b5746209c6773da27acaec7bd535a69019d2) )
	ROM_LOAD16_BYTE( "6.bin", 0x100000, 0x80000, CRC(526733ef) SHA1(21a921416d1ae7b9d49789d70ae99f240b012489) )
	ROM_LOAD16_BYTE( "2.bin", 0x100001, 0x80000, CRC(2a877266) SHA1(b8e909b3bd21af71782c501fa6eef590045b81e0) )
	ROM_LOAD16_BYTE( "7.bin", 0x200000, 0x80000, CRC(2efdd6d3) SHA1(de35d7a1bcd3ad608b8dfc184e06d6719253a1c7) )
	ROM_LOAD16_BYTE( "3.bin", 0x200001, 0x80000, CRC(1ddacade) SHA1(78f09fdb541e369765abfdf39607ca8f4c771d16) )
	ROM_LOAD16_BYTE( "8.bin", 0x300000, 0x80000, CRC(be7cf043) SHA1(5dadafb6f89f2fc373b77b18746b461117228f08) )
	ROM_LOAD16_BYTE( "4.bin", 0x300001, 0x80000, CRC(374ab900) SHA1(bd7f649bdf2927c1f5cb53492a08cc66c4658a72) )

	ROM_REGION( 0x80000, "user1", 0 ) /* Samples */
	ROM_LOAD( "12.bin",  0x00000, 0x80000,  CRC(203897c1) SHA1(c2495871c796bc7f2dabca1630317313b5aa740a) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_COPY( "user1", 0x000000, 0x000000, 0x020000 )
	ROM_COPY( "user1", 0x000000, 0x020000, 0x020000 )
	ROM_COPY( "user1", 0x000000, 0x040000, 0x020000 )
	ROM_COPY( "user1", 0x020000, 0x060000, 0x020000 )
	ROM_COPY( "user1", 0x000000, 0x080000, 0x020000 )
	ROM_COPY( "user1", 0x040000, 0x0a0000, 0x020000 )
	ROM_COPY( "user1", 0x000000, 0x0c0000, 0x020000 )
	ROM_COPY( "user1", 0x060000, 0x0e0000, 0x020000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "10.bin",  0x00000, 0x80000, CRC(d6ed11a5) SHA1(585aad4e962e7c9ba33e96d4d53e2feddd1a6cd9) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "11.bin",  0x000000, 0x80000, CRC(566e09a3) SHA1(b04d23bd82c609f35e6b651006b5c029f36f54dc) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "9.bin",   0x000000, 0x80000, CRC(a8144a7e) SHA1(9dfdd6c17a91cad6b56c622671042ac2ee2c9ec8) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "pzletime.nv", 0x0000, 0x0080, CRC(e5ed3d40) SHA1(8c163a6e5839e5c82d52f046d3268202fdf9f4d1) )
ROM_END

} // anonymous namespace


GAME( 199?, pzletime, 0, pzletime,  pzletime, pzletime_state, empty_init, ROT0, "Elettronica Video-Games", "Puzzle Time (prototype)", MACHINE_SUPPORTS_SAVE )
