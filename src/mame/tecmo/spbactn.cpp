// license:BSD-3-Clause
// copyright-holders:David Haywood

/*******************************************************************************
 Super Pinball Action (c) 1991 Tecmo
********************************************************************************
 driver by David Haywood
 inputs, dipswitches etc by Stephh

-general info-------------------------------------------------------------------

 A Pinball Game from Tecmo, the Hardware seems to be somewhere between that used
 for Tecmo's classic game Ninja Gaiden (see gaiden.cpp) and that used in Comad's
 Gals Pinball (see galspnbl.cpp). I imagine Comad took the hardware that this uses
 as a basis for writing their game on, adding a couple of features such as the
 pixel layer.

The manual defines the controls as 4 push buttons:

   Left Push Buttons       Right Push Buttons

|   o    |      o       |    o    |      o       |
|--------+--------------+---------+--------------|
|Flipper | Shot & Shake | Flipper | Shot & Shake |
| Left   |    Left      |  Right  |    Right     |
|--------+--------------+---------+--------------|


-readme file--------------------------------------------------------------------

 Super Pinball Action
 (c)1991 Tecmo

 CPU  : MC68000P12
 Sound: Z80A YM3812 Y3014B M6295
 OSC  : 12.000MHz 22.656MHz 4.00MHz

 ------
 9002-A
 ------
 ROMs:
 a-u68.1 - Main programs (27c101)
 a-u67.2 /

 a-u14.3 - Sound program (27512)

 a-u19 - Samples (27c1001)

 1 custom chip (u94, surface scratched)

 ------
 9002-B
 ------
 ROMs:
 b-u98  - Graphics (Mask, read as 27c2001)
 b-u99  |
 b-u104 |
 b-u105 /


 b-u110 - Graphics (Mask, read as 27c2001)
 b-u111 /

 Custom chips:
 U101, U102, U106, U107: surface scratched
 probably 2 pairs of TECMO-3&4
 U133: surface scratched
 probably TECMO-6
 U112: TECMO-5

 --- Team Japump!!! ---
 Dumped by Noel Miruru
 17/Oct/2000

-working notes------------------------------------------------------------------

 68k interrupts
 lev 1 : 0x64 : ffff ffff - invalid
 lev 2 : 0x68 : ffff ffff - invalid
 lev 3 : 0x6c : 0000 1a0a - vblank?
 lev 4 : 0x70 : ffff ffff - invalid
 lev 5 : 0x74 : ffff ffff - invalid
 lev 6 : 0x78 : 0000 1ab2 - writes to 90031
 lev 7 : 0x7c : ffff ffff - invalid

TODO : (also check the notes from the galspnbl.cpp driver)

  - coin insertion is not recognized consistently.
  - all the unknown regs
  - Oki fills the log with 'Requested to play sample 0X on non-stopped voice'


Unmapped writes (P.O.S.T.)

cpu #0 (PC=00001C3A): unmapped memory word write to 00090080 = 0F30 & FFFF
cpu #0 (PC=00001C42): unmapped memory word write to 00090090 = 0E00 & FFFF
cpu #0 (PC=00001C4A): unmapped memory word write to 000900A0 = 0F74 & FFFF
cpu #0 (PC=00001C52): unmapped memory word write to 000900B0 = 0FBA & FFFF
cpu #0 (PC=00001C5A): unmapped memory word write to 000900C0 = 0FDA & FFFF
cpu #0 (PC=00001C62): unmapped memory word write to 000900D0 = 0F20 & FFFF
cpu #0 (PC=00001C6A): unmapped memory word write to 000900E0 = 0FE7 & FFFF
cpu #0 (PC=00001C72): unmapped memory word write to 000900F0 = 0FF1 & FFFF
cpu #0 (PC=00001C7A): unmapped memory word write to 000A0110 = 0001 & FFFF
cpu #0 (PC=00001C80): unmapped memory word write to 000A0010 = 0001 & FFFF
cpu #0 (PC=00001C88): unmapped memory word write to 000A0200 = 001F & FFFF
cpu #0 (PC=00001C90): unmapped memory word write to 000A0202 = 0010 & FFFF
cpu #0 (PC=00001C98): unmapped memory word write to 000A0204 = 00E0 & FFFF
cpu #0 (PC=00001CA0): unmapped memory word write to 000A0206 = 0001 & FFFF
cpu #0 (PC=00002BFA): unmapped memory word write to 00090000 = 0000 & 00FF
cpu #0 (PC=00002C08): unmapped memory word write to 000A0100 = FF85 & FFFF
cpu #0 (PC=00002C10): unmapped memory word write to 000A0000 = FF85 & FFFF
cpu #0 (PC=00002C18): unmapped memory word write to 000A0108 = 0010 & FFFF
cpu #0 (PC=00002C20): unmapped memory word write to 000A0008 = 0010 & FFFF
cpu #0 (PC=00002C28): unmapped memory word write to 000A0104 = 0000 & FFFF
cpu #0 (PC=00002C2E): unmapped memory word write to 000A010C = 0000 & FFFF
cpu #0 (PC=00002C34): unmapped memory word write to 000A0004 = 0000 & FFFF
cpu #0 (PC=00002C3A): unmapped memory word write to 000A000C = 0000 & FFFF
cpu #0 (PC=00002C42): unmapped memory word write to 00090050 = 0004 & 00FF
cpu #0 (PC=00001A14): unmapped memory word write to 00090020 = 00AA & 00FF
cpu #0 (PC=00001A1A): unmapped memory word write to 00090030 = 0055 & 00FF

   Unmapped writes (when Dip Switches are displayed)

cpu #0 (PC=00001A14): unmapped memory word write to 00090020 = 00FF & 00FF
cpu #0 (PC=00001A1A): unmapped memory word write to 00090030 = 00FF & 00FF

   Unmapped writes (when grid is displayed)

cpu #0 (PC=0000326A): unmapped memory word write to 00090010 = 00FF & 00FF (only once)
cpu #0 (PC=00001A14): unmapped memory word write to 00090020 = 00F6 & 00FF
cpu #0 (PC=00001A1A): unmapped memory word write to 00090030 = 00F7 & 00FF

*******************************************************************************/

