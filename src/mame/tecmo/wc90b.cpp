// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*
World Cup 90 bootleg driver
---------------------------

Ernesto Corvi
(ernesto@imagina.com)

CPU #1 : Handles background & foreground tiles, controllers, dipswitches.
CPU #2 : Handles sprites and palette
CPU #3 : Audio

Memory Layout:

CPU #1
0000-8000 ROM
8000-9000 RAM
a000-a800 Color Ram for background #1 tiles
a800-b000 Video Ram for background #1 tiles
c000-c800 Color Ram for background #2 tiles
c800-c000 Video Ram for background #2 tiles
e000-e800 Color Ram for foreground tiles
e800-f000 Video Ram for foreground tiles
f800-fc00 Common Ram with CPU #2
fd00-fd00 Stick 1, Coin 1 & Start 1 input port
fd02-fd02 Stick 2, Coin 2 & Start 2 input port
fd06-fc06 Dip Switch A
fd08-fc08 Dip Switch B

CPU #2
0000-c000 ROM
c000-d000 RAM
d000-d800 RAM Sprite Ram
e000-e800 RAM Palette Ram
f800-fc00 Common Ram with CPU #1

CPU #3
0000-0xc000 ROM
???????????

Notes:
-----
The bootleg video hardware is quite different from the original machine.
I could not figure out the encoding of the scrolling for the new
video hardware. The memory positions, in case anyone wants to try, are
the following ( CPU #1 memory addresses ):
fd06: scroll bg #1 X coordinate
fd04: scroll bg #1 Y coordinate
fd08: scroll bg #2 X coordinate
fd0a: scroll bg #2 Y coordinate
fd0e: ????

What I used instead, was the local copy kept in RAM. These values
are the ones the original machine uses. This will differ when trying
to use some of this code to write a driver for a similar Tecmo bootleg.

Sprites are also very different. There's a code snippet in the ROM
that converts the original sprites to the new format, which only allows
16x16 sprites. That snippet also does some ( nasty ) clipping.

Colors are accurate. The graphics ROMs have been modified severely
and encoded in a different way from the original machine. Even if
sometimes it seems colors are not entirely correct, this is only due
to the crappy artwork of the person that did the bootleg.

Dip switches are not complete and they don't seem to differ from
the original machine.

Last but not least, the set of ROMs I have for Euro League seem to have
the sprites corrupted. The game seems to be exactly the same as the
World Cup 90 bootleg.

Noted added by ClawGrip 28-Mar-2008:
-----------------------------------
-Dumped and added all the PCB GALs.
-Removed the second YM2203, Ernesto said it wasn't present on his board,
 and also isn't on mine.
-My PCB has a different ROM (a05.bin), but only two bytes are different.
 Dox suggested that it can be just a year or text mod, so I decided not
 to include my set. If anyone wants it, please mail me:
 clawgrip at hotmail dot com. I can't find any graphical difference
 between my set and the one already on MAME.

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class wc90b_state : public driver_device
{
public:
	wc90b_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_spriteram(*this, "spriteram"),
		m_mainbank(*this, "mainbank"),
		m_subbank(*this, "subbank"),
		m_audiobank(*this, "audiobank"),
		m_scrollx(*this, "scrollx%u", 1U),
		m_scrolly(*this, "scrolly%u", 1U),
		m_scroll_x_lo(*this, "scroll_x_lo")
	{ }

	void wc90b(machine_config &config);
	void eurogael(machine_config &config);

	void init_wc90b();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_txvideoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank m_mainbank;
	required_memory_bank m_subbank;
	required_memory_bank m_audiobank;

	void bgvideoram_w(offs_t offset, uint8_t data);
	void fgvideoram_w(offs_t offset, uint8_t data);
	void txvideoram_w(offs_t offset, uint8_t data);
	void bankswitch_w(uint8_t data);
	void adpcm_int(int state);

	void sound_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);

private:
	optional_shared_ptr_array<uint8_t, 2> m_scrollx;
	optional_shared_ptr_array<uint8_t, 2> m_scrolly;
	optional_shared_ptr<uint8_t> m_scroll_x_lo;

	void main_map(address_map &map) ATTR_COLD;

	uint8_t m_msm5205next;
	uint8_t m_toggle;

	void sub_bankswitch_w(uint8_t data);
	void adpcm_data_w(uint8_t data);
	void adpcm_control_w(uint8_t data);
	uint8_t master_irq_ack_r();
	void slave_irq_ack_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
};


class eurogael_state : public wc90b_state
{
public:
	eurogael_state(const machine_config &mconfig, device_type type, const char *tag) :
		wc90b_state(mconfig, type, tag),
		m_bgscroll(*this, "bgscroll")
	{ }

	void eurogael(machine_config &config);

protected:
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	virtual void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority) override;

private:
	void master_irq_ack_w(uint8_t data);
	required_shared_ptr<uint8_t> m_bgscroll;

	void main_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(wc90b_state::get_bg_tile_info)
{
	int attr = m_bgvideoram[tile_index];
	int tile = m_bgvideoram[tile_index + 0x800];
	tileinfo.set(1,
			 ((((attr & 3) + ((attr >> 1) & 4))) << 8) | tile | 0x800,
			(attr >> 4) | 0x10,
			0);
}

TILE_GET_INFO_MEMBER(wc90b_state::get_fg_tile_info)
{
	int attr = m_fgvideoram[tile_index];
	int tile = m_fgvideoram[tile_index + 0x800];
	tileinfo.set(1,
			((((attr & 3) + ((attr >> 1) & 4))) << 8) | tile,
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(wc90b_state::get_tx_tile_info)
{
	tileinfo.set(0,
			m_txvideoram[tile_index + 0x800] + ((m_txvideoram[tile_index] & 0x07) << 8),
			m_txvideoram[tile_index] >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void wc90b_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wc90b_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wc90b_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wc90b_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(15);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void wc90b_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void wc90b_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void wc90b_state::txvideoram_w(offs_t offset, uint8_t data)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x7ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void wc90b_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	// draw all visible sprites of specified priority
	for (int offs = m_spriteram.bytes() - 8 ; offs >= 0 ; offs -= 8)
	{
		if ((~(m_spriteram[offs + 3] >> 7 ) & 1) == priority)
		{

			// 0   bbbb bbff   b = tile lower , f = flip bits
			// 1   yyyy yyyy
			// 2   xxxx xxxx
			// 3   PXcc cccc   P = priority X = x high, c = tile upper
			// 4   pppp ----   palette

			int tilehigh = (m_spriteram[offs + 3] & 0x3f) << 6;
			int tilelow = m_spriteram[offs + 0];
			int flags = m_spriteram[offs + 4];

			tilehigh += (tilelow & 0xfc) >> 2;

			int sx = m_spriteram[offs + 2];
			if (!(m_spriteram[offs + 3] & 0x40)) sx -= 0x0100;

			int sy = 240 - m_spriteram[offs + 1];

			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, tilehigh,
					flags >> 4, // color
					tilelow & 1,   // flipx
					tilelow & 2,   // flipy
					sx,
					sy, 15);
		}
	}
}

uint32_t wc90b_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, 8 * (m_scrollx[1][0] & 0x7f) + 256 - 4 + (m_scroll_x_lo[0] & 0x07));
	m_bg_tilemap->set_scrolly(0, m_scrolly[1][0] + 1 + ((m_scrollx[1][0] & 0x80) ? 256 : 0));
	m_fg_tilemap->set_scrollx(0, 8 * (m_scrollx[0][0] & 0x7f) + 256 - 6 + ((m_scroll_x_lo[0] & 0x38) >> 3));
	m_fg_tilemap->set_scrolly(0, m_scrolly[0][0] + 1 + ((m_scrollx[0][0] & 0x80) ? 256 : 0));

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 1);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	// TODO: if scoring on same Y as GOAL message, ball will be above it. Might be a BTANB (or needs single pass draw + mix?)
	draw_sprites(bitmap, cliprect, 0);
	return 0;
}

