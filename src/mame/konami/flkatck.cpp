// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Flak Attack / MX5000 (Konami GX669)

    Driver by:
        Manuel Abadia <emumanu+mame@gmail.com>

NOTE: A USA version of Flak Attack is known to exist  - currently not dumped

24MHz & 3.579545MHz OSCs

***************************************************************************/

#include "emu.h"

#include "konamipt.h"
#include "k007452.h"
#include "k007121.h"

#include "cpu/m6809/hd6309.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/k007232.h"
#include "sound/ymopm.h"

#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class flkatck_state : public driver_device
{
public:
	flkatck_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_spriteram(*this, "spriteram"),
		m_mainbank(*this, "mainbank"),
		m_coin(*this, "COIN"),
		m_pl(*this, "P%u", 1U),
		m_dsw(*this, "DSW%u", 1U),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007121(*this, "k007121"),
		m_k007232(*this, "k007232"),
		m_watchdog(*this, "watchdog"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void flkatck(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank m_mainbank;

	// video-related
	tilemap_t *m_k007121_tilemap[2];
	uint8_t m_flipscreen;

	// misc
	required_ioport m_coin;
	required_ioport_array<2> m_pl;
	required_ioport_array<3> m_dsw;
	uint8_t m_irq_enabled;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007121_device> m_k007121;
	required_device<k007232_device> m_k007232;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<generic_latch_8_device> m_soundlatch;

	void bankswitch_w(uint8_t data);
	uint8_t ls138_r(offs_t offset);
	void ls138_w(offs_t offset, uint8_t data);
	void vram_w(offs_t offset, uint8_t data);
	void k007121_regs_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info_a);
	TILE_GET_INFO_MEMBER(get_tile_info_b);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	void volume_callback(uint8_t data);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the K007121

***************************************************************************/

TILE_GET_INFO_MEMBER(flkatck_state::get_tile_info_a)
{
	uint8_t ctrl_0 = m_k007121->ctrlram_r(0);
	uint8_t ctrl_2 = m_k007121->ctrlram_r(2);
	uint8_t ctrl_3 = m_k007121->ctrlram_r(3);
	uint8_t ctrl_4 = m_k007121->ctrlram_r(4);
	uint8_t ctrl_5 = m_k007121->ctrlram_r(5);
	int attr = m_vram[tile_index];
	int code = m_vram[tile_index + 0x400];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0 + 2)) & 0x02) |
			((attr >> (bit1 + 1)) & 0x04) |
			((attr >> (bit2    )) & 0x08) |
			((attr >> (bit3 - 1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	if ((attr == 0x0d) && (!ctrl_0) && (!ctrl_2))
		bank = 0;   /*  this allows the game to print text
		            in all banks selected by the k007121 */

	tileinfo.set(0,
			code + 256 * bank,
			(attr & 0x0f) + 16,
			(attr & 0x20) ? TILE_FLIPY : 0);
}

TILE_GET_INFO_MEMBER(flkatck_state::get_tile_info_b)
{
	int attr = m_vram[tile_index + 0x800];
	int code = m_vram[tile_index + 0xc00];

	tileinfo.set(0,
			code,
			(attr & 0x0f) + 16,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void flkatck_state::video_start()
{
	m_k007121_tilemap[0] = &machine().tilemap().create(*m_k007121, tilemap_get_info_delegate(*this, FUNC(flkatck_state::get_tile_info_a)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_k007121_tilemap[1] = &machine().tilemap().create(*m_k007121, tilemap_get_info_delegate(*this, FUNC(flkatck_state::get_tile_info_b)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void flkatck_state::vram_w(offs_t offset, uint8_t data)
{
	m_vram[offset] = data;
	if (offset & 0x800) // score
		m_k007121_tilemap[1]->mark_tile_dirty(offset & 0x3ff);
	else
		m_k007121_tilemap[0]->mark_tile_dirty(offset & 0x3ff);
}

void flkatck_state::k007121_regs_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x04:  // ROM bank select
			if (data != m_k007121->ctrlram_r(4))
				machine().tilemap().mark_all_dirty();
			break;

		case 0x07:  // flip screen + IRQ control
			m_flipscreen = data & 0x08;
			machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			m_irq_enabled = data & 0x02;
			break;
	}

	m_k007121->ctrl_w(offset, data);
}


/***************************************************************************

    Display Refresh

***************************************************************************/

uint32_t flkatck_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip[2];
	const rectangle &visarea = screen.visible_area();
	// TODO: reversed polarity? Hard to say, FWIW Combat School uses this in reverse ...
	uint16_t sprite_buffer = (m_k007121->ctrlram_r(3) & 8) * 0x100;

	if (m_flipscreen)
	{
		clip[0] = visarea;
		clip[0].max_x -= 40;

		clip[1] = visarea;
		clip[1].min_x = clip[1].max_x - 40;

		m_k007121_tilemap[0]->set_scrollx(0, m_k007121->ctrlram_r(0) - 56 );
		m_k007121_tilemap[0]->set_scrolly(0, m_k007121->ctrlram_r(2));
		m_k007121_tilemap[1]->set_scrollx(0, -16);
	}
	else
	{
		clip[0] = visarea;
		clip[0].min_x += 40;

		clip[1] = visarea;
		clip[1].max_x = 39;
		clip[1].min_x = 0;

		m_k007121_tilemap[0]->set_scrollx(0, m_k007121->ctrlram_r(0) - 40 );
		m_k007121_tilemap[0]->set_scrolly(0, m_k007121->ctrlram_r(2));
		m_k007121_tilemap[1]->set_scrollx(0, 0);
	}

	// compute clipping
	clip[0] &= cliprect;
	clip[1] &= cliprect;

	// draw the graphics
	m_k007121_tilemap[0]->draw(screen, bitmap, clip[0], 0, 0);
	m_k007121->sprites_draw(bitmap, cliprect, &m_spriteram[sprite_buffer], 0, 40, 0, screen.priority(), (uint32_t)-1, true);
	m_k007121_tilemap[1]->draw(screen, bitmap, clip[1], 0, 0);
	return 0;
}


// machine
INTERRUPT_GEN_MEMBER(flkatck_state::interrupt)
{
	if (m_irq_enabled)
		device.execute().set_input_line(HD6309_IRQ_LINE, HOLD_LINE);
}

void flkatck_state::bankswitch_w(uint8_t data)
{
	// bits 3-4: coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x08);
	machine().bookkeeping().coin_counter_w(1, data & 0x10);

	// bits 0-1: bank #
	if ((data & 0x03) != 0x03)  // for safety
		m_mainbank->set_entry(data & 0x03);
}

uint8_t flkatck_state::ls138_r(offs_t offset)
{
	int data = 0;

	switch ((offset & 0x1c) >> 2)
	{
		case 0x00:
			if (offset & 0x02)
				data = (offset & 0x01) ? m_coin->read() : m_dsw[2]->read();
			else
				data = (offset & 0x01) ? m_pl[1]->read() : m_pl[0]->read();
			break;
		case 0x01:
			if (offset & 0x02)
				data = (offset & 0x01) ? m_dsw[0]->read() : m_dsw[1]->read();
			break;
	}

	return data;
}

void flkatck_state::ls138_w(offs_t offset, uint8_t data)
{
	switch ((offset & 0x1c) >> 2)
	{
		case 0x04:  // bankswitch
			bankswitch_w(data);
			break;
		case 0x05:  // sound code number
			m_soundlatch->write(data);
			break;
		case 0x06:  // cause interrupt on audio CPU
			m_audiocpu->set_input_line(0, HOLD_LINE);
			break;
		case 0x07:  // watchdog reset
			m_watchdog->watchdog_reset();
			break;
	}
}

void flkatck_state::main_map(address_map &map)
{
	map(0x0000, 0x0007).ram().w(FUNC(flkatck_state::k007121_regs_w));
	map(0x0008, 0x03ff).ram();
	map(0x0400, 0x041f).rw(FUNC(flkatck_state::ls138_r), FUNC(flkatck_state::ls138_w)); // inputs, DIPS, bankswitch, counters, sound command
	map(0x0800, 0x0bff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x1000, 0x1fff).ram().share(m_spriteram);
	map(0x2000, 0x2fff).ram().w(FUNC(flkatck_state::vram_w)).share(m_vram); // 007121
	map(0x3000, 0x3fff).ram();
	map(0x4000, 0x5fff).bankr(m_mainbank);
	map(0x6000, 0xffff).rom().region("maincpu", 0x6000);
}

void flkatck_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9007).rw("k007452", FUNC(k007452_device::read), FUNC(k007452_device::write));    // Protection (see wecleman, but unused here?)
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xb000, 0xb00d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
}


static INPUT_PORTS_START( flkatck )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	// "Invalid" = both coin slots disabled

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30K, Every 70K" )
	PORT_DIPSETTING(    0x10, "40K, Every 80K" )
	PORT_DIPSETTING(    0x08, "30K Only" )
	PORT_DIPSETTING(    0x00, "40K Only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC(   0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )        // Listed as "Unused"
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_B12_UNK(1)

	PORT_START("P2")
	KONAMI8_B12_UNK(2)
INPUT_PORTS_END

static GFXDECODE_START( gfx_flkatck )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x4_packed_msb, 0, 32 )
GFXDECODE_END

void flkatck_state::volume_callback(uint8_t data)
{
	m_k007232->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void flkatck_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	m_mainbank->configure_entries(0, 3, &ROM[0x0000], 0x2000);

	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_flipscreen));
}

void flkatck_state::machine_reset()
{
	m_k007232->set_bank(0, 1);

	m_irq_enabled = 0;
	m_flipscreen = 0;
}

void flkatck_state::flkatck(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, 24_MHz_XTAL / 8); // HD63C09EP, 3MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &flkatck_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(flkatck_state::interrupt));

	Z80(config, m_audiocpu, 3.579545_MHz_XTAL); // NEC D780C-1
	m_audiocpu->set_addrmap(AS_PROGRAM, &flkatck_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	WATCHDOG_TIMER(config, m_watchdog);

	KONAMI_007452_MATH(config, "k007452");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(37*8, 32*8);
	screen.set_visarea(0*8, 35*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(flkatck_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 512).set_endianness(ENDIANNESS_LITTLE);

	K007121(config, m_k007121, 0, "palette", gfx_flkatck);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM2151(config, "ymsnd", 3.579545_MHz_XTAL).add_route(0, "lspeaker", 1.0).add_route(0, "rspeaker", 1.0);

	K007232(config, m_k007232, 3.579545_MHz_XTAL);
	m_k007232->port_write().set(FUNC(flkatck_state::volume_callback));
	m_k007232->add_route(0, "lspeaker", 0.50);
	m_k007232->add_route(0, "rspeaker", 0.50);
	m_k007232->add_route(1, "lspeaker", 0.50);
	m_k007232->add_route(1, "rspeaker", 0.50);
}



ROM_START( mx5000 )
	ROM_REGION( 0x10000, "maincpu", 0 )  // 6309 code
	ROM_LOAD( "669_r01.16c", 0x00000, 0x10000, CRC(79b226fc) SHA1(3bc4d93717230fecd54bd08a0c3eeedc1c8f571d) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "669_m02.16b", 0x0000, 0x8000, CRC(7e11e6b9) SHA1(7a7d65a458b15842a6345388007c8f682aec20a7) )

	ROM_REGION( 0x80000, "gfx", 0 )  // tiles + sprites
	ROM_LOAD16_WORD_SWAP( "gx669f03.5e", 0x00000, 0x80000, CRC(ff1d718b) SHA1(d44fe3ed5a3ba1b3036264e37f9cd3500b706635) ) // MASK4M

	ROM_REGION( 0x40000, "k007232", 0 )
	ROM_LOAD( "gx669f04.11a", 0x00000, 0x40000, CRC(6d1ea61c) SHA1(9e6eb9ac61838df6e1f74e74bb72f3edf1274aed) ) // MASK2M
ROM_END

ROM_START( flkatck )
	ROM_REGION( 0x10000, "maincpu", 0 )  // 6309 code
	ROM_LOAD( "669_p01.16c", 0x00000, 0x10000, CRC(c5cd2807) SHA1(22ddd911a23954ff2d52552e07323f5f0ddaeead) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "669_m02.16b", 0x0000, 0x8000, CRC(7e11e6b9) SHA1(7a7d65a458b15842a6345388007c8f682aec20a7) )

	ROM_REGION( 0x80000, "gfx", 0 )  // tiles + sprites
	ROM_LOAD16_WORD_SWAP( "gx669f03.5e", 0x00000, 0x80000, CRC(ff1d718b) SHA1(d44fe3ed5a3ba1b3036264e37f9cd3500b706635) ) // MASK4M

	ROM_REGION( 0x40000, "k007232", 0 )
	ROM_LOAD( "gx669f04.11a", 0x00000, 0x40000, CRC(6d1ea61c) SHA1(9e6eb9ac61838df6e1f74e74bb72f3edf1274aed) ) // MASK2M
ROM_END

// identical to flkatck except for the board / ROM type configuration
ROM_START( flkatcka )
	ROM_REGION( 0x10000, "maincpu", 0 )  // 6309 code
	ROM_LOAD( "669_p01.16c", 0x00000, 0x10000, CRC(c5cd2807) SHA1(22ddd911a23954ff2d52552e07323f5f0ddaeead) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "669_m02.16b", 0x0000, 0x8000, CRC(7e11e6b9) SHA1(7a7d65a458b15842a6345388007c8f682aec20a7) )

	ROM_REGION( 0x80000, "gfx", 0 )  // tiles + sprites, same data as above set, on PWB 450593 sub-board instead.
	ROM_LOAD16_BYTE( "669_f03a.4b", 0x00000, 0x10000, CRC(f0ed4c1e) SHA1(58efe3cd81054d22de54a7d195aa3b865bde4a01) )
	ROM_LOAD16_BYTE( "669_f03e.4d", 0x00001, 0x10000, CRC(95a57a26) SHA1(c8aa30c2c734c0740630b1b04ae43c69931cc7c1) )
	ROM_LOAD16_BYTE( "669_f03b.5b", 0x20000, 0x10000, CRC(e2593f3c) SHA1(aa0f6d04015650eaef17c4a39f228eaccf9a2948) )
	ROM_LOAD16_BYTE( "669_f03f.5d", 0x20001, 0x10000, CRC(c6c9903e) SHA1(432ad6d03992499cc533273226944a666b40fa58) )
	ROM_LOAD16_BYTE( "669_f03c.6b", 0x40000, 0x10000, CRC(47be92dd) SHA1(9ccc62d7d42fccbd5ad60e35e3a0478a04405cf1) )
	ROM_LOAD16_BYTE( "669_f03g.6d", 0x40001, 0x10000, CRC(70d35fbd) SHA1(21384f738684c5da4a7a84a1c9aa173fffddf47a) )
	ROM_LOAD16_BYTE( "669_f03d.7b", 0x60000, 0x10000, CRC(18d48f9e) SHA1(b95e38aa813e0f3a0dc6bd45fdb4bf71f7e2066c) )
	ROM_LOAD16_BYTE( "669_f03h.7d", 0x60001, 0x10000, CRC(abfe76e7) SHA1(f8661f189308e83056ec442fa6c936efff67ba0a) )

	ROM_REGION( 0x40000, "k007232", 0 )
	ROM_LOAD( "gx669f04.11a", 0x00000, 0x40000, CRC(6d1ea61c) SHA1(9e6eb9ac61838df6e1f74e74bb72f3edf1274aed) ) // MASK2M
ROM_END

} // anonymous namespace


GAME( 1987, mx5000,   0,      flkatck, flkatck, flkatck_state, empty_init, ROT90, "Konami", "MX5000", MACHINE_SUPPORTS_SAVE )
GAME( 1987, flkatck,  mx5000, flkatck, flkatck, flkatck_state, empty_init, ROT90, "Konami", "Flak Attack (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, flkatcka, mx5000, flkatck, flkatck, flkatck_state, empty_init, ROT90, "Konami", "Flak Attack (Japan, PWB 450593 sub-board)", MACHINE_SUPPORTS_SAVE )