#include "emu.h"

#include "tecmo_spr.h"
#include "tecmo_mix.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "spbactnp.lh"

// configurable logging
#define LOG_PROTO     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_PROTO)

#include "logmacro.h"

#define LOGPROTO(...)     LOGMASKED(LOG_PROTO,     __VA_ARGS__)


namespace {

class spbactn_state : public driver_device
{
public:
	spbactn_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_mixer(*this, "mixer"),
		m_soundlatch(*this, "soundlatch"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spvideoram(*this, "spvideoram")
	{ }

	void spbactn(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;
	required_device<tecmo_mix_device> m_mixer;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_bgvideoram;
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_spvideoram;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	void main_irq_ack_w(uint16_t data);

	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	int draw_video(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bool alt_sprites);

	void sound_map(address_map &map) ATTR_COLD;

private:
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	bitmap_ind16 m_tile_bitmap_bg;
	bitmap_ind16 m_tile_bitmap_fg;
	bitmap_ind16 m_sprite_bitmap;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
};

class spbactnp_state : public spbactn_state
{
public:
	spbactnp_state(const machine_config &mconfig, device_type type, const char *tag) :
		spbactn_state(mconfig, type, tag),
		m_extracpu(*this, "extracpu"),
		m_extralatch(*this, "extralatch"),
		m_extraram(*this, "extraram%u", 1U),
		m_extrapalette(*this, "extrapalette"),
		m_extragfxdecode(*this, "extragfxdecode"),
		m_extrascreen(*this, "extrascreen")
	{ }

	void spbactnp(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_extracpu;
	required_device<generic_latch_16_device> m_extralatch;
	required_shared_ptr_array<uint8_t, 2> m_extraram;
	required_device<palette_device> m_extrapalette;
	required_device<gfxdecode_device> m_extragfxdecode;
	required_device<screen_device> m_extrascreen;

	tilemap_t *m_extra_tilemap = nullptr;

	void extraram_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_extra_tile_info);

	void _9000a_w(uint16_t data);
	void _9000c_w(uint16_t data);
	void extrascreen_latch_w(uint16_t data);

	void bg_scrolly_w(uint16_t data);
	void bg_scrollx_w(uint16_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t extrascreen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t extra_latch_r(offs_t offset);

	void extra_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


void spbactn_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bgvideoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset & 0x1fff);
}

TILE_GET_INFO_MEMBER(spbactn_state::get_bg_tile_info)
{
	int const attr = m_bgvideoram[tile_index];
	int const tileno = m_bgvideoram[tile_index + 0x2000];
	tileinfo.set(1, tileno, ((attr & 0x00f0) >> 4), 0);
}


void spbactn_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset & 0x1fff);
}

TILE_GET_INFO_MEMBER(spbactn_state::get_fg_tile_info)
{
	int const attr = m_fgvideoram[tile_index];
	int const tileno = m_fgvideoram[tile_index + 0x2000];

	int color = ((attr & 0x00f0) >> 4);

	// blending
	if (attr & 0x0008)
		color += 0x0010;

	tileinfo.set(0, tileno, color, 0);
}



void spbactn_state::video_start()
{
	// allocate bitmaps
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spbactn_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 64, 128);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spbactn_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 64, 128);
	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);
}