void eurogael_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	// draw all visible sprites of specified priority

	// entry at start of RAM might not be a sprite
	for (int offs = 0x200 - 4 ; offs >= 4 ; offs -= 4)
	{
		if (((m_spriteram[offs + 3] >> 4) & 1) == priority)
		{
			// this is wrong

			// 0      bbbb bbbb   b = tile lower
			// 1      yyyy yyyy
			// 2      xxxx xxxx
			// 3      ffXP cccc   f = flip bits, P = priority (inverted vs. other bootlegs) X = X high?, c = tile upper
			// 0x200  ---- -ppp   p = palette

			int tilehigh = (m_spriteram[offs + 3] & 0x0f) << 8;
			int attr = (m_spriteram[offs + 3] & 0xf0) >> 4;

			int tilelow = m_spriteram[offs + 0];
			int flags = m_spriteram[offs + 0x200];

			tilehigh += tilelow;

			int sx = m_spriteram[offs + 2];
			if (!(attr & 0x02)) sx -= 0x0100;

			int sy = 240 - m_spriteram[offs + 1];

			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, tilehigh,
					(flags & 0x7) | 8, // color - palettes 0x0 - 0x7 never written?
					attr & 4,   // flipx
					attr & 8,   // flipy
					sx,
					sy, 15);
		}
	}
}

uint32_t eurogael_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// the code to write / clear tilemaps for fb and tx layers has been specifically modified to avoid writing to the last 4 bytes
	// and the game instead writes scroll values there instead, there is no code to copy from there, so it looks like these are the scroll regs

	// each of the 3 layer has its own PCB, all PCBs look identical, so why does handling differ slightly?

	int fg_scrollx = ((m_fgvideoram[0xffc]) | (m_fgvideoram[0xffd] << 8)) + 33;
	int fg_scrolly = ((m_fgvideoram[0xffe]) | (m_fgvideoram[0xfff] << 8)) + 1;
	int bg_scrollx = ((m_bgscroll[0xf00]) | (m_bgscroll[0xf01] << 8)) + 33;
	int bg_scrolly = ((m_bgscroll[0xf02]) | (m_bgscroll[0xf03] << 8)) + 1;
	int tx_scrollx = ((m_txvideoram[0xffc]) | (m_txvideoram[0xffd] << 8)) + 33;
	int tx_scrolly = ((m_txvideoram[0xffe]) | (m_txvideoram[0xfff] << 8)) + 1;

	m_bg_tilemap->set_scrollx(0, bg_scrollx);
	m_bg_tilemap->set_scrolly(0, bg_scrolly);
	m_fg_tilemap->set_scrollx(0, fg_scrollx);
	m_fg_tilemap->set_scrolly(0, fg_scrolly);
	m_tx_tilemap->set_scrollx(0, tx_scrollx);
	m_tx_tilemap->set_scrolly(0, tx_scrolly);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 1);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 0);
	return 0;
}


void wc90b_state::bankswitch_w(uint8_t data)
{
	m_mainbank->set_entry(data >> 3);
}

void wc90b_state::sub_bankswitch_w(uint8_t data)
{
	m_subbank->set_entry(data >> 3);
}

void wc90b_state::adpcm_control_w(uint8_t data)
{
	m_audiobank->set_entry(data & 0x01);
	m_msm->reset_w(data & 0x08);
}

void wc90b_state::adpcm_data_w(uint8_t data)
{
	m_msm5205next = data;
}

uint8_t wc90b_state::master_irq_ack_r()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0xff;
}

void wc90b_state::slave_irq_ack_w(uint8_t data)
{
	m_subcpu->set_input_line(0, CLEAR_LINE);
}


void wc90b_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram(); // Main RAM
	map(0xa000, 0xafff).ram().w(FUNC(wc90b_state::fgvideoram_w)).share(m_fgvideoram);
	map(0xc000, 0xcfff).ram().w(FUNC(wc90b_state::bgvideoram_w)).share(m_bgvideoram);
	map(0xe000, 0xefff).ram().w(FUNC(wc90b_state::txvideoram_w)).share(m_txvideoram);
	map(0xf000, 0xf7ff).bankr(m_mainbank);
	map(0xf800, 0xfbff).ram().share("main_sub");
	map(0xfc00, 0xfc00).w(FUNC(wc90b_state::bankswitch_w));
	map(0xfd00, 0xfd00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xfd04, 0xfd04).writeonly().share(m_scrolly[0]);
	map(0xfd06, 0xfd06).writeonly().share(m_scrollx[0]);
	map(0xfd08, 0xfd08).writeonly().share(m_scrolly[1]);
	map(0xfd0a, 0xfd0a).writeonly().share(m_scrollx[1]);
	map(0xfd0e, 0xfd0e).writeonly().share(m_scroll_x_lo);
	map(0xfd00, 0xfd00).portr("P1");
	map(0xfd02, 0xfd02).portr("P2");
	map(0xfd06, 0xfd06).portr("DSW1");
	map(0xfd08, 0xfd08).portr("DSW2");
	map(0xfd0c, 0xfd0c).r(FUNC(wc90b_state::master_irq_ack_r));
}

void wc90b_state::sub_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd7ff).ram().share(m_spriteram);
	map(0xd800, 0xdfff).ram();
	map(0xe000, 0xe7ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xe800, 0xefff).rom();
	map(0xf000, 0xf7ff).bankr(m_subbank);
	map(0xf800, 0xfbff).ram().share("main_sub");
	map(0xfc00, 0xfc00).w(FUNC(wc90b_state::sub_bankswitch_w));
	map(0xfd0c, 0xfd0c).w(FUNC(wc90b_state::slave_irq_ack_w));
}