void spbactnp_state::video_start()
{
	spbactn_state::video_start();

	m_extra_tilemap = &machine().tilemap().create(*m_extragfxdecode, tilemap_get_info_delegate(*this, FUNC(spbactnp_state::get_extra_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

void spbactnp_state::_9000c_w(uint16_t data)
{
	LOGPROTO("%s: _9000c_w %04x\n", machine().describe_context(), data);
}

void spbactnp_state::extrascreen_latch_w(uint16_t data)
{
	LOGPROTO("%s: extrascreen_latch_w %04x\n", machine().describe_context(), data);
	m_extralatch->write(data);
}

void spbactnp_state::_9000a_w(uint16_t data)
{
	LOGPROTO("%s: _9000a_w %04x\n", machine().describe_context(), data);
}

void spbactnp_state::bg_scrolly_w(uint16_t data)
{
	LOGPROTO("bg_scrolly_w %04x\n", data);
	m_bg_tilemap->set_scrolly(0, data);
}

void spbactnp_state::bg_scrollx_w(uint16_t data)
{
	LOGPROTO("bg_scrollx_w %04x\n", data);
	m_bg_tilemap->set_scrollx(0, data);
}


void spbactnp_state::extraram_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_extraram[0][offset]);
	m_extra_tilemap->mark_tile_dirty(offset & 0x7ff);
}

TILE_GET_INFO_MEMBER(spbactnp_state::get_extra_tile_info)
{
	int tileno = m_extraram[0][(tile_index)];
	tileno |= m_extraram[0][(tile_index + 0x800)] << 8;
	tileinfo.set(0, tileno & 0xfff, tileno >> 12, 0);
}




int spbactn_state::draw_video(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bool alt_sprites)
{
	m_tile_bitmap_bg.fill(0, cliprect);
	m_tile_bitmap_fg.fill(0, cliprect);
	m_sprite_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_sprgen->gaiden_draw_sprites(screen, m_sprite_bitmap, cliprect, &m_spvideoram[0], 0, 0, flip_screen());
	m_bg_tilemap->draw(screen, m_tile_bitmap_bg, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, m_tile_bitmap_fg, cliprect, 0, 0);

	m_mixer->mix_bitmaps(screen, bitmap, cliprect, *m_palette, &m_tile_bitmap_bg, &m_tile_bitmap_fg, (bitmap_ind16*)nullptr, &m_sprite_bitmap);

	return 0;
}

uint32_t spbactn_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return draw_video(screen, bitmap, cliprect, false);
}

uint32_t spbactnp_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return draw_video(screen, bitmap, cliprect, true);
}

uint32_t spbactnp_state::extrascreen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_extra_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void spbactn_state::main_irq_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
}