void wc90b_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_audiobank);
	map(0xe000, 0xe000).w(FUNC(wc90b_state::adpcm_control_w));
	map(0xe400, 0xe400).w(FUNC(wc90b_state::adpcm_data_w));
	map(0xe800, 0xe801).rw("ymsnd1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xec00, 0xec01).rw("ymsnd2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r("soundlatch", FUNC(generic_latch_8_device::read));
}


void eurogael_state::master_irq_ack_w(uint8_t data)
{
	// this seems to be write based instead of read based
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

void eurogael_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram(); // Main RAM
	map(0xa000, 0xafff).ram().w(FUNC(eurogael_state::fgvideoram_w)).share(m_fgvideoram);
	map(0xc000, 0xcfff).ram().w(FUNC(eurogael_state::bgvideoram_w)).share(m_bgvideoram);
	map(0xd000, 0xdfff).ram().share(m_bgscroll); // there are a bunch of read / write accesses in here (is it meant to mirror the bgram? - bg scroll regs are at df00 - df03
	map(0xe000, 0xefff).ram().w(FUNC(eurogael_state::txvideoram_w)).share(m_txvideoram);
	map(0xf000, 0xf7ff).bankr(m_mainbank);
	map(0xf800, 0xfbff).ram().share("main_sub");
	map(0xfc00, 0xfc00).w(FUNC(eurogael_state::bankswitch_w));
	map(0xfd00, 0xfd00).portr("P1");
	map(0xfd02, 0xfd02).portr("P2");
	map(0xfd06, 0xfd06).portr("DSW1");
	map(0xfd08, 0xfd08).portr("DSW2");
	map(0xfd0c, 0xfd0c).w(FUNC(eurogael_state::master_irq_ack_w));
	map(0xfd0e, 0xfd0e).w("soundlatch", FUNC(generic_latch_8_device::write));
}



static INPUT_PORTS_START( wc90b )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, "Countdown Speed" )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )           // 60/60
	PORT_DIPSETTING(    0x00, "Fast" )                      // 56/60
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "1 Player Game Time" )
	PORT_DIPSETTING(    0x01, "1:00" )
	PORT_DIPSETTING(    0x02, "1:30" )
	PORT_DIPSETTING(    0x03, "2:00" )
	PORT_DIPSETTING(    0x00, "2:30" )
	PORT_DIPNAME( 0x1c, 0x1c, "2 Player Game Time" )
	PORT_DIPSETTING(    0x0c, "1:00" )
	PORT_DIPSETTING(    0x14, "1:30" )
	PORT_DIPSETTING(    0x04, "2:00" )
	PORT_DIPSETTING(    0x18, "2:30" )
	PORT_DIPSETTING(    0x1c, "3:00" )
	PORT_DIPSETTING(    0x08, "3:30" )
	PORT_DIPSETTING(    0x10, "4:00" )
	PORT_DIPSETTING(    0x00, "5:00" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	RGN_FRAC(1,4),   // 2048 characters
	4,  // 4 bits per pixel
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },    // the bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every char takes 8 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 32*32 characters
	RGN_FRAC(1,4),
	4,  // 4 bits per pixel
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) }, // the bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		(16*8)+0, (16*8)+1, (16*8)+2, (16*8)+3, (16*8)+4, (16*8)+5, (16*8)+6, (16*8)+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 8*8+1*8, 8*8+2*8, 8*8+3*8, 8*8+4*8, 8*8+5*8, 8*8+6*8, 8*8+7*8 },
	32*8    // every char takes 128 consecutive bytes
};

static GFXDECODE_START( gfx_wc90b )
	GFXDECODE_ENTRY( "chargfx",   0x00000, charlayout,   0x100, 0x10 )
	GFXDECODE_ENTRY( "tilegfx",   0x00000, spritelayout, 0x200, 0x20 )
	GFXDECODE_ENTRY( "spritegfx", 0x00000, spritelayout, 0x000, 0x10 )
GFXDECODE_END



void wc90b_state::adpcm_int(int state)
{
	m_toggle ^= 1;
	if(m_toggle)
	{
		m_msm->data_w((m_msm5205next & 0xf0) >> 4);
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
	else
		m_msm->data_w((m_msm5205next & 0x0f) >> 0);
}

void wc90b_state::machine_start()
{
	m_mainbank->configure_entries(0, 32, memregion("maincpu")->base() + 0x10000, 0x800);
	m_subbank->configure_entries(0, 32, memregion("sub")->base() + 0x10000, 0x800);
	m_audiobank->configure_entries(0, 2, memregion("audiocpu")->base() + 0x8000, 0x4000);

	save_item(NAME(m_msm5205next));
	save_item(NAME(m_toggle));
}


void wc90b_state::wc90b(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(14'318'181) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &wc90b_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(wc90b_state::irq0_line_assert));

	Z80(config, m_subcpu, XTAL(14'318'181) / 2);
	m_subcpu->set_addrmap(AS_PROGRAM, &wc90b_state::sub_map);
	m_subcpu->set_vblank_int("screen", FUNC(wc90b_state::irq0_line_assert));

	Z80(config, m_audiocpu, XTAL(20'000'000) / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &wc90b_state::sound_map);
	// IRQs are triggered by the main CPU

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(wc90b_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wc90b);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 1024).set_endianness(ENDIANNESS_BIG);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0, HOLD_LINE);

	YM2203(config, "ymsnd1", XTAL(20'000'000) / 16).add_route(ALL_OUTPUTS, "mono", 0.40);
	YM2203(config, "ymsnd2", XTAL(20'000'000) / 16).add_route(ALL_OUTPUTS, "mono", 0.40);

	MSM5205(config, m_msm, XTAL(384'000));
	m_msm->vck_legacy_callback().set(FUNC(wc90b_state::adpcm_int)); // interrupt function
	m_msm->set_prescaler_selector(msm5205_device::S96_4B);  // 4KHz 4-bit
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.20);
}

void eurogael_state::eurogael(machine_config &config)
{
	// main board XTAL near Z80s is 16Mhz here
	// sound board has 20Mhz and !6Mhz XTALs
	// DSWs are on the sound board near the YM2203Cs
	// use is guessed

	// basic machine hardware
	Z80(config, m_maincpu, XTAL(16'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &eurogael_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(eurogael_state::irq0_line_assert));

	Z80(config, m_subcpu, XTAL(16'000'000) / 2);
	m_subcpu->set_addrmap(AS_PROGRAM, &eurogael_state::sub_map);
	m_subcpu->set_vblank_int("screen", FUNC(eurogael_state::irq0_line_assert));

	Z80(config, m_audiocpu, XTAL(20'000'000) / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &eurogael_state::sound_map);
	// IRQs are triggered by the main CPU

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(eurogael_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wc90b);
	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 1024).set_endianness(ENDIANNESS_BIG);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0, HOLD_LINE);

	YM2203(config, "ymsnd1", XTAL(16'000'000) / 4).add_route(ALL_OUTPUTS, "mono", 0.40); // YM2203C
	YM2203(config, "ymsnd2", XTAL(16'000'000) / 4).add_route(ALL_OUTPUTS, "mono", 0.40); // YM2203C

	MSM5205(config, m_msm, XTAL(384'000));
	m_msm->vck_legacy_callback().set(FUNC(eurogael_state::adpcm_int)); // interrupt function
	m_msm->set_prescaler_selector(msm5205_device::S96_4B);  // 4KHz 4-bit
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.20);
}


// these were dumped from unprotected pal16l8 devices found on a twcup90b2 set, probably the same for all sets?
#define TWCUP90B_PLD_DEVICES \
	ROM_LOAD( "pal16l8.1",  0x0000, 0x0104, CRC(1f13c98f) SHA1(dbcac8a47bd6fe050a731132396b42dc35704dde) ) \
	ROM_LOAD( "pal16l8.2",  0x0200, 0x0104, CRC(54af6bf3) SHA1(9373250b501ec4a9cd4ef697da40b41c2411f046) ) \
	ROM_LOAD( "pal16l8.3",  0x0400, 0x0104, CRC(afbdd4fc) SHA1(7cdf1c0da8889749270016a6c906734c695f164a) ) \
	ROM_LOAD( "hy18cv8s.4", 0x0600, 0x0155, NO_DUMP )  /* protected peel18cv8, registered (also seen pal16r6 and gal16v8 used on other boards) */ \
	ROM_LOAD( "pal16l8.5",  0x0800, 0x0104, CRC(04cdf238) SHA1(213b4c94900cbd2647151f273f0d2706ebf354b7) )


ROM_START( twcup90b1 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "a02.bin", 0x00000, 0x10000, CRC(192a03dd) SHA1(ab98d370bba5437f956631b0199b173be55f1c27) )  // c000-ffff is not used
	ROM_LOAD( "a03.bin", 0x10000, 0x10000, CRC(f54ff17a) SHA1(a19850fc28a5a0da20795a5cc6b56d9c16554bce) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "a04.bin", 0x00000, 0x10000, CRC(3d535e2f) SHA1(f1e1878b5a8316e770c74a1e1f29a7a81a4e5dfe) )  // c000-ffff is not used
	ROM_LOAD( "a05.bin", 0x10000, 0x10000, CRC(9e421c4b) SHA1(e23a1f1d5d1e960696f45df653869712eb889839) )  // banked at f000-f7ff

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a01.bin", 0x00000, 0x10000, CRC(3d317622) SHA1(ae4e8c5247bc215a2769786cb8639bce2f80db22) )

	ROM_REGION( 0x010000, "chargfx", 0 )
	ROM_LOAD( "a06.bin", 0x000000, 0x04000, CRC(3b5387b7) SHA1(b839b4eafe8bf6f9e841e19fee1bdb64a66f3448) )
	ROM_LOAD( "a08.bin", 0x004000, 0x04000, CRC(c622a5a3) SHA1(468c8c24af1f6f244228b66df04cb0ea81c1875e) )
	ROM_LOAD( "a10.bin", 0x008000, 0x04000, CRC(0923d9f6) SHA1(4b10ee3fc17bb63cda51b2a978d066b6a140a551) )
	ROM_LOAD( "a20.bin", 0x00c000, 0x04000, CRC(b8dec83e) SHA1(fe617ddccdd0dbd05ca09a1507074aa14b529322) )

	ROM_REGION( 0x080000, "tilegfx", 0 )
	ROM_LOAD( "a07.bin", 0x000000, 0x20000, CRC(38c31817) SHA1(cb24ed8702d62066366924c033c07ffc78bd1fad) )
	ROM_LOAD( "a09.bin", 0x020000, 0x20000, CRC(32e39e29) SHA1(44f22ed6c983541c7fea5857ba0456aaa87b36d1) )
	ROM_LOAD( "a11.bin", 0x040000, 0x20000, CRC(5ccec796) SHA1(2cc191a4267819eb31962726e2ed4567c825c39e) )
	ROM_LOAD( "a21.bin", 0x060000, 0x20000, CRC(0c54a091) SHA1(3eecb285b5a7bbc310c87492516d7ffb2841aa3b) )

	ROM_REGION( 0x080000, "spritegfx", ROMREGION_INVERT )
	ROM_LOAD( "152_a18.bin", 0x000000, 0x10000, CRC(516b6c09) SHA1(9d02514dece864b087f67886009ce54bd51b5575) )
	ROM_LOAD( "153_a19.bin", 0x010000, 0x10000, CRC(f36390a9) SHA1(e5ea36e91b3ced068281524ee79d0432f489715c) )
	ROM_LOAD( "150_a16.bin", 0x020000, 0x10000, CRC(0da825f9) SHA1(cfba0c85fc767726c1d63f87468335d1c2f1eed8) )
	ROM_LOAD( "151_a17.bin", 0x030000, 0x10000, CRC(228429d8) SHA1(3b2dbea53807929c24d593c469a83172f7747f66) )
	ROM_LOAD( "148_a14.bin", 0x040000, 0x10000, CRC(26371c18) SHA1(0887041d86dc9f19dad264ae27dc56fb89ac3265) )
	ROM_LOAD( "149_a15.bin", 0x050000, 0x10000, CRC(75aa9b86) SHA1(0c221bd2e8a5472bb0e515f27fb72b0c8e8c0ca4) )
	ROM_LOAD( "146_a12.bin", 0x060000, 0x10000, CRC(d5a60096) SHA1(a8e351a4b020b4fc2b2cb7d3f0fdfb43fc44d7d9) )
	ROM_LOAD( "147_a13.bin", 0x070000, 0x10000, CRC(36bbf467) SHA1(627b5847ffb098c92edfd58c25391799f3b209e0) )

	ROM_REGION( 0x1000, "plds", 0 ) // from twcup90b2 set
	TWCUP90B_PLD_DEVICES
ROM_END


/* Different bootleg set with only one new ROM, a05 (added as "el_ic98_27c512_05.bin"), probably just a minor text mod from the supported set
(only two bytes differs), although I cannot find the difference:
   Comparing files a05.bin and el_ic98_27c512_05.bin
    00000590: 0F 0B
    00000591: FF FA
*/
ROM_START( twcup90ba )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "a02.bin", 0x00000, 0x10000, CRC(192a03dd) SHA1(ab98d370bba5437f956631b0199b173be55f1c27) )  // c000-ffff is not used
	ROM_LOAD( "a03.bin", 0x10000, 0x10000, CRC(f54ff17a) SHA1(a19850fc28a5a0da20795a5cc6b56d9c16554bce) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "a04.bin",              0x00000, 0x10000, CRC(3d535e2f) SHA1(f1e1878b5a8316e770c74a1e1f29a7a81a4e5dfe) )  // c000-ffff is not used
	ROM_LOAD( "el_ic98_27c512_05.bin",0x10000, 0x10000, CRC(c70d8c13) SHA1(365718725ea7d0355c68ba703b7f9624cb1134bc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a01.bin", 0x00000, 0x10000, CRC(3d317622) SHA1(ae4e8c5247bc215a2769786cb8639bce2f80db22) )

	ROM_REGION( 0x010000, "chargfx", 0 )
	ROM_LOAD( "a06.bin", 0x000000, 0x04000, CRC(3b5387b7) SHA1(b839b4eafe8bf6f9e841e19fee1bdb64a66f3448) )
	ROM_LOAD( "a08.bin", 0x004000, 0x04000, CRC(c622a5a3) SHA1(468c8c24af1f6f244228b66df04cb0ea81c1875e) )
	ROM_LOAD( "a10.bin", 0x008000, 0x04000, CRC(0923d9f6) SHA1(4b10ee3fc17bb63cda51b2a978d066b6a140a551) )
	ROM_LOAD( "a20.bin", 0x00c000, 0x04000, CRC(b8dec83e) SHA1(fe617ddccdd0dbd05ca09a1507074aa14b529322) )

	ROM_REGION( 0x080000, "tilegfx", 0 )
	ROM_LOAD( "a07.bin", 0x000000, 0x20000, CRC(38c31817) SHA1(cb24ed8702d62066366924c033c07ffc78bd1fad) )
	ROM_LOAD( "a09.bin", 0x020000, 0x20000, CRC(32e39e29) SHA1(44f22ed6c983541c7fea5857ba0456aaa87b36d1) )
	ROM_LOAD( "a11.bin", 0x040000, 0x20000, CRC(5ccec796) SHA1(2cc191a4267819eb31962726e2ed4567c825c39e) )
	ROM_LOAD( "a21.bin", 0x060000, 0x20000, CRC(0c54a091) SHA1(3eecb285b5a7bbc310c87492516d7ffb2841aa3b) )

	ROM_REGION( 0x080000, "spritegfx", ROMREGION_INVERT )
	ROM_LOAD( "152_a18.bin", 0x000000, 0x10000, CRC(516b6c09) SHA1(9d02514dece864b087f67886009ce54bd51b5575) )
	ROM_LOAD( "153_a19.bin", 0x010000, 0x10000, CRC(f36390a9) SHA1(e5ea36e91b3ced068281524ee79d0432f489715c) )
	ROM_LOAD( "150_a16.bin", 0x020000, 0x10000, CRC(0da825f9) SHA1(cfba0c85fc767726c1d63f87468335d1c2f1eed8) )
	ROM_LOAD( "151_a17.bin", 0x030000, 0x10000, CRC(228429d8) SHA1(3b2dbea53807929c24d593c469a83172f7747f66) )
	ROM_LOAD( "148_a14.bin", 0x040000, 0x10000, CRC(26371c18) SHA1(0887041d86dc9f19dad264ae27dc56fb89ac3265) )
	ROM_LOAD( "149_a15.bin", 0x050000, 0x10000, CRC(75aa9b86) SHA1(0c221bd2e8a5472bb0e515f27fb72b0c8e8c0ca4) )
	ROM_LOAD( "146_a12.bin", 0x060000, 0x10000, CRC(d5a60096) SHA1(a8e351a4b020b4fc2b2cb7d3f0fdfb43fc44d7d9) )
	ROM_LOAD( "147_a13.bin", 0x070000, 0x10000, CRC(36bbf467) SHA1(627b5847ffb098c92edfd58c25391799f3b209e0) )

	ROM_REGION( 0x1000, "plds", 0 ) // from twcup90b2 set
	TWCUP90B_PLD_DEVICES
ROM_END


ROM_START( twcup90b2 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "a02",     0x00000, 0x08000, CRC(84cb2bf5) SHA1(de8343c991fc752de46448e4f6db1c3a70fc4459) )  // 2x 27c256
	ROM_LOAD( "a03.bin", 0x10000, 0x08000, CRC(68156be5) SHA1(c90b873a147d00f313084cbe5d0a5a7688af1485) )

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "a04.bin", 0x00000, 0x10000, CRC(3d535e2f) SHA1(f1e1878b5a8316e770c74a1e1f29a7a81a4e5dfe) )  // 2x 27c512
	ROM_LOAD( "a05.bin", 0x10000, 0x10000, CRC(9e421c4b) SHA1(e23a1f1d5d1e960696f45df653869712eb889839) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a01.bin", 0x00000, 0x10000, CRC(3d317622) SHA1(ae4e8c5247bc215a2769786cb8639bce2f80db22) )  // 27c512

	ROM_REGION( 0x010000, "chargfx", 0 )
	ROM_LOAD( "a06", 0x000000, 0x04000, CRC(0c054481) SHA1(eebab099a4db5fbf13522ecd67bfa741e16e40d4) )  // 4x 27c256
	ROM_CONTINUE (   0x000000, 0x04000)
	ROM_LOAD( "a08", 0x004000, 0x04000, CRC(ebb3eb48) SHA1(9cb133e02004bc04a9d7016b8cf5f6865e3ccf26) )
	ROM_CONTINUE (   0x004000, 0x04000)
	ROM_LOAD( "a10", 0x008000, 0x04000, CRC(c0232af8) SHA1(5bbab00403a47feae153e179c04212021036b8a7) )
	ROM_CONTINUE (   0x008000, 0x04000)
	ROM_LOAD( "a20", 0x00c000, 0x04000, CRC(a36e17fb) SHA1(45e4df4b4a22658f6dad21853e87fae734698fbd) )
	ROM_CONTINUE (   0x00c000, 0x04000)

	ROM_REGION( 0x080000, "tilegfx", 0 )
	ROM_LOAD( "a07.bin", 0x000000, 0x20000, CRC(38c31817) SHA1(cb24ed8702d62066366924c033c07ffc78bd1fad) )  // 4x 27c010
	ROM_LOAD( "a09.bin", 0x020000, 0x20000, CRC(32e39e29) SHA1(44f22ed6c983541c7fea5857ba0456aaa87b36d1) )
	ROM_LOAD( "a11.bin", 0x040000, 0x20000, CRC(5ccec796) SHA1(2cc191a4267819eb31962726e2ed4567c825c39e) )
	ROM_LOAD( "a21.bin", 0x060000, 0x20000, CRC(0c54a091) SHA1(3eecb285b5a7bbc310c87492516d7ffb2841aa3b) )

	ROM_REGION( 0x080000, "spritegfx", ROMREGION_INVERT )
	ROM_LOAD( "152_a18.bin", 0x000000, 0x10000, CRC(516b6c09) SHA1(9d02514dece864b087f67886009ce54bd51b5575) )  // 8x 27c512
	ROM_LOAD( "153_a19",     0x010000, 0x10000, CRC(8caa2745) SHA1(41efb92c98e063f5ed5fb0e68fa014f89da00cda) )
	ROM_LOAD( "150_a16.bin", 0x020000, 0x10000, CRC(0da825f9) SHA1(cfba0c85fc767726c1d63f87468335d1c2f1eed8) )
	ROM_LOAD( "151_a17",     0x030000, 0x10000, CRC(af98778e) SHA1(5bbce33a4cec5a234ed78e30899a4a166d71447a) )
	ROM_LOAD( "148_a14.bin", 0x040000, 0x10000, CRC(26371c18) SHA1(0887041d86dc9f19dad264ae27dc56fb89ac3265) )
	ROM_LOAD( "149_a15",     0x050000, 0x10000, CRC(b2423962) SHA1(098bc06411cf3f9c7cf69933eba360fd059b5d3f) )
	ROM_LOAD( "146_a12.bin", 0x060000, 0x10000, CRC(d5a60096) SHA1(a8e351a4b020b4fc2b2cb7d3f0fdfb43fc44d7d9) )
	ROM_LOAD( "147_a13",     0x070000, 0x10000, CRC(5b16fd48) SHA1(b167d6a7da0c696cde39581822fc61d20756321c) )

	ROM_REGION( 0x1000, "plds", 0 )
	TWCUP90B_PLD_DEVICES
ROM_END


ROM_START( twcup90b3 ) // most similar to twcup90b1. First main CPU ROM has a small routine added
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "03.bin", 0x00000, 0x10000, CRC(1e6e94c9) SHA1(1731e3e3b5d17ba676a7e42638d7206212a0080d) )  // c000-ffff is not used
	ROM_LOAD( "02.bin", 0x10000, 0x10000, CRC(f54ff17a) SHA1(a19850fc28a5a0da20795a5cc6b56d9c16554bce) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "05.bin", 0x00000, 0x10000, CRC(3d535e2f) SHA1(f1e1878b5a8316e770c74a1e1f29a7a81a4e5dfe) )  // c000-ffff is not used
	ROM_LOAD( "04.bin", 0x10000, 0x10000, CRC(9e421c4b) SHA1(e23a1f1d5d1e960696f45df653869712eb889839) )  // banked at f000-f7ff

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "01.bin", 0x00000, 0x10000, CRC(3d317622) SHA1(ae4e8c5247bc215a2769786cb8639bce2f80db22) )

	ROM_REGION( 0x010000, "chargfx", 0 ) // uses 0x10000 size ROMs instead of 0x4000
	ROM_LOAD( "07.bin", 0x000000, 0x04000, CRC(b82d19ba) SHA1(e6fcfee178d4b5c7a07af8b5637791d02703c5b7) )
	ROM_IGNORE(                   0x0c000 ) // BADADDR        --xxxxxxxxxxxxxx
	ROM_LOAD( "09.bin", 0x004000, 0x04000, CRC(0a488017) SHA1(134606f9328118be6f8a5b403041b57ef2eaac90) )
	ROM_IGNORE(                   0x0c000 ) // BADADDR        --xxxxxxxxxxxxxx
	ROM_LOAD( "11.bin", 0x008000, 0x04000, CRC(cf0dec64) SHA1(b66712408379572d5c0dc28a8207b08dd53f3d8f) )
	ROM_IGNORE(                   0x0c000 ) // BADADDR        --xxxxxxxxxxxxxx
	ROM_LOAD( "13.bin", 0x00c000, 0x04000, CRC(11489cc6) SHA1(34aa4e4da2b5125d555b89894a77901673b89dd1) )
	ROM_IGNORE(                   0x0c000 ) // BADADDR        --xxxxxxxxxxxxxx

	ROM_REGION( 0x080000, "tilegfx", 0 )
	ROM_LOAD( "06.bin", 0x000000, 0x20000, CRC(38c31817) SHA1(cb24ed8702d62066366924c033c07ffc78bd1fad) )
	ROM_LOAD( "08.bin", 0x020000, 0x20000, CRC(32e39e29) SHA1(44f22ed6c983541c7fea5857ba0456aaa87b36d1) )
	ROM_LOAD( "10.bin", 0x040000, 0x20000, CRC(5ccec796) SHA1(2cc191a4267819eb31962726e2ed4567c825c39e) )
	ROM_LOAD( "12.bin", 0x060000, 0x20000, CRC(0c54a091) SHA1(3eecb285b5a7bbc310c87492516d7ffb2841aa3b) )

	ROM_REGION( 0x080000, "spritegfx", ROMREGION_INVERT )
	ROM_LOAD( "15.bin", 0x000000, 0x10000, CRC(516b6c09) SHA1(9d02514dece864b087f67886009ce54bd51b5575) )
	ROM_LOAD( "14.bin", 0x010000, 0x10000, CRC(f36390a9) SHA1(e5ea36e91b3ced068281524ee79d0432f489715c) )
	ROM_LOAD( "17.bin", 0x020000, 0x10000, CRC(0da825f9) SHA1(cfba0c85fc767726c1d63f87468335d1c2f1eed8) )
	ROM_LOAD( "16.bin", 0x030000, 0x10000, CRC(228429d8) SHA1(3b2dbea53807929c24d593c469a83172f7747f66) )
	ROM_LOAD( "19.bin", 0x040000, 0x10000, CRC(26371c18) SHA1(0887041d86dc9f19dad264ae27dc56fb89ac3265) )
	ROM_LOAD( "18.bin", 0x050000, 0x10000, CRC(75aa9b86) SHA1(0c221bd2e8a5472bb0e515f27fb72b0c8e8c0ca4) )
	ROM_LOAD( "21.bin", 0x060000, 0x10000, CRC(d5a60096) SHA1(a8e351a4b020b4fc2b2cb7d3f0fdfb43fc44d7d9) )
	ROM_LOAD( "20.bin", 0x070000, 0x10000, CRC(36bbf467) SHA1(627b5847ffb098c92edfd58c25391799f3b209e0) )

	ROM_REGION( 0x1000, "plds", 0 ) // from twcup90b2 set
	TWCUP90B_PLD_DEVICES