void spbactn_state::main_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom();
	map(0x40000, 0x43fff).ram();   // main RAM
	map(0x50000, 0x50fff).ram().share(m_spvideoram);
	map(0x60000, 0x67fff).ram().w(FUNC(spbactn_state::fg_videoram_w)).share(m_fgvideoram);
	map(0x70000, 0x77fff).ram().w(FUNC(spbactn_state::bg_videoram_w)).share(m_bgvideoram);
	map(0x80000, 0x827ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x90000, 0x90001).portr("IN0");
	map(0x90010, 0x90011).portr("IN1");
	map(0x90020, 0x90021).portr("SYSTEM");
	map(0x90030, 0x90031).portr("DSW1");
	map(0x90040, 0x90041).portr("DSW2");

	// these are an awful lot of unknowns
	map(0x90000, 0x90001).nopw();
	map(0x90011, 0x90011).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x90020, 0x90021).w(FUNC(spbactn_state::main_irq_ack_w));
	map(0x90030, 0x90031).nopw();
	map(0x90050, 0x90051).nopw();

	map(0x90080, 0x90081).nopw();
	map(0x90090, 0x90091).nopw();
	map(0x900a0, 0x900a1).nopw();
	map(0x900b0, 0x900b1).nopw();
	map(0x900c0, 0x900c1).nopw();
	map(0x900d0, 0x900d1).nopw();
	map(0x900e0, 0x900e1).nopw();
	map(0x900f0, 0x900f1).nopw();

	map(0xa0000, 0xa0001).nopw();
	map(0xa0004, 0xa0005).nopw();
	map(0xa0008, 0xa0009).nopw();
	map(0xa000c, 0xa000d).nopw();
	map(0xa0010, 0xa0011).nopw();

	map(0xa0100, 0xa0101).nopw();
	map(0xa0104, 0xa0105).nopw();
	map(0xa0108, 0xa0109).nopw();
	map(0xa010c, 0xa010d).nopw();
	map(0xa0110, 0xa0111).nopw();

	map(0xa0200, 0xa0201).nopw();
	map(0xa0202, 0xa0203).nopw();
	map(0xa0204, 0xa0205).nopw();
	map(0xa0206, 0xa0207).nopw();
}



void spbactnp_state::main_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom();
	map(0x40000, 0x43fff).ram();   // main RAM
	map(0x50000, 0x50fff).ram().share(m_spvideoram);
	map(0x60000, 0x67fff).ram().w(FUNC(spbactnp_state::fg_videoram_w)).share(m_fgvideoram);
	map(0x70000, 0x77fff).ram().w(FUNC(spbactnp_state::bg_videoram_w)).share(m_bgvideoram);
	map(0x80000, 0x827ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");   // yes R and G are swapped vs. the released version

	map(0x90000, 0x90001).portr("IN0");
	map(0x90002, 0x90003).portr("IN1").w(FUNC(spbactnp_state::main_irq_ack_w));
	map(0x90006, 0x90007).portr("DSW");
	map(0x90007, 0x90007).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x9000a, 0x9000b).w(FUNC(spbactnp_state::_9000a_w)); // maybe lamps?
	map(0x9000c, 0x9000d).w(FUNC(spbactnp_state::_9000c_w)); // maybe lamps?
	map(0x9000e, 0x9000f).w(FUNC(spbactnp_state::extrascreen_latch_w));

	map(0x90124, 0x90125).w(FUNC(spbactnp_state::bg_scrolly_w)); // bg scroll
	map(0x9012c, 0x9012d).w(FUNC(spbactnp_state::bg_scrollx_w)); // bg scroll
}

void spbactn_state::sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xf810, 0xf811).w("ymsnd", FUNC(ym3812_device::write));

	map(0xfc00, 0xfc00).nopr().nopw(); // IRQ ack ??
	map(0xfc20, 0xfc20).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


uint8_t spbactnp_state::extra_latch_r(offs_t offset)
{
	uint16_t latch = m_extralatch->read();
	return (offset & 1) ? (latch >> 8) : (latch & 0xff);
}


void spbactnp_state::extra_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram().share(m_extraram[1]);
	map(0xe000, 0xefff).ram().w(FUNC(spbactnp_state::extraram_w)).share(m_extraram[0]);
	map(0xd000, 0xd1ff).ram().w(m_extrapalette, FUNC(palette_device::write8)).share("extrapalette");
	map(0xd200, 0xd200).ram();

	map(0xd800, 0xd801).r(FUNC(spbactnp_state::extra_latch_r));
}