ROM_END


/*
  World Cup '90
  Hack with European teams, like 'Euro League'
  Board found in Argentina.
*/
ROM_START( twcup90bb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "27c512.02", 0x00000, 0x10000, CRC(192a03dd) SHA1(ab98d370bba5437f956631b0199b173be55f1c27) )
	ROM_LOAD( "27c512.03", 0x10000, 0x10000, CRC(f54ff17a) SHA1(a19850fc28a5a0da20795a5cc6b56d9c16554bce) )

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "27c512.04", 0x00000, 0x10000, CRC(3d535e2f) SHA1(f1e1878b5a8316e770c74a1e1f29a7a81a4e5dfe) )
	ROM_LOAD( "27c512.05", 0x10000, 0x10000, CRC(9e421c4b) SHA1(e23a1f1d5d1e960696f45df653869712eb889839) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "27c512.bin", 0x00000, 0x10000, CRC(3d317622) SHA1(ae4e8c5247bc215a2769786cb8639bce2f80db22) )

	ROM_REGION( 0x010000, "chargfx", 0 )
	ROM_LOAD( "27c256.06", 0x000000, 0x04000, CRC(0c054481) SHA1(eebab099a4db5fbf13522ecd67bfa741e16e40d4) )
	ROM_CONTINUE (         0x000000, 0x04000)
	ROM_LOAD( "27256.08",  0x004000, 0x04000, CRC(ebb3eb48) SHA1(9cb133e02004bc04a9d7016b8cf5f6865e3ccf26) )
	ROM_CONTINUE (         0x004000, 0x04000)
	ROM_LOAD( "27128.10",  0x008000, 0x04000, CRC(0923d9f6) SHA1(4b10ee3fc17bb63cda51b2a978d066b6a140a551) )
	ROM_LOAD( "27128k.20", 0x00c000, 0x04000, CRC(b8dec83e) SHA1(fe617ddccdd0dbd05ca09a1507074aa14b529322) )

	ROM_REGION( 0x080000, "tilegfx", 0 )
	ROM_LOAD( "ds40986_27c010.07", 0x000000, 0x20000, CRC(38c31817) SHA1(cb24ed8702d62066366924c033c07ffc78bd1fad) )
	ROM_LOAD( "ds40986_27c010.09", 0x020000, 0x20000, CRC(32e39e29) SHA1(44f22ed6c983541c7fea5857ba0456aaa87b36d1) )
	ROM_LOAD( "ds40986_27c010.11", 0x040000, 0x20000, CRC(5ccec796) SHA1(2cc191a4267819eb31962726e2ed4567c825c39e) )
	ROM_LOAD( "ds40986_27c010.21", 0x060000, 0x20000, CRC(0c54a091) SHA1(3eecb285b5a7bbc310c87492516d7ffb2841aa3b) )

	ROM_REGION( 0x080000, "spritegfx", ROMREGION_INVERT )
	ROM_LOAD( "27c512.18", 0x000000, 0x10000, CRC(516b6c09) SHA1(9d02514dece864b087f67886009ce54bd51b5575) )
	ROM_LOAD( "27c512.19", 0x010000, 0x10000, CRC(f9df54f6) SHA1(cee8da5d8e4959e5546b2f7dcc740e98bedda07a) )
	ROM_LOAD( "27c512.16", 0x020000, 0x10000, CRC(0da825f9) SHA1(cfba0c85fc767726c1d63f87468335d1c2f1eed8) )
	ROM_LOAD( "27c512.17", 0x030000, 0x10000, CRC(c387c804) SHA1(519a63c337d443f0876fcd44b88ed508b999912f) )
	ROM_LOAD( "27c512.14", 0x040000, 0x10000, CRC(26371c18) SHA1(0887041d86dc9f19dad264ae27dc56fb89ac3265) )
	ROM_LOAD( "27c512.15", 0x050000, 0x10000, CRC(77700f2d) SHA1(a39987f8ac1bb26d5aa0ae8cfe67fac823a0d1af) )
	ROM_LOAD( "27c512.12", 0x060000, 0x10000, CRC(6a828204) SHA1(0d8e90ee069fe16db3869cbc47991511244e1b34) )
	ROM_LOAD( "27c512.13", 0x070000, 0x10000, CRC(4706bad2) SHA1(f79460f094454b544b2637ff09bc41c9e107c764) )

	ROM_REGION( 0x1000, "plds", 0 ) // from twcup90b2 set
	TWCUP90B_PLD_DEVICES