static INPUT_PORTS_START( spbactn )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Left Flippers" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Launch Ball / Shake (Left Side)" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Launch Ball / Shake (Right Side)" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Right Flippers" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Start" )  // needed to avoid confusion with # of players. Press multiple times for multiple players
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, "2 Coins/1 Credit 3/2" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Credit 5/6" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, "2 Coins/1 Credit 3/2" )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x28, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Credit 5/6" )
	PORT_DIPNAME( 0xc0, 0xc0, "Balls" )         PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x40, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Extra Ball" )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, "100k and 500k" )
	PORT_DIPSETTING(    0x0c, "200k and 800k" )
	PORT_DIPSETTING(    0x08, "200k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x10, 0x10, "Hit Difficulty" )        PORT_DIPLOCATION("SW2:5")   // From .xls file - WHAT does that mean ?
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x20, 0x20, "Display Instructions" )  PORT_DIPLOCATION("SW2:6") // Listed in manual as "Change Software", but seems to have no effect?
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7") // As listed in manual, but seems to have no effect? Works on the prototype, though
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Match" )         PORT_DIPLOCATION("SW2:8")   // Check code at 0x00bf8c
	PORT_DIPSETTING(    0x80, "1/20" )
	PORT_DIPSETTING(    0x00, "1/40" )
INPUT_PORTS_END

static INPUT_PORTS_START( spbactnp )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Right Flippers" )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Left Flippers" )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )  PORT_NAME( "Start" )  // needed to avoid confusion with # of players. Press multiple times for multiple players
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Launch Ball / Shake" )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") // TODO: double check 0x0100, 0x0800, 0xc000
	PORT_DIPNAME( 0x0003, 0x0003, "Balls" )         PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0002, "5" )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0010, "2 Coins/1 Credit 3/2" )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0014, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin/1 Credit 5/6" )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(      0x0040, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0080, "2 Coins/1 Credit 3/2" )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00a0, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin/1 Credit 5/6" )
	PORT_DIPNAME( 0x0100, 0x0100, "Match" )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0100, "1/20" )
	PORT_DIPSETTING(      0x0000, "1/40" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Display Instructions" )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Hit Difficulty" )        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x3000, 0x3000, "Extra Ball" )        PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(      0x2000, "100k and 500k" )
	PORT_DIPSETTING(      0x3000, "200k and 800k" )
	PORT_DIPSETTING(      0x1000, "200k" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
INPUT_PORTS_END


static const gfx_layout fgtilelayout =
{
	16,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0, 1) },
	{ STEP8(0, 4), STEP8(4*8*8, 4) },
	{ STEP8(0, 4*8) },
	16*8*4
};

static const gfx_layout bgtilelayout =
{
	16,8,
	RGN_FRAC(1,2),
	4,
	{ 3, 2, 1, 0 },

	{ RGN_FRAC(1,2)+1*4, RGN_FRAC(1,2)+0*4, 1*4, 0*4,
	RGN_FRAC(1,2)+3*4, RGN_FRAC(1,2)+2*4, 3*4, 2*4,
	16*8+RGN_FRAC(1,2)+1*4,16*8+RGN_FRAC(1,2)+0*4, 16*8+1*4,16*8+0*4,
	16*8+RGN_FRAC(1,2)+3*4, 16*8+RGN_FRAC(1,2)+2*4, 16*8+3*4,16*8+2*4 },

	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	32*8
};

static GFXDECODE_START( gfx_spbactn )
	GFXDECODE_ENTRY( "fgtiles", 0, fgtilelayout, 0x0200, 16 + 240 )
	GFXDECODE_ENTRY( "bgtiles", 0, bgtilelayout, 0x0300, 16 + 128 )
GFXDECODE_END

static GFXDECODE_START( gfx_spbactn_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_packed_msb, 0x0000, 0x100 )
GFXDECODE_END


static GFXDECODE_START( gfx_spbactnp )
	GFXDECODE_ENTRY( "fgtiles", 0, fgtilelayout,   0x0200, 16 + 240 )
	GFXDECODE_ENTRY( "bgtiles", 0, fgtilelayout,   0x0300, 16 + 128 ) // wrong
GFXDECODE_END

static GFXDECODE_START( gfx_spbactnp_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_packed_msb, 0x0000, 16 + 384 )
GFXDECODE_END

static GFXDECODE_START( gfx_extraspbactnp )
	GFXDECODE_ENTRY( "extragfx",  0, gfx_8x8x4_packed_msb, 0x0000, 16 )
GFXDECODE_END