ROM_END


// Modular System is a stack of boards in a cage, there are apparently other games on this 'system' that wouldn't even share any hardware with this apart from the metal cage itself.
ROM_START( eurogael )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3z-1_fu301.ic17", 0x00000, 0x10000, CRC(74acc161) SHA1(d8660dd6d05164df4a66125c68627e955b35bef3) )  // c000-ffff is not used
	ROM_LOAD( "3z-1_fu302.ic15", 0x10000, 0x10000, CRC(f54ff17a) SHA1(a19850fc28a5a0da20795a5cc6b56d9c16554bce) )  // banked at f000-f7ff

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "3z-1_fu303.ic19", 0x00000, 0x10000, CRC(348195fa) SHA1(41c59e38ec4ba4f3c2185dd32dbf4ea0318ab375) )  // c000-ffff is not used
	ROM_LOAD( "3z-1_fu304.ic16", 0x10000, 0x10000, CRC(9e421c4b) SHA1(e23a1f1d5d1e960696f45df653869712eb889839) )  // banked at f000-f7ff

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "system2_fu101.ic6", 0x00000, 0x10000, CRC(712566ca) SHA1(9e46f9d449ff549b7a6d82283d8f903189b058e7) )

	ROM_REGION( 0x20000, "chargfx", ROMREGION_INVERT ) // 1ST AND 2ND HALF IDENTICAL (all ROMs in this region)
	ROM_LOAD( "r1_fu404.ic14", 0x00000, 0x8000, CRC(1659529a) SHA1(ad7a2bab43922871ce7081d46efdeefa9db5e45f) )
	ROM_LOAD( "r1_fu403.ic15", 0x08000, 0x8000, CRC(f1effd53) SHA1(0299678e1716a92cd6ecc67d28dd95422f84d15d) )
	ROM_LOAD( "r1_fu402.ic16", 0x10000, 0x8000, CRC(da7f3ce3) SHA1(80ae95208a807e3866560ad5f192951d33f1da34) )
	ROM_LOAD( "r1_fu401.ic17", 0x18000, 0x8000, CRC(b93201e0) SHA1(edc8dbab15d901aab678244811d8e08d4cb974f2) )

	ROM_REGION( 0x080000, "tilegfx", ROMREGION_INVERT )
	ROM_LOAD( "r2_fu4a01.ic17", 0x060000, 0x10000, CRC(1bcdd99d) SHA1(e37a8308eff79b04ce3da75fec2fffe4021c70d3) )  // fg layer
	ROM_LOAD( "r2_fu4a02.ic16", 0x040000, 0x10000, CRC(e625de0c) SHA1(ca5a00ea27a5107d2957a1637b260d5aa1ff5563) )
	ROM_LOAD( "r2_fu4a03.ic15", 0x020000, 0x10000, CRC(995df90d) SHA1(121874c807001bf65a9cec90dd30212f1889a398) )
	ROM_LOAD( "r2_fu4a04.ic14", 0x000000, 0x10000, CRC(44031f34) SHA1(0b417c6cc1ed029394bcdce80e81262d776105b7) )
	ROM_LOAD( "r3_fu4b01.ic17", 0x070000, 0x10000, CRC(adc7feda) SHA1(ff82e973599f13c76ecaf3c027a2967468cd9e72) )  // bg layer
	ROM_LOAD( "r3_fu4b02.ic16", 0x050000, 0x10000, CRC(1e5c0fda) SHA1(74163098810ff079467bad77796898426acbfac5) )
	ROM_LOAD( "r3_fu4b03.ic15", 0x030000, 0x10000, CRC(cc9dab0b) SHA1(7e5735e57b45de1b344455fa5a047ae17933de27) )
	ROM_LOAD( "r3_fu4b04.ic14", 0x010000, 0x10000, CRC(d54704d2) SHA1(7d4ec0120c3516a88abbe687e8916b44bffb7bcd) )

	ROM_REGION( 0x080000, "spritegfx", ROMREGION_INVERT )
	ROM_LOAD( "r4_fu504.bin",  0x000000, 0x10000, CRC(516b6c09) SHA1(9d02514dece864b087f67886009ce54bd51b5575) )
	ROM_LOAD( "r4_fu5a04.bin", 0x010000, 0x10000, CRC(f36390a9) SHA1(e5ea36e91b3ced068281524ee79d0432f489715c) )
	ROM_LOAD( "r4_fu503.bin",  0x020000, 0x10000, CRC(0da825f9) SHA1(cfba0c85fc767726c1d63f87468335d1c2f1eed8) )
	ROM_LOAD( "r4_fu5a03.bin", 0x030000, 0x10000, CRC(228429d8) SHA1(3b2dbea53807929c24d593c469a83172f7747f66) )
	ROM_LOAD( "r4_fu502.bin",  0x040000, 0x10000, CRC(26371c18) SHA1(0887041d86dc9f19dad264ae27dc56fb89ac3265) )
	ROM_LOAD( "r4_fu5a02.bin", 0x050000, 0x10000, CRC(75aa9b86) SHA1(0c221bd2e8a5472bb0e515f27fb72b0c8e8c0ca4) )
	ROM_LOAD( "r4_fu501.bin",  0x060000, 0x10000, CRC(d5a60096) SHA1(a8e351a4b020b4fc2b2cb7d3f0fdfb43fc44d7d9) )
	ROM_LOAD( "r4_fu5a01.bin", 0x070000, 0x10000, CRC(36bbf467) SHA1(627b5847ffb098c92edfd58c25391799f3b209e0) )

	ROM_REGION( 0x100, "prom", ROMREGION_ERASEFF )
	ROM_LOAD( "r4_p0502_82s129.ic10",      0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )

	ROM_REGION( 0x1000, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "r2_p0403_pal16r8a.ic29", 0x000, 0x104, CRC(506156cc) SHA1(5560671fc2c9872ed28620491af5dc486909fc6e) )
	ROM_LOAD( "r3_p0403_pal16r8a.ic29", 0x000, 0x104, CRC(d8c6ac25) SHA1(d6184e491313ff8da5b1ce60ffe8ef517716807c) )
	ROM_LOAD( "r4_p0503_pal16r6.ic46",  0x000, 0x104, CRC(07eb86d2) SHA1(482eb325df5bc60353bac85412cf45429cd03c6d) )
	// these were read protected
	ROM_LOAD( "3z-1_3138_gal16v8.ic22",    0x0, 0x1, NO_DUMP )
	ROM_LOAD( "3z-1_3238_gal16v8.ic24",    0x0, 0x1, NO_DUMP )
	ROM_LOAD( "r1_403_gal16v8.ic29",       0x0, 0x1, NO_DUMP )
	ROM_LOAD( "system2_9138_gal16v8.ic42", 0x0, 0x1, NO_DUMP )
	ROM_LOAD( "system2_9238_gal20v8.ic18", 0x0, 0x1, NO_DUMP )
	ROM_LOAD( "system2_9338_gal16v8.ic10", 0x0, 0x1, NO_DUMP )
ROM_END


void wc90b_state::init_wc90b()
{
	// reorganize graphics into something we can decode with a single pass
	uint8_t *src = memregion("tilegfx")->base();
	int len = memregion("tilegfx")->bytes();

	std::vector<uint8_t> buffer(len);
	{
		for (int i = 0; i < len; i++)
		{
			int j = bitswap<19>(i, 18, 17, // above bitplane separation limit, no swap, keep value
								   16, 15, 14, 13, // above tile bank limit, no swap, keep value
								   10, 9, 8, 7, 6, 5, 4, 3,
								   12, 11,
								   2, 1, 0); // 8x8x1 tile data, no swap, keep value
			buffer[j] = src[i];
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}
}

} // anonymous namespace


GAME( 1989, twcup90b1, twcup90, wc90b, wc90b, wc90b_state, init_wc90b, ROT0, "bootleg", "Euro League (Italian hack of Tecmo World Cup '90, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, twcup90ba, twcup90, wc90b, wc90b, wc90b_state, init_wc90b, ROT0, "bootleg", "Euro League (Italian hack of Tecmo World Cup '90, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, twcup90b3, twcup90, wc90b, wc90b, wc90b_state, init_wc90b, ROT0, "bootleg", "Euro League (Italian hack of Tecmo World Cup '90, set 3)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, twcup90b2, twcup90, wc90b, wc90b, wc90b_state, init_wc90b, ROT0, "bootleg", "Worldcup '90 (hack)",                                      MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, twcup90bb, twcup90, wc90b, wc90b, wc90b_state, init_wc90b, ROT0, "bootleg", "World Cup '90 (European hack, different title)",           MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
// not sure if it best fits here, in wc90.cpp, or in a new driver, it shares the weird tile decoding with the bootlegs tho
// Gaelco requested the registry of the "Euro League" trademark on 1990, and it was a Gaelco protected trademark (in Spain) until 1999 (they paid a 5-year renew in 1994): https://www.patentes-y-marcas.com/marca/euro-league-m1546246
GAME( 1989, eurogael,  twcup90, eurogael, wc90b, eurogael_state, init_wc90b, ROT0, "bootleg (Gaelco / Ervisa)", "Euro League (Gaelco bootleg, Modular System)", MACHINE_IMPERFECT_SOUND )