void spbactn_state::spbactn(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &spbactn_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(spbactn_state::irq3_line_assert));

	Z80(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &spbactn_state::sound_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: verify actual blanking frequencies (should be close to NTSC)
	m_screen->set_raw(XTAL(22'656'000) / 2, 720, 0, 512, 262, 16, 240);
	m_screen->set_screen_update(FUNC(spbactn_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_spbactn);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 0x2800 / 2);

	TECMO_SPRITE(config, m_sprgen, 0, m_palette, gfx_spbactn_spr);

	TECMO_MIXER(config, m_mixer, 0);
	m_mixer->set_mixer_shifts(8,10,4);
	m_mixer->set_blendcols(   0x0000 + 0x300, 0x0000 + 0x200, 0x0000 + 0x100, 0x0000 + 0x000 );
	m_mixer->set_regularcols( 0x0800 + 0x300, 0x0800 + 0x200, 0x0800 + 0x100, 0x0800 + 0x000 );
	m_mixer->set_blendsource( 0x1000 + 0x000, 0x1000 + 0x100);
	m_mixer->set_bgpen(0x800 + 0x300, 0x000 + 0x300);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(4'000'000))); // Was 3.579545MHz, a common clock, but no way to generate via on PCB OSCs
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	// Was 1.056MHz, a common clock, but no way to generate via on PCB OSCs. Clock frequency & pin 7 not verified
	okim6295_device &oki(OKIM6295(config, "oki", XTAL(4'000'000) / 4, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void spbactnp_state::spbactnp(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &spbactnp_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(spbactnp_state::irq3_line_assert));

	Z80(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &spbactnp_state::sound_map);

	Z80(config, m_extracpu, XTAL(4'000'000));
	m_extracpu->set_addrmap(AS_PROGRAM, &spbactnp_state::extra_map);
	m_extracpu->set_vblank_int("extrascreen", FUNC(spbactnp_state::irq0_line_hold));

	GENERIC_LATCH_16(config, m_extralatch);
	m_extralatch->data_pending_callback().set_inputline(m_extracpu, INPUT_LINE_NMI);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(spbactnp_state::screen_update));
	m_screen->set_orientation(ROT90);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_spbactnp);
	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 0x2800 / 2);

	SCREEN(config, m_extrascreen, SCREEN_TYPE_RASTER);
	m_extrascreen->set_refresh_hz(60);
	m_extrascreen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_extrascreen->set_size(32*8, 32*8);
	m_extrascreen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_extrascreen->set_screen_update(FUNC(spbactnp_state::extrascreen_update));

	GFXDECODE(config, m_extragfxdecode, m_extrapalette, gfx_extraspbactnp);
	PALETTE(config, m_extrapalette).set_format(palette_device::xBRG_444, 0x1000 / 2);
	m_extrapalette->set_endianness(ENDIANNESS_BIG);

	config.set_default_layout(layout_spbactnp);

	TECMO_SPRITE(config, m_sprgen, 0, m_palette, gfx_spbactnp_spr);

	TECMO_MIXER(config, m_mixer, 0);
	m_mixer->set_mixer_shifts(12,14,8);
	m_mixer->set_blendcols(   0x0000 + 0x300, 0x0000 + 0x200, 0x0000 + 0x100, 0x0000 + 0x000 );
	m_mixer->set_regularcols( 0x0800 + 0x300, 0x0800 + 0x200, 0x0800 + 0x100, 0x0800 + 0x000 );
	m_mixer->set_blendsource( 0x1000 + 0x000, 0x1000 + 0x100);
	m_mixer->set_bgpen(0x800 + 0x300, 0x000 + 0x300);
	m_mixer->set_screen(m_screen);

	// sound hardware - different?
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(4'000'000)));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(4'000'000) / 4, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( spbactn )
	// Board 9002-A (CPU Board)
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1.bin", 0x00000, 0x20000, CRC(6741bd3f) SHA1(844eb6465a15d339043fd6d2b6ba20ba216de493) )
	ROM_LOAD16_BYTE( "rom2.bin", 0x00001, 0x20000, CRC(488cc511) SHA1(41b4a01f35e0e93634b4843dbb894ab9840807bf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a-u14.3", 0x00000, 0x10000, CRC(57f4c503) SHA1(e5ddc63a43ba824bcaa4340eeba25a0d3f26cad9) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "a-u19",   0x00000, 0x20000,  CRC(87427d7d) SHA1(f76b0dc3f0d87deb0f0c81084aff9756b236e867) )

	// Board 9002-B (GFX Board)
	ROM_REGION( 0x080000, "fgtiles", 0 ) // 16x8
	ROM_LOAD16_BYTE( "b-u98",   0x00000, 0x40000, CRC(315eab4d) SHA1(6f812c85981dc649caca8b4635e3b8fd3a3c054d) )
	ROM_LOAD16_BYTE( "b-u99",   0x00001, 0x40000, CRC(7b76efd9) SHA1(9f23460aebe12cb5c4193776bf876d6044892979) )

	ROM_REGION( 0x080000, "bgtiles", 0 ) // 16x8
	ROM_LOAD( "b-u104",  0x00000, 0x40000, CRC(b648a40a) SHA1(1fb756dcd027a5702596e33bbe8a0beeb3ceb22b) )
	ROM_LOAD( "b-u105",  0x40000, 0x40000, CRC(0172d79a) SHA1(7ee1faa65c85860bd81988329df516bc34940ef5) )

	ROM_REGION( 0x080000, "sprites", 0 ) // 8x8
	ROM_LOAD16_BYTE( "b-u110",  0x00000, 0x40000, CRC(862ebacd) SHA1(05732e8524c50256c1db29317625d0edc19b87d2) )
	ROM_LOAD16_BYTE( "b-u111",  0x00001, 0x40000, CRC(1cc1379a) SHA1(44fdab8cb5ab1488688f1ac52f005454e835efee) )
ROM_END

ROM_START( spbactnj )
	// Board 9002-A (CPU Board)
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a-u68.1", 0x00000, 0x20000, CRC(b5b2d824) SHA1(be04ca370a381d7396f39e31fb2680973193daee) )
	ROM_LOAD16_BYTE( "a-u67.2", 0x00001, 0x20000, CRC(9577b48b) SHA1(291d890a9d0e434455f183eb12ae6edf3156688d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a-u14.3", 0x00000, 0x10000, CRC(57f4c503) SHA1(e5ddc63a43ba824bcaa4340eeba25a0d3f26cad9) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "a-u19",   0x00000, 0x20000,  CRC(87427d7d) SHA1(f76b0dc3f0d87deb0f0c81084aff9756b236e867) )

	// Board 9002-B (GFX Board)
	ROM_REGION( 0x080000, "fgtiles", 0 ) // 16x8
	ROM_LOAD16_BYTE( "b-u98",   0x00000, 0x40000, CRC(315eab4d) SHA1(6f812c85981dc649caca8b4635e3b8fd3a3c054d) )
	ROM_LOAD16_BYTE( "b-u99",   0x00001, 0x40000, CRC(7b76efd9) SHA1(9f23460aebe12cb5c4193776bf876d6044892979) )

	ROM_REGION( 0x080000, "bgtiles", 0 ) // 16x8
	ROM_LOAD( "b-u104",  0x00000, 0x40000, CRC(b648a40a) SHA1(1fb756dcd027a5702596e33bbe8a0beeb3ceb22b) )
	ROM_LOAD( "b-u105",  0x40000, 0x40000, CRC(0172d79a) SHA1(7ee1faa65c85860bd81988329df516bc34940ef5) )

	ROM_REGION( 0x080000, "sprites", 0 ) // 8x8
	ROM_LOAD16_BYTE( "b-u110",  0x00000, 0x40000, CRC(862ebacd) SHA1(05732e8524c50256c1db29317625d0edc19b87d2) )
	ROM_LOAD16_BYTE( "b-u111",  0x00001, 0x40000, CRC(1cc1379a) SHA1(44fdab8cb5ab1488688f1ac52f005454e835efee) )
ROM_END


ROM_START( spbactnp )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "spa.18k", 0x00000, 0x10000, CRC(40f6a1e6) SHA1(533e1eb96f54b976f50d5b8927160b46b2740c83) )
	ROM_LOAD16_BYTE( "spa.22k", 0x00001, 0x10000, CRC(ce31871e) SHA1(8670c051d775fee6dcd2aa82cdb6f3fcc4338bd5) )
	ROM_LOAD16_BYTE( "spa.17k", 0x20000, 0x10000, CRC(c9860ae9) SHA1(3c2479be75ee84165470e9ca0a9d3b2ce679703d) )
	ROM_LOAD16_BYTE( "spa.21k", 0x20001, 0x10000, CRC(8226f644) SHA1(2d3e32368fbfec7437bd972096fd92972f52f6b0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "spa.17g", 0x00000, 0x10000, CRC(445fc2c5) SHA1(c0e40496cfcaa0a8c90fb05111fadee74582f91a) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "spa_data_2-21-a10.8e",   0x00000, 0x20000,  CRC(87427d7d) SHA1(f76b0dc3f0d87deb0f0c81084aff9756b236e867) ) // same as regular

	ROM_REGION( 0x080000, "fgtiles", 0 ) // 16x8
	ROM_LOAD16_BYTE( "spa_back0_split0_5-17-p-1.27b",  0x00000, 0x20000, CRC(37922110) SHA1(8edb6745ab6b6937f1365d35bfcdbe86198de668) )
	ROM_LOAD16_BYTE( "spa_back0_split1_5-17-p-1.27c",  0x00001, 0x20000, CRC(9d6ef9ab) SHA1(338ff1bd9d30a61d782616cccb4108daac6a8612) )

	ROM_REGION( 0x080000, "bgtiles", 0 ) // 16x8. It only ever draws the background from the rocket level, for all levels??
	ROM_LOAD16_BYTE( "spa_back1_split0_3-14-a-11.26b",  0x00000, 0x20000, CRC(6953fd62) SHA1(fb6061f5ad48e0d91d3dad96afbac2d64908f0a7) )
	ROM_LOAD16_BYTE( "spa_back1_split1_3-14-a-11.26c",  0x00001, 0x20000, CRC(b4123511) SHA1(c65b912238bab74bf46b5d5486c1d998813ef511) )

	ROM_REGION( 0x040000, "sprites", 0 ) // 8x8
	ROM_LOAD( "spa_sp0_4-18-p-8.5m",  0x00000, 0x20000, CRC(cd6ba360) SHA1(a01f65a678b6987ae877c381f74515efee4b492e) )
	ROM_LOAD( "spa_sp1_3-14-a-10.4m", 0x20000, 0x20000, CRC(86406336) SHA1(bf091dc13404535e6baee990f5e957d3538841ac) )

	ROM_REGION( 0x10000, "extracpu", 0 )
	ROM_LOAD( "6204_6-6.29c",   0x00000, 0x10000, CRC(e8250c26) SHA1(9b669878790c8e3c5d80f165b5ffa1d6830f4696) )

	ROM_REGION( 0x080000, "extragfx", 0 )
	ROM_LOAD( "spa.25c", 0x00000, 0x20000, CRC(02b69ab9) SHA1(368e774693a6fab756faaeec4ffd42406816e6e2) )

	ROM_REGION( 0x253, "misc", 0 ) //misc
	ROM_LOAD( "p109.18d",      0x000, 0x100, CRC(2297a725) SHA1(211ebae11ca55cc67df29291c3e0916836550bfb) ) // mostly empty.. is this correct?
	ROM_LOAD( "pin.b.sub.23g", 0x000, 0x100, CRC(3a0c70ed) SHA1(9be38c421e9a14f6811752a4464dd5dbf037e385) ) // mostly empty.. is this correct?
	ROM_LOAD( "tcm1.19g.bin",  0x000, 0x053, CRC(2c54354a) SHA1(11d8b6cdaf052b5a9fbcf6b6fbf99c5f89575cfa) )
ROM_END

} // anonymous namespace


GAME( 1991, spbactn,  0,       spbactn,  spbactn,  spbactn_state,  empty_init, ROT90, "Tecmo", "Super Pinball Action (US)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, spbactnj, spbactn, spbactn,  spbactn,  spbactn_state,  empty_init, ROT90, "Tecmo", "Super Pinball Action (Japan)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
 // early proto, (c) date is 2 years earlier and seems to have been designed for a 'pinball' style cabinet with 2nd display?
GAME( 1989, spbactnp, spbactn, spbactnp, spbactnp, spbactnp_state, empty_init, ROT0, "Tecmo", "Super Pinball Action (US, prototype, dual screen)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
